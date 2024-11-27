#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
pthread_mutex_t lockA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockB = PTHREAD_MUTEX_INITIALIZER;
int thread_iterations[3] = {0, 0, 0};
struct thread_data {
    int thread_id;
};
void acquire_locks(int thread_id) {
    printf("T%d waiting for Lock A\n", thread_id + 1);
    pthread_mutex_lock(&lockA);
    printf("T%d acquired Lock A\n", thread_id + 1);
        
    usleep(100000);
        
    printf("T%d waiting for Lock B\n", thread_id + 1);
    pthread_mutex_lock(&lockB);
    printf("T%d acquired Lock B\n", thread_id + 1);
}
void release_locks(int thread_id) {
    pthread_mutex_unlock(&lockB);
    printf("T%d released Lock B\n", thread_id + 1);
    
    pthread_mutex_unlock(&lockA);
    printf("T%d released Lock A\n", thread_id + 1);
}
void* thread_function(void* arg) {
    struct thread_data* my_data = (struct thread_data*)arg;
    int thread_id = my_data->thread_id;
    
    while (thread_iterations[thread_id] < 3) {
        usleep(rand() % 500000);
        acquire_locks(thread_id);
        
        printf("T%d in critical section (iteration %d)\n", 
               thread_id + 1, thread_iterations[thread_id] + 1);
        usleep(100000);       
        
        release_locks(thread_id);       
        
        thread_iterations[thread_id]++;        
        
        usleep(200000);
    }
    
    printf("T%d completed all iterations\n", thread_id + 1);
    return NULL;
}
int main() {
    pthread_t threads[3];
    struct thread_data thread_data_array[3];    
    srand(time(NULL));    
    for (int i = 0; i < 3; i++) {
        thread_data_array[i].thread_id = i;
        if (pthread_create(&threads[i], NULL, thread_function, 
                          (void*)&thread_data_array[i]) != 0) {
            perror("Failed to create thread");
            exit(1);
        }
    }    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }    
    pthread_mutex_destroy(&lockA);
    pthread_mutex_destroy(&lockB);    
    printf("All threads completed successfully\n");
    return 0;
}