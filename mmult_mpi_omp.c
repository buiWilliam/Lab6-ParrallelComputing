#include "matrix.h"
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#define min(x, y) ((x)<(y)?(x):(y))

//void fill_matrix(int n, int m, FILE *fp, double **matrix);
//int mmult(double **c, double **a, int aRows, int aCols, double **b, int bRows, int bCols);
//void compare_matrix(double **a, double **b, int nRows, int nCols);

/** 
  Program to multiply a matrix times a matrix using both
  mpi to distribute the computation among nodes and omp
  to distribute the computation among threads.
  */

int main(int argc, char* argv[])
{
    double *aa;	/* the A matrix */
    double *bb;	/* the B matrix */
    double *cc1;	/* A x B computed using the omp-mpi code you write */
    double *cc2;	/* A x B computed using the conventional algorithm */
    int myid, numprocs, nrows, ncols;
    double starttime, endtime;
    MPI_Status status;
    /* insert other global variables here */ 
    int i, j, k, rows, offset, dest, source, 
        arows, acols, brows, bcols, numworkers,
        totalrows;   
    FILE *fileA;
    FILE *fileB;
    FILE *fileC;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if (argc == 3){
        fileA = fopen(argv[1], "r");
        fileB = fopen(argv[2], "r");
        arows = getRows(fileA);
        acols = getCols(fileA);
        brows = getRows(fileB);
        bcols = getCols(fileB);
        if (acols == brows){
            nrows = arows;
            ncols = bcols;
        } else {
	    printf("arows = %d, bcols = %d\n",arows,bcols);
            fprintf(stderr, "These 2 matrices cannot be multiplied.\n");
            return 0;
        }

        aa = malloc(sizeof(double) * arows * acols);
        bb = malloc(sizeof(double) * brows * bcols);
        gen_matrix(aa, arows, acols, fileA);
        gen_matrix(bb, brows, bcols, fileB);
        cc1 = malloc(sizeof(double) * nrows * ncols);
        double a[arows][acols];
        double b[brows][bcols];
        double c[nrows][ncols];
        double c2[nrows][ncols];
	
        if (myid == 0) {
            printf("MatrixA\nrows=%d, cols=%d\n",arows,acols);
            i = j = 0;
            for (i = 0; i < arows; i++){
            	for (j = 0; j < acols; j++){
		    printf("%f\n",aa[i*acols+j]);
                    a[i][j]=aa[i*acols+j];
		    printf("%f\n",a[i][j]);
            	}
	    }
            printf("\nMatrixB\nrows=%d, cols=%d\n",brows,bcols);
            i = j = 0;
            for (i = 0; i < brows; i++){
            	for (j = 0; j < bcols; j++){
		     printf("%f\n",bb[i*bcols+j]);
                    b[i][j]=bb[bcols*i+j];
		    printf("%f\n",b[i][j]);
            	}
	    }

		for (i = 0; i < arows; i++){
                        for (j = 0; j < acols; j++){
                                printf("a[%d][%d]=%f\n",i,j,a[i][j]);
                        }
                }

                for (i = 0; i < brows; i++){
                        for (j = 0; j < bcols; j++){
                                printf("b[%d][%d]=%f\n",i,j,b[i][j]);
                        }
                }

            starttime = MPI_Wtime();
            offset = 0;
            numworkers = numprocs-1;
            rows = nrows/numworkers;
            if (rows == 0) rows = 1;
            int averow, extra;
            averow = nrows/numworkers;
            extra = nrows%numworkers;
            for (dest=1; dest<=min(numworkers, nrows); dest++){
                rows = (dest <= extra) ? averow+1 : averow;
		printf("\nCheck 0\n");
                MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
		printf("\nCheck 1\n");
                MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
		printf("\nCheck 2\n");
                MPI_Send(&a[offset][0], rows*nrows, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
		printf("\nCheck 3\n");
                MPI_Send(&b, brows*bcols, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
		printf("\nCheck 4\n");
                offset = offset + rows;
                totalrows -= rows;
            } 

            for (i=1; i<=min(numworkers, nrows); i++){
                source = i;
                MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
                MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);

                MPI_Recv(&c[offset][0], rows*nrows, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
            }
            endtime = MPI_Wtime();
            printf("%f\n", (endtime - starttime));
            //      mmult(cc2, aa, arows, acols, bb, brows, bcols); 
            for (i=0; i<nrows; i++){
                for (j=0; j<ncols; j++){
                    c2[i][j] = 0;
		    printf("c[%d][%d]= ",i,j);
                    for (k=0; k<acols; k++){
			printf("%f * %f + ", aa[acols*i+k], bb[bcols*k+j]);
                        c2[i][j] += aa[acols*i+k] * bb[bcols*k+j];
                    }
		    printf("\n");
                }
		
            }

            int isSame = 1; 

            for (i=0; i<nrows; i++){
                for (j=0; j<ncols; j++){
                    if (c[i][j] != c2[i][j]) isSame = 0;
                    printf("c[%d][%d] = %lf c2[%d][%d] = %lf\n", i, j, c[i][j], i, j, c2[i][j]);
                }
            }
            if (isSame){
                printf("These matrices are the same.\n");
            } else {
                printf("These matrices are NOT the same.\n");
            }

            fileC = fopen("MatrixC.txt", "w");
            char space[] = " ";
            char nline[] = "\n";
            char s[sizeof(double)*nrows];
	    sprintf(s,"rows(%d) cols(%d)\n",nrows,ncols);
	    fwrite(s,2,sizeof(char*),fileC);
            for (i=0; i<nrows; i++){
                for (j=0; j<ncols; j++){
                    sprintf(s, "%f", c[i][j]);
                    fwrite(s, 1, sizeof(double), fileC);
                    if (j != ncols-1) fwrite(space, 1, sizeof(char), fileC);
                }
                fwrite(nline, 1, sizeof(char), fileC); 
            }
            printf("MatrixC has been stored in file: MatrixC.txt.\n");

        }else {
        // Slave Code goes here
            if (nrows >= myid){
                source = 0;
                MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
                MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);	
                MPI_Recv(&a, rows*nrows, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
                MPI_Recv(&b, brows*bcols, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
                //printf("all info recieved...\n");     
                for (k=0; k<ncols; k++){
                    for (i=0; i<rows; i++){
                        c[i][k] = 0; 
                        for(j=0; j<brows; j++){
                            c[i][k] += a[i][j] * b[j][k];
                       	}
                    }
                }
                printf("Multiplication from worker %d complete!\n", myid);
                MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
                MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
                MPI_Send(&c, rows*ncols, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
            }
        }
    } else {
        fprintf(stderr, "Two files with MatrixA and MatrixB must be passed as arguments\n");
        return 0;
    }
    MPI_Finalize();

    return 0;
}
