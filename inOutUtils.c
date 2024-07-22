/**
 * @file inOutUtils.c
 * @brief This file contains functions for input/output operations on matrices.
 */

#include "inOutUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Generates random values for two matrices.
 * 
 * This function generates random values between 0 and 9 for two matrices of size n x n.
 * The generated values are stored in the provided arrays matrixA and matrixB.
 * 
 * @param matrixA Pointer to the first matrix.
 * @param matrixB Pointer to the second matrix.
 * @param n The size of the matrices.
 */
void generateMatrix(int* matrixA, int* matrixB, int n){
    srand(SRAND_SEED);
    for(int i=0; i<n*n; i++){
        matrixA[i] = rand() % 10;
        matrixB[i] = rand() % 10;
    }
}

/**
 * @brief Prints the values of a matrix.
 * 
 * This function prints the values of a matrix of size n x n.
 * The matrix is printed in row-major order.
 * 
 * @param matrix Pointer to the matrix.
 * @param n The size of the matrix.
 */
void printMatrix(int* matrix, int n){
    printf("Matrix n: %d*%d\n", n, n);
    for(int i = 0; i<n*n; i++){
        if(i % n == 0) printf("\n");
        printf("\t%d", matrix[i]);
    }
    printf("\n");
}

/**
 * @brief Reads a matrix from a file.
 * 
 * This function reads a matrix of size n x n from a binary file.
 * The matrix is stored in the provided array matrix.
 * 
 * @param matrix Pointer to the matrix.
 * @param n The size of the matrix.
 * @param filename The name of the file to read from.
 * @return 0 if the file was successfully read, 1 otherwise.
 */
int readMatrixFromFile(int* matrix, int n, char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return 1;
    }

    fread(matrix, sizeof(int), n*n, file);

    fclose(file);
    return 0;
}

/**
 * @brief Writes a matrix to a file.
 * 
 * This function writes a matrix of size n x n to a binary file.
 * The matrix is stored in the provided array matrix.
 * 
 * @param matrix Pointer to the matrix.
 * @param n The size of the matrix.
 * @param filename The name of the file to write to.
 * @return 0 if the file was successfully written, 1 otherwise.
 */
int writeMatrixToFile(int* matrix, int n, char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        return 1;
    }

    fwrite(matrix, sizeof(int), n*n, file);
    fclose(file);
    return 0;
}