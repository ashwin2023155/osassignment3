#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define MAX_TASKS 100

typedef struct {
    int row, col, n;
    int **A, **B, **C;
} Task;

typedef struct {
    Task tasks[MAX_TASKS];
    int front, rear, count;
    pthread_mutex_t mutex;
    pthread_cond_t cond_var;
    int shutdown;
} TaskQueue;

TaskQueue* create_task_queue() {
    TaskQueue* queue = malloc(sizeof(TaskQueue));
    queue->front = queue->rear = queue->count = 0;
    queue->shutdown = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond_var, NULL);
    return queue;
}

void enqueue_task(TaskQueue* queue, Task task) {
    pthread_mutex_lock(&queue->mutex);
    queue->tasks[queue->rear] = task;
    queue->rear = (queue->rear + 1) % MAX_TASKS;
    queue->count++;
    pthread_cond_signal(&queue->cond_var);
    pthread_mutex_unlock(&queue->mutex);
}

Task dequeue_task(TaskQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->count == 0 && !queue->shutdown) {
        pthread_cond_wait(&queue->cond_var, &queue->mutex);
    }
    Task task = {0};
    if (queue->count > 0) {
        task = queue->tasks[queue->front];
        queue->front = (queue->front + 1) % MAX_TASKS;
        queue->count--;
    }
    pthread_mutex_unlock(&queue->mutex);
    return task;
}

void* worker(void* arg) {
    TaskQueue* queue = (TaskQueue*)arg;
    while (1) {
        Task task = dequeue_task(queue);
        if (queue->shutdown) break;

        int sum = 0;
        for (int k = 0; k < task.n; k++) {
            sum += task.A[task.row][k] * task.B[k][task.col];
        }
        task.C[task.row][task.col] = sum;
    }
    return NULL;
}

int** allocate_matrix(int rows, int cols) {
    int** matrix = malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = malloc(cols * sizeof(int));
    }
    return matrix;
}

void free_matrix(int** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void print_matrix(int** matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void sequential_multiply(int **A, int **B, int **C, int m, int n, int p) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    int m, n, p, num_threads;
    printf("Enter dimensions of Matrix A (m x n): ");
    scanf("%d %d", &m, &n);
    printf("Enter dimensions of Matrix B (n x p): ");
    scanf("%d %d", &n, &p);

    int** A = allocate_matrix(m, n);
    int** B = allocate_matrix(n, p);
    int** C_parallel = allocate_matrix(m, p);
    int** C_sequential = allocate_matrix(m, p);

    printf("Enter elements of Matrix A:\n");
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            scanf("%d", &A[i][j]);

    printf("Enter elements of Matrix B:\n");
    for (int i = 0; i < n; i++)
        for (int j = 0; j < p; j++)
            scanf("%d", &B[i][j]);

    // Sequential version timing
    clock_t seq_start = clock();
    sequential_multiply(A, B, C_sequential, m, n, p);
    clock_t seq_end = clock();
    double sequential_time = ((double)(seq_end - seq_start)) / CLOCKS_PER_SEC;

    // Parallel version timing
    num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t threads[num_threads];
    TaskQueue* queue = create_task_queue();

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, queue);
    }

    clock_t par_start = clock();
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            Task task = {i, j, n, A, B, C_parallel};
            enqueue_task(queue, task);
        }
    }

    sleep(1); // Allow threads to process tasks
    queue->shutdown = 1;
    pthread_cond_broadcast(&queue->cond_var);

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_t par_end = clock();
    double parallel_time = ((double)(par_end - par_start)) / CLOCKS_PER_SEC;

    // Results
    printf("\nSequential Resultant Matrix C:\n");
    print_matrix(C_sequential, m, p);

    printf("\nParallel Resultant Matrix C:\n");
    print_matrix(C_parallel, m, p);

    double speedup = sequential_time / parallel_time;
    printf("\nTime taken by Sequential Version: %.6f seconds\n", sequential_time);
    printf("Time taken by Parallel Version: %.6f seconds\n", parallel_time);
    printf("Speed-up: %.2f\n", speedup);

    // Cleanup
    free_matrix(A, m);
    free_matrix(B, n);
    free_matrix(C_parallel, m);
    free_matrix(C_sequential, m);
    free(queue);

    return 0;
}
