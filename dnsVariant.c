/**
 * @file dnsVariant.c
 * @brief This file contains the implementation of a DNS variant program.
 *
 * The program uses MPI (Message Passing Interface) to perform parallel computations on a 3D grid.
 * It creates different communicators to handle communication between processes in different dimensions.
 * The program also defines a struct to store information about adjacent cells in the grid.
 *
 * @author Cezar Narcis Culcea
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "inOutUtils.h"

#define X 2 /**< The X dimension index */
#define Y 1 /**< The Y dimension index */
#define Z 0 /**< The Z dimension index */

/**
 * @struct Communicators
 * @brief Struct to store different MPI communicators.
 *
 * This struct stores different MPI communicators used for communication between processes in different dimensions.
 */
struct Communicators{
    MPI_Comm commCart; /**< Communicator for X, Y, and Z dimensions */
    MPI_Comm commXYplanes; /**< Communicator for horizontal planes */
    MPI_Comm commYZplanes; /**< Communicator for vertical lateral planes */
    MPI_Comm commZXplanes; /**< Communicator for front vertical planes */
    MPI_Comm commXsingleDim; /**< Communicator for processes along X with fixed YZ */
    MPI_Comm commYsingleDim; /**< Communicator for processes along Y with fixed XZ */
    MPI_Comm commZsingleDim; /**< Communicator for processes along Z with fixed XY */
    MPI_Comm commSubMatrixX; /**< Communicator for processes along X with equal rank modulo n/m */
    MPI_Comm commSubMatrixY; /**< Communicator for processes along Y with equal rank modulo n/m */
};

/**
 * @struct AdjacentCells
 * @brief Struct to store information about adjacent cells.
 *
 * This struct stores the rank of the adjacent cells in the Cartesian communicator.
 */
typedef struct {
    int up; /**< Rank of the cell above (with wrap around) in the submatrices */
    int down; /**< Rank of the cell below (with wrap around) in the submatrices */
    int left; /**< Rank of the cell to the left (with wrap around) in the submatrices */
    int right; /**< Rank of the cell to the right (with wrap around) in the submatrices */
} AdjacentCells;

/**
 * @brief Create different communicators for the program.
 *
 * This function creates different MPI communicators based on the given dimensions, periods, and ranks.
 * It also takes a discriminanteColore parameter to determine the color of the processes in the communicators.
 *
 * @param comms Pointer to the struct to store the created communicators.
 * @param dims Array of dimensions for the Cartesian grid.
 * @param periods Array of periods for the Cartesian grid.
 * @param cartRank Pointer to the rank of the current process in the Cartesian communicator.
 * @param coords Pointer to the coordinates of the current process in the Cartesian grid.
 * @param discriminanteColore The color to determine the processes in the communicators.
 */
void createCommunicators(struct Communicators* comms, int* dims, int* periods, int* cartRank, int* coords, int discriminanteColore);

/**
 * @brief Finds the indices of the adjacent cells in a grid with wrap-around.
 * 
 * This function calculates the indices of the adjacent cells in a grid with wrap-around.
 * The grid has dimensions (n/m)^2, and the distance between adjacent cells in the x and y directions is specified.
 * The indices of the adjacent cells are stored in the AdjacentCells struct.
 * The indices used to get cells position in matrix use a cartesian style indexing, not the usual row, col indexing.
 * 
 * @param index The index of the current cell.
 * @param n The number of cells in the x direction.
 * @param m The number of cells in the y direction.
 * @param distX The distance between adjacent cells in the x direction.
 * @param distY The distance between adjacent cells in the y direction.
 * @param adj Pointer to the struct storing the indices of the adjacent cells.
 */
void findAdjacentCells(int index, int n, int m, int distX, int distY, AdjacentCells *adj);

/**
 * @brief The main function of the program.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return 0 on success.
 */
int main(int argc, char* argv[]){
    double total_time; /**< Timer total time */
    double input_time; /**< Timer input time */
    int myRank; /**< The rank of the current process */
    int cartRank; /**< The rank of the current process in the Cartesian communicator */
    int cartCoords[3]; /**< The Cartesian coordinates of the current process */
    int p; /**< The total number of processes */
    int n; /**< The dimension of the matrix */
    int m; /**< The depth of procs cube */

    int* matrixA, *matrixB, *matrixC = NULL; /**< Pointers to the matrices */
    int localA, localB, localC = 0; /**< Local variables for each process */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    /************************** INPUT ************************************/
    if(argc != 2 && !myRank){
        printf("Abort... usage ./betterDns [n]\n\n");
        fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    MPI_Barrier(MPI_COMM_WORLD); //Wait for proc 0 to check input parameters

    n = strtol(argv[1],NULL, 10);
    m = p/(n*n);

    if((n%m) && !myRank){
        printf("Abort... n non è divisibile per m\n\n");
        fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    /* Start the timer */
    MPI_Barrier(MPI_COMM_WORLD);
    total_time = -MPI_Wtime();
    /****************************** INPUT ************************************/ 

    if(!myRank){
        matrixA = malloc(n*n*sizeof(int));
        matrixB = malloc(n*n*sizeof(int));

        if(matrixA==NULL || matrixB==NULL){ //Check succesfull memory allocation
            printf("Abort... error allocating memory.\n");
            MPI_Abort(MPI_COMM_WORLD, 2);
        }                

        //Check successful reading
        if(readMatrixFromFile(matrixA, n, "matrixA.bin") || readMatrixFromFile(matrixB, n, "matrixB.bin")){
            printf("Error reading matrixA or matrixB\n");
            fflush(stdout);
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
    }

    /****************************** COMUNICATORS ************************************/
    struct Communicators* comms;
    comms = malloc(sizeof(struct Communicators));

    int num_dimensions = 3;
    int dims[3] = {0, 0, 0};
    dims[X] = n;
    dims[Y] = n;
    dims[Z] = m;
    int periods[3] = {0, 0, 0};
    
    createCommunicators(comms, dims, periods, &cartRank, cartCoords, n/m);
    
    /****************************** SCATTER ************************************/
    //Proc 0 distribute the matrices along n^2 procs in layer 0
    if(cartCoords[Z] == 0){
        MPI_Scatter(matrixA, 1, MPI_INT, &localA, 1, MPI_INT, 0, comms->commXYplanes); 
        MPI_Scatter(matrixB, 1, MPI_INT, &localB, 1, MPI_INT, 0, comms->commXYplanes);
    }

    if(!myRank){
        free(matrixA);
        free(matrixB);
    }

    /* Start take input timer here, since the algorithm is supposed to start from this configuration */
    MPI_Barrier(MPI_COMM_WORLD);
    input_time = total_time + MPI_Wtime();

    /****************************** BCAST A Columns ************************************/
    MPI_Bcast(&localA, 1, MPI_INT, 0, comms->commZsingleDim);

    /****************************** BCAST B Rows ************************************/
    MPI_Bcast(&localB, 1, MPI_INT, 0, comms->commZsingleDim);

    /****************************** BCAST A values over their rows in each layer, if layer = col ************************************/
    MPI_Bcast(&localA, 1, MPI_INT, cartCoords[Z], comms->commSubMatrixX);

    /******************************* BCAST B  values over their cols in each layer, if layer = row ************************************/
    MPI_Bcast(&localB, 1, MPI_INT, cartCoords[Z], comms->commSubMatrixY);

    /****************************** COMPUTATION ************************************/
    AdjacentCells adj;
    int planeRank;
    MPI_Comm_rank(comms->commXYplanes, &planeRank);

    //Initial alignment
    int rigaSubMatrix = cartCoords[Y] % (n/m);
    int colonnaSubMatrix = cartCoords[X] % (n/m);
    findAdjacentCells(planeRank, n, m, rigaSubMatrix, colonnaSubMatrix, &adj);
    MPI_Sendrecv_replace(&localA, 1, MPI_INT, adj.left, 0, adj.right, 0, comms->commXYplanes, MPI_STATUS_IGNORE);
    MPI_Sendrecv_replace(&localB, 1, MPI_INT, adj.up, 0, adj.down, 0, comms->commXYplanes, MPI_STATUS_IGNORE);
    
    //Compute
    for(int i=0; i<n/m; i++){
        localC += localA * localB;        
        findAdjacentCells(planeRank, n, m, 1, 1, &adj);
        MPI_Sendrecv_replace(&localA, 1, MPI_INT, adj.right, 0, adj.left, 0, comms->commXYplanes, MPI_STATUS_IGNORE);
        MPI_Sendrecv_replace(&localB, 1, MPI_INT, adj.down, 0, adj.up, 0, comms->commXYplanes, MPI_STATUS_IGNORE);
    }
    
    /****************************** REDUCE C LOCALE ************************************/
    int finalC;
    MPI_Reduce(&localC, &finalC, 1, MPI_INT, MPI_SUM, 0, comms->commZsingleDim);


    /* Stop the timer. Algorithm ends when layer 0 has the whole C matrix */
    MPI_Barrier(MPI_COMM_WORLD);
    total_time += MPI_Wtime();

    /****************************** GATHER C ************************************/
    if(!myRank) matrixC = malloc(n*n*sizeof(int));
    if(cartCoords[Z] == 0){
        MPI_Gather(&finalC, 1, MPI_INT, matrixC, 1, MPI_INT, 0, comms->commXYplanes);
    }

    /****************************** OUTPUT ************************************/
    if(!myRank){
        printf("%10.6f\t%10.6f\n",input_time, total_time - input_time);
        if(writeMatrixToFile(matrixC, n, "matrixC_dnsVariant.bin")){
            printf("Error writing matrixC\n");
            fflush(stdout);
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
        //printMatrix(matrixC, n);
        free(matrixC);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    
    return 0;
}
/**
 * @brief Creates communicators for the given dimensions and periods.
 * 
 * This function creates communicators for a Cartesian topology with the specified dimensions and periods.
 * It also creates sub-communicators for different planes and single dimensions.
 * Additionally, it splits the communicator based on the coordinates and a discriminant color.
 * 
 * @param comms Pointer to the struct containing the communicators.
 * @param dims Array of dimensions for the Cartesian topology.
 * @param periods Array of periods for the Cartesian topology.
 * @param cartRank Pointer to the variable storing the rank in the Cartesian communicator.
 * @param coords Pointer to the array storing the coordinates in the Cartesian communicator.
 * @param discriminanteColore The discriminant color used for splitting the communicator in submatrices.
 */
void createCommunicators(struct Communicators* comms, int* dims, int* periods, int* cartRank, int* coords, int discriminanteColore){
    MPI_Cart_create(MPI_COMM_WORLD, 3, dims, periods, 0, &comms->commCart);
    MPI_Comm_rank(comms->commCart, cartRank);
    MPI_Cart_coords(comms->commCart, *cartRank, 3, coords);
    int remaining_dims[3] = {0, 0, 0};

    remaining_dims[X] = 1;
    remaining_dims[Y] = 1;
    remaining_dims[Z] = 0;
    MPI_Cart_sub(comms->commCart, remaining_dims, &comms->commXYplanes);

    remaining_dims[X] = 0;
    remaining_dims[Y] = 1;
    remaining_dims[Z] = 1;
    MPI_Cart_sub(comms->commCart, remaining_dims, &comms->commYZplanes);

    remaining_dims[X] = 1;
    remaining_dims[Y] = 0;
    remaining_dims[Z] = 1;
    MPI_Cart_sub(comms->commCart, remaining_dims, &comms->commZXplanes);

    remaining_dims[X] = 1;
    remaining_dims[Y] = 0;
    remaining_dims[Z] = 0;
    MPI_Cart_sub(comms->commCart, remaining_dims, &comms->commXsingleDim);

    remaining_dims[X] = 0;
    remaining_dims[Y] = 1;
    remaining_dims[Z] = 0;
    MPI_Cart_sub(comms->commCart, remaining_dims, &comms->commYsingleDim);

    remaining_dims[X] = 0;
    remaining_dims[Y] = 0;
    remaining_dims[Z] = 1;
    MPI_Cart_sub(comms->commCart, remaining_dims, &comms->commZsingleDim);
    //These communicators include all the processes in the same row, that have the same rank inside all'submatrices
    int color = coords[X] % discriminanteColore;
    MPI_Comm_split(comms->commXsingleDim, color, *cartRank, &comms->commSubMatrixX);
    //These communicators include all the processes in the same col, that have the same rank inside all'submatrices
    color = coords[Y] % discriminanteColore;
    MPI_Comm_split(comms->commYsingleDim, color, *cartRank, &comms->commSubMatrixY);
}

/**
 * @brief Finds the indices of the adjacent cells in a grid with wrap-around.
 * 
 * This function calculates the indices of the adjacent cells in a grid with wrap-around.
 * The grid has dimensions (n/m)^2, and the distance between adjacent cells in the x and y directions is specified.
 * The indices of the adjacent cells are stored in the AdjacentCells struct.
 * The indices used to get cells position in matrix use a cartesian style indexing, not the usual row, col indexing.
 * 
 * @param index The index of the current cell.
 * @param n The number of cells in the x direction.
 * @param m The number of cells in the y direction.
 * @param distX The distance between adjacent cells in the x direction.
 * @param distY The distance between adjacent cells in the y direction.
 * @param adj Pointer to the struct storing the indices of the adjacent cells.
 */
void findAdjacentCells(int index, int n, int m, int distX, int distY, AdjacentCells *adj) {
    int sm_size = n / m;  // submatrix size
    int x = index % n;    // coord x cell
    int y = index / n;    // coord y cell

    // Get offset due to submatrix
    int sub_x = x / sm_size * sm_size;
    int sub_y = y / sm_size * sm_size;

    // Get adjacent cells coordinate in submatrix with wrap around
    int up_x = x;
    int up_y = (y - distY + sm_size) % sm_size + sub_y;
    adj->up = up_y * n + up_x;

    int down_x = x;
    int down_y = (y + distY) % sm_size + sub_y;
    adj->down = down_y * n + down_x;

    int left_x = (x - distX + sm_size) % sm_size + sub_x;
    int left_y = y;
    adj->left = left_y * n + left_x;

    int right_x = (x + distX) % sm_size + sub_x;
    int right_y = y;
    adj->right = right_y * n + right_x;
}
