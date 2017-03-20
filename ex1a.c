#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE_ARR 100
#define THREADS_NUM 6

void * doWork(void*);
void * change_arr(int*);
void * read_arr(int*);
void clean_mem(void *);

int main() {

    pthread_t thread_id[THREADS_NUM];
    int thread_arr[THREADS_NUM];
    //int * thread_answer;
    //memset(thread_answer, 0, THREADS_NUM);

    int shd_arr[SIZE_ARR];
    memset(shd_arr,0,SIZE_ARR);
    for (int i = 0; i < THREADS_NUM; ++i) {
        thread_arr[i] = i;
        int status = pthread_create(&thread_id[i],NULL,(void*)doWork,&thread_arr[i]);
        if (status != 0){
            fputs("Can't create pthread in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }
    int * thread_answer = (int *) malloc(sizeof(int));
    if (thread_answer == NULL){
        perror("Allocating memory failed. main()");
        exit(EXIT_FAILURE);
    }
    pthread_cleanup_push(clean_mem, thread_answer);
    for (int i = 0; i < THREADS_NUM; ++i) {
        pthread_join(thread_id[i],(void**) &thread_answer);
        //Here be thread answer int
        printf("thread %d says: %d\n", i+1,*(thread_answer));
    }
    pthread_exit(NULL);
    pthread_cleanup_pop(0);
}

void * doWork(void* args){
    int* thread_number = (int*)args;
    if (*(thread_number) >=THREADS_NUM/2 && *(thread_number) < THREADS_NUM){
        change_arr(thread_number);
    }
    else{
        read_arr(thread_number);
        printf("%d in thread\n", *thread_number);
    }
    //Next line is unused
    pthread_exit(thread_number);
}

void * read_arr(int * thread_ans){
    *thread_ans = 2;
    return NULL;
}

void * change_arr(int* nop){
    *(nop) = 1;
    pthread_exit((void*)nop);
}

void clean_mem(void * mem){
    free(mem);
}