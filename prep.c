#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#define ERR(source) (perror(source),\
                     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     exit(EXIT_FAILURE))
void usage(){
    fprintf(stderr, "USAGE: ./prep n k[1-60]\n");
}

typedef struct argsThread {
    pthread_t tid;
    double* arr;
    int arraySize;
    unsigned int seed;
    double* sqrtsResult;
    int* sqrtsComputed;
    pthread_mutex_t *mxSqrtsComputed;
    pthread_mutex_t *mxSqrtsResult;
} argsThread_t;

void* calcSqrt(void* args){
    argsThread_t *arguments = args;
    printf("Thread #%ld started\n", pthread_self());
    while(1)
    {   
        int position = rand_r(&arguments->seed) % (arguments->arraySize);
        pthread_mutex_lock(arguments->mxSqrtsComputed);
        if(*arguments->sqrtsComputed > 0)
        {
            if(arguments->sqrtsResult[position]==-1)
            {
                (*arguments->sqrtsComputed) -= 1;
                pthread_mutex_unlock(arguments->mxSqrtsComputed);

                pthread_mutex_lock(&arguments->mxSqrtsResult[position]);
                arguments->sqrtsResult[position] = sqrt(arguments->arr[position]);
                pthread_mutex_unlock(&arguments->mxSqrtsResult[position]);

                printf("Thread #%ld altered position %d\n", pthread_self(), position);
            }
            else{
                pthread_mutex_unlock(arguments->mxSqrtsComputed);
                continue;
            }
        }
        else {
            pthread_mutex_unlock(arguments->mxSqrtsComputed);
            break;
        }    

    }
    printf("Thread #%ld finished\n", pthread_self());
    return NULL;
}

int main(int argc, char** argv)
{
    if(argc != 3){
        usage();
        exit(EXIT_FAILURE);
    }
    int threadCount = atoi(argv[1]); int arraySize = atoi(argv[2]);
    if(threadCount <= 0 || arraySize <= 0 || arraySize > 60){
        usage();
        exit(EXIT_FAILURE);
    }

    int* returnValueOfThread;
    srand(time(NULL));
    argsThread_t* args = (argsThread_t*)malloc(sizeof(argsThread_t) * threadCount);
    double* Array = (double*)malloc(sizeof(double) * arraySize);
    double* sqrtResults = (double*)malloc(sizeof(double) * arraySize);
    pthread_mutex_t mxSqrtsComputed = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxSqrtsResult[arraySize];

    for(int i = 0; i < arraySize; i++){
        int err = pthread_mutex_init(&mxSqrtsResult[i], NULL);
        if(err!=0) ERR("Mutex init failed:");
    }

    int* sqrtsComputed = (int*)malloc(sizeof(int)); *sqrtsComputed = arraySize;
    printf("Array is: ");
    for(int i = 0; i < arraySize; i++){
        Array[i] = 228+i*5;
        sqrtResults[i] = -1;
        printf("%lf ", Array[i]);
    }
    for(int i = 0; i < arraySize; i++){
        args[i].arr = Array;
        args[i].sqrtsResult = sqrtResults;
        args[i].arraySize = arraySize;
        args[i].seed = rand(); 
        args[i].sqrtsComputed = sqrtsComputed;
        args[i].mxSqrtsResult = mxSqrtsResult;
        args[i].mxSqrtsComputed = &mxSqrtsComputed;
    }
    printf("\n");
    for(int i = 0; i < threadCount; i++){
        int err = pthread_create(&(args[i].tid), NULL, calcSqrt, args);
        if(err!=0) ERR("Could not create new thread");
    }
    for(int i = 0; i < threadCount; i++){
        int err = pthread_join(args[i].tid, (void*)&returnValueOfThread);
        if(err!=0) ERR("Could not join thread");
    }
    //printing resulting arrays
    printf("Resulting array is:");
    for(int i = 0; i < arraySize; i++){
        printf("%lf ", sqrtResults[i]);
    }
    printf("\n");
    for (int i = 0; i < arraySize; i++) {
        pthread_mutex_destroy(&mxSqrtsResult[i]);
    }
    pthread_mutex_destroy(&mxSqrtsComputed);
    free(args);
    return EXIT_SUCCESS;
}