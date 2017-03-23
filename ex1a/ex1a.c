/**
 * Created by @author Omer Schwartz
 * know.one.omer at gmail d.o.t com
 *
 * Program reads and writes using threads to a single array, managing it with
 *		read/write locks.
 *	The writers try to add or remove numbers from the array and the readers
 *     try to find numbers in the array
 * The program will print to standard out the results of the thread
 * reading and writing.
 * */

/*-------------------------- Include Section ---------------------------------*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
/*-------------------------- Define Section ----------------------------------*/
#define SIZE_ARR 100
#define THREADS_NUM 6
#define ITERATE 1000
#define RANGE 10
/*------------------------ Type Definition -----------------------------------*/
typedef struct {
    int added;
    int removed;
    int read;
}thread_data_t;
/*------------------------ Func Declaration ----------------------------------*/

void * doWork(void*);               //Spilts work between threads
void * change_arr(void);            //Thread A job
bool insert_to_arr(int num);        //ADDS numbers to arr - Thread A
bool rmv_from_arr(int num);         //DELETES numbers from arr - Thread A
void * count_arr (void);            //Thread B job

/*------------------------ Global Variables ----------------------------------*/
int global_arr[SIZE_ARR];           //Shared array by al threads
pthread_rwlock_t lock;              //global rw_lock
/*------------------------ Main implementation -------------------------------*/
/**
* The main thread in the program creates six secondary threads of two types:
* a.	The first three threads randomly create a number between -10 to 10,
* insert the number in an array if is a positive else do nothing,
*  search the opposite number and empty the cell if is a negative.
* Each thread counts how many he has added or removed finally
*  he returns to the main thread and prints it.
* b.	The last three threads, each thread creates 1000 times a random number
* between 1 to 10.
* Each thread returns how many values he found in an array.
* Meanwhile the main thread is waiting for them to finish and at the end he
* prints the information which was returned.
*
*/
int main() {
    int status;
    memset(global_arr, 0, SIZE_ARR);                //Zero out the global arr
    pthread_t thread_id[THREADS_NUM];               //required for thread creating
    int thread_arr[THREADS_NUM];                    //needed for thread job assignment

    thread_data_t * thread_answer[THREADS_NUM];     //Holds counters ret from thread
    if(pthread_rwlock_init(&lock,NULL) == -1){      //init the lock
        fputs("Cannot create lock in main()", stderr);
        exit(EXIT_FAILURE);
    }


    for (int i = 0; i < THREADS_NUM; ++i) {         //create threads
        thread_arr[i] = i;                          //identify each thread

        //create threads, send to doWork(), with thread number param
        status = pthread_create(&thread_id[i],NULL,(void*)doWork,&thread_arr[i]);
        if (status != 0){
            fputs("Can't create pthread in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < THREADS_NUM; ++i) {         //Wait for 6 threads, print the output
        status = pthread_join(thread_id[i],(void**) &(thread_answer[i]));
        //thread_answer[i] is allocated from thread
        if (status!=0){
            fputs("Error with pthread_join in main()",stderr);
            exit(EXIT_FAILURE);
        }
        //Check answer exists

        if (thread_answer[i] != NULL) {
            //Print output by thread assignment
            if (thread_arr[i] < THREADS_NUM/2) {
                printf("thread %d with id=(%ld) added %d numbers and removed %d numbers\n",
                       i + 1,
                       thread_id[i],
                       thread_answer[i]->added, thread_answer[i]->removed);
            }
            else{
                printf("thread %d with id=(%ld) found %d of its numbers in the array\n",
                       i+1,
                       thread_id[i],
                       thread_answer[i]->read);
            }
        }//endif, skip if no answer is returned from thread.
        // Error handling should be here
    }

    //Free allocated space before main() exits
    for (int j = 0; j < THREADS_NUM; ++j) {
        if (thread_answer[j] != NULL) {
            free(thread_answer[j]);
        }
    }

    pthread_rwlock_destroy(&lock);  //Release lock
    exit(EXIT_SUCCESS);            //Can also use pthread_exit()

}
/*----------------------------------------------------------------------------*/
/**
* doWork splits the work between the threads
 * @param args contains the thread number (id) for its assignment
 * @return NULL to pthread_exit()
*/
void * doWork(void* args){
    int* thread_number = (int*)args;    //pointer to thread id
    if (*(thread_number) < THREADS_NUM/2 && *(thread_number) >= 0){
        change_arr();                   //For Threads type A
    }
    else{
        count_arr();                    //For threads type B
    }
    //Next line is unused
    pthread_exit(NULL);
}
/*----------------------------------------------------------------------------*/
/**
* Threads type A perform insertion or removal of value from array
 * Also, they count how many operations where performed by the threads
 * @return counter pointer to allocated memory containing the counter of how
 *  many threads operations completed successfully
*/
void * change_arr(){    //Thread A does this

    srand((unsigned int)time(NULL));

    //Allocate memory for counter
    thread_data_t * counter = (thread_data_t*) malloc (sizeof(thread_data_t));
    if (counter == NULL){
        fputs("cannot allocate memory in change_arr()", stderr);
        exit(EXIT_FAILURE); //should be soft exit - with cleanup
    }

    for (int i = 0; i < ITERATE; ++i) {
        int minus = rand()%2 == 0 ? 1 : -1;     //decide if - or +
        int rand_num = (rand()%(RANGE+1))*minus;
        if (rand_num > 0) {
            if (insert_to_arr(rand_num)){       //call C.S part
                ++(counter->added);             //count results
            }
        }
        else if (rand_num < 0){
            if (rmv_from_arr(-rand_num)){       //Use the opposite of rand_num
                ++(counter->removed);
            }

        }//ignore case rand_num=0
    }
    pthread_exit(counter);
}
/*----------------------------------------------------------------------------*/
/**
 * Perform the insert operation on the array.
 * insert @param num to global array by using wrlock
 * @return true if successfully inserted a number to array
 *          false if failed
 * */
bool insert_to_arr(int num){
    int status;
    for (int i = 0; i < SIZE_ARR; ++i) {
        /*Critical Section:*/
        status = pthread_rwlock_rdlock(&lock);      //get reader lock
        if (status!=0){
            fputs("Error with acquire read_lock in insert_to_arr()",stderr);
            pthread_exit(NULL);   //Should try again
        }

        if (global_arr[i] == 0){                    //available space
            pthread_rwlock_unlock(&lock);           //release reader lock
            /*Critical Section ends*/
            pthread_rwlock_wrlock(&lock);           //acquire writer lock
            /*Critical Section:*/
            if (global_arr[i] == 0){                //verify empty cell in array
                global_arr[i] = num;                //write number to array
                pthread_rwlock_unlock(&lock);       //release lock
                /*Critical Section ends*/
                return true;
            }
            else{
                pthread_rwlock_unlock(&lock);       //only release the lock
                //if cell no longer empty
                i = 0; //resets the loop
            }
        }
        else{
            pthread_rwlock_unlock(&lock);           //release the rdlock
            //if cell not empty
        }
    }
    return false;
}
/*----------------------------------------------------------------------------*/
/**
 *  Perform the removal operation on the array.
 * Deletes @param num from global array by using wrlock
 * @return true if successfully removed the number from array
 *          false if failed
 * */
bool rmv_from_arr(int num){
    int status;
    for (int i = 0; i < SIZE_ARR; ++i) {
        status = pthread_rwlock_rdlock(&lock);

        /*Critical Section:*/
        if (status!=0){
            fputs("Error with acquire read_lock in rmv_from_arr()",stderr);
            pthread_exit(NULL);
        }
        if (global_arr[i] == num){       //Found instance of num in array
            pthread_rwlock_unlock(&lock);   //release rdlock
            /*Critical Section ends*/

            pthread_rwlock_wrlock(&lock);   //get wrlock

            /*Critical Section:*/
            if (global_arr[i] == num){      //recheck
                global_arr[i] = 0;          //zero out the cell
                pthread_rwlock_unlock(&lock);   //release wrlock
                /*Critical Section ends*/

                return true;
            }
            else{
                pthread_rwlock_unlock(&lock);
                i = 0; //Restart the loop, try to find again
            }

        }
        else{
            pthread_rwlock_unlock(&lock);
        }
    }
    return false; //if unsuccessful
}
/*----------------------------------------------------------------------------*/
/**
 * Threads type B try to find a random number in the array.
 * @return pointer to counter containing how many successful attempts where
 *  made to locate that random number.
 * */
void * count_arr (){
    int status;
    srand((unsigned) time(NULL));
    bool found;

    //Allocate memory for counter
    thread_data_t * counter = (thread_data_t*) malloc (sizeof(thread_data_t));
    if (counter == NULL){
        fputs("cannot allocate memory in count_arr()", stderr);
        exit(EXIT_FAILURE); //should be soft exit - with cleanup
    }

    for (int i = 0; i < ITERATE; ++i) {
        found = false;
        int rand_num = rand() % RANGE + 1;
        for (int j = 0; j < SIZE_ARR; ++j) {
            status = pthread_rwlock_rdlock(&lock);      //acquire rdlock
            if (status!=0){
                fputs("Error with acquire read_lock in count_arr()",stderr);
                pthread_exit(NULL);
            }
            /*Critical Section:*/

            if (global_arr[j] == rand_num) {            //found my random number
                found = true;                           //mark as found
            }
            // Release the lock after every single arr read
            pthread_rwlock_unlock(&lock);               //release the rdlock
            /*Critical Section ends*/

            if (found) {
                ++(counter->read);                      //count successful attmp
                break;
            }
        }
    }
    pthread_exit((void*)counter);
}
/*----------------------------------------------------------------------------*/