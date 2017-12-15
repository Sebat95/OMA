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

#define ESC1 100 //how many non-improving cycle we accept? (timeslot swap)
#define ESC2 50	//how many non-improving cycle we accept? (exam swap)
#define ESCFR 30 //how many equal-result cycle we accept?
#define OUT 300 //how many total cycle we wanna make?
#define DIV 7 //how much are we gonna cut down the temperature?


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

static void swap_ts(int *x, int E, int exch, int t){ //swap the chosen exams in timeslots t and exch
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


void optimizationMethod3(int *x, int T, int E, int S, int **n, int *students_per_exam, int **conflictual_students, char *instance_name)
{
	int best[E], i, exch1, exch2, outer=0, cnt=0, cntFR=0, ind1, ind2;
	double best_pen, new_pen, temp=1.0, prob;

	srand(time(NULL)); //rand() initialization

	best_pen=penalty(x, T, E, S, n); //best_pen init

	memcpy(best, x, E*sizeof(int)); //initialize best solution

	#ifdef DEBUG_METHOD3
	printf("\nInitial penalty: %f\n", best_pen);
	#endif

	while(outer<OUT){//stopping condition??
		cnt=0; //how many cycles without improvement
		//cntFR=0; //how many cycles with the same penalty (Flat Region)

		while(1){
			exch1 = rand() % T; //swap two random timeslot
			exch2 = exch1;
			prob = (double)rand() / (double)RAND_MAX; //probability to test with the temperature
			while(exch2 == exch1) //different from each others
				exch2 = rand() % T;
			swap_ts(x, E, exch2, exch1); //swap timeslots
			new_pen = penalty(x, T, E, S, n); //compute new results
			#ifdef DEBUG_METHOD3
			printf("New Penalty: %f; Best penalty: %f\n", new_pen, best_pen);
			#endif
			if (new_pen < best_pen && new_pen!=-1) { //if it's an improvement update "best" variables
				memcpy(best, x, E*sizeof(int));
				best_pen=new_pen;
				cnt=0;
				cntFR=0;
				temp=1.0;
			}
			else{ //if it wasn't an improvement
				if(new_pen==-1) //if unfeasable
					memcpy(x, best, E*sizeof(int)); //restore the best one
				if(temp>=prob){ //if temp allows it keep a worsening solution
					memcpy(x, best, E*sizeof(int)); //otherwise restore the best one
					//temp+=temp/DIV;
					//temp=1.0;
				}
				if(++cnt==ESC1) //we reached enough cycles without improvement?
					break;
				if(new_pen==best_pen || new_pen==-1){
					if(++cntFR==ESCFR){ //we reached enough cycles with the same result?
						temp-=temp/DIV;
						break;
					}
				}
				else
					cntFR=0;
			}
		}

		#ifdef DEBUG_METHOD3
		printf("\n\n\nImproved penalty: %f\n", best_pen);
		printf("Improved solution:\n");
		for(i=0; i<E; i++){
			printf( "x[%d] = %d\n", i+1, best[i]);
		}
		#endif

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cnt=0;
		//cntFR=0;

		while(1){ //stopping condition??
			ind1=rand() % E;
			/*ind2=rand() % E;
			while(n[ind2][ind1]!=0 && ind1!=ind2){
				ind2=rand() % E;
			}*/
			exch1 =x[ind1]; //swap one exam's timeslot
			//exch2=x[ind2];
			prob = (double)rand() / (double)RAND_MAX;
			for(i=0; i<T; i++){ //try every timeslot
				if(i==exch1) //except the one it already occupies
					continue;
				x[ind1]=i; //x[ind2]=x[ind1]=i;
				new_pen = penalty(x, T, E, S, n); //compute new results
				#ifdef DEBUG_METHOD3
				printf("New Penalty: %f; Best penalty: %f\n", new_pen, best_pen);
				#endif
				if (new_pen < best_pen && new_pen!=-1) { //if it's an improvement update "best" variables
					best[ind1]=i; //best[ind1]=best[ind2]=i;
					best_pen=new_pen;
					cnt=0;
					cntFR=0;
					temp=1.0;
				}
				else{ //if it wasn't an improvement
					if(new_pen==-1){ //if it's unfeasable
						x[ind1]=exch1; //undo the swap
						//x[ind2]=exch2;
					}
					if(temp<prob){ //if the temp allows it, keep a worsening solution
						x[ind1]=exch1; //otherwise undo the swap
						//x[ind2]=exch2;
						//temp+=temp/DIV;
						//temp=1.0;
					}
					if(++cnt==ESC2) //we reached enough cycles without improvement?
						break;
					if(new_pen==best_pen || new_pen==-1){
						if(++cntFR==ESCFR){ //we reached enough cycles with the same result?
							temp-=temp/DIV;
							break;
						}
					}
					else
						cntFR=0;
				}
			}
			if(cnt==ESC2)
				break;
		}

		#ifdef DEBUG_METHOD3
		printf("\n\n\nImproved penalty: %f\n", best_pen);
		/*printf("Improved solution:\n");
		for(i=0; i<E; i++){
			printf( "x[%d] = %d\n", i+1, best[i]);
		}*/
		#endif

	outer++; //update overall cycle index
	}
}
