#define PRINT_FINAL_BEST

#define TABU_LENGTH 10
#define MIN_TABU_LENGTH 3
#define MAX_TABU_LENGTH 10

#define N_BEST 10
#define RANDOMNESS_BEST 0.1
#define RANDOMNESS_BEST_MIN 0.05
#define RANDOMNESS_BEST_MAX 0.2

#define N_BEST_SINGLE 10
#define RANDOMNESS_BEST_SINGLE 0.2
#define RANDOMNESS_BEST_SINGLE_MIN 0.1
#define RANDOMNESS_BEST_SINGLE_MAX 0.5

#define RANDOMNESS_RANDOMorCHEAP_GROUP 0.8
#define RANDOMNESS_RANDOMorCHEAP_GROUP_MIN 0.6
#define RANDOMNESS_RANDOMorCHEAP_GROUP_MAX 0.9
#define RANDOMNESS_RANDOMorCHEAP_SINGLE 0.5
#define RANDOMNESS_RANDOMorCHEAP_SINGLE_MIN 0.2
#define RANDOMNESS_RANDOMorCHEAP_SINGLE_MAX 0.7

#define IT_GROUP_BEST_RANDOM 50
#define IT_SINGLE_BEST_RANDOM 100

#define NO_IMPR_DECREASE 100
#define DESTROY_THRESHOLD 100

#define DYNAMIC_PARAMETERS 1
#define DYNAMIC_TABULIST 1
#define TABULIST_INCREASING 1
#define DYNAMIC_RANDOMNESS_SINGLE 0
#define RANDOMNESS_SINGLE_INCREASING 0
#define DYNAMIC_RANDOMNESS_GROUP 0
#define RANDOMNESS_GROUP_INCREASING 0
#define DYNAMIC_RANDOMorCHEAP_SINGLE 1
#define RANDOMorCHEAP_SINGLE_INCREASING 0
#define DYNAMIC_RANDOMorCHEAP_GROUP 1
#define RANDOMorCHEAP_GROUP_INCREASING 0

#include "method2.h"
#include "tabu_search.h"
#include "initialization.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <omp.h>
#include <time.h>

typedef struct {
	int exam;
	int penalty;
} ExamPenalty;

// ** PROTOTYPES (almost all static)

// NEIGHBORHOOD1
static int neighborhood1_bestRandom(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double randomness_group);// O(T^3)
static int neighborhood1_bestRandom_cheap(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double randomness_group); // O(T^2)
static void neighborhood1_setup(int *x, int **n, int T, int E, int *group_positions, int **group_conflicts); // O(E^2*T^2)
static int update_pen_groups(int pen, int group1, int group2, int *group_position, int** group_conflicts, int T); // O(T)

// NEIGHBORHOOD2
static int neighborhood2_bestRandom(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double randomness_single); // O(E^2*T)
static int neighborhood2_bestRandom_cheap(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double randomness_single); // O(E*T)
static void update_exam_penalties(ExamPenalty *exam_penalty, int *x, int **n, int E, int to_swap, int old_timeslot, int new_timeslot); // O(E)
static void neighborhood2_setup(int *x, int **n, int T, int E, ExamPenalty *exam_penalty); // O(E^2)

// UTILITIES
static void swap(int* a, int* b);
static int compute_penalty_complete(int *x, int **n, int E); // O(E^2)
static int update_penalty(int *x, int **n, int E, int old_pen, int to_swap, int old_timeslot, int new_timeslot); // O(E)
static void update_parameter(int no_impr_times, double *randomness_single_P, double *randomness_group_P, int *tabu_length_P, TABU tl, double *randomness_randomOrCheap_group_P, double *randomness_randomOrCheap_single_P);
static void destroySolution_swappingRandom(int *x, int **n, int E, int T, TABU tl);
static void printBestSolution(int *x, int E, char *instance_name);

void optimizationMethod2(int *x, int T, int E, int S, int **n, char *instance_name, double ex_time) 
{
	double total_best_pen = INT_MAX;
	double end = ex_time*CLOCKS_PER_SEC + (double)clock(); // set end time in clock cycle number for timeout
	double pen, best_pen = INT_MAX;//, total_best_pen = INT_MAX;
	int i, improvements_number = 0, partial_iteration = 0, tabu_length = TABU_LENGTH, x_best[E];
	int actual_neighborhood = 0;
	int last_improvements_number = -1, no_improvement_times = 0;
	TABU tl = new_TabuList(TABU_LENGTH, MIN_TABU_LENGTH, MAX_TABU_LENGTH);
	int *group_positions, **group_conflicts;
	double randomness_single = RANDOMNESS_BEST_SINGLE, randomness_group = RANDOMNESS_BEST, randomness_randomOrCheap_group = RANDOMNESS_RANDOMorCHEAP_GROUP, randomness_randomOrCheap_single = RANDOMNESS_RANDOMorCHEAP_SINGLE;
	ExamPenalty *exam_penalty = malloc(E * sizeof(ExamPenalty));

	// setup neighborhood1
	group_positions = malloc(T * sizeof(int)); // used in neighborhood1 in order to understand where "group of exams" have been moved
	group_conflicts = malloc(T * sizeof(int*)); // used in neighborhood1 in order to efficiently compute penalty
	for (i = 0; i < T; i++)
		group_conflicts[i] = malloc(T * sizeof(int));
	neighborhood1_setup(x, n, T, E, group_positions, group_conflicts); // the algorithm starts from neighborhood1
	pen = compute_penalty_complete(x, n, E);

	// start optimization metaheuristic
	while((double)clock() < end)
	{
		// detect local minimum and implement multistart
		if (no_improvement_times >= DESTROY_THRESHOLD)
		{
			no_improvement_times = 0;
			update_parameter(no_improvement_times,  &randomness_single, &randomness_group, &tabu_length, tl, &randomness_randomOrCheap_group, &randomness_randomOrCheap_single);
			partial_iteration = 0;
			best_pen = INT_MAX; //reset the best
			destroySolution_swappingRandom(x, n, E, T, tl);
			pen = compute_penalty_complete(x, n, E);

			//setup again data structure
			actual_neighborhood = 0;
			neighborhood1_setup(x, n, T, E, group_positions, group_conflicts);
			continue; // go to the next iteration of the while
		}

		switch (actual_neighborhood) {
		case 0:
			if (partial_iteration > IT_GROUP_BEST_RANDOM) // change neighborhood structure
			{
				// setup neighborhood 2
				neighborhood2_setup(x, n, T, E, exam_penalty);
				partial_iteration = 0;
				actual_neighborhood = 1;
				// update no_improvement_times, used in order to detect local minimum
				if (improvements_number == last_improvements_number)
				{
					no_improvement_times++;
					update_parameter(no_improvement_times,  &randomness_single, &randomness_group, &tabu_length, tl, &randomness_randomOrCheap_group, &randomness_randomOrCheap_single);
				}
				else
				{
					last_improvements_number = improvements_number;
					no_improvement_times =
							(no_improvement_times > NO_IMPR_DECREASE) ?
									no_improvement_times - NO_IMPR_DECREASE :
									0;
				update_parameter(no_improvement_times,  &randomness_single, &randomness_group, &tabu_length, tl, &randomness_randomOrCheap_group, &randomness_randomOrCheap_single);
				}
				continue;
			}
			// swap two timeslots
			if(rand()/(double)RAND_MAX < randomness_randomOrCheap_group)
				pen = neighborhood1_bestRandom(x, n, T, E, tl, group_positions, group_conflicts, pen, randomness_group); // consider all swaps
			else
				pen = neighborhood1_bestRandom_cheap(x, n, T, E, tl, group_positions, group_conflicts, pen, randomness_group); // consider all swaps for a randomly selected timeslot
			break;
		case 1:
			if (partial_iteration > IT_SINGLE_BEST_RANDOM) // change neighborhood structure
			{
				// setup neighborhood 1
				neighborhood1_setup(x, n, T, E, group_positions, group_conflicts);
				partial_iteration = 0;
				actual_neighborhood = 0;
				// update no_improvement_times, used in order to detect local minimum
				if (improvements_number == last_improvements_number)
				{
					no_improvement_times++;
					update_parameter(no_improvement_times,  &randomness_single, &randomness_group, &tabu_length, tl, &randomness_randomOrCheap_group, &randomness_randomOrCheap_single);
				}
				else
				{
					last_improvements_number = improvements_number;
					no_improvement_times = (no_improvement_times > NO_IMPR_DECREASE) ?
							no_improvement_times - NO_IMPR_DECREASE :
							0;
					update_parameter(no_improvement_times,  &randomness_single, &randomness_group, &tabu_length, tl, &randomness_randomOrCheap_group, &randomness_randomOrCheap_single);
				}
				continue;
			}
			// move a single exam
			if (rand() / (double) RAND_MAX < randomness_randomOrCheap_single)
				pen = neighborhood2_bestRandom(x, n, T, E, tl, exam_penalty, pen, randomness_single); // consider all moves
			else
				pen = neighborhood2_bestRandom_cheap(x, n, T, E, tl, exam_penalty, pen, randomness_single); // consider all moves for a randomly selected exam
			break;
		}

		// update counter and best solution
		partial_iteration++;
		if (best_pen > pen)
		{
			improvements_number++; // used in order to understand if we are getting into a flat region or not
			best_pen = pen;
			memcpy(x_best, x, E*sizeof(int));
			if(total_best_pen > best_pen)
			{
				total_best_pen = best_pen;
				printBestSolution(x, E, instance_name);
			}
		}
	}

#ifdef PRINT_FINAL_BEST
	printf("BEST = %f\n", total_best_pen / S);
#endif
	// free allocated memory
	for(i=0; i<T; i++)
		free(group_conflicts[i]);
	free(group_conflicts);
	free(group_positions);
	delete_TabuList(tl);
}

// 1 NEIGHBORHOOD STRUCTURE: to swap entire group of exams (corrisponding to a timeslot)
static int neighborhood1_bestRandom(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double randomness_group) // O(T^3)
{
	int i, j, group1, group2, actual_pen, moves = 0;
	int N_best[N_BEST][3];

	for (i = 0; i < N_BEST; i++) // setup
		N_best[i][0] = INT_MAX, N_best[i][1] = N_best[i][2] = -1;

	int threads = omp_get_max_threads();
#pragma omp parallel num_threads(threads) default(none) shared(n, T, E, tl, N_best, group_position, group_conflict, pen, moves) private(group1, group2, actual_pen, i, j)
	{
#pragma omp for schedule(dynamic)
	for (group1 = 0; group1 < T; group1++)
		for (group2 = group1 + 1; group2 < T; group2++)
		{
			if (check_TabuList(tl,
					(group_position[group1] < group_position[group2]) ?
							group_position[group1] : group_position[group2],
							(group_position[group1] > group_position[group2]) ?
									group_position[group1] : group_position[group2], 1))
				continue; // timeslots group_position[group1] and group_position[group2] already swapped in the last moves (is referred to timeslot, not to group)

			actual_pen = update_pen_groups(pen, group1, group2, group_position, group_conflict, T); // compute how much the penalty would be if I swap group1 and group2

	#pragma omp critical
			{
				// update N_best array (if this swap is in the N best)
				for (i = 0; i < N_BEST && N_best[i][1] != -1 && N_best[i][0] < actual_pen; i++);
				if (i != N_BEST) // if this move is better then the other N_BEST
				{
					for (j = moves; j > i; j--)
					{
						N_best[j][0] = N_best[j - 1][0];
						N_best[j][1] = N_best[j - 1][1];
						N_best[j][2] = N_best[j - 1][2];
					}
					N_best[i][0] = actual_pen;
					N_best[i][1] = group1;
					N_best[i][2] = group2;
					if (moves < N_BEST - 1)
						moves++;
				}
			}
		}
	}
	// select one of the best moves
	for (i = 0;; i = (i + 1) % N_BEST)
	{
		if (N_best[i][1] == -1) // last element of N_best are empty, go back to the beginning
		{
			i = 0;
			continue;
		}
		if (rand() / (double) RAND_MAX < randomness_group) // randomness_group is used to select one of the best allowed moves
			continue;
		insert_TabuList(tl,
				(group_position[N_best[i][1]] < group_position[N_best[i][2]]) ?
						group_position[N_best[i][1]] :
						group_position[N_best[i][2]],
						(group_position[N_best[i][1]] > group_position[N_best[i][2]]) ?
								group_position[N_best[i][1]] :
								group_position[N_best[i][2]], 1);
		for (j = 0; j < E; j++) // perform the move
		{
			if (x[j] == group_position[N_best[i][1]])
				x[j] = group_position[N_best[i][2]];
			else if (x[j] == group_position[N_best[i][2]])
				x[j] = group_position[N_best[i][1]];
		}
		swap(group_position + N_best[i][1], group_position + N_best[i][2]); // update group_position
		break;
	}
	return N_best[i][0]; // return the penalty
}

static int neighborhood1_bestRandom_cheap(int *x, int **n, int T, int E, TABU tl, int *group_position, int **group_conflict, int pen, double randomness_group) // O(T^2)
{
	int i, j, group1, group2, actual_pen, moves = 0;
	int N_best[N_BEST][3];

	for (i = 0; i < N_BEST; i++) // setup
		N_best[i][0] = INT_MAX, N_best[i][1] = N_best[i][2] = -1;

	for (group1 = rand() % T; group2 != T; group1 = (group1 + 1) % T)
		for (group2 = 0; group2 < T; group2++)
		{
			if (group2 == group1 || check_TabuList(tl,(group_position[group1] < group_position[group2]) ? group_position[group1] : group_position[group2], (group_position[group1] > group_position[group2]) ? group_position[group1] : group_position[group2], 1))
				continue; // timeslots group_position[group1] and group_position[group2] already swapped in the last moves (is referred to timeslot, not to group)

			actual_pen = update_pen_groups(pen, group1, group2, group_position,	group_conflict, T); // compute how much the penalty would be if I swap group1 and group2

			// update N_best array (if this swap is in the N best)
			for (i = 0;	i < N_BEST && N_best[i][1] != -1 && N_best[i][0] < actual_pen; i++);

			if (i != N_BEST) // if this move is better then the other N_BEST
			{
				for (j = moves; j > i; j--) {
					N_best[j][0] = N_best[j - 1][0];
					N_best[j][1] = N_best[j - 1][1];
					N_best[j][2] = N_best[j - 1][2];
				}
				N_best[i][0] = actual_pen;
				N_best[i][1] = group1;
				N_best[i][2] = group2;
				if (moves < N_BEST - 1)
					moves++;
			}
		}
	// select one of the best moves
	for (i = 0; ; i = (i + 1) % N_BEST)
	{
		if (N_best[i][1] == -1)
		{
			i = 0;
			continue;
		}
		if (rand() / (double) RAND_MAX < randomness_group) // randomness_group is used to select one of the best allowed moves
			continue;
		insert_TabuList(tl,
				(group_position[N_best[i][1]] < group_position[N_best[i][2]]) ?
						group_position[N_best[i][1]] :
						group_position[N_best[i][2]],
						(group_position[N_best[i][1]] > group_position[N_best[i][2]]) ?
								group_position[N_best[i][1]] :
								group_position[N_best[i][2]], 1);
		for (j = 0; j < E; j++) // perform the move
		{
			if (x[j] == group_position[N_best[i][1]])
				x[j] = group_position[N_best[i][2]];
			else if (x[j] == group_position[N_best[i][2]])
				x[j] = group_position[N_best[i][1]];
		}
		swap(group_position + N_best[i][1], group_position + N_best[i][2]); // update group_position
		break;
	}
	return N_best[i][0]; // return the penalty
}

static int update_pen_groups(int pen, int group1, int group2, int *group_position, int** group_conflicts, int T) // O(T)
{
	int i, i_lower1, i_higher1, i_lower2, i_higher2;
	int group1_position = group_position[group1], group2_position =	group_position[group2];
	int timeslot_group[T];
	for (i = 0; i < T; i++)
		timeslot_group[group_position[i]] = i; // in timeslot group_position[i] there is group of exam i
	i_lower1 = (group1_position > 5) ? group1_position - 5 : 0;
	i_higher1 = (group1_position + 5 < T) ? group1_position + 5 : T - 1;
	for (i = i_lower1; i <= i_higher1; i++) {
		pen -= pow(2, 5 - abs(group1_position - i))
						* group_conflicts[timeslot_group[i]][group1];
	}
	i_lower2 = (group2_position > 5) ? group2_position - 5 : 0;
	i_higher2 = (group2_position + 5 < T) ? group2_position + 5 : T - 1;
	for (i = i_lower2; i <= i_higher2; i++) {
		pen -= pow(2, 5 - abs(group2_position - i))
						* group_conflicts[timeslot_group[i]][group2];
	}
	swap(timeslot_group + group1_position, timeslot_group + group2_position);
	for (i = i_lower1; i <= i_higher1; i++)
		pen += pow(2, 5 - abs(group1_position - i))
		* group_conflicts[timeslot_group[i]][group2];
	for (i = i_lower2; i <= i_higher2; i++)
		pen += pow(2, 5 - abs(group2_position - i))
		* group_conflicts[timeslot_group[i]][group1];
	return pen;
}

static void neighborhood1_setup(int *x, int **n, int T, int E, int *group_positions, int **group_conflicts) // O(E^2*T^2)
{
	int i, j, ii, jj;
	for (i = 0; i < T; i++) {
		group_positions[i] = i;
	}
	for (i = 0; i < T; i++)
		for (j = 0; j < T; j++)
			group_conflicts[i][j] = 0;
#pragma omp parallel default(none) shared(group_conflicts, n, E, T, x) private(i, j, ii, jj)
	{
	#pragma omp for
		for (i = 0; i < T; i++)
			for (j = i + 1; j < T; j++)
				for (ii = 0; ii < E; ii++)
					if (x[ii] == i)
						for (jj = 0; jj < E; jj++)
							if (x[jj] == j && n[ii][jj]) {
								group_conflicts[i][j] += n[ii][jj];
								group_conflicts[j][i] += n[ii][jj];
							}
	}
}

// 2 NEIGHBORHOOD STRUCTURE: to move single exams
static int neighborhood2_bestRandom(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double randomness_single) // O(E^2*T)
{
	int i, j, k, to_swap, pen, chosen = 0, moves = 1;
	int N_best[N_BEST_SINGLE][3];


	for (i = 0; i < N_BEST_SINGLE; i++)
		N_best[i][0] = -1;

	int threads=omp_get_max_threads();
#pragma omp parallel num_threads(threads) default(none) shared(T, E, x, N_best, moves, tl, actual_pen, randomness_single, n) private(to_swap, k, i, j, pen)
	{
		to_swap = -1;
	#pragma omp for schedule(dynamic) collapse(2)
		for(chosen = 0; chosen < E; chosen++)
		{
			for (i = 0; i < T; i++)
			{
				for (j = 0; j < E; j++){
					to_swap = chosen;
					if (x[to_swap] == i || (x[j] == i && n[j][to_swap])	|| check_TabuList(tl, to_swap, i, 0))
						break; // unfeasible swap or not allowed
				}

				if (j != E)
					continue;

				pen = update_penalty(x, n, E, actual_pen, to_swap, x[to_swap], i);

#pragma omp critical
				{
					for (j = 0; j < moves && N_best[j][0] != -1 && pen > N_best[j][0]; j++); // j is where I have to insert in N_best
					if (j != moves)
					{
						for (k = moves; k > j; k--)
						{
							N_best[k][0] = N_best[k - 1][0];
							N_best[k][1] = N_best[k - 1][1];
							N_best[k][2] = N_best[k - 1][2]; //shift
						}
						N_best[j][0] = pen;
						N_best[j][1] = i;
						N_best[j][2] = to_swap;
						if (moves < N_BEST_SINGLE - 1)
							moves++; // number of storede moves in N_best
					}
				}
			}
		}
	}

	while (1)
	{
		for (i = 0; i < N_BEST_SINGLE && N_best[i][0] != -1; i++)
		{
			if (rand() / (double) RAND_MAX < randomness_single) // randomness_single is used to select one of the best allowed moves
				continue;
			insert_TabuList(tl, N_best[i][2], N_best[i][1], 0);
			update_exam_penalties(exam_penalty, x, n, E, N_best[i][2],
					x[N_best[i][2]], N_best[i][1]);
			x[N_best[i][2]] = N_best[i][1];
			break;
		}
		if (i != N_BEST_SINGLE)
			if (N_best[i][0] != -1)
				break; // the move has been done
	}
	return N_best[i][0];
}

static int neighborhood2_bestRandom_cheap(int *x, int **n, int T, int E, TABU tl, ExamPenalty *exam_penalty, int actual_pen, double randomness_single) // O(E*T)
{
	int i, j, k, to_swap, pen, chosen = E, moves = 1;
	int N_best[N_BEST_SINGLE][2], max_pen = -1;
	to_swap = -1;

	for (i = 0; i < E; i++)
		if (exam_penalty[i].penalty > max_pen) {
			max_pen = exam_penalty[i].penalty;
			to_swap = exam_penalty[i].exam;
	}
	for (i = 0; i < N_BEST_SINGLE; i++)
		N_best[i][0] = -1;

	while (chosen == E)
	{
		for (i = 0; i < T; i++)
		{
			for (j = 0; j < E; j++)
				if (x[to_swap] == i || (x[j] == i && n[j][to_swap]) || check_TabuList(tl, to_swap, i, 0))
					break; // unfeasible swap or not allowed
			if (j != E)
				continue;

			pen = update_penalty(x, n, E, actual_pen, to_swap, x[to_swap], i);

			for (j = 0; j < moves && N_best[j][0] != -1 && pen > N_best[j][0]; j++); // j is where I have to insert in N_best
			if (j != moves)
			{
				for (k = moves; k > j; k--)
				{
					N_best[k][0] = N_best[k - 1][0];
					N_best[k][1] = N_best[k - 1][1]; //shift
				}
				N_best[j][0] = pen;
				N_best[j][1] = i;
				if (moves < N_BEST_SINGLE - 1)
					moves++; // number of stored moves in N_best
			}
		}
		if (N_best[0][0] == -1) // no move found
			to_swap = (to_swap + 1) % E; // try with another
		else
			chosen = 0; // break the cycle
	}

	while (1)
	{
		for (i = 0; i < N_BEST_SINGLE && N_best[i][0] != -1; i++)
		{
			if (rand() / (double) RAND_MAX < randomness_single)
				continue;
			insert_TabuList(tl, to_swap, N_best[i][1], 0);
			update_exam_penalties(exam_penalty, x, n, E, to_swap, x[to_swap],
					N_best[i][1]);
			x[to_swap] = N_best[i][1];
			break;
		}
		if (i != N_BEST_SINGLE)
			if (N_best[i][0] != -1)
				break; // the move has been done
	}
	return N_best[i][0];
}

static void update_exam_penalties(ExamPenalty *exam_penalty, int *x, int **n, int E, int to_swap, int old_timeslot, int new_timeslot) // O(E) update exam_penalty array when I move exam to_swap in new_timeslot
{
	int i, val;
	exam_penalty[to_swap].penalty = 0;

	for (i = 0; i < E; i++) {
		if (n[i][to_swap]) {
			if (abs(x[i] - old_timeslot) <= 5)
				exam_penalty[i].penalty -= pow(2, abs(x[i] - old_timeslot))
				* n[i][to_swap];
			if (abs(x[i] - new_timeslot) <= 5) {
				val = pow(2, abs(x[i] - new_timeslot)) * n[i][to_swap];
				exam_penalty[i].penalty += val;
				exam_penalty[to_swap].penalty += val;
			}
		}
	}
}

static int update_penalty(int *x, int **n, int E, int old_pen, int to_swap,	int old_timeslot, int new_timeslot) // O(E)
{
	int i;
	for (i = 0; i < E; i++) {
		if (n[i][to_swap] && abs(x[i] - old_timeslot) <= 5) // old penalty
			old_pen -= pow(2, 5 - abs(x[i] - old_timeslot)) * n[i][to_swap];
		if (n[i][to_swap] && abs(x[i] - new_timeslot) <= 5) // new penalty
			old_pen += pow(2, 5 - abs(x[i] - new_timeslot)) * n[i][to_swap];
	}
	return old_pen;
}

static void neighborhood2_setup(int *x, int **n, int T, int E, ExamPenalty *exam_penalty) // O(E^2)
{
	int i, j;
#pragma omp parallel for collapse(2)
	for (i = 0; i < E; i++)
		for (j = 0; j < E; j++){ // j starts from 0 to count each combination just two times (otherwise x[j] would not pay penalties for i<j)
			exam_penalty[i].exam = i;
			exam_penalty[i].penalty = 0;
			if (abs(x[i] - x[j]) <= 5 && n[i][j])
				exam_penalty[i].penalty += pow(2, 5 - abs(x[i] - x[j]))	* n[i][j];
		}
}

// UTILITIES
static int compute_penalty_complete(int *x, int **n, int E) // O(E^2)
{
	int i, j, pen = 0;
	int threads = omp_get_max_threads();
#pragma omp parallel num_threads(threads) default(none) shared(x, n, pen, E) private(i, j)
	{
	#pragma omp for schedule(dynamic)
	for (i = 0; i < E; i++)
		for (j = i + 1; j < E; j++)
			if (abs(x[i] - x[j]) <= 5 && n[i][j])
				#pragma omp critical
				{
					pen += (int) pow(2, 5 - abs(x[i] - x[j])) * n[i][j];
				}
	}
	return pen; // we have to count 2 times each combination
}

static void swap(int* a, int* b) {
	*a += *b;
	*b = *a - *b;
	*a -= *b;
}

static void update_parameter(int no_impr_times, double *randomness_single_P, double *randomness_group_P, int *tabu_length_P, TABU tl, double *randomness_randomOrCheap_group_P, double *randomness_randomOrCheap_single_P)
{
	double var = (double) no_impr_times / DESTROY_THRESHOLD; // between 0 and 1

	if(DYNAMIC_TABULIST == 1)
	{
		if(TABULIST_INCREASING == 1)
			*tabu_length_P = update_TabuList(tl, MIN_TABU_LENGTH + var * (MAX_TABU_LENGTH - MIN_TABU_LENGTH));
		else
			*tabu_length_P = update_TabuList(tl, MAX_TABU_LENGTH + var*(MIN_TABU_LENGTH - MAX_TABU_LENGTH));
	}
	if(DYNAMIC_RANDOMNESS_SINGLE == 1)
	{
		if(RANDOMNESS_SINGLE_INCREASING == 1)
			*randomness_single_P = RANDOMNESS_BEST_SINGLE_MIN + var * (RANDOMNESS_BEST_SINGLE_MAX - RANDOMNESS_BEST_SINGLE_MIN);
		else
			*randomness_single_P = RANDOMNESS_BEST_SINGLE_MAX + var * (RANDOMNESS_BEST_SINGLE_MIN - RANDOMNESS_BEST_SINGLE_MAX);
	}
	if(DYNAMIC_RANDOMNESS_GROUP == 1)
	{
		if(RANDOMNESS_GROUP_INCREASING == 1)
			*randomness_group_P = RANDOMNESS_BEST_MIN + var * (RANDOMNESS_BEST_MAX - RANDOMNESS_BEST_MIN);
		else
			*randomness_group_P = RANDOMNESS_BEST_MAX + var * (RANDOMNESS_BEST_MIN - RANDOMNESS_BEST_MAX);
	}
	if(DYNAMIC_RANDOMorCHEAP_SINGLE == 1)
	{
		if(RANDOMorCHEAP_SINGLE_INCREASING == 1)
			*randomness_randomOrCheap_single_P = RANDOMNESS_RANDOMorCHEAP_GROUP_MIN + var * (RANDOMNESS_RANDOMorCHEAP_GROUP_MAX - RANDOMNESS_RANDOMorCHEAP_GROUP_MIN);
		else
			*randomness_randomOrCheap_single_P = RANDOMNESS_RANDOMorCHEAP_GROUP_MAX + var * (RANDOMNESS_RANDOMorCHEAP_GROUP_MIN - RANDOMNESS_RANDOMorCHEAP_GROUP_MAX);
	}
	if(DYNAMIC_RANDOMorCHEAP_GROUP == 1)
	{
		if(RANDOMorCHEAP_GROUP_INCREASING == 1)
			*randomness_randomOrCheap_group_P = RANDOMNESS_RANDOMorCHEAP_SINGLE_MIN + var * (RANDOMNESS_RANDOMorCHEAP_SINGLE_MAX - RANDOMNESS_RANDOMorCHEAP_SINGLE_MIN);
		else
			*randomness_randomOrCheap_group_P = RANDOMNESS_RANDOMorCHEAP_SINGLE_MAX + var * (RANDOMNESS_RANDOMorCHEAP_SINGLE_MIN - RANDOMNESS_RANDOMorCHEAP_SINGLE_MAX);
	}
}

static void printBestSolution(int *x, int E, char *instance_name)
{
	int i;
	char file_name[50];
	strcpy(file_name, instance_name); strcat(file_name, "_OMAAL_group21.sol");
	FILE *fp = fopen(file_name, "w");
	if(fp == NULL)
	{
		fprintf(stderr, "Error while opening file %s.\n", file_name);
		exit(-1);
	}
	for(i=0; i<E; i++)
		fprintf(fp, "%d %d\n", i+1, x[i]+1);
	fclose(fp);
}

static void destroySolution_swappingRandom(int *x, int **n, int E, int T, TABU tl)
{
	int iteration = 0, i, j, counter, N_ITERATION = 10, N_SINGLE = 200, N_GROUP = 50;
	int to_swap;

	while(iteration < N_ITERATION)
	{
		// swap single exams
		for(counter = 0; counter < N_SINGLE; counter++)
		{
			to_swap = rand() % E;
			for(i=0; i<T; i++)
			{
				if(check_TabuList(tl, to_swap, i, 2)) // not allowed move
					continue;
				for(j=0; j<E; j++)
					if(x[j] == i && n[to_swap][j]) // unfeasible move
						break;
				if(j!=E)
					continue;
				insert_TabuList(tl, to_swap, x[to_swap], 2);
				x[to_swap] = i;
			}
		}
		// swap timeslot
		for(counter = 0; counter < N_GROUP; counter++)
		{
			to_swap = rand() % T;
			for(i=rand()%T, j=0; (i==to_swap || check_TabuList(tl, (to_swap>i)?to_swap:i, (to_swap>i)?i:to_swap, 3)) && j<T; i=(i+1)%T, j++);
			if(j==T) // no possible swap for timeslot to_swap
				continue;
			for(j=0; j<E; j++)
				if(x[j]==to_swap)
					x[j] = i;
				else if(x[j]==i)
					x[j] = to_swap;
			insert_TabuList(tl, (to_swap>i)?to_swap:i, (to_swap>i)?i:to_swap, 3);
		}
		iteration++;
	}
}
