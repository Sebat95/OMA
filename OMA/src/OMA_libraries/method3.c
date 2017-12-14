/*
 * method3.c
 *
 *  Created on: 12 dic 2017
 *      Author: Samuele
 */
#define DEBUG_METHOD3

#include "method1.h"
#include "method3.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h> //per srand
#include <string.h> //for memcpy
#include <math.h>

#define STOP 10e10
#define TOP 10e10
#define ESC1 100
#define ESC2 1000
#define OUT 50


/*static double penalty_ex(int *x, int T, int E, int S, int **n, pen_per_ex *p){ //compute penalty
	int i, j, delta;
	double pen=0.0, adder;

	for(i=0; i<T;i++){ //reinitialize pen_ts
		p[i].pen=0.0;
		p[i].ex=0;
	}

	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++){
			delta=abs(x[i]-x[j]);
			if(delta==0 && n[i][j]!=0) //unfeasable solution
				return -1;
			if(delta<=5 && delta >=0 && n[i][j]!=0){
				adder=pow(2, 5-delta)*n[i][j]/S;
				pen+=adder;
				p[i].ex=i;
				p[i].pen+=adder;
			}
		}

	return pen;
}*/

static double penalty(int *x, int T, int E, int S, int **n){ //compute penalty
	int i, j, delta;
	double pen=0.0;

	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++){
			delta=abs(x[i]-x[j]);
			if(delta==0 && n[i][j]!=0) //unfeasable solution
				return -1;
			if(delta<=5 && delta >=0 && n[i][j]!=0)
				pen+=pow(2, 5-delta)*n[i][j]/S;;
		}

	return pen;
}

static void swap_ts(int *x, int E, int exch, int t){ //swap the chosen timeslots t and exch
	int i;

	for(i=0; i<E; i++){
		if (x[i]==t){
			x[i]=exch;
			continue;
		}
		if (x[i]==exch){
			x[i]=t;
			continue;
		}
	}
}

/*int ppt_desc(const void* a, const void* b) //function to be passed to qsort for ordering pen_per_ts decreasingly
{
	pen_per_ex A = *(pen_per_ex*)a, B = *(pen_per_ex*) b;
	double x = B.pen - A.pen;
	if(x>0)
		return 1;
	if(x<0)
		return -1;
	return 0;
}*/

void optimizationMethod3(int *x, int T, int E, int S, int **n, int *students_per_exam, int **conflictual_students, char *instance_name)
{
	int iter, best[E], i, exch1, exch2, outer=0, cnt=0;
	double best_pen, new_pen, temp=1.0;

	srand(time(NULL)); //rand initialization

	best_pen=penalty(x, T, E, S, n); //best_pen & pen_best init

	//qsort(pen_best, T, sizeof(pen_per_ts), ppt_desc); //sorting pen_ts in decreascing order

	/*memcpy(pen_new, pen_best, T*sizeof(pen_per_ts)); //pen_new init

	#ifdef DEBUG_METHOD3
	printf("\nTime slot ordered per penalty:\n");
	for(i=0; i<T; i++){
		printf( "x[%d] = %f\n", pen_best[i].ts, pen_best[i].pen);
	}
	#endif*/

	memcpy(best, x, E*sizeof(int)); //initialize best solution

	#ifdef DEBUG_METHOD3
	printf("\nInitial penalty: %f\n", best_pen);
	#endif

	while(outer<OUT){
		iter=0;
		cnt=0;
		temp-=temp/outer;
	while(iter<STOP){ //stopping condition??
		exch1 = rand() % T; //swap two random timeslot
		exch2 = exch1;
		//prob= rand() / RAND_MAX;
		while(exch2 == exch1) //different from each others
			exch2 = rand() % T;
		swap_ts(x, E, exch2, exch1);
		new_pen = penalty(x, T, E, S, n); //compute new results
   		#ifdef DEBUG_METHOD3
		printf("New Penalty: %f; Best penalty: %f\n", new_pen, best_pen);
		#endif
		if (new_pen < best_pen && new_pen!=-1) { //if it's an improvement update "best" variables
			memcpy(best, x, E*sizeof(int));
			best_pen=new_pen;
			cnt=0;
		}
		else{ //if it wasn't an improvement
			//if(new_pen==-1)
				memcpy(x, best, E*sizeof(int)); //restore the best one
			/*if(temp<prob)
				memcpy(x, best, E*sizeof(int)); //restore the best one*/
			if(++cnt==ESC1)
				break;
		}
		iter++; //count the iteration
	}

	#ifdef DEBUG_METHOD3
	printf("\n\n\nImproved penalty: %f\n", best_pen);
	printf("Improved solution:\n");
	for(i=0; i<E; i++){
		printf( "x[%d] = %d\n", i+1, best[i]);
	}
	#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*best_pen=penalty_ex(x, T, E, S, n, pen_best); //best_pen & pen_best init

	qsort(pen_best, E, sizeof(pen_per_ex), ppt_desc); //sorting pen_ts in decreascing order

	memcpy(pen_new, pen_best, E*sizeof(pen_per_ex)); //pen_new init

	#ifdef DEBUG_METHOD3
	printf("\nExam ordered per penalty:\n");
	for(i=0; i<E; i++){
		printf( "%d = %f\n", pen_best[i].ex, pen_best[i].pen);
	}
	#endif*/

	cnt=0;
	iter=0;
	while(iter<TOP){ //stopping condition??
		exch1 = rand() % E; //swap the one of the TOP worst timeslot with exchange
		exch2=x[exch1];
		//prob= rand() / RAND_MAX;
		for(i=0; i<T; i++){
			x[exch1]=i;
			new_pen = penalty(x, T, E, S, n); //compute new results
			#ifdef DEBUG_METHOD3
			printf("New Penalty: %f; Best penalty: %f\n", new_pen, best_pen);
			#endif
			if (new_pen < best_pen && new_pen!=-1) { //if it's an improvement update "best" variables
				best[exch1]=i;
				/*qsort(pen_new, E, sizeof(pen_per_ex), ppt_desc); //sorting pen_ts in decreascing order
				memcpy(pen_best, pen_new, E*sizeof(pen_per_ex)); //pen_new init*/
				best_pen=new_pen;
				cnt=0;
			}
			else{ //if it wasn't an improvement
				if(new_pen==-1)
					x[exch1]=exch2;
				if(++cnt==ESC2)
					break;
			}
		}
		if(cnt==ESC2)
			break;
		iter++; //count the iteration
	}

	#ifdef DEBUG_METHOD3
	printf("\n\n\nImproved penalty: %f\n", best_pen);
	printf("Improved solution:\n");
	for(i=0; i<E; i++){
		printf( "x[%d] = %d\n", i+1, best[i]);
	}
	#endif

	outer++;
	}

	//free(pen_best);
	//free(pen_new);
}

