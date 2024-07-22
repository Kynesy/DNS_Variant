/**
 * @file printMatrix.c
 * @brief This file contains the main function to print a matrix from a file.
 */

#include <stdio.h>
#include <stdlib.h>
#include "inOutUtils.h"

/**
 * @brief The main function to print a matrix from a file.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 *             The first argument should be the size of the matrix, and the second argument should be the filename.
 * 
 * @return 0 if the program executed successfully, 1 otherwise.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <size> <filename>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    char *filename = argv[2];

    int* matrix = malloc(size * size * sizeof(int));
    if(readMatrixFromFile(matrix, size, filename)) {
        printf("Error reading matrix from file\n");
        return 1;
    }
    printf("Filename %s - Size %d*%d:\n", filename, size, size);
    printMatrix(matrix, size);

    return 0;
}