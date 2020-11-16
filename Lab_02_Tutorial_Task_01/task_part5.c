#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

volatile sig_atomic_t sig_count = 0;
volatile sig_atomic_t last_signal = 0;

void sethandler(void (*f)(int), int sigNo) 
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig) 
{
	if (sig == SIGUSR1) sig_count++;
    last_signal = sig;    
}

void sigchld_handler(int sig) 
{
	pid_t pid;
    int ccount = 0;

	for(;;)
    {
		pid=waitpid(0, NULL, WNOHANG);
		if(pid>0) printf("%d child(s) terminated\n", ++ccount);
        if(pid == 0) return;
		if(pid <= 0) 
        {
			if(errno==ECHILD) return;
			ERR("waitpid");
		}
	}
}

void child_work() 
{    
    sethandler(SIG_DFL, SIGUSR2);
    srand(time(NULL)*getpid());
    int s = 100 + rand()%(101);
    printf("Child [%d] selected s= %d\n", getpid(), s);
    struct timespec t = {0, s*10000};    

    while(1)
    {
		nanosleep(&t,NULL); 
    	if(kill(getppid(), SIGUSR1)) ERR("kill");						
	}    
}

void parent_work(sigset_t oldmask) 
{
	int count=0;
	while(1)
    {		
		sigsuspend(&oldmask);
		count++;
		if (last_signal == SIGUSR1) printf("[PARENT] received %d SIGUSR1\n", sig_count);
        if (count == 100) kill(0, SIGUSR2);
        if (last_signal == SIGUSR2) return;                    
 	}    
}

void create_children(int n)
{
    pid_t s;
    //for(n--; n >= 0; n--)
    while(n--)
    {
        if((s=fork()) < 0 ) ERR("Fork:");
        if(!s) // i.e. if s == 0 !!!!
        {
            printf("CHILD PROCESS with PID = %lu working\n", (long)getpid());
            child_work(n);
            printf("CHILD PROCESS with PID = %lu TERMINATING\n", (long)getpid());
            exit(EXIT_SUCCESS);
        }  
    }
}

void usage(char *name)
{
	fprintf(stderr,"USAGE: %s n  p\n",name);
	fprintf(stderr,"n - number of child processes\n");	
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int n;
	if(argc!=2) usage(argv[0]);
	n = atoi(argv[1]);	
	
    sethandler(sigchld_handler, SIGCHLD);
	sethandler(sig_handler, SIGUSR1);
    sethandler(sig_handler, SIGUSR2);
    
	
	sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
    
    create_children(n);
	parent_work(oldmask);   
	
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    
    while(wait(NULL)>0);
	
    printf("Parent TERMINATING!\n");
    return EXIT_SUCCESS;
}