#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define DEFAULT_ARRAYSIZE 7
#define DEFAULT_THREADCOUNT 3

// NEXT_DOUBLE returns a double value between 1 and 60
#define NEXT_DOUBLE(seedptr) (((59.0 * (double) rand_r(seedptr) / (double) RAND_MAX)) + 1.0)

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

typedef unsigned int UINT;

typedef struct argsSignalHandler 
{
	pthread_t tid;
	int arraySize;
    UINT seed;
 	double *array;
} argsSignalHandler_t;

void ReadArguments(int argc, char** argv, int *arraySize, int* threadCount);
void printArray(double *array, int arraySize);
void* work(void*);

int main(int argc, char** argv) 
{
	srand(time(NULL));
    
    int arraySize, threadCount;
    double *array;

	pthread_mutex_t mxQuitFlag = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mxArray = PTHREAD_MUTEX_INITIALIZER;

	ReadArguments(argc, argv, &arraySize, &threadCount);

	if(NULL==(array = (double*) malloc(sizeof(double) * arraySize))) ERR("Malloc error for array!");
    argsSignalHandler_t* args = (argsSignalHandler_t*) malloc(sizeof(argsSignalHandler_t) * threadCount);

	for (int i =0; i < arraySize; i++)
    { 
		UINT r = (UINT)rand();
        array[i] = NEXT_DOUBLE(&r);
    }    
		
	for (int i =0; i < threadCount; i++) 
	{
        args[i].seed = rand();
        args[i].arraySize = arraySize;
        args[i].array = array;
	}
	
	for(int i = 0; i < threadCount; i++)  // create threads
        if(pthread_create(&args[i].tid, NULL, work, &args[i])) ERR("Couldn't create signal handling thread!");
    
	for(int i = 0; i < threadCount; i++)  // wait for threads
	{	
		if(pthread_join(args[i].tid, NULL)) ERR("Can't join with 'signal handling' thread");
		printf("Thread %d joined\n", i);
	}

    printArray(array, arraySize);
	free(array);
	
	exit(EXIT_SUCCESS);
}

void ReadArguments(int argc, char** argv, int *arraySize, int* threadCount)
{
	*arraySize = DEFAULT_ARRAYSIZE;
    *threadCount = DEFAULT_THREADCOUNT;

	if (argc >= 2) 
	{
		*arraySize = atoi(argv[1]);
		
		if (*arraySize <= 0) 
		{
			printf("Invalid value for 'array size'");
			exit(EXIT_FAILURE);
		}
	}
}

void printArray(double* array, int arraySize) 
{
	printf("[");

	for (int i =0; i < arraySize; i++)		
			printf(" %f", array[i]);

	printf(" ]\n");
}

void* work(void* voidArgs) 
{
	argsSignalHandler_t* args = voidArgs;
	
    int idx = rand_r(&args->seed) % (args->arraySize);

    printf("Array size: %d, Chosen random idx: %d\n", args->arraySize, idx);

	return NULL;
}
