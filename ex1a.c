#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define SIZE_ARR 100
#define THREADS_NUM 6
#define ITERATE 1000

void * doWork(void*);               //Spilts work between threads
void * change_arr(void);            //Thread A job
bool insert_to_arr(int num);        //ADDS numbers to arr - Thread A
bool del_from_arr(int num);         //DELETES numbers from arr - Thread A
void * count_arr (void);   //Thread B job


int global_arr[SIZE_ARR];
pthread_rwlock_t lock;
/*----------------------------------------------------------------------------*/
int main() {

    int * thread_answer[THREADS_NUM];

    memset(global_arr, 0, SIZE_ARR);
    if(pthread_rwlock_init(&lock,NULL) == -1){
        perror("Cannot create lock in main()");
        exit(EXIT_FAILURE);
    }


    pthread_t thread_id[THREADS_NUM];
    int thread_arr[THREADS_NUM];
    //int * thread_answer;
    //memset(thread_answer, 0, THREADS_NUM);

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
        pthread_join(thread_id[i],(void**) &thread_answer+i);
        //Here be thread answer int
        printf("thread %d says: %d\n", i+1,**(thread_answer+i));
    }
    free(thread_answer);
    pthread_exit(NULL);

}
/*----------------------------------------------------------------------------*/
void * doWork(void* args){
    int* thread_number = (int*)args;
    if (*(thread_number) >=THREADS_NUM/2 && *(thread_number) < THREADS_NUM){
        change_arr();
    }
    else{
        count_arr();
        printf("%d in thread\n", *thread_number);
    }
    //Next line is unused
    pthread_exit(thread_number);
}
/*----------------------------------------------------------------------------*/
void * change_arr(){    //Thread A does this
    srand((unsigned int)time(NULL));
    int * counter = (int*) malloc (sizeof(int));
    for (int i = 0; i < ITERATE; ++i) {
        int minus = rand()%1 == 0 ? 1 : -1;
        int rand_num = rand()%11*minus;
        if (rand_num > 0) {
            pthread_rwlock_rdlock(&lock);
            if (insert_to_arr(rand_num)){
                ++(*counter);
            }
        }
        else if (rand_num < 0){
            pthread_rwlock_rdlock(&lock);
            if (del_from_arr(-rand_num)){
                ++(*counter);
            }

        }
        pthread_rwlock_unlock(&lock);
    }
    pthread_exit(counter);
}
/*----------------------------------------------------------------------------*/
bool insert_to_arr(int num){
    for (int i = 0; i < SIZE_ARR; ++i) {
        if (global_arr[i] == 0){
            pthread_rwlock_unlock(&lock);
            pthread_rwlock_wrlock(&lock);
            if (global_arr[i] == 0){
                global_arr[i] = num;
                return true;
            }
            else{
                pthread_rwlock_unlock(&lock);
                pthread_rwlock_rdlock(&lock);
                i = 0; //Maybe While()? - resets the loop
            }
        }
    }
    return false;
}
/*----------------------------------------------------------------------------*/
bool del_from_arr(int num){
    for (int i = 0; i < SIZE_ARR; ++i) {
        if (global_arr[i] == num){
            pthread_rwlock_unlock(&lock);
            pthread_rwlock_wrlock(&lock);
            if (global_arr[i] == num){
                global_arr[i] = 0;
                return true;
            }
            pthread_rwlock_unlock(&lock);
            pthread_rwlock_wrlock(&lock);
            i = 0; //Restart the loop, try to find again
        }
    }
    return false; //if unsuccessful
}
/*----------------------------------------------------------------------------*/
void * count_arr (){
    srand((unsigned) time(NULL));
    int * counter = (int*) malloc (sizeof(int));
    for (int i = 0; i < ITERATE; ++i) {
        int rand_num = rand()%10+1;
        for (int j = 0; j < SIZE_ARR; ++j) {
            pthread_rwlock_rdlock(&lock);
            if (global_arr[j] == rand_num){
                ++(*counter);
            }
            pthread_rwlock_unlock(&lock);
        }
    }

    pthread_exit((void*)counter);
}
/*----------------------------------------------------------------------------*/