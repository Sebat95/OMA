/*
 * initialization.c
 *
 *  Created on: 03 dic 2017
 *      Author: Nicola
 */
#define DEBUG_INITIALIZATION

#define RANDOMNESS 0.4
#define TABU_LENGTH 10

#include "initialization.h"
#include "tabu_search.h"
#include <stdlib.h>
#include <stdio.h>

// DATA STRUCTURES *************************++

typedef struct
{
	int timeslot;
	int conflict;
}Conflict;
// PROTOTYPES (almost all static functions) ***************

static void initializationMetaheuristic_tabuSearch(int *x, int **n, int E, int T);
int compare_conflict_increasing(const void* a, const void* b);

// DEFINITIONS ***************************

void initialization(int *x, int **n, int E, int T)
{
	int i, j, colors = T;
	unsigned char *timeslots_not_available = calloc(T, sizeof(int)); // boolean array (1 means that the color is not available)

	// GREEDY PART ************************
	x[0] = 0; // put exam 0 in timeslot 0
	for(i=1; i<E; i++)
		x[i] = -1; // reset other exams timeslots

	for(i=1; i<E; i++)
	{
		for(j=0; j<E; j++)
			if(n[i][j] != 0 && x[j] != -1) // exam i and j in conflict and exam j color is fixed
				timeslots_not_available[x[j]] = 1; //exam j color is not available for exam i

		for(j=0; j<colors; j++)
			if(timeslots_not_available[j] == 0)
				break;
		if(j == colors)
			j--; // VERSIONE CHE NON USA TIMESLOT NON DISPONIBILI
			//colors++; // add a "dummy" timeslot, that I will try to eliminate by means of metaheuristic

		x[i] = j; // set exam i color to the first available color found

		for(j=0; j<T; j++)
			timeslots_not_available[j] = 0; //reset color array
	}
	free(timeslots_not_available);

	/*if(colors <= T)
		return; // feasible solution found by means of greedy algorithm
	*/

	// METAHEURISTIC PART ***********************
#ifdef DEBUG_INITIALIZATION
	fprintf(stdout, "Greedy algorithm for the initialization didn't found a feasible solution.\nA metaheuristic is required.\n");
#endif

	initializationMetaheuristic_tabuSearch(x, n, E, T);

	//check if the solution is correct
	for(i=0; i<E; i++)
		for(j=0;j<E;j++)
			if(x[i] == x[j] && n[i][j])
			{
				fprintf(stdout, "Conflictual exams %d and %d are both in slot %d.\n", i+1, j+1, x[i]);
				break;
			}

	return;
}

static void initializationMetaheuristic_tabuSearch(int *x, int **n, int E, int T)
{
	int i, j, found = 0, to_swap, candidate_timeslot;
	TABU tl = new_TabuList(TABU_LENGTH);
	Conflict conflict_for_timeslot[T];

	for(i=0; i<T; i++)
	{
		conflict_for_timeslot[i].conflict = 0;
		conflict_for_timeslot[i].timeslot = i;
	}

	while(1)
	{
		for(i=0;i<E;i++) fprintf(stdout, "%d ", x[i]);
		fprintf(stdout, "\n");

		///* QUESTO MODO DI GENERARE IL VICINATO FORSE NON FUNZIONA
		//for(candidate_timeslot = to_swap-1; to_swap != candidate_timeslot; to_swap = (to_swap+1) % E) // scan exams in a circular way
		//	if(x[to_swap] >= T) // unavailable timeslot
		//		break;
		//if(to_swap==candidate_timeslot) // feasible initial solution found
		//	break;
		// now to_swap specifies the exam that is in a not available timeslot, that we want to swap
		to_swap = rand() % E;
		for(candidate_timeslot = to_swap-1; to_swap != candidate_timeslot; to_swap = (to_swap+1) % E) // scan exams in a circular way
		{
			for(i=0; i<E; i++)
				if(x[to_swap] == x[i] && n[to_swap][i])
					break; // to_swap is in conflict with an exam in the same timeslot
			if(i != E) break;
		}
		if(to_swap==candidate_timeslot) // feasible initial solution found
			break;

		for(i=0; i<T; i++) // SI PUO' FARE PIù EFFICIENTEMENTE IN QUALCHE MODO? cosi ogni neighborhood costa O(T*E)
			for(j=0; j<E; j++)
				if(x[j] == i && n[to_swap][j]) // if j-exam is in timeslot T and j-exam and to_swap-exam are conflictual
					conflict_for_timeslot[i].conflict++; // increase counter of conflict present in timeslot T (of course all timeslot will have at least 1 conflict, otherwise we would put to_swap in that timeslot already backward)

		qsort(conflict_for_timeslot, T, sizeof(Conflict), compare_conflict_increasing); // sort the array by decreasing conflict

		for(i=0; i<T && !found; i++)
		{
			if(rand()/(double)RAND_MAX < RANDOMNESS)
				continue;
			candidate_timeslot = conflict_for_timeslot[i].timeslot; // candidate_timeslot is the candidate_timeslot timeslot
			for(j=0; j<E && !found; j++)
				if(x[j] == candidate_timeslot && !check_TabuList(tl, to_swap, j)) // if j-exam is in the candidate_timeslot timeslot and the swap is allowed
					found = j;
		}
		//candidate_timeslot is the timeslot where we're putting exam to_swap
		x[to_swap] = candidate_timeslot;
		insert_TabuList(tl, to_swap, candidate_timeslot);
		//now we look for a timeslot for the exam j, otherwise we put in a not available timeslot
		to_swap = found;
		for(i=0; i<T; i++)
		{
			for(j=0; j<E; j++)
				if(x[j] == i && n[to_swap][j])
					break;
			if(j==E) // no conflictual exam with to_swap in timeslot i
				break;
		}
		x[to_swap] = i; // if there are conflictual exams, put in a not available timeslot (timeslot T)

		//reset conflict_for_timeslot array
		for(i=0; i<T; i++)
		{
			conflict_for_timeslot[i].conflict = 0;
			conflict_for_timeslot[i].timeslot = i;
		}
		found = 0;

		//TENTATIVO BANALE
/*		to_swap = rand() % E;
				for(candidate_timeslot = to_swap-1; to_swap != candidate_timeslot; to_swap = (to_swap+1) % E) // scan exams in a circular way
				{
					for(i=0; i<E; i++)
						if(x[to_swap] == x[i] && n[to_swap][i])
							break; // to_swap is in conflict with an exam in the same timeslot
					if(i != E) break;
				}
				if(to_swap==candidate_timeslot) // feasible initial solution found
					break;
		x[to_swap] = (x[to_swap]+1) % T; // PROVA CON NEIGHBORHOOD BANALE
*/
	}
}
int compare_conflict_increasing(const void* a, const void* b)
{
	Conflict* A = (Conflict*)a;
	Conflict* B = (Conflict*)b;
	return A->conflict - B->conflict;
}

