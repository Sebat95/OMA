/*
 * method2.c
 *
 *  Created on: 02 dic 2017
 *      Author: Nicola
 */
#define DEBUG_METHOD2

#define TABU_LENGTH 5
#define MIN_TABU_LENGTH 1
#define MAX_TABU_LENGTH 50

#define N_BEST 200
#define RANDOMNESS_BEST 0.8

#define N_BEST_SINGLE 200
#define RANDOMNESS_BEST_SINGLE 0.8

#define PENALTIES_NUMBER 100

#define ALFA 0.1
#define BETA 0.2
#define ITERATION 100

#include "method2.h"
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

typedef struct
{
	int exam;
	int penalty;
}ExamPenalty;

// ********** PROTOTYPES (almost all static)

static int neighborhood1_bestOnly(int *x, int **n, int T, int E, TABU tl, int actual_pen);
static int neighborhood1_bestRandom(int *x, int **n, int T, int E, TABU tl, int *group_position, int** group_conflict, int pen);
static void neighborhood1_setup(int *x, int **n, int T, int E, int *group_positions, int **group_conflicts);
static int compute_pen_groups(int **group_conflicts, int *group_position, int T);
static int update_pen_groups(int pen, int group1, int group2, int *group_position, int** group_conflicts, int T);

static void compute_ordered_exam_penalty(ExamPenalty *ordered, ExamPenalty *penalty, int E);

static void compute_worst_penalties(int *x, int **n, int T, int E, Penalty* penalties);
static void update_penalties_complete(Penalty* penalties, int pen, int exam1, int exam2);

static void swap(int* a, int* b);
static int compute_penalty_complete(int *x, int **n, int E);
static int update_penalty(int *x, int **n, int E, int old_pen, int to_swap, int old_timeslot, int new_timeslot);
// ********** DEFINITIONS

void optimizationMethod2(int *x, int T, int E, int S, int **n, int *students_per_exam, char *instance_name)
{
	long int iteration_counter = 0;
	double initial_pen, pen, best_pen = INT_MAX, trend = -1, B = 0, pen_norm;
	int i, improvements_number = 0, partial_iteration = 0, tabu_length = TABU_LENGTH, initial_penalties[ITERATION];
	int actual_neighborhood = 0;
	//Penalty* penalties = malloc(PENALTIES_NUMBER * sizeof(Penalty));
	TABU tl = new_TabuList(TABU_LENGTH, MIN_TABU_LENGTH, MAX_TABU_LENGTH);
	int *group_positions , **group_conflicts;
	//Penalty best_move;
	//best_move.penalty = INT_MAX; // E' SOLO UNA PROVA

	group_positions = malloc(T * sizeof(int));
	group_conflicts = malloc(T * sizeof(int*));
	for(i=0; i<T; i++)
		group_conflicts[i] = malloc(T * sizeof(int));

	initial_pen = pen = compute_penalty_complete(x, n, E);

	neighborhood1_setup(x, n, T, E, group_positions, group_conflicts);
	while(1)
	{
#ifdef DEBUG_METHOD2
		fprintf(stdout, "Iteration: %5d\tTL_len = %d\tT = %f\tPenalty: %5.3f\tBest: %5.3f\tInitial: %5.3f\n", (int)iteration_counter, tabu_length, trend, pen/S, best_pen/S, initial_pen/S);
#endif

		switch(actual_neighborhood)
		{
		case 0:
			if(T != -1)
			{
				// uso T
			}
			pen = neighborhood1_bestRandom(x, n, T, E, tl, group_positions, group_conflicts, pen);
			//pen = neighborhood1_bestOnly(x, n, T, E, tl, pen);
			break;
		case 1:

			break;
		}

		iteration_counter++;
		partial_iteration++;
		if(best_pen > pen)
		{
			improvements_number++;
			best_pen = pen;
		}

		// MATEMATICI
		if(iteration_counter <= ITERATION)
		{
			//pen_norm = pen;
			pen_norm = 1;
			initial_penalties[iteration_counter-1] = pen;
			if(iteration_counter == ITERATION)
			{
				trend = (pen/pen_norm - initial_penalties[0]/pen_norm) / (ITERATION-1);
				for(i=0; i<ITERATION; i++)
					B += initial_penalties[i]/pen_norm - (i+1)*trend;
				B = B / ITERATION;
			}
		}
		else
		{
			i = B;
			B = ALFA*pen/pen_norm + (1-ALFA)*(i + trend);
			trend = BETA*(B - i) + (1-BETA) * trend;
		}
	}

}

// 1 NEIGHBORHOOD STRUCTURE: to swap entire group of exam (corrisponding to a timeslot)
static int neighborhood1_bestOnly(int *x, int **n, int T, int E, TABU tl, int actual_pen)
{
	int i, group1 = rand() % T, group2 = rand() % T;
	int best_group1 = -1, best_group2 = -1, best = INT_MAX, pen;

	for(group1 = 0; group1 < T; group1++)
		for(group2 = group1+1; group2 < T; group2++)
		{
//			if(check_TabuList(tl, group1, group2, 1))
//				continue;
			for(i=0; i<E; i++) // swap
				{
					if(x[i] == group1)
						x[i] = group2;
					else if(x[i] == group2)
						x[i] = group1;
				}
			pen = compute_penalty_complete(x, n, E);
			if(pen < best)
			{
				best_group1 = group1;
				best_group2 = group2;
				best = pen;
			}
			for(i=0; i<E; i++) // backtrack
				{
					if(x[i] == group1)
						x[i] = group2;
					else if(x[i] == group2)
						x[i] = group1;
				}
		}
	if(actual_pen < best)
		return 0;
	for(i=0; i<E; i++)
		{
			if(x[i] == best_group1)
				x[i] = best_group2;
			else if(x[i] == best_group2)
				x[i] = best_group1;
		}
	//insert_TabuList(tl, best_group1, best_group2, 1);
	return best;

	// BEST N_BEST
	int N_best[N_BEST][3], j;
	for(i=0; i<N_BEST; i++) // setup
	{
		N_best[i][0] = INT_MAX, N_best[i][1] = N_best[i][2] = -1;
	}
	for(group1 = 0; group1 < T; group1++)
			for(group2 = group1+1; group2 < T; group2++)
			{
				if(check_TabuList(tl, group1, group2, 1))
					continue;
				for(i=0; i<E; i++) // swap
					{
						if(x[i] == group1)
							x[i] = group2;
						else if(x[i] == group2)
							x[i] = group1;
					}
				pen = compute_penalty_complete(x, n, E);
				for(i=0; i<N_BEST && N_best[i][1] != -1 && N_best[i][0] < pen; i++);
				for(j=N_BEST-1; j > i; j--)
				{	N_best[j][0] = N_best[j-1][0]; N_best[j][1] = N_best[j-1][1]; N_best[j][2] = N_best[j-1][2]; }
				N_best[i][0] = pen; N_best[i][1] = group1; N_best[i][2] = group2;

				for(i=0; i<E; i++) // backtrack
					{
						if(x[i] == group1)
							x[i] = group2;
						else if(x[i] == group2)
							x[i] = group1;
					}
			}
	for(i=0; ; i = (i+1)%N_BEST)
	{
		if(rand()/(double)RAND_MAX < RANDOMNESS_BEST)
			continue;
		insert_TabuList(tl, N_best[i][1], N_best[i][2], 1);
		for(j=0; j<E; j++) // backtrack
			{
				if(x[j] == N_best[i][1])
					x[j] = N_best[i][2];
				else if(x[j] == N_best[i][2])
					x[j] = N_best[i][1];
			}
		break;
	}
	return N_best[i][0];

}
static int neighborhood1_bestRandom(int *x, int **n, int T, int E, TABU tl, int *group_position, int** group_conflict, int pen)
{
	int i, j, group1, group2, actual_pen;
	int N_best[N_BEST][3];

	for(i=0; i<N_BEST; i++) // setup
	{
		N_best[i][0] = INT_MAX, N_best[i][1] = N_best[i][2] = -1;
	}
	for(group1 = 0; group1 < T; group1++)
			for(group2 = group1+1; group2 < T; group2++)
			{
				if(check_TabuList(tl, (group_position[group1]<group_position[group2])?group_position[group1]:group_position[group2], (group_position[group1]>group_position[group2])?group_position[group1]:group_position[group2], 1))
					continue; // timeslots group_position[group1] and group_position[group2] already swapped in the last moves (is referred to timeslot, not to group)

				actual_pen = update_pen_groups(pen, group1, group2, group_position, group_conflict, T); // compute how much the penalty would be if I swap group1 and group2

				// update N_best array (if this swap is in the N best)
				for(i=0; i<N_BEST && N_best[i][1] != -1 && N_best[i][0] < actual_pen; i++);
				if(i != N_BEST)
				{
					for(j=N_BEST-1; j > i; j--)
					{	N_best[j][0] = N_best[j-1][0]; N_best[j][1] = N_best[j-1][1]; N_best[j][2] = N_best[j-1][2]; }
					N_best[i][0] = actual_pen; N_best[i][1] = group1; N_best[i][2] = group2;
				}
			}
	// select one of the best moves
	for(i=0; ; i = (i+1)%N_BEST)
	{
		if(N_best[i][1] == -1)
		{
			i = 0;
			continue;
		}
		if(rand()/(double)RAND_MAX < RANDOMNESS_BEST)
			continue;
		insert_TabuList(tl, (group_position[N_best[i][1]]<group_position[N_best[i][2]])?group_position[N_best[i][1]]:group_position[N_best[i][2]], (group_position[N_best[i][1]]>group_position[N_best[i][2]])?group_position[N_best[i][1]]:group_position[N_best[i][2]], 1);
		for(j=0; j<E; j++) // perform the move
			{
				if(x[j] == group_position[N_best[i][1]])
					x[j] = group_position[N_best[i][2]];
				else if(x[j] == group_position[N_best[i][2]])
					x[j] = group_position[N_best[i][1]];
			}
		swap(group_position+N_best[i][1], group_position+N_best[i][2]); // update group_position
		break;
	}
	return N_best[i][0]; // return the penalty

}
static int update_pen_groups(int pen, int group1, int group2, int *group_position, int** group_conflicts, int T)
{
	int i, i_lower1, i_higher1, i_lower2, i_higher2;
	int group1_position = group_position[group1], group2_position = group_position[group2];
	int timeslot_group[T];
	for(i=0; i<T; i++)
		timeslot_group[group_position[i]] = i; // in timeslot group_position[i] there is group of exam i
	i_lower1 = (group1_position>5) ? group1_position-5 : 0;
	i_higher1 = (group1_position+5<T) ? group1_position+5 : T-1;
	for(i = i_lower1; i <= i_higher1; i++)
	{
			pen -= pow(2, 5-abs(group1_position-i))*group_conflicts[timeslot_group[i]][group1];
	}
	i_lower2 = (group2_position>5) ? group2_position-5 : 0;
	i_higher2 = (group2_position+5<T) ? group2_position+5 : T-1;
	for(i = i_lower2; i <= i_higher2; i++)
	{
			pen -= pow(2, 5-abs(group2_position-i))*group_conflicts[timeslot_group[i]][group2];
	}
	swap(timeslot_group+group1_position, timeslot_group+group2_position);
	for(i=i_lower1; i<=i_higher1; i++)
		pen += pow(2, 5-abs(group1_position-i))*group_conflicts[timeslot_group[i]][group2];
	for(i=i_lower2; i<=i_higher2; i++)
		pen += pow(2, 5-abs(group2_position-i))*group_conflicts[timeslot_group[i]][group1];
	return pen;
}
static void neighborhood1_setup(int *x, int **n, int T, int E, int *group_positions, int **group_conflicts)
{
	int i, j, ii, jj;
	for(i=0; i<T; i++)
	{
		group_positions[i] = i;
	}
	for(i=0; i<T; i++)
		for(j=0; j<T; j++)
			group_conflicts[i][j] = 0;
	for(i=0; i<T; i++)
		for(j=i+1; j<T; j++)
			for(ii=0; ii<E; ii++)
				if(x[ii] == i)
					for(jj=0; jj<E; jj++)
						if(x[jj] == j && n[ii][jj])
						{
							group_conflicts[i][j] += n[ii][jj];
							group_conflicts[j][i] += n[ii][jj];
						}
}
// 2 NEIGHBORHOOD STRUCTURE: mantengo un vettore che mi dice quanta penalità pago per ogni esame, quindi tra scelgo uno tra quelli che costano di più e lo sposto tra i posti migliori in cui può andare
static int neighborhood3_bestRandom(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen)
{
	int i, j, to_swap, pen;
	int N_best[N_BEST_SINGLE][3];
	to_swap = -1;
	ExamPenalty ordered_exam_penalty[E];
	compute_ordered_exam_penalty(ordered_exam_penalty, exam_penalty, E);
	for(i=0; i<N_BEST_SINGLE; i++)
		N_best[i][0] = -1;
	while(to_swap == -1)
	{
		for(i=0; i<E; i++)
		{
			if(rand()/(double)RAND_MAX < RANDOMNESS_BEST_SINGLE)
				continue;
			to_swap = ordered_exam_penalty[i].exam; // to_swap is one of the most "costly" exam (exam_penalty is ordered)
			break;
		}
	}

	for(i=0; i<T; i++)
	{
		for(j=0; j<E; j++)
			if(x[j] == i && n[j][to_swap])
				continue; // unfeasible swap
		pen = update_penalty(x, n, E, actual_pen, to_swap, x[to_swap], i);

		//for(j=0; j<N_BEST && pen )
	}



}
static int update_exam_penalties(ExamPenalty *exam_penalty, int *x, int **n, int E, int to_swap, int old_timeslot, int new_timeslot) // update exam_penalty array when I move exam to_swap in new_timeslot
{
	int i, val;
	exam_penalty[to_swap].penalty = 0;
	for(i=0; i<E; i++)
	{
		if(n[i][to_swap])
		{
			if(abs(x[i]-old_timeslot) <= 5)
				exam_penalty[i].penalty -= pow(2, abs(x[i]-old_timeslot))*n[i][to_swap];
			if(abs(x[i]-new_timeslot) <= 5)
			{
				val = pow(2, abs(x[i]-new_timeslot))*n[i][to_swap];
				exam_penalty[i].penalty += val;
				exam_penalty[to_swap].penalty += val;
			}
		}
	}
}
static void neighborhood2_setup(int *x, int **n, int T, int E, ExamPenalty *exam_penalty)
{
	int i, j;
	for(i=0; i<E; i++)
	{
		exam_penalty[i].exam = i; exam_penalty[i].penalty = 0;
		for(j=0; j<E; j++) // j starts from 0 to count each combination just two times (otherwise x[j] would not pay penalties for i<j)
			if(abs(x[i]-x[j]) <= 5 && n[i][j])
				exam_penalty[i].penalty += pow(2, abs(x[i]-x[j]))*n[i][j];
	}
}
static void compute_ordered_exam_penalty(ExamPenalty *ordered, ExamPenalty *penalty, int E)
{
	int i, j, k;
	for(i=0; i<E; i++)
		ordered[i].exam = -1; // empty slot
	for(i=0; i<E; i++) // loop on penalty
	{
		for(j=0; ordered[j].exam != -1 && ordered[j].penalty > penalty[i].penalty; j++);
		if(ordered[j].exam == -1) // empty slot
			ordered[j] = penalty[i]; // just insert
		else
		{
			for(k = E-1; k > j; k++)
				ordered[k] = ordered[k-1]; // shift
			ordered[j] = penalty[i]; // insert
		}
	}
}
// 3 NEIGHBORHOOD STRUCTURE ??????
static void compute_worst_penalties(int *x, int **n, int T, int E, Penalty *penalties) // SI PUò FARE IN MODO INCREMENTALE (credo di si)
{
	int i, j;
	for(i=0; i<PENALTIES_NUMBER; i++)
	{
		penalties[i].penalty = 0; penalties[i].exam1 = -1; penalties[i].exam2 = -1;
	}
	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++)
			if(abs(x[i] - x[j]) <= 5 && n[i][j]) // conflictual exams near, penalty paid
				update_penalties_complete(penalties, (int)pow(2, 5-abs(x[i]-x[j]))*n[i][j], i, j);
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

// UTILITIES

static int compute_penalty_complete(int *x, int **n, int E)
{
	int i, j, pen=0;
	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++)
			if(abs(x[i]-x[j]) <= 5 && n[i][j])
				pen += (int)pow(2, 5-abs(x[i]-x[j]))*n[i][j];
	return pen; // we have to count 2 times each combination
}
static int update_penalty(int *x, int **n, int E, int old_pen, int to_swap, int old_timeslot, int new_timeslot) // O(E)
{
	int i;
	for(i=0; i<E; i++)
	{
		if(n[i][to_swap] && abs(x[i]-old_timeslot) <= 5) // old penalty
			old_pen -= pow(2, 5-abs(x[i]-old_timeslot) * n[i][to_swap]);
		if(n[i][to_swap] && abs(x[i]-new_timeslot) <= 5) // new penalty
			old_pen += pow(2, 5-abs(x[i]-new_timeslot) * n[i][to_swap]);
	}
	return old_pen;
}
static void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
