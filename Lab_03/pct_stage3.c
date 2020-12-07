// By decree 27/2020 of University Rector you must add the following statement to the
// uploads:
// ------------------------------------------------------------------------
// I declare that this piece of work which is the basis for recognition of
// achieving learning outcomes in the OPS1 course was completed on my own.
// [First and last name] [Student record book number (Student ID number)]
// ------------------------------------------------------------------------

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define MAXLINE 4096
#define DEFAULT_N 10
#define DEFAULT_M 10
#define DEFAULT_K 10
#define ELAPSED(start,end) ((end).tv_sec-(start).tv_sec)+(((end).tv_nsec - (start).tv_nsec) * 1.0e-9)
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0; 

typedef unsigned int UINT;
typedef struct threadArgs 
 {
    pthread_t tid;
    int n;
    int m;
    UINT seed;
    char** matrix;
    sigset_t* mask;
    pthread_mutex_t* mxRows;
 } threadArgs_t;


void readArguments(int argc, char** argv, int *n, int* m, int* k);
void* work(void *voidArgs);

void sethandler( void (*f)(int), int sigNo) 
{
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig) 
{

    printf("[%d] received signal %d\n", getpid(), sig);
    last_signal = sig;
}

int main(int argc, char** argv) 
{
	srand(time(NULL));
    int n, m, k;
    
	readArguments(argc, argv, &n, &m, &k);
	
	char** matrix = (char**)malloc(sizeof(char*) * n);
    if (matrix == NULL) ERR("Malloc error for matrix!");
    for(int r = 0; r < n; r++)
        matrix[r] = (char*)malloc(sizeof(char) * m);

    for (int r = 0; r < n; r++) 
    {
        for (int c = 0; c < m; c++) 
        {
            int i =  rand();
            matrix[r][c] = 'a'+(i%('z'-'a'+1));
        }
    }

	sethandler(sig_handler, SIGUSR1);
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, &oldmask);
    
    threadArgs_t* threads = (threadArgs_t*) malloc(sizeof(threadArgs_t) * k);
    if (threads == NULL) ERR("Malloc error for threads!");

    pthread_mutex_t mxRows[n];
    for (int i =0; i < n; i++) 
        {            
            if(pthread_mutex_init(&mxRows[i], NULL)) ERR("Couldn't initialize mutex!");		
        }
    
    for (int i=0;i<k;i++) 
    {
		threads[i].seed = rand();
        threads[i].n = n;
        threads[i].m = m;
        threads[i].matrix = matrix;
        threads[i].mask = &mask;
        threads[i].mxRows = mxRows;
    }
    
	for (int i = 0; i < k; i++)
	 	if(pthread_create(&threads[i].tid, NULL, work, &threads[i])) ERR("Failed to create student thread!");
	
	char buf[12];
    int num = -1;
    while(true)
    {
        scanf("%8s %d",buf, &num);
        if(strlen(buf)>10) ERR("Input  too long");
        if(strcmp(buf, "print") == 0)
        {
            for (int r = 0; r < n; r++) 
            {
                {
                    for (int c = 0; c < m; c++) 
                    printf(" %c", matrix[r][c]);            
                }
                printf("\n");
            }
        }
        if(strcmp(buf, "replace") == 0)
        {
            for (int i = 0; i < num; i++) 
            {
                printf("Sending USR1\n");
                kill(0, SIGUSR1);
            }
        }

    }	
	
	for (int i = 0; i < k; i++) 
	 	if(pthread_join(threads[i].tid, NULL)) ERR("Failed to join with a student thread!");
	
	exit(EXIT_SUCCESS);
}

void readArguments(int argc, char** argv, int* n, int* m, int* k)
{
	if (argc != 4) 
    {
			printf("Invalid value for 'n, m or k'");
			exit(EXIT_FAILURE);
	}

	if (argc == 4) 
	{
		*n = atoi(argv[1]);
        *m = atoi(argv[2]);
        *k = atoi(argv[3]);
         
		if (*n < 1 || *n > 100 || *m < 1 || *m > 100 || *k < 3 || *k > 100)
		{
			printf("Invalid value for 'n, m or k'");
			exit(EXIT_FAILURE);
		}
	}
}

void* work(void *voidArgs)
{
    threadArgs_t* args= voidArgs;
    int sigNo;
    int randomrow, randomcol;

    printf("My TID : [%d]\n", (int)args->tid);
    sleep(1);
    printf("My TID after 1 sec: [%d]\n", (int)args->tid);   

    while(true)
    {
        sigwait(args->mask, &sigNo);
        
        switch (sigNo) 
		{
			case SIGUSR1:
                randomrow = rand_r(&args->seed) % args->n;
                randomcol = rand_r(&args->seed) % args->m;
				
                pthread_mutex_lock(&args->mxRows[randomrow]);
                int i =  rand_r(&args->seed);
                char newChar = 'a'+(i%('z'-'a'+1)); 
                printf("[%d] received SIGUSR1: Replacing '%c' to '%c' at index [%d, %d]\n", (int)args->tid, (args->matrix[randomrow][randomcol]), newChar, randomrow, randomcol);
                args->matrix[randomrow][randomcol] = newChar; 
				pthread_mutex_unlock(&args->mxRows[randomrow]);
                break;
			case SIGINT:				
				return NULL;
			default:
				printf("unexpected signal %d\n", sigNo);
				exit(1);
		}
    }
    
    return NULL;
}

