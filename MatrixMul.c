#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>
#define MAX 20
#define FILENAME_MAX 256
int matrixA[MAX][MAX], matrixB[MAX][MAX], matrixC[MAX][MAX];
int rowA, colA, rowB, colB, rowC, colC;
char outputFileName[FILENAME_MAX];
struct pair
{
    int i;
    int j;
};


void readMatrixFromFile(char *fileName, int matrixNumber) {
    FILE *file = fopen(fileName, "r");
    if (file) {
        int row, col;
        fscanf(file, "row=%d col=%d", &row, &col);
        if (matrixNumber == 1) {
            rowA = row;
            colA = col;
        } else {
            rowB = row;
            colB = col;
        }
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                if (matrixNumber == 1) {
                    fscanf(file, "%d", &matrixA[i][j]);
                } else {
                    fscanf(file, "%d", &matrixB[i][j]);
                }
            }
        }
        fclose(file);
    }
    else {
        if (matrixNumber == 1) {
            rowA = -1;
            colA = -1;
        }
        else {
            rowB = -1;
            colB = -1;
        }
    }
}

bool validateInput() {
    if (colA != rowB) {
        printf("Matrix multiplication is not possible\n");
        return false;
    }
    if (rowA > MAX || colA > MAX || rowB > MAX || colB > MAX) {
        printf("Matrix size is too large\n");
        return false;
    }
    if (rowA <= 0 || colA <= 0 || rowB <= 0 || colB <= 0) {
        printf("Matrix is empty\n");
        return false;
    }
    if (rowA == -1) {
        printf("Matrix A is not found\n");
        return false;
    }
    if (rowB == -1) {
        printf("Matrix B is not found\n");
        return false;
    }
    return true;
}

void writeMatrixToFile(char *method, char *methodName, unsigned long time) {
    char fileName[FILENAME_MAX];
    sprintf(fileName, "%s_%s.txt", outputFileName, method);
    FILE *file = fopen(fileName, "w");
    if (file) {
        fprintf(file, "Time taken by %s: %lu microseconds\n", methodName, time);
        int threadsNumber = 1;
        if (strcmp(method, "per_row") == 0) {
            threadsNumber = rowC;
        } else if (strcmp(method, "per_element") == 0) {
            threadsNumber = rowC * colC;
        }
        fprintf(file, "Number of threads: %d\n", threadsNumber);
        fprintf(file, "row=%d col=%d\n", rowC, colC);
        for (int i = 0; i < rowC; i++) {
            for (int j = 0; j < colC; j++) {
                fprintf(file, "%d ", matrixC[i][j]);
            }
            fprintf(file, "\n");
        }
        fclose(file);
    }
}

void multiplyMatrix() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < rowA; i++) {
        for (int j = 0; j < colB; j++) {
            matrixC[i][j] = 0;
            for (int k = 0; k < colA; k++) 
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
        }
    }
    gettimeofday(&end, NULL);
    unsigned long timeTaken = end.tv_usec - start.tv_usec;
    writeMatrixToFile("per_matrix", "A thread per matrix", timeTaken);
}

void *multiplyRow(void *row) {
    int i = *(int *) row;
    for (int j = 0; j < colB; j++) {
        matrixC[i][j] = 0;
        for (int k = 0; k < colA; k++) {
            matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
        }
    }
    free(row);
    return NULL;
}

void *multiplyElement(void *pair) {
    struct pair *p = (struct pair *) pair;
    int i = p->i;
    int j = p->j;
    matrixC[i][j] = 0;
    for (int k = 0; k < colA; k++) {
        matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
    }
    free(pair);
    return NULL;
}

int main(int argc, char *argv[]) {
    // Default filenames if not provided as arguments
    char inputFileA[FILENAME_MAX] = "a.txt";
    char inputFileB[FILENAME_MAX] = "b.txt";
    char outputFilePrefixDefault[FILENAME_MAX] = "c";

    // Process command-line arguments
    if (argc > 1) {
        // printf("Input file A: %s\n", argv[1]);
        strcpy(inputFileA, argv[1]);
        strcat(inputFileA, ".txt");
    }
    if (argc > 2) {
        // printf("Input file B: %s\n", argv[2]);
        strcpy(inputFileB, argv[2]);
        strcat(inputFileB, ".txt");
    }
    if (argc > 3) {
        // printf("Output file prefix: %s\n", argv[3]);
        strcpy(outputFileName, argv[3]);
    } else {
        strcpy(outputFileName, outputFilePrefixDefault);
    }

    // Read matrices from files
    readMatrixFromFile(inputFileA, 1);
    readMatrixFromFile(inputFileB, 2);

    // Validate matrix dimensions
    if (!validateInput()) {
        return 0;
    }
    // printf("Matrix multiplication is possible\n");
    rowC = rowA;
    colC = colB;

    multiplyMatrix();

    struct timeval start, end;
    pthread_t ROW_THREADS[rowC];
    gettimeofday(&start, NULL);
    for (int i = 0; i < rowC; i++) {
        int *row = malloc(sizeof(int));
        *row = i;
        pthread_create(&ROW_THREADS[i], NULL, multiplyRow, row);
    }
    for (int i = 0; i < rowC; i++) {
        pthread_join(ROW_THREADS[i], NULL);
    }
    gettimeofday(&end, NULL);
    unsigned long timeTaken = end.tv_usec - start.tv_usec;
    writeMatrixToFile("per_row", "A thread per row", timeTaken);

    pthread_t ELEMENT_THREADS[rowC][colC];
    gettimeofday(&start, NULL);
    for (int i = 0; i < rowA; i++) {
        for (int j = 0; j < colB; j++) {
            struct pair *p = malloc(sizeof(struct pair));
            p->i = i;
            p->j = j;
            pthread_create(&ELEMENT_THREADS[i][j], NULL, multiplyElement, p);
        }
    }
    for (int i = 0; i < rowA; i++) {
        for (int j = 0; j < colB; j++) {
            pthread_join(ELEMENT_THREADS[i][j], NULL);
        }
    }
    gettimeofday(&end, NULL);
    timeTaken = end.tv_usec - start.tv_usec;
    writeMatrixToFile("per_element", "A thread per element", timeTaken);
    printf("Matrix multiplication is done\n");
    return 0;
}