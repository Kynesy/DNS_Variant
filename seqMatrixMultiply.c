/**
 * @file seqMatrixMultiply.c
 * @brief Sequential matrix multiplication program
 *
 * This program performs matrix multiplication sequentially using the provided matrices.
 * It reads two matrices from binary files, multiplies them, and stores the result in a third matrix.
 * The program measures the execution time and outputs it along with the input time.
 *
 * The input matrices should be in the same directory and named matrixA.bin and matrixB.bin
 * The result matrix is written to a binary file named matrixC_sequential.bin
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "inOutUtils.h"


#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif
/**
 * @brief Performs sequential matrix multiplication
 *
 * This function multiplies two matrices sequentially and stores the result in a third matrix.
 *
 * @param matrixA Pointer to the first matrix
 * @param matrixB Pointer to the second matrix
 * @param matrixC Pointer to the result matrix
 * @param n The size of the matrices
 */
void sequentialMatrixMultiply(int* matrixA, int* matrixB, int* matrixC, int n);

/**
 * @brief Main function
 *
 * The main function reads the size of the matrices from the command line arguments.
 * It allocates memory for the matrices, reads the matrices from binary files,
 * performs sequential matrix multiplication, measures the execution time,
 * and outputs the input time and total time.
 *
 * @param argc The number of command line arguments
 * @param argv An array of strings containing the command line arguments
 * @return 0 if the program executed successfully, otherwise a non-zero value
 */
int main(int argc, char* argv[]){  
    clock_t total_time;
    clock_t input_time;

    // Check if the correct number of command line arguments is provided
    if(argc != 2){
        fprintf(stdout, "Usage: ./seqMatrixMultiply [size]\n");
        return 1;
    }

    int n = strtol(argv[1], NULL, 10);

    /* Start the timer */
    total_time = -clock();

    // Allocate memory for the matrices
    int *matrixA = malloc(n*n*sizeof(int));
    int *matrixB = malloc(n*n*sizeof(int));
    int *matrixC = malloc(n*n*sizeof(int));

    // Check if memory allocation was successful
    if(matrixA == NULL || matrixB == NULL || matrixC == NULL){
        fprintf(stdout, "Error allocating memory\n");
        return 2;
    }

    // Read the matrices from binary files
    if(readMatrixFromFile(matrixA, n, "matrixA.bin") || readMatrixFromFile(matrixB, n, "matrixB.bin")){
        fprintf(stdout, "Error reading matrixA or matrixB\n");
        return 3;
    }

    input_time = total_time + clock();

    // Perform sequential matrix multiplication
    sequentialMatrixMultiply(matrixA, matrixB, matrixC, n);

    total_time += clock();

    double t_input = (double) input_time/CLOCKS_PER_SEC;
    double t_total = (double) total_time/CLOCKS_PER_SEC;
    // Output the input time and total time
    printf("%10.6f\t%10.6f\n", t_input, t_total-t_input);
    
    // Write the result matrix to a binary file
    if(writeMatrixToFile(matrixC, n, "matrixC_sequential.bin")){
        fprintf(stdout, "Error writing matrixC\n");
        return 4;
    }
    
    // Free the allocated memory
    free(matrixA);
    free(matrixB);
    free(matrixC);

    return 0;
}

/**
 * @brief Performs sequential matrix multiplication
 *
 * This function multiplies two matrices sequentially and stores the result in a third matrix.
 * It uses three nested loops to iterate over the elements of the matrices and perform the multiplication.
 *
 * @param matrixA Pointer to the first matrix
 * @param matrixB Pointer to the second matrix
 * @param matrixC Pointer to the result matrix
 * @param n The size of the matrices
 */
void sequentialMatrixMultiply(int* matrixA, int* matrixB, int* matrixC, int n){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            matrixC[i*n+j] = 0;
            for(int k = 0; k < n; k++){
                matrixC[i*n+j] += matrixA[i*n+k] * matrixB[k*n+j];
            }
        }
    }
}
