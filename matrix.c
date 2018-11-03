#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void gen_matrix(double *matrix, int n, int m, FILE *fp) {    
    int i, j;
    double c;
    i = j = 0;
    while ((c = fgetc(fp)) != '\n'){
    }
	for (i = 0; i < n; i++){
		for (j = 0; j < m; j++){
			if (!fscanf(fp, "%lf", &matrix[n*i+j])) break;
			}
	}	
    rewind(fp);
}

int getRows(FILE *matrix) {
    int n = 0;
    /*int c; 
    while ((c = fgetc(matrix)) != '\n'){
	}
    while ((c = fgetc(matrix)) != EOF) {
        if (c == '\n') n++;
    }
   */
    fscanf(matrix,"rows(%d)",&n);
    rewind(matrix);
    return n;
}

int getCols(FILE *matrix) {
    int m = 0;
    /*int c;
    while ((c = fgetc(matrix)) != '\n'){
        }
    while ((c = fgetc(matrix)) != '\n') {
        if (c == ' ') m++;
    } 
    */
    fscanf(matrix,"%*s cols(%d)",&m);
    rewind(matrix);
    return m;
}

//Returns 1 if mmult is successful, else returns 0
int mmult(double *c, double *a, int aRows, int aCols, double *b, int bRows, int bCols){
	int i,j,k, nRows, nCols;	
	//check for mmult condition
	nRows = aRows;
	nCols = bCols; 
	//multiply *a and *b 
	for (int i=0; i<nRows; i++){
		for (int j=0; j<nCols; j++){
			c[nRows*i+j] = 0;
			for (k=0; k<nRows; k++){
				c[nRows*i+j] += a[nRows*i+k] * b[nRows*k+j];
			}
		}
	}
	
	return 1;
}

void compare_matrix(double *a, double *b, int nRows, int nCols){
	int i, j;
	int isSame = 1; 	
	for (i=0; i<nRows; i++){
		for (j=0; j<nCols; j++){
			if (a[nRows*i+j] != b[nRows*i+j]){
				isSame = 0; 
			}
		}
	}
	
	if (isSame){
		printf("These matrices are the same.\n");
	} else {
		printf("These matrices are NOT the same.\n");
	}	

}

/*
void alloc_matrix(double **a, int rows, int cols){
  int i;
  for (i=0; i<rows; i++){
    a[i] = malloc(sizeof(double)*cols);
  }

}
*/
