#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define NUM_SERVERS 5
#define PROCESS_COUNT 3

sem_t channels[NUM_SERVERS];
int process_counter[NUM_SERVERS] = {0};

typedef struct {
    int id;
} server_args;

int get_left_channel(int server_id) {
    return server_id;
}

int get_right_channel(int server_id) {
    return (server_id + 1) % NUM_SERVERS;
}

void* server_process(void* arg) {
    server_args* args = (server_args*)arg;
    int server_id = args->id;
    int left_channel = get_left_channel(server_id);
    int right_channel = get_right_channel(server_id);

    while (process_counter[server_id] < PROCESS_COUNT) {
        printf("Server %d is waiting\n", server_id + 1);

        if (left_channel < right_channel) {
            sem_wait(&channels[left_channel]);
            sem_wait(&channels[right_channel]);
        } else {
            sem_wait(&channels[right_channel]);
            sem_wait(&channels[left_channel]);
        }
        printf("Server %d is processing\n", server_id + 1);
        sleep(1); 
        process_counter[server_id]++;
        sem_post(&channels[left_channel]);
        sem_post(&channels[right_channel]);
        usleep(100000); 
    }

    free(args);
    return NULL;
}

int main() {
    pthread_t servers[NUM_SERVERS];
    for (int i = 0; i < NUM_SERVERS; i++) {
        sem_init(&channels[i], 0, 1);
    }
    for (int i = 0; i < NUM_SERVERS; i++) {
        server_args* args = malloc(sizeof(server_args));
        args->id = i;
        pthread_create(&servers[i], NULL, server_process, (void*)args);
    }
    for (int i = 0; i < NUM_SERVERS; i++) {
        pthread_join(servers[i], NULL);
    }
    for (int i = 0; i < NUM_SERVERS; i++) {
        sem_destroy(&channels[i]);
    }
    return 0;
}
