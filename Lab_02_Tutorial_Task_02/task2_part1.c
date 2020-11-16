#include <signal.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

volatile sig_atomic_t ccount = 0;

void sethandler( void (*f)(int), int sigNo) 
{
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void child_work(int Ni) 
{
	srand(time(NULL)*getpid());
    int c = 1 + rand()%(Ni);
    printf("Ni = %d, C = %d\n", Ni, c);
}

void parent_work(char* name)  //currently not used
{
	return;   
}

void create_children(int argc, char** argv) 
{
	int n = 0;
    while (++n < argc-1) 
    {
		switch (fork()) 
        {
			case 0: 
                child_work(atoi(argv[n]));
				exit(EXIT_SUCCESS);
			case -1:perror("Fork:");
				exit(EXIT_FAILURE);
		}
	}
}

void sigchld_handler(int sig) 
{
	pid_t pid;
    
	for(;;)
    {
		pid=waitpid(0, NULL, WNOHANG);
		if(pid>0) printf("[SIGCHILD] %d child(s) terminated\n", ++ccount);
        if(pid == 0) return;
		if(pid <= 0) 
        {
			if(errno==ECHILD) return;
			ERR("waitpid");
		}
	}
}

void usage(char *name)
{
	fprintf(stderr,"USAGE: %s Ni \n",name);
	fprintf(stderr,"Ni - 1-100 natural numbers\n");
	fprintf(stderr,"name of the output file\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) 
{
	char *name;
	if(argc<=2 || argc>101) usage(argv[0]);    
    name=argv[argc-1];
	
	sethandler(sigchld_handler, SIGCHLD);   
	
    create_children(argc, argv);
    parent_work(name);
	
    while(wait(NULL)>0) printf("[WAIT] %d child(s) terminated.\n", ++ccount);

    return EXIT_SUCCESS;
}