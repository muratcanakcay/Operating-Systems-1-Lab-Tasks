#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>


#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

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
    last_signal = sig;    
}

void sigchld_handler(int sig) 
{
	pid_t pid;
    
    for(;;)
    {
		pid=waitpid(0, NULL, WNOHANG);
		
        if(pid == 0) return;
		if(pid <= 0) 
        {
			if(errno==ECHILD) return;
			ERR("waitpid");
		}
	}
}

void child_work(sigset_t* oldmask, int n) 
{    
    int c = 0;
    int counter = 0;
    int fd, fsize;    
    char fname[255];
    
    snprintf(fname, 255, "state.%248d", n);   // create name string: "state.XX"  

    if((fd=open(fname,O_RDONLY|O_CREAT, 0777)) < 0) ERR("open");       
    if ((fsize = read(fd, &counter, sizeof(int))) < 0) ERR("read");
    if (fsize < sizeof(int)) counter = 0;            
       
    close(fd);   
    
    srand(time(NULL)*getpid());
    int alrm = 2 + rand() % 9;

    fprintf(stderr, "Child %d with PID: [%d] Starting counter: %d\n", n, getpid(), counter);   

    while(1)
    { 
        alarm(alrm);        
        sigsuspend(oldmask);

        if (last_signal == SIGALRM)
            break;        
        else if (last_signal == SIGUSR1) 
            c++; 
        else if (last_signal == SIGUSR2)
        {            
            fprintf(stderr, "Child %d with PID: [%d] Signals received: %d. Old Counter: %d New Counter: ", n, getpid(), c, counter);
            counter += (c * (-2 + rand()%(5)));
            fprintf(stderr, "%d\n", counter);
            c = 0;
        }
        else if (last_signal == SIGTERM) 
            break; 
    }
      
    if((fd=open(fname, O_WRONLY|O_TRUNC, 0777)) < 0) ERR("open");
    if(write(fd, &counter, sizeof(int)) < 0) ERR("write");

    close(fd);    
    
    // nanosleep(&t,NULL); 
    // printf("Child PID: [%d] I was sleeping for %d ms.\n", getpid(), s);   
}

void parent_work(int m) 
{
	
    int num, fs = 1;    
    struct timespec t = {0, m*1000000};    
    char buf[255];
	
    while(1)
    {        
        if (fs > 0) printf("Enter number of signals to send (exit to exit):\n"); 
        
        if ((fs = fscanf(stdin, "%255s", buf)) < 0) ERR("fscanf");
        else if (fs == 0) continue;
        
        if (0 == strcmp(buf, "exit")) break;
        num = atoi(buf);        
       
        if (num < 2 || num > 20) 
        { 
            printf("Number should be between 2-20\n"); 
            continue;
        }       
        
        for (int i=0; i < num; i++)
        {
            nanosleep(&t,NULL); 
            kill(0, SIGUSR1);            
        }
        
        nanosleep(&t,NULL); 
        kill(0, SIGUSR2);               
 	}
    
    kill(0, SIGTERM);    
}

void create_children(sigset_t* oldmask, int n)
{
    pid_t s;
    
    for(int i = 1; i <= n; i++)
    {
        if((s=fork()) < 0 ) ERR("Fork:");
        if(s == 0) 
        {            
            child_work(oldmask, i);         
            exit(EXIT_SUCCESS);
        }  
    }
}

void usage(char *name)
{
	fprintf(stderr,"USAGE: %s n p\n",name);
	fprintf(stderr,"n - number of child processes (3-10)\n");
    fprintf(stderr,"m - length of pause (1-500 ms)\n");		
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int n, m;
	if(argc!=3) usage(argv[0]);
	n = atoi(argv[1]);	 m = atoi(argv[2]);
	if (n < 3 || n > 10 || m < 1 || m > 500)  usage(argv[0]);
    
    sethandler(sigchld_handler, SIGCHLD);
	sethandler(sig_handler, SIGUSR1);
    sethandler(sig_handler, SIGUSR2);
    sethandler(sig_handler, SIGTERM);
    sethandler(sig_handler, SIGALRM);
    	
	sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
    
    create_children(&oldmask, n);
	parent_work(m);   
	
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    
    while(wait(NULL)>0);
	    
    return EXIT_SUCCESS;
}