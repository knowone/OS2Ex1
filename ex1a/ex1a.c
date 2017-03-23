/**
 * Created by @author Omer Schwartz
 * know.one.omer at gmail d.o.t com
 *
 *
 *
 * */

/*------------------------- Include Section --------------------------------*/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZE_ARR 100
#define THREADS_NUM 6
#define ITERATE 1000
#define RANGE 10

typedef struct {
    int added;
    int removed;
    int read;
}thread_data_t;

void * doWork(void*);               //Spilts work between threads
void * change_arr(void);            //Thread A job
bool insert_to_arr(int num);        //ADDS numbers to arr - Thread A
bool rmv_from_arr(int num);         //DELETES numbers from arr - Thread A
void * count_arr (void);            //Thread B job


int global_arr[SIZE_ARR];           //Shared array by al threads
pthread_rwlock_t lock;              //global rw_lock
/*----------------------------------------------------------------------------*/
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
        status = pthread_join(thread_id[i],(void**) &(thread_answer[i]));    //thread_answer[i] is allocated from thread
        if (status!=0){
            fputs("Error with pthread_join in main()",stderr);
            exit(EXIT_FAILURE);
        }
        //Check answer exists

        if (thread_answer[i] != NULL) {
            //Print output by thread assignment
            if (thread_arr[i] < THREADS_NUM/2) {
                printf("thread %d with id=(%ld) added %d numbers and removed %d numbers\n", i + 1, thread_id[i],
                       thread_answer[i]->added, thread_answer[i]->removed);
            }
            else{
                printf("thread %d with id=(%ld) found %d of its numbers in the array\n", i+1,thread_id[i],
                       thread_answer[i]->read);
            }
        }//endif, skip if no answer is returned from thread. Error handling should be here
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
void * doWork(void* args){
    int* thread_number = (int*)args;
    if (*(thread_number) < THREADS_NUM/2 && *(thread_number) >= 0){
        change_arr();
    }
    else{
        count_arr();
    }
    //Next line is unused
    pthread_exit(NULL);
}
/*----------------------------------------------------------------------------*/
void * change_arr(){    //Thread A does this
    srand((unsigned int)time(NULL));
    thread_data_t * counter = (thread_data_t*) malloc (sizeof(thread_data_t));
    if (counter == NULL){
        fputs("cannot allocate memory in change_arr()", stderr);
        exit(EXIT_FAILURE); //should be soft exit - with cleanup
    }
    for (int i = 0; i < ITERATE; ++i) {
        int minus = rand()%2 == 0 ? 1 : -1;
        int rand_num = (rand()%(RANGE+1))*minus;
        if (rand_num > 0) {
            if (insert_to_arr(rand_num)){
                ++(counter->added);
            }
        }
        else if (rand_num < 0){
            if (rmv_from_arr(-rand_num)){
                ++(counter->removed);
            }

        }
    }
    pthread_exit(counter);
}
/*----------------------------------------------------------------------------*/
bool insert_to_arr(int num){
    int status;
    for (int i = 0; i < SIZE_ARR; ++i) {
        status = pthread_rwlock_rdlock(&lock);
        if (status!=0){
            fputs("Error with acquire read_lock in insert_to_arr()",stderr);
            pthread_exit(NULL); //Should try again
        }
        if (global_arr[i] == 0){
            pthread_rwlock_unlock(&lock);
            pthread_rwlock_wrlock(&lock);
            if (global_arr[i] == 0){
                global_arr[i] = num;
                pthread_rwlock_unlock(&lock);
                return true;
            }
            else{
                pthread_rwlock_unlock(&lock);
                i = 0; //resets the loop
            }
        }
        else{
            pthread_rwlock_unlock(&lock);
        }
    }
    return false;
}
/*----------------------------------------------------------------------------*/
bool rmv_from_arr(int num){
    int status;
    for (int i = 0; i < SIZE_ARR; ++i) {
        status = pthread_rwlock_rdlock(&lock);
        if (status!=0){
            fputs("Error with acquire read_lock in rmv_from_arr()",stderr);
            pthread_exit(NULL);
        }        if (global_arr[i] == num){
            pthread_rwlock_unlock(&lock);
            pthread_rwlock_wrlock(&lock);
            if (global_arr[i] == num){
                global_arr[i] = 0;
                pthread_rwlock_unlock(&lock);
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
void * count_arr (){
    int status;
    srand((unsigned) time(NULL));
    bool found;
    thread_data_t * counter = (thread_data_t*) malloc (sizeof(thread_data_t));
    if (counter == NULL){
        fputs("cannot allocate memory in count_arr()", stderr);
        exit(EXIT_FAILURE); //should be soft exit - with cleanup
    }

    for (int i = 0; i < ITERATE; ++i) {
        found = false;
        int rand_num = rand() % RANGE + 1;
        for (int j = 0; j < SIZE_ARR; ++j) {
            status = pthread_rwlock_rdlock(&lock);
            if (status!=0){
                fputs("Error with acquire read_lock in count_arr()",stderr);
                pthread_exit(NULL);
            }
            if (global_arr[j] == rand_num) {
                found = true;
            }
            pthread_rwlock_unlock(&lock);
            if (found) {
                ++(counter->read);
                break;
            }
        }
    }
    pthread_exit((void*)counter);
}
/*----------------------------------------------------------------------------*/