#ifndef MATRIX_H_
#define MATRIX_H_
#include <stdio.h>


void gen_matrix(double*, int, int, FILE*);
int getRows(FILE*);
int getCols(FILE*);
int mmult(double*, double*, int, int, double*, int, int);
void compare_matrix(double*, double*, int, int);
//void alloc_matrix(double**, int, int);
#endif 
