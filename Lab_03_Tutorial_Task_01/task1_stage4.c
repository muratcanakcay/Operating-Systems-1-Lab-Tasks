#include <math.h>
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
typedef struct timespec timespec_t;

typedef struct argsWork 
{
    pthread_t tid;
    int arraySize;
    UINT seed;
    int* arrayCount;
    double* array;
    double* resultarray;
    pthread_mutex_t* mxCells;	    
} argsWork_t;

void ReadArguments(int argc, char** argv, int *arraySize, int* threadCount);
void printArray(double *array, int arraySize);
void msleep(UINT milisec);
void* work(void*);

int main(int argc, char** argv) 
{
    srand(time(NULL));

    int arraySize, threadCount;
    double *array, *resultarray;

    ReadArguments(argc, argv, &arraySize, &threadCount);

    pthread_mutex_t mxCells[arraySize];
    for (int i =0; i < arraySize; i++) // set bins to zero and initialize mutexes for bins
    {
        if(pthread_mutex_init(&mxCells[i], NULL)) ERR("Couldn't initialize mutex!");
    }

    int arrayCount = arraySize;

    if(NULL==(array = (double*) malloc(sizeof(double) * arraySize))) ERR("Malloc error for array!");
    if(NULL==(resultarray = (double*) malloc(sizeof(double) * arraySize))) ERR("Malloc error for array!");
    argsWork_t* args = (argsWork_t*) malloc(sizeof(argsWork_t) * threadCount);

    for (int i =0; i < arraySize; i++)
    {
        UINT r = (UINT)rand();
        array[i] = NEXT_DOUBLE(&r);
        resultarray[i] = 0.0;
    }

	for (int i =0; i < threadCount; i++)
    {
        args[i].seed = rand();
        args[i].arraySize = arraySize;
        args[i].array = array;
        args[i].arrayCount = &arrayCount;
        args[i].resultarray = resultarray;
        args[i].mxCells = mxCells;
    }
	
	for(int i = 0; i < threadCount; i++)  // create threads
        if(pthread_create(&args[i].tid, NULL, work, &args[i])) ERR("Couldn't create signal handling thread!");
    
    for(int i = 0; i < threadCount; i++)  // wait for threads
    {
        if(pthread_join(args[i].tid, NULL)) ERR("Can't join with 'signal handling' thread");
		//printf("Thread %d joined\n", i);
    }

    printArray(array, arraySize);
    printArray(resultarray, arraySize);
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

void msleep(UINT milisec) 
{
    time_t sec= (int)(milisec/1000);
    milisec = milisec - (sec*1000);
    timespec_t req= {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    
    if(nanosleep(&req, &req)) ERR("nanosleep");
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
    argsWork_t* args = voidArgs;
    bool repeat = true;
    int idx;    
    
    while (*(args->arrayCount) != 0)
    {
        idx = rand_r(&args->seed) % (args->arraySize);
        pthread_mutex_lock(&args->mxCells[idx]);
        
        if(args->resultarray[idx] != 0.0)
        {
            //printf("Chosen random idx: %d but idx is already calculated: %f\n", args->arraySize, idx, args->resultarray[idx]);            
            pthread_mutex_unlock(&args->mxCells[idx]);
            continue;
        }
        
        //printf("Chosen random idx: %d\n", args->arraySize, idx);
        double result = sqrt(args->array[idx]);    
        //printf("Sqrt of %f, at chosen  idx: %d is %f\n", args->array[idx], idx, result);
        args->resultarray[idx] = result;
        (*args->arrayCount)--;
        //printf("Remaining: %d\n", *args->arrayCount);
        pthread_mutex_unlock(&args->mxCells[idx]);

        msleep(100);
    }

    msleep(100);

    return NULL;
}
