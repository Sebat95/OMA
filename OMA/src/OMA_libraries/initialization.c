//#define DEBUG_INITIALIZATION

#define RANDOMNESS_INIT 0.2
#define TABU_LENGTH_INIT 5

#include "initialization.h"
#include "tabu_search.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <omp.h>

// DATA STRUCTURES *************************

typedef struct
{
	int timeslot;
	int conflict;
}Conflict;
typedef struct
{
	int exam;
	int value;
}Value;

// PROTOTYPES (almost all static functions) ***************

static void initializationMetaheuristic_tabuSearch(int *x, int **n, int E, int T, int *flag); // GRASP + TABU SEARCH

static int insert_exam_in_better_timeslot(int *x, int **n, int E, int T, TABU tl, Conflict *conflict_for_timeslot, int to_swap);
//static int count_number_conflicts(int *x, int **n, int E); //unused
	// compare functions for qsort
static int compare_conflict_increasing(const void* a, const void* b);
static int compare_Value_decreasing(const void* a, const void* b);

// DEFINITIONS ***************************

void initialization(int *x, int **n, int E, int T)
{
	int i, j, greedy_successfull = 1, found = 0;
	Value* rank = malloc(E * sizeof(Value)); // sorting exam by conflicts with other deeply improves initial solution (without this, metaheuristic does not converge quickly for instance06)
	unsigned char *timeslots_not_available = calloc(T, sizeof(int)); // boolean array (1 means that the timeslot is not available for a certain exam)

	for(i=0; i<E; i++)
	{
		rank[i].exam = i;
		rank[i].value = 0;
	}
	for(i=0; i<E; i++)
		for(j=i+1; j<E; j++)
			if(n[i][j]) // exam i and j in conflict
			{
				rank[i].value+=n[i][j];
				rank[j].value+=n[i][j];
			}

	qsort(rank, E, sizeof(Value), compare_Value_decreasing); // sort exam by decreasing number of conflicting students

	// GREEDY PART ************************
	x[rank[0].exam] = 0; // put first exam in timeslot 0 (or in any other timeslot)
	for(i=1; i<E; i++)
		x[rank[i].exam] = -1; // set other exams timeslots to an invalid value

	for(i=1; i<E; i++) // for each exam (each cycle decide the timeslot for exam i)
	{
		for(j=0; j<E; j++) // for any other exam
			if(n[rank[i].exam][j] != 0 && x[j] != -1) // exam rank[i].exam and j in conflict and exam j timeslot is already fixed
				timeslots_not_available[x[j]] = 1; //exam j timeslot is not available for exam i

		for(j=0; j<T; j++)
			if(timeslots_not_available[j] == 0) // if it founds an available timeslot, break
				break;
		if(j == T) // no available timeslot found
		{
			j = rand() % T; // no timeslot is available for exam rank[i].exam. Put a random timeslot (metaheuristic will fix it)
			greedy_successfull = 0;
		}

		x[rank[i].exam] = j; // set exam i color to the first available color found

		// reset
		for(j=0; j<T; j++)
			timeslots_not_available[j] = 0; //reset not available timeslots array
	}
	free(timeslots_not_available);
	free(rank);

	// METAHEURISTIC PART ***********************
#ifdef DEBUG_INITIALIZATION
	clock_t t1, t2;
	t1 = clock();
#endif

	if(!greedy_successfull) // greedy_successfull = 1 if greedy algorithm has found a feasible solution (no metaheuristic required)
	{
#ifdef DEBUG_INITIALIZATION
		//fprintf(stdout, "Soluzione iniziale greedy con %d conflitti.\n", count_number_conflicts(x, n, E));
		fprintf(stdout, "Greedy algorithm for the initialization didn't found a feasible solution.\nA metaheuristic is required.\n");
#endif

	#pragma omp parallel shared(found, x)
			{
				int x_thread[E];
				while(!found)
				{

	#ifdef DEBUG_INITIALIZATION
					printf("thread  %d started metaheuristic phase.\n", omp_get_thread_num());
					fflush(stdout);
	#endif

					memcpy(x_thread, x, E*sizeof(int));

					srand(omp_get_thread_num());
					initializationMetaheuristic_tabuSearch(x_thread, n, E, T, &found);

	#ifdef DEBUG_INITIALIZATION
					printf("thread  %d ended metaheuristic phase.\n", omp_get_thread_num());
					fflush(stdout);
	#endif

					#pragma omp critical
					{
						if(!found) // if no thread has found a feasible solution still
						{
							memcpy(x, x_thread, E*sizeof(int));
							found = 1; // shared flag used in order to stop all other threads
						}
					}
				}
			}
	}

#ifdef DEBUG_INITIALIZATION
	t2 = clock();
	//check if the solution is correct
	for(i=0; i<E; i++)
	{
		if(x[i] >= T)
		{
			fprintf(stdout, "Exam %d in unavailable slot %d.\n", i+1, x[i]);
			break;
		}
		for(j=0;j<E;j++)
			if(x[i] == x[j] && n[i][j])
			{
				fprintf(stdout, "Conflictual exams %d and %d are both in slot %d.\n", i+1, j+1, x[i]);
				break;
			}
	}
	fprintf(stdout, "REQUIRED TIME: about %.3f seconds.\n", (t2 - t1) * (1.0 / CLOCKS_PER_SEC));
//	fprintf(stdout, "\nInitial solution:\n");
//	for(i=0; i<E; i++)
//		fprintf(stdout, "x[%d] = %d\n", i, x[i]);
	fflush(stdout);
#endif

	return;
}

static void initializationMetaheuristic_tabuSearch(int *x, int **n, int E, int T, int *flag)
{
	int i, j, to_swap, candidate_timeslot = 0;
	Value mark[E];
	TABU tl = new_TabuList(TABU_LENGTH_INIT, TABU_LENGTH_INIT, TABU_LENGTH_INIT);
	Conflict *conflict_for_timeslot = malloc(T * sizeof(Conflict));

	srand(time(NULL)); // each execution may be different

	while(!(*flag))
	{

#ifdef DEBUG_INITIALIZATION
		for(i=0;i<E;i++) fprintf(stdout, "%2d ", x[i]);
		//fprintf(stdout, "\nNumber of conflicts: %d. Thread %d\n", count_number_conflicts(x, n, E), omp_get_thread_num());
#endif

		// look for an exam in conflict with another exam in the same timeslot
		to_swap = rand() % E; // start the scan from a random exam
		for(j=0; j<E; j++) // scan exams in a circular way
		{
			to_swap = (to_swap+1) % E;
			for(i=0; i<E; i++)
				if(x[to_swap] == x[i] && n[to_swap][i])
					break; // to_swap is in conflict with an exam in the same timeslot
			if(i != E)
				break; // I found an exam in conflict with exam to_swap in the same timeslot
		}
		if(j==E) // feasible initial solution found (there is no an exam in conflict with another exam in the same timeslot)
			break; // exit from while(1)

		insert_TabuList(tl, to_swap, x[to_swap], 0); // move back exam to_swap in timeslot x[to_swap] is forbidden for the next moves
		candidate_timeslot = insert_exam_in_better_timeslot(x, n, E, T, tl, conflict_for_timeslot, to_swap); // insert to_swap in the exam best timeslot I can (with less conflicts). Candidate_timeslot is the timeslot where i putted the exam to_swap

		// look for an exam in timeslot candidate_timeslot to move in another timeslot (that may be also candidate_timeslot). The reason is to allow "splitting" group of exams not in conflict eachother, to better explore the solution space. Note that I doo not consider this an action (don't update the tabu list)

		// Possibility 1: select exam in candidate_timeslot which has more conflicts in that timeslot
		for(i=0;i<E;i++) 
		{
			mark[i].value = 0; // reset conflict counter
			mark[i].exam = i;
		}
		for(i=0;i<E;i++)
			if(x[i] == candidate_timeslot) // considering exam i in candidate_timeslot
				for(j=i+1;j<E;j++)
					if(x[j] == candidate_timeslot && n[i][j] && j != to_swap && j != i) // exam j is in the same timeslot and is in conflict
					{
						mark[i].value++; // increase conflicts counter for exam i and j
						mark[j].value++;
					}
		qsort(mark, E, sizeof(int), compare_Value_decreasing); // sort exam by decreasing number of conflicts with other exams in candidate_timeslot

		do
		{
			for(i=0;i<E && mark[i].value > 0;i++)
			{
				if(rand()/(double)RAND_MAX < RANDOMNESS_INIT)
					continue;
				break;
			}
		}while(i == E || mark[i].value != 0);

		insert_exam_in_better_timeslot(x, n, E, T, tl, conflict_for_timeslot, mark[i].exam); // insert the selected exam in the best timeslot (it may be also the same candidate_timeslot)

		/*// Possibility 2: select a random exam in candidate_timeslot in conflict with another exam
		to_swap = rand() % E;
		for(i=0; i<E; i++)
		{
			to_swap = (to_swap+1) % E;
			if(x[to_swap] == candidate_timeslot)
				for(j=0; j<E; j++)
					if(x[j] == candidate_timeslot && n[to_swap][j])
						break; // to_swap is in conflict with an exam in the same timeslot
			if(j == E)
				break;
		}*/

		/*// Possibility 3: select a random exam in that timeslot
		i = rand() % E;
		for(j=0; j<E; j++)
			if(x[(i+j)%E] == candidate_timeslot)
				break;
		insert_exam_in_better_timeslot(x, n, E, T, tl, conflict_for_timeslot, (i+j)%E);
		*/

	}
	free(conflict_for_timeslot);
	delete_TabuList(tl);
}

static int insert_exam_in_better_timeslot(int *x, int **n, int E, int T, TABU tl, Conflict *conflict_for_timeslot, int to_swap)
{
	int i, j, found = 0, candidate_timeslot = -1;
	for(i=0; i<T; i++)
	{
		conflict_for_timeslot[i].conflict = 0;
		conflict_for_timeslot[i].timeslot = i;
	}

	// count how many conflicts I would have putting exam to_swap in each timeslot, and sort the array
	for(i=0; i<T; i++)
		for(j=0; j<E; j++)
			if(x[j] == i && n[to_swap][j]) // if j-exam is in timeslot i and j-exam and to_swap-exam are conflictual
				conflict_for_timeslot[i].conflict++; // increase counter of conflict present in timeslot i

	qsort(conflict_for_timeslot, T, sizeof(Conflict), compare_conflict_increasing); // sort the array by increasing conflict

	// select the timeslot where move exam to_swap
	for(i=0; i<T && !found; i++)
	{
		if(rand()/(double)RAND_MAX < RANDOMNESS_INIT) // GRASP (greedy randomized) + TabuSearch
			continue;
		candidate_timeslot = conflict_for_timeslot[i].timeslot;
		if(!check_TabuList(tl, to_swap, candidate_timeslot, 0)) // if move to_swap into candidate_timeslot is allowed, break
			break;
	}
	if(i==T) // always continued, repeat without RANDOMNESS_INIT
	{
		candidate_timeslot = conflict_for_timeslot[0].timeslot; // select the best timeslot
	}

	x[to_swap] = candidate_timeslot; // move exam to_swap into candidate_timeslot (updating the tabu list is done by the caller)

	return candidate_timeslot;
}

int compare_conflict_increasing(const void* a, const void* b)
{
	Conflict* A = (Conflict*)a;
	Conflict* B = (Conflict*)b;
	return A->conflict - B->conflict;
}

int compare_Value_decreasing(const void* a, const void* b)
{
	Value A = *(Value*)a, B = *(Value*) b;
	int x = B.value - A.value;
	if(x>0)
		return 1;
	if(x<0)
		return -1;
	return 0;
}
/* unused
static int count_number_conflicts(int *x, int **n, int E)
{
	int i, j, conflicts = 0;
	for(i=0; i<E; i++)
		for(j=0; j<E; j++)
			if(x[i] == x[j] && n[i][j] && i != j)
				conflicts++;
	return conflicts;
}
*/
