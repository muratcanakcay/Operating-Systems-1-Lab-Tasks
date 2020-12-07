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
volatile sig_atomic_t last_signal = 0;

void sethandler( void (*f)(int), int sigNo) 
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

void child_work(int Ni) 
{
	srand(time(NULL)*getpid());
    int c = 1 + rand()%(Ni);
    printf("Ni = %d, C = %d\n", Ni, c);
    
    struct timespec t = {0, c*100*10000};
	
	//sethandler(SIG_DFL,SIGUSR1);
	
	while(c--)
	{
		printf("Child %d sending signal...\n", getpid());
		if(kill(getppid(), SIGUSR1)) ERR("kill");
		nanosleep(&t,NULL);		
	}
}

ssize_t bulk_read(int fd, char *buf, size_t count){
	ssize_t c;
	ssize_t len=0;
	do{
		c=TEMP_FAILURE_RETRY(read(fd,buf,count));
		if(c<0) return c;
		if(c==0) return len; //EOF
		buf+=c;
		len+=c;
		count-=c;
	}while(count>0);
	return len ;
}

ssize_t bulk_write(int fd, char *buf, size_t count){
	ssize_t c;
	ssize_t len=0;
	do{
		c=TEMP_FAILURE_RETRY(write(fd,buf,count));
		if(c<0) return c;
		buf+=c;
		len+=c;
		count-=c;
	}while(count>0);
	return len ;
}

void parent_work(int n, char* name) 
{
	
    int fd;
    ssize_t rcount, wcount, totcount;
    char *chars=malloc(100);
    char *temp=malloc(100);
    if(!chars || !temp) ERR("malloc");
    if((fd=TEMP_FAILURE_RETRY(open(name, O_RDWR|O_CREAT, 0777)))<0) ERR("open");

    while(1)
    {			
	    last_signal=0;
		while(last_signal!=SIGUSR1 && ccount != n);		
		if (ccount == n) break; // returns to main if all children are terminated

        // init chars
        srand(time(NULL)*getpid());
        for(int k = 0; k < 100; k++)   
            chars[k] = 'a' + rand()%(('z' - 'a' + 1));
        //

        TEMP_FAILURE_RETRY(lseek(fd, 0, SEEK_SET)); 
        int k = 0;
        int check = bulk_read(fd, temp, 100);
        do        
        {            
            if(check != 0 || k != 0)
            {
                TEMP_FAILURE_RETRY(lseek(fd, k*100, SEEK_SET)); 
                if((rcount=bulk_read(fd, temp, 100)) < 0) ERR("read");
                TEMP_FAILURE_RETRY(lseek(fd, k*100, SEEK_SET));
            }
            
            if((wcount=bulk_write(fd, chars,100)) < 0) ERR("write");
            
            TEMP_FAILURE_RETRY(strcpy(chars, temp));
            //TEMP_FAILURE_RETRY(fprintf(stderr, "rcount = %d, k = %d\n", (int)rcount, k));
            k++;

        } while (rcount);

        totcount += wcount;
        if(TEMP_FAILURE_RETRY(fprintf(stderr,"Total %ld bytes written to file. \n", (long)totcount)) < 0) ERR("fprintf");   
        
 	}
     
    if(TEMP_FAILURE_RETRY(close(fd)))ERR("close");
    free(chars);
    free(temp);
    //if(kill(0,SIGUSR1))ERR("kill");      
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
		if(pid>0)
        {
            ccount++;
            //printf("[SIGCHLD] %d child(s) terminated\n", ccount);
        }
        if(pid == 0) return;
		if(pid <= 0) 
        {
			if(errno==ECHILD) return;
			ERR("waitpid");
		}
	}
}

void usage(char *name){
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
    sethandler(sig_handler, SIGUSR1);
	
	// sigset_t mask, oldmask;
	// sigemptyset(&mask);
	// sigaddset(&mask, SIGUSR1);
	// sigprocmask(SIG_BLOCK, &mask, &oldmask);

    create_children(argc, argv);
    parent_work(argc-2, name);
	
    while(wait(NULL)>0) printf("[WAIT] %d child(s) terminated.\n", ++ccount);
	printf("Parent exiting.\n");

    //sigprocmask(SIG_UNBLOCK, &mask, NULL);
    return EXIT_SUCCESS;
}