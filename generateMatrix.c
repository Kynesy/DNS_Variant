/**
 * @file generateMatrix.c
 * @brief This file contains the main function to generate and save matrices.
 */

#include <stdlib.h>
#include <stdio.h>
#include "inOutUtils.h"

/**
 * @brief The main function to generate and save matrices.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return 0 if the program executed successfully, otherwise a non-zero value.
 */
int main(int argc, char* argv[]){    
    if(argc != 2){
        fprintf(stdout, "Usage: ./generateMatrix [size]\n");
        return 1;
    }
    int n = strtol(argv[1], NULL, 10);

    // Matrices allocation
    int* matrixA = malloc(n*n*sizeof(int));
    int* matrixB = malloc(n*n*sizeof(int));
    if(matrixA == NULL || matrixB == NULL){
        fprintf(stdout, "Error allocating memory\n");
        return 2;
    }

    // Generate matrices
    generateMatrix(matrixA, matrixB, n);
    
    // Write matrices to files
    if(writeMatrixToFile(matrixA, n, "matrixA.bin")){
        fprintf(stdout, "Error writing matrixA\n");
        return 3;
    }

    if(writeMatrixToFile(matrixB, n, "matrixB.bin")){
        fprintf(stdout, "Error writing matrixB\n");
        return 3;
    }

    printf("Matrices generated and saved in matrixA.bin and matrixB.bin\n");

    return 0;
}