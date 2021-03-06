/**
 * Created by @author Omer Schwartz
 * know.one.omer at gmail d.o.t com
 *
 * Program creates 6 threads, 3 producer threads that fill a global array with
 * random prime numbers, and 3 consumer threads waiting in pool, being called
 * by the producer threads to clean the full array and print the variance of
 * the full array.
 *
 * Only one consumer is allowed to run, and only 1 producer is allowed writing
 * in the global array, and that is controlled by using a POSIX mutex and
 * conditional variable.
 *
 * Program will output to stdout the variance of the current array when a
 * consumer was called upon.
 *
 * */

/*--------------------------- Include Section --------------------------------*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
/*-------------------------- Define Section ----------------------------------*/
#define N 10                        //As defined in exercise
#define THREADS_NUM 6               //Amount of threads, half will be
                                    //producers and half consumers
#define ITERATE 100                 //Thread's loop iteration
#define RANGE 100                   //Range of prime numbers
#define STOP_VALUE -1               //Cleanup threads stop marker for counter

#define SLEEP usleep((unsigned)rand()%SLEEP_DELAY);
#define SLEEP_DELAY 200000          //Used to generate different prime numbers
                                    //(in milliseconds)
                                    //The bigger the number, the slower the program
                                    //Recommended value - 200000
                                    //(minimum 1 - no sleep)

/*#define DEBUG                     //Uncomment this to see verbose(extended)
                                    //data about the program running and
                                    //threads work*/
/*------------------------ Type Definition -----------------------------------*/

typedef struct{
    int _nums[N];
    int _counter;
}Data;
/*------------------------ Global Variables ----------------------------------*/

Data data;                                          //For Thread operations

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;    //Mutex lock
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;       //cond.var for thread sleep
/*------------------------ Func Declaration ----------------------------------*/

void * assign_thread(void *);
void * updater_thread(int);
void * cleaner_thread(int);
bool isPrime(int);
double calcVariance(int *arr, int arr_len);
int generatePrime(int);
/*------------------------ Main implementation -------------------------------*/
/**
 * Main creates all the threads with their assignment number as param for the
 * thread function.
 * After pthread_joining for the first 3 threads (the producer threads) it
 * signals the consumer threads to terminate (with data.counter=STOP_VALUE)
 * and destroys locks and exits.
 * */
int main() {
    int status;
    pthread_t thread_id[THREADS_NUM];        //required for thread creating
    srand((unsigned)time(NULL));
    int thread_assign[THREADS_NUM];          //needed for thread job assignment
    data._counter = 0;                              //Reset data.counter
    //(already zeroed; better safe then sorry)
    for (int i = 0; i < THREADS_NUM; ++i) {
        thread_assign[i] = i;                       //assign each thread a role
        //Create each thread
        status = pthread_create(&thread_id[i],
                                NULL,
                                assign_thread,
                                &thread_assign[i]);
        if (status != 0) {
            fputs("error creating threads in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }
    //Wait for producer threads
    for (int j = 0; j < THREADS_NUM / 2; ++j) {

        status = pthread_join(thread_id[j], NULL);
        if (status != 0) {
            fputs("Error joining threads in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }

    data._counter = STOP_VALUE;         //Send STOP signal to consumer threads
    pthread_cond_signal(&cv);           //Wake consumer threads from sleep
    //Wait for threads to finished exiting
    for (int k = THREADS_NUM/2; k < THREADS_NUM ; ++k) {
        status = pthread_join(thread_id[k], NULL);
        if (status != 0) {
            fputs("Error joining threads in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }
    //Release mutex and cond.var
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cv);
    exit(EXIT_SUCCESS);
}
/*------------------------ Function implementation ---------------------------*/
/**
 * Function to assign a thread a job - updater_thread (producer thread)
 *                                   - cleaner_thread (consumer thread)
 * @param args pointer to function arguments. Should contain the thread
            number(id)
 * @return function returns NULL as ret_value in pthread_exit()
 * */
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
/**
 * Performs the first assignment: Generates a random prime, and tries to insert
 * it into global array. If unsuccessful (when array is full), signals the cleaner
 * threads to start working.
 * Uses mutex lock to control writing into the array.
 * @param thread_id the thread_id number used for printing information.
 * @return NULL as ret_val in pthread_exit()
 *
 * Note the DEBUG macro. If DEBUG definition is uncommented the extra printf()
 * will give useful information on thread operations.
 * */
void * updater_thread(int thread_id){

    for (int i = 0; i < ITERATE ; ++i) {
        pthread_mutex_lock(&mtx);           //Am I allowed to continue?
        pthread_mutex_unlock(&mtx);         //Currently no cleaner thread is working
        bool wasInserted = false;           //To mark whether to call the cleaner_thread

        SLEEP;                              //Allows the random generator to
        //get new random numbers
        int rand_prime = generatePrime(RANGE);
#ifdef DEBUG
        printf("Thread %d generated %d\n", thread_id,rand_prime);
#endif
        pthread_mutex_lock(&mtx);           //acquire lock

/*-------------------------- Critical Section --------------------------*/
        if (data._counter < N){             //array is not full
//Verbose information:
#ifdef DEBUG
            printf("thread=%d says: rand_numer=%d, data_counter=%d\n",
            thread_id,
            rand_prime,
            data._counter);
            fflush(stdout);
#endif
            data._nums[data._counter] = rand_prime; //insert random prime to array
            ++data._counter;                        //increase counter
            pthread_mutex_unlock(&mtx);             //release the lock
/*-------------------------- End Crit.Section---------------------------*/

            wasInserted = true;                 //mark insertion as success
        }
        else {
            pthread_mutex_unlock(&mtx);         //array is full, only release lock
        }
        //array is full:
        if (!wasInserted){
//Verbose information:
#ifdef DEBUG
            printf("Thread %d calls upon a cleaner thread\n",thread_id);
#endif
            pthread_cond_signal(&cv);           //Notify all cleaners array is full
        }
    }
//#ifdef DEBUG
    printf("Thread %d finished running. Bye!\n", thread_id);
    //Notify user Thread has finished
//#endif
    pthread_exit(NULL);
}
/*----------------------------------------------------------------------------*/
/**
 * Performs the second assignment: Sleeps on the cond.var, and if awoken by
 * updater_thread, acquires the lock and copies the global array, zeros out the
 * counter and prints the (population) variance.
 * @param thread_id the thread id number used to display information
 * @return NULL as ret_val in pthread_exit()
 * */
void * cleaner_thread(int thread_id){
    while (data._counter != STOP_VALUE){    //Predefined STOP_VALUE
        int my_arr[N];                      //local copy of global array
        pthread_cond_wait(&cv, &mtx);       //Wait to be called

        if (data._counter == STOP_VALUE){   //STOP_VALUE was issued during run
            pthread_mutex_unlock(&mtx);     //release the lock acquired by cond_wait()
            pthread_cond_signal(&cv);       //signal next thread waiting
            printf("Thread %d has finished.\n",thread_id);
            pthread_exit(NULL);             //exit
        }
        //Action was already performed, release lock and wait again
        if (data._counter == 0){
            pthread_mutex_unlock(&mtx);
        }
        else {
            /*----- Critical Section ------*/
            //Copy the global array
            for (int i = 0; i < N; ++i) {
                my_arr[i] = data._nums[i];
            }
            //Zero the counter
            data._counter = 0;
            pthread_mutex_unlock(&mtx);
            /*-------- End Section --------*/

            double vrn = calcVariance(my_arr, N);   //Calculate variance
            printf("Cleanup performed by thread %d. Calculated variance %.2lf\n",
                   thread_id, vrn);
        }
    }
    pthread_exit(NULL);
}
/*----------------------------------------------------------------------------*/
/**
 * Efficient prime checker
 * @return true if @param num is a prime integer
 *
 * */
bool isPrime(int num){

    if (num == 1){return false;}
    if (num == 2 || num == 3)       {return true;}
    if (!(num & 1))                 {return false;}   //Bitwise Parity check -
    //if is paired than not a prime
    if (!(num+1)%6 || !(num-1)%6)   {return false;}   //every number that can be
    //represented as 6n+1 or 6n-1 is a prime
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
/**
 * Calculates the (population) variance
 * @return the variance based on the @param arr of integers and @param arr_len
 * indicating the length of the array.
 * */
double calcVariance(int *arr, int arr_len){
    if (arr_len == 0){                 //Do Not Divide by 0!
        return 0;
    }
    //Calculate the average
    double avg = 0, variance = 0;
    int i = 0;

    for (int j = 0; j < arr_len; ++j) {
        avg+=arr[j];
    }
    avg /=arr_len;

    //Calc (sum for all 0<=i<sizeArray (Xi-Xavg)^2) /sizeOfArray;
    for (i=0; i<arr_len; i++){
        variance = variance + pow((arr[i]-avg),2);
    }
    return variance/(double)arr_len;
}
/*----------------------------------------------------------------------------*/
/**
 * Prime generator
 * Based on @param range to limit the size of the prime generated p to be
 * p < (@param range).
 * Using a random integer generator and usleep() to roll a random number,
 * check if it is a prime and @return a random prime number
 * */
int generatePrime(int range){

    //Sleep to pause between threads
    int num =rand()&range;
    while (!isPrime(num)){
        num = rand()%range;
    }
    return num;
}
