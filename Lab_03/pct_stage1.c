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

#define MAXLINE 4096
#define DEFAULT_N 10
#define DEFAULT_M 10
#define DEFAULT_K 10
#define ELAPSED(start,end) ((end).tv_sec-(start).tv_sec)+(((end).tv_nsec - (start).tv_nsec) * 1.0e-9)
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

// typedef unsigned int UINT;
// typedef struct timespec timespec_t;

 typedef struct threadArgs 
 {
// 	bool* removed;
    pthread_t tid;
// 	int count;
// 	int present;
 } threadArgs_t;

// typedef struct yearCounters 
// {
// 	int values[4];
// 	pthread_mutex_t mxCounters[4];
// } yearCounters_t;

// typedef struct argsModify 
// {
// 	yearCounters_t *pYearCounters;
// 	int year;
// } argsModify_t;

void ReadArguments(int argc, char** argv, int *n, int* m, int* k);
void* work(void *voidArgs);
// void* student_life(void*);
// void increment_counter(argsModify_t *args);
// void decrement_counter(argsModify_t *args);
// void msleep(UINT milisec);
// void kick_student(studentsList_t *studentsList);

int main(int argc, char** argv) 
{
	int n, m, k;
    
    // int studentsCount;
	
	ReadArguments(argc, argv, &n, &m, &k);
	
	// yearCounters_t counters = 
	// {
	// 	.values = { 0, 0, 0, 0 },
	// 	.mxCounters = {
	// 			PTHREAD_MUTEX_INITIALIZER,
	// 			PTHREAD_MUTEX_INITIALIZER,
	// 			PTHREAD_MUTEX_INITIALIZER,
	// 			PTHREAD_MUTEX_INITIALIZER}
	// };
	
	threadArgs_t* threads = (threadArgs_t*) malloc(sizeof(threadArgs_t) * k);
    if (threads == NULL) ERR("Malloc error for threads!");
    
    // studentsList_t studentsList;
	// studentsList.count = studentsCount;
	// studentsList.present = studentsCount;
	// studentsList.thStudents = (pthread_t*) malloc(sizeof(pthread_t) * studentsCount);
	// studentsList.removed = (bool*) malloc(sizeof(bool) * studentsCount);
	
	// if (studentsList.thStudents == NULL || studentsList.removed == NULL) 
	// 	ERR("Failed to allocate memory for 'students list'!");
	
	// for (int i = 0; i < studentsCount; i++) 
	// 	studentsList.removed[i] = false;
	
	for (int i = 0; i < k; i++)
	 	if(pthread_create(&threads[i].tid, NULL, work, &threads[i])) ERR("Failed to create student thread!");
	
	// srand(time(NULL));
	// timespec_t start, current;
	
	// if (clock_gettime(CLOCK_REALTIME, &start)) ERR("Failed to retrieve time!");
	
	// do 
	// {
	// 	msleep(rand() % 201 + 100);
	// 	if (clock_gettime(CLOCK_REALTIME, &current)) ERR("Failed to retrieve time!");
	// 	kick_student(&studentsList);
	// } while (ELAPSED(start, current) < 4.0);
	
	for (int i = 0; i < k; i++) 
	 	if(pthread_join(threads[i].tid, NULL)) ERR("Failed to join with a student thread!");
	
	// printf(" First year: %d\n", counters.values[0]);
	// printf("Second year: %d\n", counters.values[1]);
	// printf(" Third year: %d\n", counters.values[2]);
	// printf("  Engineers: %d\n", counters.values[3]);
	
	// free(studentsList.removed);
	// free(studentsList.thStudents);
	
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
// void* student_life(void *voidArgs) 
// {
// 	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
// 	argsModify_t args;
// 	args.pYearCounters = voidArgs;
	
// 	for(args.year = 0;args.year < 3;args.year++)
// 	{
// 		increment_counter(&args);
// 		pthread_cleanup_push(decrement_counter, &args);
// 		msleep(1000);
// 		pthread_cleanup_pop(1);
// 	}
	
// 	increment_counter(&args);
// 	return NULL;
// }

// void increment_counter(argsModify_t *args) 
// {
// 	pthread_mutex_lock(&(args->pYearCounters->mxCounters[args->year]));
// 	args->pYearCounters->values[args->year] += 1;
// 	pthread_mutex_unlock(&(args->pYearCounters->mxCounters[args->year]));
// }

// void decrement_counter(argsModify_t *args) 
// {
// 	pthread_mutex_lock(&(args->pYearCounters->mxCounters[args->year]));
// 	args->pYearCounters->values[args->year] -= 1;
// 	pthread_mutex_unlock(&(args->pYearCounters->mxCounters[args->year]));
// }

// void msleep(UINT milisec) 
// {
//     time_t sec= (int)(milisec/1000);
//     milisec = milisec - (sec*1000);
//     timespec_t req= {0};
//     req.tv_sec = sec;
//     req.tv_nsec = milisec * 1000000L;
    
// 	if(nanosleep(&req, &req)) ERR("nanosleep");
// }

// void kick_student(studentsList_t *studentsList) 
// {
// 	int idx;
// 	if(0==studentsList->present) return;
	
// 	do 
// 	{
// 		idx = rand() % studentsList->count;
// 	} while(studentsList->removed[idx] == true);
	
// 	pthread_cancel(studentsList->thStudents[idx]);
// 	studentsList->removed[idx] = true;
// 	studentsList->present--;
// }
