#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include <time.h>

int portion;
int remaining;
int low_bound;
int upper_bound;

int** setupMatrix(int rows, int cols);
void fillMatriceswithRandomNumbers();

int** firstMatrix, ** resultMatrix;
int* secondMatrix, *sendcounts, *displacement, *high_bound_proc;
clock_t t;
int rows1, cols1, rows2, cols2;
int counter = 0;
int i, j, k, count;

int main(int argc, char* argv[]) {
	int rank;
	int size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); //get the rank
	MPI_Comm_size(MPI_COMM_WORLD, &size); //get number of the processors

	MPI_Status status;
	MPI_Request request;

	sendcounts = (int*)malloc(sizeof(int) * size);
	displacement = (int*)malloc(sizeof(int) * size);
	high_bound_proc = (int*)malloc(sizeof(int) * size);

	sendcounts[0] = 0;
	displacement[0] = 0;
	high_bound_proc[0] = 0;

	if (rank == 0) {
		printf("Pmpi enter the matrices dimensions:-\n");
		fflush(stdout);
		scanf("%d", &rows1);
		scanf("%d", &cols1);
		scanf("%d", &rows2);
		scanf("%d", &cols2);

		if (cols1 != rows2) {
			printf("Invalid Operation\n");
			return;
		}
		portion = (rows1 / (size - 1)); // calculate portion without master
		remaining = (rows1 % (size - 1));


		for (i = 1; i < size; i++) { // for each slave other than the master
			low_bound = (i - 1) * portion;
			if (((i + 1) == size) && remaining != 0) { //if rows of [A] cannot be equally divided among slaves
				upper_bound = low_bound + remaining + portion; //last slave gets all the remaining rows
			}
			else {
				upper_bound = low_bound + portion; //rows of [A] are equally divisable among slaves
			}

			sendcounts[i] = (upper_bound - low_bound) * cols1;
			displacement[i] = low_bound * cols1; //Edited

		}

		//sending to the slaves portions , displacement index , highbound .
		MPI_Bcast(&sendcounts[0], size, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&displacement[0], size, MPI_INT, 0, MPI_COMM_WORLD);

		t = clock();
		// initializing matrices 
		firstMatrix = setupMatrix(rows1, cols1);
		secondMatrix = (int*)malloc(sizeof(int) * rows2 * cols2);
		resultMatrix = setupMatrix(rows1, cols2);

		if (!secondMatrix) // error checking
		{
			printf("Invalid Operation\n");
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
		// filling data in the matrices
		fillMatriceswithRandomNumbers();

		// sending to the slaves 	
		MPI_Bcast(&cols1, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Scatterv(&firstMatrix[0][0], sendcounts, displacement, MPI_INT, MPI_IN_PLACE, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&rows2, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&cols2, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&secondMatrix[0], rows2 * cols2, MPI_INT, 0, MPI_COMM_WORLD);

	}

	if (rank > 0) {
		//recieving second matrix for each slave 
		MPI_Bcast(&sendcounts[0], size, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&displacement[0], size, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&cols1, 1, MPI_INT, 0, MPI_COMM_WORLD); //Edited: Moved to here to be used in setting up the first matrix

		firstMatrix = setupMatrix(sendcounts[rank] / cols1, cols1);
		MPI_Scatterv(&firstMatrix[0][0], sendcounts, displacement, MPI_INT, &firstMatrix[0][0], sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

		MPI_Bcast(&rows2, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&cols2, 1, MPI_INT, 0, MPI_COMM_WORLD);

		//printf("sjdakjaslkdjaklda");
		secondMatrix = (int*)malloc(sizeof(int) * rows2 * cols2);
		MPI_Bcast(&secondMatrix[0], rows2 * cols2, MPI_INT, 0, MPI_COMM_WORLD);

		// initialzing result matrix 
		resultMatrix = setupMatrix(sendcounts[rank] / cols1, cols2);

		// multiplying matrices 
		for (i = 0; i < (sendcounts[rank] / cols1); ++i) {
			for (j = 0; j < cols2; ++j) {
				resultMatrix[i][j] = 0;
				for (k = 0; k < cols1; ++k) {
					resultMatrix[i][j] += firstMatrix[i][k] * secondMatrix[k * cols2 + j];
				}
			}
		}
		// Gathering 
		MPI_Gatherv(&resultMatrix[0][0], sendcounts[rank], MPI_INT, &resultMatrix[0][0], sendcounts, displacement, MPI_INT, 0, MPI_COMM_WORLD);
	}

	if (rank == 0) {
		MPI_Gatherv(MPI_IN_PLACE, sendcounts[rank], MPI_INT, &resultMatrix[0][0], sendcounts, displacement, MPI_INT, 0, MPI_COMM_WORLD);

		printf("final result: \n");
		for (i = 0; i < rows1; i++) {
			for (j = 0; j < cols2; j++) {
				printf("%d ", resultMatrix[i][j]);
			}
			printf("\n");
		}
		printf("finished\n");
		t = clock() - t ;
		double time_taken = ((double)t) / CLOCKS_PER_SEC; // calculate the elapsed time
		printf("The program took %f seconds to execute", time_taken);
	}

	MPI_Finalize();
	return 0;
}




int** setupMatrix(int rows, int cols) {
	int* linear, ** mat;

	linear = malloc(sizeof(int) * rows * cols);
	mat = malloc(sizeof(int*) * rows);
	for (i = 0; i < rows; i++) {
		mat[i] = &linear[i * cols];
	}
	return mat;
}


void fillMatriceswithRandomNumbers() {
	time_t t;

	/* Intializes random number generator */
	srand((unsigned)time(&t));

	/* Print 5 random numbers from 0 to 49 */
	for (i = 0; i < rows1; i++) {
		for (j = 0; j < cols1; j++) {
			firstMatrix[i][j] = (rand() % 10);
		}
	}

	for (i = 0; i < rows2; i++) {
		for (j = 0; j < cols2; j++) {
			secondMatrix[i * rows2 + j] = rand() % 10;
		}
	}

	for (i = 0; i < rows1; i++) {
		for (j = 0; j < cols2; j++) {
			resultMatrix[i][j] = 0;
		}
	}
}
