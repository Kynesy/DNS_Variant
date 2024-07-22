/**
 * @file inOutUtils.h
 * @brief Header file containing input/output utility functions.
 */

#ifndef INOUTUTILS_H
#define INOUTUTILS_H

/**
 * @def SRAND_SEED
 * @brief Seed value for srand function.
 */
#define SRAND_SEED 12345678

/**
 * @brief Generates two matrices of size n and fills them with random values.
 * 
 * @param matrixA Pointer to the first matrix.
 * @param matrixB Pointer to the second matrix.
 * @param n Size of the matrices.
 */
void generateMatrix(int* matrixA, int* matrixB, int n);

/**
 * @brief Prints the values of a matrix.
 * 
 * @param matrix Pointer to the matrix.
 * @param n Size of the matrix.
 */
void printMatrix(int* matrix, int n);

/**
 * @brief Reads a matrix from a file.
 * 
 * @param matrix Pointer to the matrix.
 * @param n Size of the matrix.
 * @param filename Name of the file to read from.
 * @return 0 if the matrix was successfully read, -1 otherwise.
 */
int readMatrixFromFile(int* matrix, int n, char* filename);

/**
 * @brief Writes a matrix to a file.
 * 
 * @param matrix Pointer to the matrix.
 * @param n Size of the matrix.
 * @param filename Name of the file to write to.
 * @return 0 if the matrix was successfully written, -1 otherwise.
 */
int writeMatrixToFile(int* matrix, int n, char* filename);

#endif // INOUTUTILS_H