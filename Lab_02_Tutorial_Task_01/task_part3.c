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

void sethandler(void (*f)(int), int sigNo) 
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig) 
{
	sig_count++;    
}

void child_work() 
{    
    srand(time(NULL)*getpid());
    int s = 100 + rand()%(101);
    printf("Child [%d] selected s= %d\n", getpid(), s);
    struct timespec t = {0, s*10000};    

    while(1)
    {
		nanosleep(&t,NULL); 
    	if(kill(getppid(), SIGUSR1)) ERR("kill");						
	}

    printf("Child [%d] TERMINATING!\n", getpid());
}


void parent_work(sigset_t oldmask) 
{
	while(1)
    {		
		sigsuspend(&oldmask);		
		printf("[PARENT] received %d SIGUSR1\n", sig_count);		
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
            exit(EXIT_SUCCESS);
        }  
    }
}

void usage(char *name){
	fprintf(stderr,"USAGE: %s n  p\n",name);
	fprintf(stderr,"n - number of child processes\n");	
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int n;
	if(argc!=2) usage(argv[0]);
	n = atoi(argv[1]);	
	
	sethandler(sig_handler, SIGUSR1);
	   
    create_children(n);
	    
    while(n>0)
    {
        sleep(3);
        pid_t pid;
        for(;;)
        {
            pid=waitpid(0, NULL, WNOHANG);
            if(pid>0) n--;
            if(0==pid) break;
            if(0>=pid) {
                if(ECHILD==errno) break;
                ERR("waitpid:");
            }
        }
        printf("PARENT: %d processes remain. SIGUSR1 count: %d \n", n, sig_count);
    }
	    
	printf("Parent TERMINATING!\n");
    return EXIT_SUCCESS;
}