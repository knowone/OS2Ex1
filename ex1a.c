#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define SIZE_ARR 100
#define THREADS_NUM 6

void * doWork(void*);
void * change_arr(void*);
void * read_arr(void*);

typedef struct{
    int thread_num;
    int thread_answer;
}thread_data_t;

int main() {
    srand((unsigned int)time(NULL));
    pthread_t thread_id[THREADS_NUM];
    thread_data_t thread_data[THREADS_NUM];

    for (int i = 0; i < THREADS_NUM; ++i) {
        thread_data[i].thread_num = i;
        int status = pthread_create(&thread_id[i],NULL,(void*)doWork,&thread_data[i]);
        if (status != 0){
            fputs("Can't create pthread in main()", stderr);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < THREADS_NUM; ++i) {
        pthread_join(thread_id[i],(void**) &(thread_data[i].thread_answer)+sizeof(int*));
        //Here be thread answer int
        printf("thread %d says: %d\n", i+1,thread_data[i].thread_answer);
    }
    pthread_exit(NULL);
}

void * doWork(void* args){

    thread_data_t * td = (thread_data_t*)args;
    if (td->thread_num >=THREADS_NUM/2 && td->thread_num < THREADS_NUM){
        change_arr(td);
    }
    else{
        read_arr(td);
        printf("in thread %d\n", td->thread_num);
    }
    //Next line is unused
    pthread_exit(td);
//    pthread_cleanup_pop(0);

}

void * read_arr(void * nop){

   sleep(((unsigned int)rand()%3+1));
    thread_data_t * tmp = (thread_data_t*)nop;
    tmp->thread_answer = 1;
    pthread_exit((void*)tmp);
}

void * change_arr(void * nop){
    thread_data_t * tmp = (thread_data_t*)nop;
    tmp->thread_answer = 2;
    pthread_exit((void*)tmp);
}
