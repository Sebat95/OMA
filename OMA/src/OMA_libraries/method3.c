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
#include <string.h> //for memcpy
#include <math.h>
#define DIV 100

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
	int i, app, factor=(int) end/3;

	for(i=init; i<factor; i++){
		app=x[i];
		x[i]=x[end-i-1];
		x[end-i-1]=app;
	}
}

void optimizationMethod3(int *x, int T, int E, int S, int **n, int *students_per_exam, int **conflictual_students, char *instance_name)
{
	int iter=0, mul=0, begin=0, end=E, best[E];
	double prob, temp=1.0, new_pen, best_pen=penalty(x, T, E, S, n);

	memcpy(best, x, E*sizeof(int)); //initialize best solution

	#ifdef DEBUG_METHOD3
	printf("\nInitial penalty: %f\n", best_pen);
	#endif

	while(iter<1000){ //stopping condition??
		prob = (double)rand() / (double)RAND_MAX; //probability to make or not the swap
		if ((prob+temp)/2 > 0.5){
			swap(x, begin, end);
			if(end==E){
				mul++;
				end = (int) end/(mul*3);
				begin=0;
			}
			else{
				begin=end;
				end = (int) end+(E/mul*3);
			}
			new_pen = penalty(x, T, E, S, n);
			if(new_pen == -1)
				continue;
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

