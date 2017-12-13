/*
 * method3.c
 *
 *  Created on: 12 dic 2017
 *      Author: Samuele
 */
#define DEBUG_METHOD2

#include "method1.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define DIV 100

static double penalty(int *x, int T, int E, int S, int **n){
	int i, j, delta;
	double pen=0.0;

	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++){
			delta=x[i]-x[j];
			if(delta==0 && n[i][j]!=0) //unfeasable solution
				return -1;
			if(delta<=5 && n[i][j]!=0)
				pen+=pow(2, delta)*n[i][j]/S;
		}

	return pen;
}

static double swap(x, T, E, iter){
	int factor=(int)(E/iter+1), i, app;

	for(i=0; i<factor; i++){
		x[i]=app;
		x[i]=x[E-i-1];
	}

	return 0.0;
}

void optimizationMethod3(int *x, int T, int E, int S, int **n, int *students_per_exam, int **conflictual_students, char *instance_name)
{
	int iter=0;
	double prob, temp=1.0, new_pen, best_pen=100000.0;
	int *best;

	memcpy(best, x, E*sizeof(int)); //initialize best solution

	while(iter<1000){ //stopping condition??
		prob = (double)rand() / (double)RAND_MAX; //probability to make or not the swap
		if ((prob+temp)/2 > 0.5){
			new_pen = swap(x, T, E, iter);
			#ifdef DEBUG_METHOD3
			printf("New Penalty: %f; Best penalty: %f\n", new_pen, best_pen);
			#endif
			if (new_pen < best_pen) {
				memcpy(best, x, E*sizeof(int));
				best_pen=new_pen;
			}
			else
				memcpy(x, best, E*sizeof(int)); //restore the better one
		}
		iter++; //count the iteration
		temp-=iter/DIV; //decrease the temperature according to the iteration number
	}
	#ifdef DEBUG_METHOD3
	int i;
	printf("\n\n\nImproved penalty: %f\n", best_pen);
	printf("Improved solution:\n");
	for(i=0; i<E; i++){
		printf( "x[%d] = %d\n", i+1, best[i]);
	}
	#endif
}

