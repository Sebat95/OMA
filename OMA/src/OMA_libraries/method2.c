/*
 * method2.c
 *
 *  Created on: 02 dic 2017
 *      Author: Nicola
 */
#define DEBUG_METHOD2

#define TABU_LENGTH 10
#define MIN_TABU_LENGTH 1
#define MAX_TABU_LENGTH 15

#define N_BEST 100
#define RANDOMNESS_BEST 0.1
#define RANDOMNESS_BEST_MIN 0.05
#define RANDOMNESS_BEST_MAX 0.7
#define TREND_THRESHOLD_BEST_RANDOM_GROUP 0.05

#define N_BEST_SINGLE 200
#define RANDOMNESS_BEST_SINGLE 0.3
#define RANDOMNESS_BEST_SINGLE_MIN 0.2
#define RANDOMNESS_BEST_SINGLE_MAX 0.9
#define TREND_THRESHOLD_BEST_RANDOM_SINGLE 0.1

// matematici
#define ALFA 0.3
#define BETA 0.4
#define ITERATION 100
#define ITERATION_THRESHOLD 5000
//

#define IT_GROUP_BEST_RANDOM 100
#define IT_SINGLE_BEST_RANDOM 100
#define IT_GROUP_RANDOM 100
#define IT_SINGLE_RANDOM 100

#define DESTROY_THRESHOLD 1000
#define DESTROY_GROUP 500
#define DESTROY_SINGLE 500

#define TEMP_PAR 0.995

#include "method2.h"
#include "tabu_search.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

typedef struct
{
	int exam;
	int penalty;
}ExamPenalty;

// ** PROTOTYPES (almost all static)

// NEIGHBORHOOD1
static int neighborhood1_bestOnly(int *x, int **n, int T, int E, TABU tl, int actual_pen);
static int neighborhood1_bestRandom(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double randomness_group);
static int neighborhood1_bestRandom_cheap(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double randomness_group);
static int neighborhood1_bestFirst(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double temperature);
static int neighborhood1_random(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen);
static void neighborhood1_setup(int *x, int **n, int T, int E, int *group_positions, int **group_conflicts);
static int update_pen_groups(int pen, int group1, int group2, int *group_position, int** group_conflicts, int T);

// NEIGHBORHOOD2
static int neighborhood2_bestRandom(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double randomness_single);
static int neighborhood2_bestRandom_cheap(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double randomness_single);
static int neighborhood2_bestFirst(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double temperature);
static int neighborhood2_random(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen);
static void update_exam_penalties(ExamPenalty *exam_penalty, int *x, int **n, int E, int to_swap, int old_timeslot, int new_timeslot);
static void neighborhood2_setup(int *x, int **n, int T, int E, ExamPenalty *exam_penalty);
static void compute_ordered_exam_penalty(ExamPenalty *ordered, ExamPenalty *penalty, int E);

// UTILITIES
static void swap(int* a, int* b);
static int compute_penalty_complete(int *x, int **n, int E);
static int update_penalty(int *x, int **n, int E, int old_pen, int to_swap, int old_timeslot, int new_timeslot);
static void update_parameter(int no_impr_times, double trend, double *randomness_single_P, double *randomness_group_P, int *tabu_length_P, TABU tl);

// ** DEFINITIONS

void optimizationMethod2(int *x, int T, int E, int S, int **n, int *students_per_exam, char *instance_name)
{
	long int iteration_counter = 0;
	double initial_pen, pen, old_pen = 0, best_pen = INT_MAX, trend = -1, B = 0, pen_norm, initial_penalties[ITERATION], tmp, temperature = 1;
	int i, j, improvements_number = 0, partial_iteration = 0, tabu_length = TABU_LENGTH, x_old[E];
	int actual_neighborhood = 0;
	int last_improvements_number = -1, no_improvement_times = 0;
	TABU tl = new_TabuList(TABU_LENGTH, MIN_TABU_LENGTH, MAX_TABU_LENGTH);
	int *group_positions , **group_conflicts;
	double randomness_single = RANDOMNESS_BEST_SINGLE, randomness_group = RANDOMNESS_BEST;
	ExamPenalty *exam_penalty = malloc(E * sizeof(ExamPenalty));

	group_positions = malloc(T * sizeof(int));
	group_conflicts = malloc(T * sizeof(int*));
	for(i=0; i<T; i++)
		group_conflicts[i] = malloc(T * sizeof(int));

	initial_pen = pen = compute_penalty_complete(x, n, E);

	neighborhood1_setup(x, n, T, E, group_positions, group_conflicts);

	while(1)
	{
#ifdef DEBUG_METHOD2
		for(i=0; i<E; i++) x_old[i] = x[i];
		fprintf(stdout, "It:%3d\tNeigh:%d\tTL_len:%d\tRandSingle:%2.2f\tRandGroup:%2.2f\tT:%+.3f\tTemp:%+3.3f\tNoImpr:%d\tPen:%5.3f\tBest:%5.3f\tInit:%5.3f\n", (int)iteration_counter, actual_neighborhood, tabu_length, randomness_single, randomness_group, trend, temperature, no_improvement_times, pen/S, best_pen/S, initial_pen/S);;
#endif

		if(no_improvement_times >= DESTROY_THRESHOLD)
		{
			no_improvement_times = 0;
			partial_iteration = 0;
			neighborhood1_setup(x, n, T, E, group_positions, group_conflicts);
			actual_neighborhood = 2;
		}

		//update_parameter(no_improvement_times, trend, &randomness_single, &randomness_group, &tabu_length, tl);

		switch(actual_neighborhood)
		{
		case 0:
			//if(trend != -1 && partial_iteration > ITERATION_THRESHOLD && abs(trend) < 0.0001)
			if(partial_iteration > IT_GROUP_BEST_RANDOM && trend < TREND_THRESHOLD_BEST_RANDOM_GROUP)
			{
				neighborhood2_setup(x, n, T, E, exam_penalty);
				partial_iteration = 0;
				actual_neighborhood = 1;
				if(improvements_number == last_improvements_number)
				{
					no_improvement_times++;
				}
				else
				{
					last_improvements_number = improvements_number;
					no_improvement_times = (no_improvement_times > 0) ? no_improvement_times-1 : no_improvement_times;
				}
				continue;
			}
			// BEST RANDOM
			if(rand()/(double)RAND_MAX < 0.8)
				pen = neighborhood1_bestRandom(x, n, T, E, tl, group_positions, group_conflicts, pen, randomness_group);
			else
				pen = neighborhood1_bestRandom_cheap(x, n, T, E, tl, group_positions, group_conflicts, pen, randomness_group);
			// BEST FIRST (only the best)
			/*pen = neighborhood1_bestFirst(x, n, T, E, tl, group_positions, group_conflicts, pen, temperature);
			if(pen == -1)
			{
				partial_iteration = IT_GROUP_BEST_RANDOM+1;
				pen = compute_penalty_complete(x, n, E);
				trend = 0;
				continue;
			}*/
			break;
		case 1:
			if(partial_iteration > IT_SINGLE_BEST_RANDOM && trend < TREND_THRESHOLD_BEST_RANDOM_SINGLE)
			{
				neighborhood1_setup(x, n, T, E, group_positions, group_conflicts);
				partial_iteration = 0;
				actual_neighborhood = 0;
				if(improvements_number == last_improvements_number)
				{
					no_improvement_times++;
				}
				else
				{
					last_improvements_number = improvements_number;
					no_improvement_times = (no_improvement_times > 0) ? no_improvement_times-1 : no_improvement_times;
				}
				continue;
			}
			// BEST RANDOM
			if(rand()/(double)RAND_MAX < 0.3)
				pen = neighborhood2_bestRandom(x, n, T, E, tl, exam_penalty, pen, randomness_single);
			else
				pen = neighborhood2_bestRandom_cheap(x, n, T, E, tl, exam_penalty, pen, randomness_single);
			// BEST FIRST
			//pen = neighborhood2_bestFirst(x, n, T, E, tl, exam_penalty, pen, temperature);
			// RANDOM
			//pen = neighborhood2_random(x, n, T, E, tl, exam_penalty, pen);
						break;
		case 2:
			if(partial_iteration > DESTROY_GROUP)
			{
				neighborhood2_setup(x, n, T, E, exam_penalty);
				partial_iteration = 0;
				actual_neighborhood = 3;
				continue;
			}
			pen = neighborhood1_random(x, n, T, E, tl, group_positions, group_conflicts, pen);
			if(pen == -1) // no available moves
			{
				partial_iteration = DESTROY_GROUP;
				pen = compute_penalty_complete(x, n, E);
			}
			break;
		case 3:
			if(partial_iteration > DESTROY_SINGLE)
			{
				neighborhood1_setup(x, n, T, E, group_positions, group_conflicts);
				partial_iteration = 0;
				actual_neighborhood = 0;
				continue;
			}
			pen = neighborhood2_random(x, n, T, E, tl, exam_penalty, pen);
			break;

		}
/*#ifdef DEBUG_METHOD2
		for(i=0; i<E; i++) if(x[i] != x_old[i])break;
		if(i == E) fprintf(stdout, "SOLUZIONE NON MODIFICATA!!!!!###########################");
		for(i=0; i<E; i++)
			for(j=0; j<E; j++)
				if(x[i] == x[j] && n[i][j])
				{
					fprintf(stdout, "SOLUZIONE UNFEASIBLE!!!!@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ò");
					exit(1);
				}
#endif*/
		iteration_counter++;
		partial_iteration++;
		if(best_pen > pen)
		{
			improvements_number++;
			best_pen = pen;
		}

		trend = (double)improvements_number/ (partial_iteration-improvements_number);
		temperature = TEMP_PAR * temperature + (1-TEMP_PAR) * (1000000 * improvements_number/iteration_counter * (double)(abs(pen-old_pen))/pen);
		old_pen = pen;
		// MATEMATICI
		/*if(partial_iteration <= ITERATION)
		{
			//pen_norm = pen;
			pen_norm = 1;
			initial_penalties[partial_iteration-1] = pen/S;
			if(iteration_counter == ITERATION)
			{
				trend = (pen/(pen_norm*S) - initial_penalties[0]/pen_norm) / (ITERATION-1);
				for(i=0; i<ITERATION; i++)
					B += initial_penalties[i]/pen_norm - (i+1)*trend;
				B = B / ITERATION;
			}
		}
		else
		{
			tmp = B;
			B = ALFA*pen/(S*pen_norm) + (1-ALFA)*(tmp + trend);
			trend = BETA*(B - tmp) + (1-BETA) * trend;

			if(trend > 0.01 && tabu_length < MAX_TABU_LENGTH)
				tabu_length = increase_TabuList(tl);
			if(trend < 0.0001 && tabu_length < MIN_TABU_LENGTH)
				tabu_length = decrease_TabuList(tl);
		}*/
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
static int neighborhood1_bestRandom(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double randomness_group) // O(T^3)
{
	int i, j, group1, group2, actual_pen, moves = 0;
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
				if(i != N_BEST) // if this move is better then the other N_BEST
				{
					for(j=moves; j > i; j--)
					{	N_best[j][0] = N_best[j-1][0]; N_best[j][1] = N_best[j-1][1]; N_best[j][2] = N_best[j-1][2]; }
					N_best[i][0] = actual_pen; N_best[i][1] = group1; N_best[i][2] = group2;
					if(moves < N_BEST-1)
						moves++;
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
		if(rand()/(double)RAND_MAX < randomness_group)
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
static int neighborhood1_bestRandom_cheap(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double randomness_group) // O(T^2)
{
	int i, j, group1, group2, actual_pen, moves = 0;
	int N_best[N_BEST][3];

	for(i=0; i<N_BEST; i++) // setup
	{
		N_best[i][0] = INT_MAX, N_best[i][1] = N_best[i][2] = -1;
	}
	for(group1 = rand() % T; group2 != T; group1 = (group1+1)%T)
			for(group2 = 0; group2 < T; group2++)
			{
				if(group2 == group1 || check_TabuList(tl, (group_position[group1]<group_position[group2])?group_position[group1]:group_position[group2], (group_position[group1]>group_position[group2])?group_position[group1]:group_position[group2], 1))
					continue; // timeslots group_position[group1] and group_position[group2] already swapped in the last moves (is referred to timeslot, not to group)

				actual_pen = update_pen_groups(pen, group1, group2, group_position, group_conflict, T); // compute how much the penalty would be if I swap group1 and group2

				// update N_best array (if this swap is in the N best)
				for(i=0; i<N_BEST && N_best[i][1] != -1 && N_best[i][0] < actual_pen; i++);
				if(i != N_BEST) // if this move is better then the other N_BEST
				{
					for(j=moves; j > i; j--)
					{	N_best[j][0] = N_best[j-1][0]; N_best[j][1] = N_best[j-1][1]; N_best[j][2] = N_best[j-1][2]; }
					N_best[i][0] = actual_pen; N_best[i][1] = group1; N_best[i][2] = group2;
					if(moves < N_BEST-1)
						moves++;
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
		if(rand()/(double)RAND_MAX < randomness_group)
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
static int neighborhood1_bestFirst(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double temperature) //O(T^3) DA MODIFICARE AGGIUNGENDO LA TEMPERATURA
{
	int j, group1, group2, actual_pen;
	double prob;

	for(group1 = 0; group1 < T; group1++)
			for(group2 = group1+1; group2 < T; group2++)
			{
				if(check_TabuList(tl, (group_position[group1]<group_position[group2])?group_position[group1]:group_position[group2], (group_position[group1]>group_position[group2])?group_position[group1]:group_position[group2], 1))
					continue; // timeslots group_position[group1] and group_position[group2] already swapped in the last moves (is referred to timeslot, not to group)

				actual_pen = update_pen_groups(pen, group1, group2, group_position, group_conflict, T); // compute how much the penalty would be if I swap group1 and group2

				if(temperature != 0)
					prob = pow(M_E, -((double)(pen-actual_pen)/(pen*temperature)));
				else
					prob = INT_MAX;
				if(actual_pen < pen || rand()/(double)RAND_MAX < prob)
				{
					insert_TabuList(tl, (group_position[group1]<group_position[group2])?group_position[group1]:group_position[group2], (group_position[group1]>group_position[group2])?group_position[group1]:group_position[group2], 1);
					for(j=0; j<E; j++) // perform the move
						{
							if(x[j] == group_position[group1])
								x[j] = group_position[group2];
							else if(x[j] == group_position[group2])
								x[j] = group_position[group1];
						}
					swap(group_position+group1, group_position+group2); // update group_position
					return actual_pen;
				}
			}
	return -1; // return the penalty
}
static int neighborhood1_random(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen) // O(T)
{
	int j, group1, group2, actual_pen = -1, possible_group1, possible_group2 = 0;
	possible_group1 = group1 = rand() % T;
	do
	{
		group1 = (group1 + 1) % T;
		possible_group2 = rand() % T;
		for(group2 = possible_group2 +1; group2 != possible_group2; group2 = (group2+1)%T)
		{
			if(group2 == group1 || check_TabuList(tl, (group_position[group1]<group_position[group2])?group_position[group1]:group_position[group2], (group_position[group1]>group_position[group2])?group_position[group1]:group_position[group2], 1))
				continue;
			break;
		}
		if(group2 == possible_group2 || group2 == group1 || group2 == T)
			continue;

		actual_pen = update_pen_groups(pen, group1, group2, group_position, group_conflict, T); // compute how much the penalty would be if I swap group1 and group2
		insert_TabuList(tl, (group_position[group1]<group_position[group2])?group_position[group1]:group_position[group2], (group_position[group1]>group_position[group2])?group_position[group1]:group_position[group2], 1);
		for(j=0; j<E; j++) // perform the move
			{
				if(x[j] == group_position[group1])
					x[j] = group_position[group2];
				else if(x[j] == group_position[group2])
					x[j] = group_position[group1];
			}
		swap(group_position+group1, group_position+group2); // update group_position
		break;
	}while(group1 != possible_group1);
	if(group1 == possible_group1)
		return -1;
	return actual_pen; // return the penalty

}
static int update_pen_groups(int pen, int group1, int group2, int *group_position, int** group_conflicts, int T) // O(T)
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
static void neighborhood1_setup(int *x, int **n, int T, int E, int *group_positions, int **group_conflicts) // O(E^2*T^2)
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
static int neighborhood2_bestRandom(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double randomness_single) // O(E^2*T)
{
	int i, j, k, to_swap, pen, chosen = 0, moves = 1;
	int N_best[N_BEST_SINGLE][3];
	to_swap = -1;
	/*ExamPenalty *ordered_exam_penalty = malloc(E * sizeof(ExamPenalty));
	compute_ordered_exam_penalty(ordered_exam_penalty, exam_penalty, E);*/
	for(i=0; i<N_BEST_SINGLE; i++)
		N_best[i][0] = -1;

	while(chosen != E)
	{
		to_swap = exam_penalty[chosen].exam; // to_swap is one of the most "costly" exam (exam_penalty is ordered)
		for(i=0; i<T; i++)
		{
			for(j=0; j<E; j++)
				if(x[to_swap] == i || (x[j] == i && n[j][to_swap]) || check_TabuList(tl, to_swap, i, 0))
					break; // unfeasible swap or not allowed
			if(j != E)
				continue;

			pen = update_penalty(x, n, E, actual_pen, to_swap, x[to_swap], i);

			for(j=0; j<moves && N_best[j][0] != -1 && pen > N_best[j][0]; j++); // j is where I have to insert in N_best
			if(j != moves)
			{
				for(k=moves; k>j; k--)
				{
					N_best[k][0] = N_best[k-1][0]; N_best[k][1] = N_best[k-1][1]; N_best[k][2] = N_best[k-1][2];//shift
				}
				N_best[j][0] = pen; N_best[j][1] = i; N_best[j][2] = to_swap;
				if(moves < N_BEST_SINGLE-1)
					moves++; // number of storede moves in N_best
			}
		}
	chosen++;
	}
	while(1)
	{
		for(i=0; i<N_BEST_SINGLE && N_best[i][0] != -1; i++)
		{
			if(rand()/(double)RAND_MAX < randomness_single)
				continue;
			insert_TabuList(tl, N_best[i][2], N_best[i][1], 0);
			update_exam_penalties(exam_penalty, x, n, E, N_best[i][2], x[N_best[i][2]], N_best[i][1]);
			x[N_best[i][2]] = N_best[i][1];
			break;
		}
		if(i != N_BEST_SINGLE)
			if(N_best[i][0] != -1)
				break;
	}
	//free(ordered_exam_penalty);
	return N_best[i][0];
}
static int neighborhood2_bestRandom_cheap(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double randomness_single) // O(E*T)
{
	int i, j, k, to_swap, pen, chosen = E, moves = 1;
	int N_best[N_BEST_SINGLE][2], max_pen = -1;
	to_swap = -1;
	/*ExamPenalty *ordered_exam_penalty = malloc(E * sizeof(ExamPenalty));
	compute_ordered_exam_penalty(ordered_exam_penalty, exam_penalty, E);
	while(chosen == E)
	{
		for(chosen = 0; chosen < E; chosen++)
		{
			if(rand()/(double)RAND_MAX < randomness_single)
				continue;
			to_swap = ordered_exam_penalty[chosen].exam;
			break;
		}
	}*/
	for(i=0; i<E; i++)
		if(exam_penalty[i].penalty > max_pen)
		{
			max_pen = exam_penalty[i].penalty;
			to_swap = exam_penalty[i].exam;
		}
	for(i=0; i<N_BEST_SINGLE; i++)
		N_best[i][0] = -1;

	while(chosen == E)
	{
		for(i=0; i<T; i++)
		{
			for(j=0; j<E; j++)
				if(x[to_swap] == i || (x[j] == i && n[j][to_swap]) || check_TabuList(tl, to_swap, i, 0))
					break; // unfeasible swap or not allowed
			if(j != E)
				continue;

			pen = update_penalty(x, n, E, actual_pen, to_swap, x[to_swap], i);

			for(j=0; j<moves && N_best[j][0] != -1 && pen > N_best[j][0]; j++); // j is where I have to insert in N_best
			if(j != moves)
			{
				for(k=moves; k>j; k--)
				{
					N_best[k][0] = N_best[k-1][0]; N_best[k][1] = N_best[k-1][1]; //shift
				}
				N_best[j][0] = pen; N_best[j][1] = i;
				if(moves < N_BEST_SINGLE-1)
					moves++; // number of storede moves in N_best
			}
		}
		if(N_best[0][0] == -1) // no move found
			to_swap = (to_swap+1) % E; // try with another
		else
			chosen = 0; // break the cycle
	}
	while(1)
	{
		for(i=0; i<N_BEST_SINGLE && N_best[i][0] != -1; i++)
		{
			if(rand()/(double)RAND_MAX < randomness_single)
				continue;
			insert_TabuList(tl, to_swap, N_best[i][1], 0);
			update_exam_penalties(exam_penalty, x, n, E, to_swap, x[to_swap], N_best[i][1]);
			x[to_swap] = N_best[i][1];
			break;
		}
		if(i != N_BEST_SINGLE)
			if(N_best[i][0] != -1)
				break;
	}
	//free(ordered_exam_penalty);
	return N_best[i][0];
}
static int neighborhood2_bestFirst(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double temperature)
{
	int i, j, k, to_swap, pen;
	double prob;
	to_swap = -1;
	k = to_swap = rand() % T;
	while(1)
	{
		to_swap = (to_swap+1) % T;
		for(i=0; i<T; i++)
		{
			for(j=0; j<E; j++)
				if(x[to_swap] == i || (x[j] == i && n[j][to_swap]) || check_TabuList(tl, to_swap, i, 0))
					break; // unfeasible swap or not allowed
			if(j != E)
				continue;

			pen = update_penalty(x, n, E, actual_pen, to_swap, x[to_swap], i);

			if(temperature != 0)
				prob = pow(M_E, -((double)(pen-actual_pen)/(pen*temperature)));
			else
				prob = INT_MAX;
			if(actual_pen < pen || rand()/(double)RAND_MAX < prob)
			{
				insert_TabuList(tl, to_swap, i, 0);
				x[to_swap] = i;
				return pen;
			}
		}
		if(to_swap == k)
			return actual_pen;
	}
	return pen;
}
static int neighborhood2_random(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen)
{
	int i, j, k, to_swap, pen;
	to_swap = -1;
	k = to_swap = rand() % T;
	while(1)
	{
		to_swap = (to_swap+1) % T;
		for(i=0; i<T; i++)
		{
			for(j=0; j<E; j++)
				if(x[to_swap] == i || (x[j] == i && n[j][to_swap]) || check_TabuList(tl, to_swap, i, 0))
					break; // unfeasible swap or not allowed
			if(j != E)
				continue;

			pen = update_penalty(x, n, E, actual_pen, to_swap, x[to_swap], i);
			insert_TabuList(tl, to_swap, i, 0);
			x[to_swap] = i;
			return pen;
		}
		if(to_swap == k)
			return actual_pen;
	}
	return pen;
}
static void update_exam_penalties(ExamPenalty *exam_penalty, int *x, int **n, int E, int to_swap, int old_timeslot, int new_timeslot) // O(E) update exam_penalty array when I move exam to_swap in new_timeslot
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
static void neighborhood2_setup(int *x, int **n, int T, int E, ExamPenalty *exam_penalty) // O(E^2)
{
	int i, j;
	for(i=0; i<E; i++)
	{
		exam_penalty[i].exam = i; exam_penalty[i].penalty = 0;
		for(j=0; j<E; j++) // j starts from 0 to count each combination just two times (otherwise x[j] would not pay penalties for i<j)
			if(abs(x[i]-x[j]) <= 5 && n[i][j])
				exam_penalty[i].penalty += pow(2, 5-abs(x[i]-x[j]))*n[i][j];
	}
}
static void compute_ordered_exam_penalty(ExamPenalty *ordered, ExamPenalty *penalty, int E) // O(E^2)
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
			for(k = E-1; k > j; k--)
				ordered[k] = ordered[k-1]; // shift
			ordered[j] = penalty[i]; // insert
		}
	}
}

// UTILITIES

static int compute_penalty_complete(int *x, int **n, int E) // O(E^2)
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
			old_pen -= pow(2, 5-abs(x[i]-old_timeslot)) * n[i][to_swap];
		if(n[i][to_swap] && abs(x[i]-new_timeslot) <= 5) // new penalty
			old_pen += pow(2, 5-abs(x[i]-new_timeslot)) * n[i][to_swap];
	}
	return old_pen;
}
static void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
static void update_parameter(int no_impr_times, double trend, double *randomness_single_P, double *randomness_group_P, int *tabu_length_P, TABU tl)
{
	// NOTA: PER ORA TREND NON è USATO
	double var = (double) no_impr_times / DESTROY_THRESHOLD; // between 0 and 1

	*tabu_length_P = update_TabuList(tl, MIN_TABU_LENGTH + var*(MAX_TABU_LENGTH - MIN_TABU_LENGTH));
	//*tabu_length_P = update_TabuList(tl, MAX_TABU_LENGTH + var*(MIN_TABU_LENGTH - MAX_TABU_LENGTH));
	//*randomness_single_P = RANDOMNESS_BEST_SINGLE_MIN + var * (RANDOMNESS_BEST_SINGLE_MAX - RANDOMNESS_BEST_SINGLE_MIN);
	//*randomness_group_P = RANDOMNESS_BEST_MIN + var * (RANDOMNESS_BEST_MAX - RANDOMNESS_BEST_MIN);
}
