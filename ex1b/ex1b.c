/*----------------------------------------------------------------------------*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
/*----------------------------------------------------------------------------*/
#define N 10
#define THREADS_NUM 6
#define ITERATE 100
#define RANGE 100
#define STOP_VALUE -1
#define SLEEP_DELAY 200
#define SLEEP SLEEP1
#define SLEEP1 usleep(SLEEP_DELAY);
#define SLEEP2 usleep(0);
//#define DEBUG
/*----------------------------------------------------------------------------*/
typedef struct{
    int _nums[N];
    int _counter;
}Data;
/*----------------------------------------------------------------------------*/
Data data;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
/*----------------------------------------------------------------------------*/
void * assign_thread(void *);
void * updater_thread(int);
void * cleaner_thread(int);
bool isPrime(int);
double calcVariance(int *arr, int arr_len);
int generatePrime(int);

/*----------------------------------------------------------------------------*/
int main() {
    int status;
    pthread_t thread_id[THREADS_NUM];               //required for thread creating

    int thread_assign[THREADS_NUM];                    //needed for thread job assignment
    memset(data._nums, 0, N);
    for (int i = 0; i < THREADS_NUM; ++i) {
        thread_assign[i] = i;
        status = pthread_create(&thread_id[i], NULL, assign_thread, &thread_assign[i]);
        if (status != 0) {
            fputs("error creating threads in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }
    //pthread_cleanup_push(close_garbage_threads, NULL);
    for (int j = 0; j < THREADS_NUM / 2; ++j) {
        status = pthread_join(thread_id[j], NULL);
        if (status != 0) {
            fputs("Error joining threads in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }
    data._counter = STOP_VALUE;
    pthread_cond_signal(&cv);

    pthread_exit(NULL);
}
/*----------------------------------------------------------------------------*/
void * assign_thread(void *args){

    int thread_id = *(int*)args;
    if (thread_id <THREADS_NUM/2 && thread_id >=0){
        updater_thread(thread_id);
    }
    else{
        cleaner_thread(thread_id);
    }
    //this line isn't reached
    pthread_exit(NULL);
}
/*----------------------------------------------------------------------------*/
void * updater_thread(int thread_id){
    int i;
    for (i = 0; i < ITERATE ; ++i) {
        pthread_mutex_lock(&mtx);           //Am I allowed to continue?
        pthread_mutex_unlock(&mtx);         //Passed Checks
        bool wasInserted = false;
        int rand_prime = generatePrime(RANGE);
#ifdef DEBUG
        printf("Thread %d generated %d\n", thread_id,rand_prime);
#endif
        pthread_mutex_lock(&mtx);
        if (data._counter < N){
#ifdef DEBUG
            printf("thread=%d says: rand_numer=%d, data_counter=%d\n",thread_id,rand_prime,data._counter);
            fflush(stdout);
#endif

            data._nums[data._counter] = rand_prime;
            ++data._counter;
//            printf("releasing lock\n");
            pthread_mutex_unlock(&mtx);
            wasInserted = true;
        }
        else {
            pthread_mutex_unlock(&mtx);
        }



        if (!wasInserted){
#ifdef DEBUG
            printf("Thread %d calls upon a cleaner thread\n",thread_id);
#endif
            pthread_cond_signal(&cv);

        }
    }
//#ifdef DEBUG
    printf("Thread %d finished running. Bye!\n", thread_id);
//#endif
    pthread_exit(NULL);
}
/*----------------------------------------------------------------------------*/
void * cleaner_thread(int thread_id){
    while (data._counter != STOP_VALUE){
        int my_arr[N];
        pthread_cond_wait(&cv, &mtx);       //Wait to be called
        if (data._counter == STOP_VALUE){
            pthread_mutex_unlock(&mtx);
            pthread_cond_signal(&cv);
            printf("Thread %d has finished.\n",thread_id);
            pthread_exit(NULL);
        }
        for (int i = 0; i < N; ++i) {
            my_arr[i] = data._nums[i];
        }
        data._counter = 0;
        pthread_mutex_unlock(&mtx);

        double vrn = calcVariance(my_arr, N);
        printf("Cleanup performed by thread %d. Calculated variance %.2lf\n",thread_id, vrn);
    }
    pthread_exit(NULL);
}
/*----------------------------------------------------------------------------*/
bool isPrime(int num){

    if (num == 1){return false;}
    if (num == 2 || num == 3)       {return true;}
    if (!(num & 1))                 {return false;}   //Bitwise Parity check - if is paired than not a prime
    if (!(num+1)%6 || !(num-1)%6)   {return false;}   //every number that can be represented as 6n+1 or 6n-1 is a prime
    int i;
    int q = (int)sqrt(num)+1;       //Calculate up-to square-root of num
    for(i = 3; i < q; i+=2){        //skip paired integers
        if (num%i == 0){
            return false;
        }
    }
    return true;
}
/*----------------------------------------------------------------------------*/
double calcVariance(int *arr, int arr_len){
    if (arr_len == 0){                 //Do Not Divide by 0!
        return 0;
    }
    double avg = 0, variance = 0;
    int i = 0;

    for (int j = 0; j < arr_len; ++j) {
        avg+=arr[j];
    }
    avg /=arr_len;

    //Calc (sum for all 0<=i<sizeArray (Xi-avg)^2) /sizeOfArray;
    for (i=0; i<arr_len; i++){
        variance = variance + pow((arr[i]-avg),2);
    }
    return variance/(double)arr_len;
}
/*----------------------------------------------------------------------------*/
int generatePrime(int range){

    srand((unsigned)time(NULL));
    int num = rand()%range;
    SLEEP;
    num *= rand()%range;
    while (!isPrime(num)){
        num = rand()%range;
        SLEEP;
        num *= rand()%range;
    }
    return num;
}