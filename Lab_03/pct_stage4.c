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

#define DEFAULT_N 10
#define DEFAULT_M 10
#define DEFAULT_K 10
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

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

int main(int argc, char** argv) 
{
    srand(time(NULL));
    int n, m, k;
    
    readArguments(argc, argv, &n, &m, &k);
		
    // allocate memory for matrix    
    char** matrix = (char**)malloc(sizeof(char*) * n);
    if (matrix == NULL) ERR("Malloc error for matrix!");
    for(int r = 0; r < n; r++)
    {
        matrix[r] = (char*)malloc(sizeof(char) * m);
        if (matrix[r] == NULL) ERR("Malloc error for matrix!");
    }

    // fill in matrix with random characters
    for (int r = 0; r < n; r++) 
    {
        for (int c = 0; c < m; c++) 
        {
            int i =  rand();
            matrix[r][c] = 'a' + (i % ('z'-'a'+1));
        }
    }
	
    // prepare and set signal mask
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGINT);
    pthread_sigmask(SIG_BLOCK, &mask, &oldmask);
    
    // allocate memory for threads array to hold k threads
    threadArgs_t* threads = (threadArgs_t*) malloc(sizeof(threadArgs_t) * k);
    if (threads == NULL) ERR("Malloc error for threads!");

    // initialize n mutexes - one for each row
    pthread_mutex_t mxRows[n];
    for (int i =0; i < n; i++) 
        if(pthread_mutex_init(&mxRows[i], NULL)) ERR("Couldn't initialize mutex!");		
    
    // prepare threads array
    for (int i=0; i < k; i++) 
    {
        threads[i].seed = rand();
        threads[i].n = n;
        threads[i].m = m;
        threads[i].matrix = matrix;
        threads[i].mask = &mask;
        threads[i].mxRows = mxRows;
    }
    
	// create threads
    for (int i = 0; i < k; i++)
	    if(pthread_create(&threads[i].tid, NULL, work, &threads[i])) ERR("Failed to create student thread!");
	
    // get input from stdin    
    while(true)
    {
        char buf[9];
        int num;

        scanf("%8s %d", buf, &num);  

        if(strlen(buf)>7) ERR("Input  too long");
        
        if(strcmp(buf, "print") == 0)
        {
            for (int r = 0; r < n; r++) 
            {
                for (int c = 0; c < m; c++) 
                    printf(" %c", matrix[r][c]);                            
                
                printf("\n");                
            }
            
            fflush(stdin);
        }

        if(strcmp(buf, "replace") == 0)
        {
            for (int i = 0; i < num; i++) 
            {
                printf("Sending USR1\n");
                kill(0, SIGUSR1);
            }

            fflush(stdin);
        }
        
        if(strcmp(buf, "exit") == 0)
        {
            for(int i=0; i < k; i++)
                pthread_kill(threads[i].tid, SIGINT);
            
            break;
        }
    }	
	
	// join with terminated threads
    for (int i = 0; i < k; i++) 
        if(pthread_join(threads[i].tid, NULL)) ERR("Failed to join with a thread!");
	
	exit(EXIT_SUCCESS);
}

void readArguments(int argc, char** argv, int* n, int* m, int* k)
{
    if (argc != 4)
    {
        *n = DEFAULT_N;
        *m = DEFAULT_M;
        *k = DEFAULT_K;            
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
    int rr, rc;

    printf("My TID : [%d]\n", (int)args->tid); 
    sleep(1);
    printf("My TID after 1 sec: [%d]\n", (int)args->tid);   

    while(true)
    {
        sigwait(args->mask, &sigNo);
        
        switch (sigNo) 
        {
            case SIGUSR1:
                rr = rand_r(&args->seed) % args->n;
                rc = rand_r(&args->seed) % args->m;

                pthread_mutex_lock(&args->mxRows[rr]);
                int i =  rand_r(&args->seed);
                char newChar = 'a'+(i%('z'-'a'+1)); 
                printf("[%d] received SIGUSR1: Replacing '%c' to '%c' at index [%d, %d]\n", (int)args->tid, (args->matrix[rr][rc]), newChar, rr, rc);
                args->matrix[rr][rc] = newChar; 
                pthread_mutex_unlock(&args->mxRows[rr]);
                
                break;
            case SIGINT:				
                printf("[%d] received SIGINT: Exiting\n", (int)args->tid);
                return NULL;
            default:
                printf("unexpected signal %d\n", sigNo);
                exit(1);
		}
    }
    
    return NULL;
}

