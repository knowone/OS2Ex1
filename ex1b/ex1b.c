#include <pthread.h>
#include <stdio.h>
#define N 10
#define THREADS_NUM 6

typedef struct{
    int _nums[N];
    int _counter;
}Data;

Data data;

pthread_mutex_t lock;

int main() {
    pthread_t thread_id[THREADS_NUM];               //required for thread creating
    int thread_arr[THREADS_NUM];                    //needed for thread job assignment
//    thread_data_t * thread_answer[THREADS_NUM];     //Holds counters ret from thread

    pthread_mutex_init(&lock, NULL);

}