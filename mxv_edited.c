#include "mpi.h"
#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define min(x, y) ((x)<(y)?(x):(y))

/** 
    Program to multiply a matrix times a vector using both
    mpi to distribute the computation among nodes and omp
    to distribute the computation among threads.
*/

int main(int argc, char* argv[])
{
  int nrows, ncols;
  double *aa, *bb, *cc;
  double *buffer, ans;
  double *times;
  double total_times;
  int run_index;
  int nruns;
  int myid, master, numprocs;
  double starttime, endtime;
  MPI_Status status;
  int i, j, numsent, sender;
  int anstype, row;
  FILE *fileA;
  FILE *fileB;
  FILE *fileC;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  if (argc > 2) {
    fileA = fopen(argv[1],"r");
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

    aa = (double*)malloc(sizeof(double) * arows * bcols);
    bb = (double*)malloc(sizeof(double) * brows * bcols);
    cc = (double*)malloc(sizeof(double) * nrows*ncols);
    buffer = (double*)malloc(sizeof(double) * ncols);
    master = 0;
    if (myid == master) {
      // Master Code goes here
      gen_matrix(aa, arows, acols, fileA);
      
      starttime = MPI_Wtime();
      numsent = 0;
      MPI_Bcast(b, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
      for (i = 0; i < min(numprocs-1, nrows); i++) {
	for (j = 0; j < ncols; j++) {
	  buffer[j] = aa[i * ncols + j];
	}  
	MPI_Send(buffer, ncols, MPI_DOUBLE, i+1, i+1, MPI_COMM_WORLD);
	numsent++;
      }
      for (i = 0; i < nrows; i++) {
	MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, 
		 MPI_COMM_WORLD, &status);
	sender = status.MPI_SOURCE;
	anstype = status.MPI_TAG;
	c[anstype-1] = ans;
	if (numsent < nrows) {
	  for (j = 0; j < ncols; j++) {
	    buffer[j] = aa[numsent*ncols + j];
	  }  
	  MPI_Send(buffer, ncols, MPI_DOUBLE, sender, numsent+1, 
		   MPI_COMM_WORLD);
	  numsent++;
	} else {
	  MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
	}
      } 
      endtime = MPI_Wtime();
      printf("%f\n",(endtime - starttime));
    } else {
      // Slave Code goes here
      MPI_Bcast(b, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
      if (myid <= nrows) {
	while(1) {
	  MPI_Recv(buffer, ncols, MPI_DOUBLE, master, MPI_ANY_TAG, 
		   MPI_COMM_WORLD, &status);
	  if (status.MPI_TAG == 0){
	    break;
          }
	  row = status.MPI_TAG;
	  #pragma omp parallel default(none)      \
	  shared(a, b, c, aRows, aCols, bRows, bCols) private(i, k, j)
	  #pragma omp for
  	  for (i = 0; i < arows; i++) {
    		for (j = 0; j < bcols; j++) {
      			c[i*bCols + j] = 0;
    		}
    		for (k = 0; k < acols; k++) {
      			for (j = 0; j < bcols; j++) {
        			c[i*bcols + j] += a[i*acols + k] * b[k*bcols + j];
      			}
    		}
  	  }

	  MPI_Send(&ans, 1, MPI_DOUBLE, master, row, MPI_COMM_WORLD);
	}
      }
    }
  } else {
    fprintf(stderr, "Usage matrix_times_vector <size>\n");
  }
  MPI_Finalize();
  return 0;
}
