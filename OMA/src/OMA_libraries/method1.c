/*
 * method1.c // ************ TENTATIVO CON OTTIMIZZAZIONE GRASP + TABU SEARCH ***************************
 *
 *  Created on: 02 dic 2017
 *      Author: Nicola
 */
/*#define DEBUG_METHOD1

#define PENALTIES_NUMBER 100
#define TABU_LENGTH 10
#define RANDOMNESS 0.5

#include "method1.h"
#include "tabu_search.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

typedef struct
{
	int exam1;
	int exam2;
	int penalty;
}Penalty;

// ********** PROTOTYPES (almost all static)

static void compute_worst_penalties(int *x, int **n, int T, int E, Penalty* penalties);
static void update_penalties_complete(Penalty* penalties, int pen, int exam1, int exam2);
static int compute_penalty_complete(int *x, int **n, int E, int T);

// ********** DEFINITIONS

void optimizationMethod1(int *x, int T, int E, int S, int **n, int *students_per_exam, char *instance_name)
{
	long int iteration_counter = 0;
	double pen, best_pen = INT_MAX;
	int i, j, to_swap;
	Penalty* penalties = malloc(PENALTIES_NUMBER * sizeof(Penalty));
	TABU tl = new_TabuList(TABU_LENGTH, TABU_LENGTH, TABU_LENGTH);

	while(1)
	{
#ifdef DEBUG_METHOD1
		pen = (double)compute_penalty_complete(x, n, E, T)/S;
		best_pen = (pen < best_pen) ? pen : best_pen;
		fprintf(stdout, "Iteration: %5d\tPenalty: %5.3f\tBest: %5.3f\n", iteration_counter, pen, 2*best_pen);
#endif

		compute_worst_penalties(x, n, T, E, penalties);

		to_swap = -1;
		do
		{
			for(i=0; i<PENALTIES_NUMBER; i++)
			{
				if(rand()/(double)RAND_MAX < RANDOMNESS)
					continue;
				to_swap = (rand()/(double)RAND_MAX < 50) ? penalties[i].exam1 : penalties[i].exam2; // POSSO FARLO IN MODO PIù INTELLIGENTE
				break;
			}
		}while(to_swap == -1);

		//insert_better_timeslot(x, n, T, E, to_swap); potrei implementare una soluzione di questo tipo intelligente (che conta la penalita per ogni timeslot)

		// PROVA BANALISSIMA (DA CANCELLARE E FARE MEGLIO)
		for(i=0; i<T; i++)
		{
			if(check_TabuList(tl, to_swap, i, 0))
				continue;
			for(j=0; j<E; j++)
				if(x[j] == T && n[i][j])
					break;
			if(j == E)
			{
				insert_TabuList(tl, to_swap, x[to_swap], 0);
				x[to_swap] = i;
				break;
			}
		}

		iteration_counter++;
	}

}
static void compute_worst_penalties(int *x, int **n, int T, int E, Penalty* penalties) // SI PUò FARE IN MODO INCREMENTALE (credo di si)
{
	int i, j;
	for(i=0; i<PENALTIES_NUMBER; i++)
	{
		penalties[i].penalty = 0; penalties[i].exam1 = -1; penalties[i].exam2 = -1;
	}
	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++)
			if(abs(x[i] - x[j]) <= 5 && n[i][j]) // conflictual exams near, penalty paid
				update_penalties_complete(penalties, (int)pow(2, abs(x[i]-x[j]))*n[i][j], i, j);
}
static void update_penalties_complete(Penalty* penalties, int pen, int exam1, int exam2)
{
	int i = 0, j;
	while(penalties[i].penalty > pen)
	{
		i++;
		if(i == PENALTIES_NUMBER)
			return;
	}
	for(j = PENALTIES_NUMBER-1; j > i; j--)
		penalties[j] = penalties[j-1];
	penalties[i].exam1 = exam1;
	penalties[i].exam2 = exam2;
	penalties[i].penalty = pen;
}
static int compute_penalty_complete(int *x, int **n, int E, int T) // SI PUò FARE IN MODO INCREMENTALE (credo di si) (dovrei passarli la mossa che vorrei fare)
{
	int i, j, pen=0;
	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++)
			if(abs(x[i]-x[j]) <= 5 && n[i][j])
				pen += (int)pow(2, abs(x[i]-x[j]))*n[i][j];
	return pen;
}
*/
