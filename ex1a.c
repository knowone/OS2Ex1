#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE_ARR 100
#define THREADS_NUM 6

void * doWork(void*);
void * change_arr(void*);
void * read_arr(void*);

int main() {

    pthread_t thread_id[THREADS_NUM];
    int answers [THREADS_NUM];
    memset(answers, 0, THREADS_NUM);

    int shd_arr[SIZE_ARR];
    memset(shd_arr,0,SIZE_ARR);
    for (int i = 0; i < THREADS_NUM; ++i) {
        int status = pthread_create(&thread_id[i],NULL,(void*)doWork,&i);
        if (status != 0){
            fputs("Can't create pthread in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < THREADS_NUM; ++i) {
        pthread_join(thread_id[i],(void**) &answers);
        printf("thread %d says: %d", i+1,answers[i]);
    }

    return 0;
}

void * doWork(void* args){
    int i = *((int*)args);
    if (i >=THREADS_NUM/2 && i < THREADS_NUM){
        change_arr(NULL);
        pthread_exit()
    }
    else{
        return read_arr(NULL);
    }
}