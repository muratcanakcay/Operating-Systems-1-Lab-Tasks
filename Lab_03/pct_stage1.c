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

#define DEFAULT_N 10
#define DEFAULT_M 10
#define DEFAULT_K 10
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

typedef struct threadArgs 
 {
	pthread_t tid;
} threadArgs_t;

void ReadArguments(int argc, char** argv, int *n, int* m, int* k);
void* work(void *voidArgs);

int main(int argc, char** argv) 
{
	int n, m, k;
    
	ReadArguments(argc, argv, &n, &m, &k);
	
	threadArgs_t* threads = (threadArgs_t*) malloc(sizeof(threadArgs_t) * k);
    if (threads == NULL) ERR("Malloc error for threads!");
    
	for (int i = 0; i < k; i++)
	 	if(pthread_create(&threads[i].tid, NULL, work, &threads[i])) ERR("Failed to create student thread!");
	
	for (int i = 0; i < k; i++) 
	 	if(pthread_join(threads[i].tid, NULL)) ERR("Failed to join with a student thread!");
	
	exit(EXIT_SUCCESS);
}

void ReadArguments(int argc, char** argv, int* n, int* m, int* k)
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
    
    printf("My TID : [%d]\n", (int)args->tid);
    sleep(1);
    printf("My TID after 1 sec: [%d]\n", (int)args->tid);
    return NULL;
}