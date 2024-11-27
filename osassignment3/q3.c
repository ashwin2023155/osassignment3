#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 10        
#define MAX_DELIVERY 5        
#define MAX_STORAGE 3         
#define TOTAL_OPERATIONS 50   


typedef struct {
    int buffer[BUFFER_SIZE];  
    int head;                 
    int tail;                 
    int count;                
} CircularBuffer;


CircularBuffer warehouse;     
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  
sem_t empty_slots;            
sem_t filled_slots;           
int total_deliveries = 0;    
int total_stored = 0;        


void initializeBuffer(CircularBuffer* buffer);
int addToBuffer(CircularBuffer* buffer, int item);
int removeFromBuffer(CircularBuffer* buffer, int requested_amount);
void* deliveryTruck(void* arg);
void* storageManager(void* arg);


void initializeBuffer(CircularBuffer* buffer) {
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer->buffer[i] = 0;
    }
}

int addToBuffer(CircularBuffer* buffer, int item) {
    if (buffer->count + item > BUFFER_SIZE) {
        return 0; 
    }

    for (int i = 0; i < item; i++) {
        buffer->buffer[buffer->tail] = 1;
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
        buffer->count++;
    }
    return item;
}


int removeFromBuffer(CircularBuffer* buffer, int requested_amount) {
    if (buffer->count < requested_amount) {
        return 0;  
    }

    for (int i = 0; i < requested_amount; i++) {
        buffer->buffer[buffer->head] = 0;
        buffer->head = (buffer->head + 1) % BUFFER_SIZE;
        buffer->count--;
    }
    return requested_amount;
}

void* deliveryTruck(void* arg) {
    int truck_id = *(int*)arg;

    while (total_deliveries < TOTAL_OPERATIONS) {
        int delivery_amount = rand() % MAX_DELIVERY + 1;

        for (int i = 0; i < delivery_amount; i++) {
            sem_wait(&empty_slots);
        }

        pthread_mutex_lock(&mutex);

        if (total_deliveries >= TOTAL_OPERATIONS) {
            pthread_mutex_unlock(&mutex);
            for (int i = 0; i < delivery_amount; i++) {
                sem_post(&empty_slots);
            }
            break;
        }

     
        int added = addToBuffer(&warehouse, delivery_amount);
        if (added > 0) {
            total_deliveries++;
            printf("Truck %d delivered %d products. Buffer now has %d products.\n",
                   truck_id, added, warehouse.count);
        }

        pthread_mutex_unlock(&mutex);

      
        for (int i = 0; i < added; i++) {
            sem_post(&filled_slots);
        }

        usleep((rand() % 500000) + 100000);  
    }

    return NULL;
}


void* storageManager(void* arg) {
    int manager_id = *(int*)arg;

    while (total_stored < TOTAL_OPERATIONS) {
        int storage_amount = rand() % MAX_STORAGE + 1;

       
        for (int i = 0; i < storage_amount; i++) {
            sem_wait(&filled_slots);
        }

        pthread_mutex_lock(&mutex);

        if (total_stored >= TOTAL_OPERATIONS) {
            pthread_mutex_unlock(&mutex);
            for (int i = 0; i < storage_amount; i++) {
                sem_post(&filled_slots);
            }
            break;
        }

       
        int removed = removeFromBuffer(&warehouse, storage_amount);
        if (removed > 0) {
            total_stored++;
            printf("Manager %d stored %d products. Buffer now has %d products.\n",
                   manager_id, removed, warehouse.count);
        }

        pthread_mutex_unlock(&mutex);

        
        for (int i = 0; i < removed; i++) {
            sem_post(&empty_slots);
        }

        usleep((rand() % 500000) + 100000);  
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_trucks> <num_storage_managers>\n", argv[0]);
        return 1;
    }

    int num_trucks = atoi(argv[1]);
    int num_storage_managers = atoi(argv[2]);

    srand(time(NULL));

    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&filled_slots, 0, 0);

    initializeBuffer(&warehouse);

    pthread_t truck_threads[num_trucks];
    pthread_t storage_manager_threads[num_storage_managers];
    int truck_ids[num_trucks];
    int storage_manager_ids[num_storage_managers];

   
    for (int i = 0; i < num_trucks; i++) {
        truck_ids[i] = i + 1;
        pthread_create(&truck_threads[i], NULL, deliveryTruck, &truck_ids[i]);
    }

    
    for (int i = 0; i < num_storage_managers; i++) {
        storage_manager_ids[i] = i + 1;
        pthread_create(&storage_manager_threads[i], NULL, storageManager, &storage_manager_ids[i]);
    }

   
    for (int i = 0; i < num_trucks; i++) {
        pthread_join(truck_threads[i], NULL);
    }

    for (int i = 0; i < num_storage_managers; i++) {
        pthread_join(storage_manager_threads[i], NULL);
    }

    printf("\nFinal Warehouse Status:\n");
    printf("Total Deliveries: %d\n", total_deliveries);
    printf("Total Stored Products: %d\n", total_stored);
    printf("Remaining Products in Buffer: %d\n", warehouse.count);

    sem_destroy(&empty_slots);
    sem_destroy(&filled_slots);
    pthread_mutex_destroy(&mutex);

    return 0;
}
