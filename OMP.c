#include <stdio.h>
#include<stdlib.h>
#include<omp.h>
#include <time.h>

int** setupMatrix(int rows, int cols);
void fillMatriceswithRandomNumbers();
 clock_t start;
 clock_t end ;

 int** firstMatrix, ** secondMatrix, ** resultMatrix;

int rows1, cols1, rows2, cols2;

void main(int argc, char** argv) {

	fflush(stdout);
	printf("Pmpi enter the matrices dimensions:-\n");
	scanf("%d", &rows1);
	scanf("%d", &cols1);
	scanf("%d", &rows2);
	scanf("%d", &cols2);
    
    if (cols1 != rows2) {
		fflush(stdout);
		printf("Invalid Operation\n");
        return;
    }
    firstMatrix = setupMatrix(rows1, cols1);
    secondMatrix = setupMatrix(rows2, cols2);
    resultMatrix = setupMatrix(rows1, cols2);

    fillMatriceswithRandomNumbers();
    int i = 0, j = 0, k = 0;
	int x = 5;  // initial value for num_threads , changeable 

	start = clock();
#pragma omp parallel num_threads(x) shared(firstMatrix,secondMatrix,resultMatrix,rows1,cols1,rows2,cols2) private(i, k, j)
    {
        #pragma omp for 
        for (i = 0; i < rows1; ++i) {
            for (j = 0; j < cols2; ++j) {
                for (k = 0; k < cols1; ++k) {
                    resultMatrix[i][j] += firstMatrix[i][k] * secondMatrix[k][j];
                }
            }
        }  
    }
    end = clock();
    double time_taken = ((double) (end - start)) / CLOCKS_PER_SEC;  // calculate the elapsed time
    printf("The program took %f seconds to execute", time_taken);

	system("pause");
}

int** setupMatrix(int rows, int cols) {
    int* linear, ** mat;
    int i;

    linear = malloc(sizeof(int) * rows * cols);
    mat = malloc(sizeof(int*) * rows);
    for (i = 0;i < rows; i++) {
        mat[i] = &linear[i * cols];
    }
    return mat;
}

void fillMatriceswithRandomNumbers() {
    time_t t;

    /* Intializes random number generator */
    srand((unsigned)time(&t));

    /* Print 5 random numbers from 0 to 49 */
    for (int index = 0;index < rows1;index++) {
        for (int secondIndex = 0;secondIndex < cols1;secondIndex++) {
            firstMatrix[index][secondIndex] = (rand() % 10);
        }
    }
    for (int index = 0;index < rows2;index++) {
        for (int secondIndex = 0;secondIndex < cols2;secondIndex++) {
            secondMatrix[index][secondIndex] = rand() % 10;
        }
    }

    for (int index = 0;index < rows1;index++) {
        for (int secondIndex = 0;secondIndex < cols2;secondIndex++) {
            resultMatrix[index][secondIndex] = 0;
        }
    }
}
