/*
 * method3.c
 *
 *  Created on: 12 dic 2017
 *      Author: Samuele
 */
#define DEBUG_METHOD3

#include "method1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for memcpy
#include <math.h>
#define STOP 1000

static double penalty(int *x, int T, int E, int S, int **n){
	int i, j, delta;
	double pen=0.0;

	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++){
			delta=abs(x[i]-x[j]);
			if(delta==0 && n[i][j]!=0) //unfeasable solution
				return -1;
			if(delta<=5 && delta >=0 && n[i][j]!=0)
				pen+=pow(2, delta)*n[i][j]/S;
		}

	return pen;
}

static void swap(int *x, int init, int end){
	int i, app, factor = ceil(((double)end - (double)init)/3.0);

	for(i=init; i<(factor+init); i++){
		app=x[i];
		x[i]=x[end-i+init-1];
		x[end-i+init-1]=app;
	}
}

static double min(int a, int b){
	if(a<=b)
		return a;
	else
		return b;
}

void optimizationMethod3(int *x, int T, int E, int S, int **n, int *students_per_exam, int **conflictual_students, char *instance_name)
{
	int iter=0, mul=0, begin=0, end=E, best[E], step=E;
	double temp=1.0, new_pen, best_pen=penalty(x, T, E, S, n);

	memcpy(best, x, E*sizeof(int)); //initialize best solution

	#ifdef DEBUG_METHOD3
	printf("\nInitial penalty: %f\n", best_pen);
	#endif

	while(iter<STOP){ //stopping condition??
		//prob = (double)rand() / (double)RAND_MAX; //probability to make or not the swap
		//if ((prob+temp)/2 > 0.5){
		if(temp){
			swap(x, begin, end);
			if(end>E-step){
				mul++;
				step = min((int) (end/(mul*3)), step-1);
				if(step<3)
					break;
				end = step;
				begin=0;
			}
			else{
				begin = end;
				end += step;
			}
			new_pen = penalty(x, T, E, S, n);
			#ifdef DEBUG_METHOD3
			printf("New Penalty %d - %d: %f; Best penalty: %f\n", begin-step, begin ,new_pen, best_pen);
			#endif
			if (new_pen <= best_pen && new_pen!=-1) {
				memcpy(best, x, E*sizeof(int));
				best_pen=new_pen;
			}
			//else
				//memcpy(x, best, E*sizeof(int)); //restore the better one
		}
		iter++; //count the iteration
		//temp= temp - iter/STOP; //decrease the temperature according to the iteration number
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

