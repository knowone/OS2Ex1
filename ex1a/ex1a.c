#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define SIZE_ARR 100
#define THREADS_NUM 6
#define ITERATE 1000
#define RANGE 10

typedef struct {
    int add;
    int remove;
    int read;
}thread_data_t;

void * doWork(void*);               //Spilts work between threads
void * change_arr(void);            //Thread A job
bool insert_to_arr(int num);        //ADDS numbers to arr - Thread A
bool rmv_from_arr(int num);         //DELETES numbers from arr - Thread A
void * count_arr (void);            //Thread B job


int global_arr[SIZE_ARR];
pthread_rwlock_t lock;
/*----------------------------------------------------------------------------*/
int main() {

    thread_data_t * thread_answer[THREADS_NUM];
    memset(global_arr, 0, SIZE_ARR);
    if(pthread_rwlock_init(&lock,NULL) == -1){
        perror("Cannot create lock in main()");
        exit(EXIT_FAILURE);
    }

    pthread_t thread_id[THREADS_NUM];
    int thread_arr[THREADS_NUM];

    for (int i = 0; i < THREADS_NUM; ++i) {
        thread_arr[i] = i;
        int status = pthread_create(&thread_id[i],NULL,(void*)doWork,&thread_arr[i]);
        if (status != 0){
            fputs("Can't create pthread in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }
    if (thread_answer == NULL){
        perror("Allocating memory failed. main()");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < THREADS_NUM; ++i) {
        pthread_join(thread_id[i],(void**) &(thread_answer[i]));
        //Here be thread answer int
        if (thread_answer[i] != NULL) {
            if (thread_arr[i] < THREADS_NUM/2)
                printf("thread %d added %d numbers and removed %d numbers\n", i + 1, thread_answer[i]->add, thread_answer[i]->remove);
            else
                printf("thread %d found %d of it's numbers in the array\n", i+1, thread_answer[i]->read);
        }
    }
    for (int j = 0; j < THREADS_NUM; ++j) {
        if (thread_answer[j] != NULL) {
            free(thread_answer[j]);
        }
    }
    pthread_rwlock_destroy(&lock);
    pthread_exit(NULL);

}
/*----------------------------------------------------------------------------*/
void * doWork(void* args){
    int* thread_number = (int*)args;
    if (*(thread_number) < THREADS_NUM/2 && *(thread_number) >= 0){
//        printf("starting change_arr in doWork() thread %d\n",*thread_number);
        change_arr();
    }
    else{
//        printf("starting count_arr in doWork() thread %d\n",*thread_number);
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
        perror("cannot allocate memory in change_arr()");
        exit(EXIT_FAILURE); //should be soft exit - with cleanup
    }
    for (int i = 0; i < ITERATE; ++i) {
        int minus = rand()%2 == 0 ? 1 : -1;
        int rand_num = (rand()%(RANGE+1))*minus;
//        printf("TEST: rand_num=%d in change_arr()\n",rand_num);
        if (rand_num > 0) {
            if (insert_to_arr(rand_num)){
                ++(counter->add);
            }
        }
        else if (rand_num < 0){
            if (rmv_from_arr(-rand_num)){
                ++(counter->remove);
            }

        }
    }
    pthread_exit(counter);
}
/*----------------------------------------------------------------------------*/
bool insert_to_arr(int num){
    for (int i = 0; i < SIZE_ARR; ++i) {
        pthread_rwlock_rdlock(&lock);
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
                i = 0; //Maybe While()? - resets the loop
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

    for (int i = 0; i < SIZE_ARR; ++i) {
        pthread_rwlock_rdlock(&lock);
        if (global_arr[i] == num){
            pthread_rwlock_unlock(&lock);
            pthread_rwlock_wrlock(&lock);
            if (global_arr[i] == num){
                global_arr[i] = 0;
                pthread_rwlock_unlock(&lock);
//                printf("removed %d from arr[%d]=%d\n", num, i, global_arr[i]);
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

    srand((unsigned) time(NULL));
    bool found;
    thread_data_t * counter = (thread_data_t*) malloc (sizeof(thread_data_t));
    if (counter == NULL){
        perror("cannot allocate memory in change_arr()");
        exit(EXIT_FAILURE); //should be soft exit - with cleanup
    }

    for (int i = 0; i < ITERATE; ++i) {
        found = false;
//        usleep(900);
        int rand_num = rand() % RANGE + 1;
        for (int j = 0; j < SIZE_ARR; ++j) {
            pthread_rwlock_rdlock(&lock);
            if (global_arr[j] == rand_num) {
//                printf("Found=%d in array[%d]=%d\n", rand_num, j, global_arr[j]);
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