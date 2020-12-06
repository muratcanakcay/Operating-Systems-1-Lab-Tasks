#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

typedef unsigned int UINT;
typedef struct timespec timespec_t;
typedef struct argsSignalHandler 
{
	pthread_t tid;
    pid_t parentPid;
	UINT seed;
	int *vector;
    pthread_mutex_t* Guessed_Value_Mutex;
    pthread_mutex_t* Vector_Mutex;
} argsSignalHandler_t;


void* signal_handling(void* voidArgs);
int n;
int GUESSED_VALUE=0;
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
    printf("[%d] received signal %d\n", getpid(), sig);
    last_signal = sig;
}

int main(int argc, char** argv) 
{
    if(argc!=3) exit(EXIT_FAILURE);
        
    n=atoi(argv[1]);//Size of a vector
    int t=atoi(argv[2]);//Number of threads
    int vector[n];//Initialize vector
    
    pthread_mutex_t Guessed_Value_Mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t Vector_Mutex = PTHREAD_MUTEX_INITIALIZER;

    sethandler(sig_handler, SIGINT);
    sethandler(sig_handler, SIGALRM);
    sethandler(sig_handler, SIGQUIT);
    sethandler(sig_handler, SIGUSR1);

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGUSR1);

    sigprocmask(SIG_BLOCK, &mask, &oldmask);


    for(int i=0;i<n;i++)
    {
        vector[i]=0;//Assign 0 to each element
    }
    
    argsSignalHandler_t* threads = (argsSignalHandler_t*) malloc(sizeof(argsSignalHandler_t) * t);
    if (threads == NULL) ERR("Malloc error for threads!");
    
    srand(time(NULL));
    
    for (int i=0;i<t;i++) 
    {
		threads[i].seed = rand();
        threads[i].vector = vector;
        threads[i].Guessed_Value_Mutex = &Guessed_Value_Mutex;
        threads[i].Vector_Mutex = &Vector_Mutex;
    }

    for (int i=0;i<t;i++) 
    {
		int err = pthread_create(&(threads[i].tid), NULL, signal_handling, &threads[i]);
		if (err != 0) ERR("Couldn't create thread");
	}
    
    struct itimerval timerval;

    timerval.it_value.tv_sec = 0;
    timerval.it_interval.tv_sec = 0;
    timerval.it_value.tv_usec = 500000;
    timerval.it_interval.tv_usec = 500000;

    if(setitimer(ITIMER_REAL, &timerval, NULL)!=0) ERR("Couldn't create timer");

    while(true)
    {
        sigsuspend(&oldmask);

        if(last_signal==SIGALRM)
        {
            pthread_mutex_lock(&Vector_Mutex);
            for(int i=0;i<n;i++)
            {
                printf("%d ", vector[i]);
            }
            pthread_mutex_unlock(&Vector_Mutex);
            printf("\n");
        }
        else if(last_signal==SIGINT)
        {
            pthread_mutex_lock(&Vector_Mutex);
            int ind = rand()%n, newVal = (rand()%255)+1, oldVal = vector[ind];
            vector[ind] = newVal;
            printf("At index %d, the value %d was changed to %d\n", ind, oldVal, newVal);
            pthread_mutex_unlock(&Vector_Mutex);
        }
        else if(last_signal==SIGUSR1)
        {
            pthread_mutex_lock(&Guessed_Value_Mutex);
            printf("The guess value %d was guessed\n", GUESSED_VALUE);
            pthread_mutex_unlock(&Guessed_Value_Mutex);
            
            int err, c_tid;
            
            do
            {
                c_tid=rand()%t;
                err = pthread_cancel(threads[c_tid].tid);
            } while(err!=0);
            
            printf("Thread %d was cancled\n", c_tid);
        }
        else if(last_signal==SIGQUIT)
        {
            for(int i=0;i<t;i++)
            {
                pthread_cancel(threads[i].tid);
            }
            
            break;
        }   
    }

    int res;
    
    for(int i=0;i<t;i++)
    {
        if(pthread_join(threads[i].tid, (void**) &res)!=0) ERR("Couldn't join thread");
        if(res != PTHREAD_CANCELED) ERR("Thread didn't cancel");
    }

    free(threads);
}

void* signal_handling(void* voidArgs) 
{
	argsSignalHandler_t* args = voidArgs;
	
    while(true)
    {
        sleep(1);
        pthread_testcancel();
        
        int Index = rand_r(&args->seed) % n;
        pthread_mutex_lock(args->Vector_Mutex);

        int Value = args->vector[Index];

        pthread_mutex_unlock(args->Vector_Mutex);

        if(Value == 0) continue;
        
        pthread_mutex_lock(args->Guessed_Value_Mutex);
        
        if(Value == GUESSED_VALUE)
        {
            //send signal
            //printf("Thread %d guessed correctly the value %d", args->tid, Value);
            kill(0, SIGUSR1);
        }
        else
        {
            GUESSED_VALUE = Value;
        }
        
        pthread_mutex_unlock(args->Guessed_Value_Mutex);
    }
}