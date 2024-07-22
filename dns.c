#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "inOutUtils.h"

#define X 1
#define Y 2
#define Z 0

struct Communicators{
    MPI_Comm commCart; // X orizzontale Y profondità Z verticale
    MPI_Comm commXYplanes; // piani orizzontali
    MPI_Comm commYZplanes; // piani verticali laterali
    MPI_Comm commZXplanes; // piani frontali verticali frontali
    MPI_Comm commXsingleDim; // processi lungo X con YZ fissati
    MPI_Comm commYsingleDim; // processi lungo Y con XZ fissati
    MPI_Comm commZsingleDim; // processi lungo Z con XY fissati
};

void createCommunicators(struct Communicators* comms, int* dims, int* periods,int* cartRank, int* coords);

int main(int argc, char* argv[]){
    double total_time; // Timer
    double input_time; // Timer    
    int myRank;
    int cartRank;
    int cartCoords[3];
    int p;
    int n;

    int* matrixA = NULL;
    int* matrixB = NULL;
    int* matrixC = NULL;
    int localA;
    int localB;
    int localC;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    /************************** INPUT ************************************/
    if(argc != 2){
        printf("Abort... usage ./betterDns [n]");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    n = strtol(argv[1],NULL, 10); //Tutti i processi conoscono la dimensione della matrice


    /* Start the timer */
    MPI_Barrier(MPI_COMM_WORLD);
    total_time = -MPI_Wtime();
    /************************** GENERAZIONE MATRICI ************************************/
    if(!myRank){//Solo il processo 0 genera le matrici
        matrixA = malloc(n*n*sizeof(int));
        matrixB = malloc(n*n*sizeof(int));

        if(matrixA==NULL || matrixB==NULL){
            printf("Abort... error allocating memory.\n");
            MPI_Abort(MPI_COMM_WORLD, 2);
        }        
        
        generateMatrix(matrixA, matrixB, n); //Genero le matrici
    }

    MPI_Barrier(MPI_COMM_WORLD);
    input_time = total_time + MPI_Wtime();
    /****************************** COMUNICATORI ************************************/
    struct Communicators* comms;
    comms = malloc(sizeof(struct Communicators));

    int num_dimensions = 3;
    int dims[3] = {0, 0, 0};
    dims[X] = n;
    dims[Y] = n;
    dims[Z] = p/(n*n);
    int periods[3] = {0, 0, 0};

    createCommunicators(comms, dims, periods, &cartRank, cartCoords);
    
    /****************************** SCATTER MATRICI ************************************/

    /* Il processo root invia in scatter un elemento della matrice ad ogni processo del livello 0*/
    if(cartCoords[Z] == 0){
        MPI_Scatter(matrixA, 1, MPI_INT, &localA, 1, MPI_INT, 0, comms->commXYplanes); 
        MPI_Scatter(matrixB, 1, MPI_INT, &localB, 1, MPI_INT, 0, comms->commXYplanes);
    }

    if(!myRank){ //Solo il processo 0 libera le matrici
        free(matrixA);
        free(matrixB);
    }

    /****************************** BCAST COLONNE DI A ************************************/
    MPI_Bcast(&localA, 1, MPI_INT, 0, comms->commZsingleDim);

    /****************************** BCAST RIGHE DI B ************************************/
    MPI_Bcast(&localB, 1, MPI_INT, 0, comms->commZsingleDim);

    /****************************** BCAST A LUNGO LE RIGHE ************************************/
    MPI_Bcast(&localA, 1, MPI_INT, cartCoords[Z], comms->commYsingleDim);

    /******************************* BCAST B LUNGO COLONNE ************************************/
    MPI_Bcast(&localB, 1, MPI_INT, cartCoords[Z], comms->commXsingleDim);

    /****************************** CALCOLO ************************************/
    localC = localA * localB;
    
    /****************************** REDUCE C LOCALE ************************************/
    int finalC;
    MPI_Reduce(&localC, &finalC, 1, MPI_INT, MPI_SUM, 0, comms->commZsingleDim);

    /****************************** GATHER C ************************************/
    if(!myRank) matrixC = malloc(n*n*sizeof(int));

    if(cartCoords[Z] == 0){
        MPI_Gather(&finalC, 1, MPI_INT, matrixC, 1, MPI_INT, 0, comms->commXYplanes);
    }

    /* Stop the timer */
    MPI_Barrier(MPI_COMM_WORLD);
    total_time += MPI_Wtime();

    /******************************* STAMPA ************************************/
    if(!myRank){
        printf("%10.6f\t%10.6f\n",input_time, total_time);
        if(writeMatrixToFile(matrixC, n, "matrixC_dns.bin")){
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

void createCommunicators(struct Communicators* comms, int* dims, int* periods,int* cartRank, int* coords){
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
}
