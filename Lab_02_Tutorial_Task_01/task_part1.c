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

void sethandler( void (*f)(int), int sigNo) 
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
    printf("Child %lu selected s= %d\n", (long)getpid(), s);    
}

void usage(char *name)
{
	fprintf(stderr,"USAGE: %s n  p\n",name);
	fprintf(stderr,"n - number of child processes\n");	
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) 
{
	int n;
	if(argc!=2) usage(argv[0]);
	n = atoi(argv[1]);	
	
	pid_t pid;
	while(n--) 
    {
        if((pid=fork())<0) ERR("fork");
        if(0==pid) 
        {
            child_work();
            exit(EXIT_SUCCESS);
        }
        else 
		{
        	while(wait(NULL)>0);
        }
    }
	    
	return EXIT_SUCCESS;
}