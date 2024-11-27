#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int row, col, n;
    int **A, **B, **C;
} ThreadData;

void* multiply_element(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int sum = 0;
    for (int k = 0; k < data->n; k++) {
        sum += data->A[data->row][k] * data->B[k][data->col];
    }
    data->C[data->row][data->col] = sum;
    pthread_exit(0);
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
    int m, n, p;
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

 
    clock_t seq_start = clock();
    sequential_multiply(A, B, C_sequential, m, n, p);
    clock_t seq_end = clock();
    double sequential_time = ((double)(seq_end - seq_start)) / CLOCKS_PER_SEC;

   
    pthread_t threads[m * p];
    ThreadData thread_data[m * p];

    clock_t par_start = clock();
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            int index = i * p + j;
            thread_data[index].row = i;
            thread_data[index].col = j;
            thread_data[index].n = n;
            thread_data[index].A = A;
            thread_data[index].B = B;
            thread_data[index].C = C_parallel;
            if (pthread_create(&threads[index], NULL, multiply_element, &thread_data[index])) {
                fprintf(stderr, "Error creating thread for element (%d, %d)\n", i, j);
                exit(1);
            }
        }
    }

    for (int i = 0; i < m * p; i++) {
        if (pthread_join(threads[i], NULL)) {
            fprintf(stderr, "Error joining thread for element %d\n", i);
            exit(1);
        }
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

    return 0;
}
