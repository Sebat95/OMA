/*
 * initialization.c
 *
 *  Created on: 03 dic 2017
 *      Author: Nicola
 */
#define DEBUG_INITIALIZATION

#include "initialization.h"
#include <stdlib.h>
#include <stdio.h>

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
			colors++; // add a "dummy" timeslot, that I will try to eliminate by means of metaheuristic

		x[i] = j; // set exam i color to the first available color found

		for(j=0; j<T; j++)
			timeslots_not_available[j] = 0; //reset color array
	}
	free(timeslots_not_available);

	if(colors <= T)
		return; // feasible solution found by means of greedy algorithm

	// METAHEURISTIC PART ***********************
#ifdef DEBUG_INITIALIZATION
	fprintf(stdout, "Greedy algorithm for the initialization didn't found a feasible solution.\nA metaheuristic is required.\n");
#endif

	wrapper_initializationMetaheuristic_tabuSearch(x, n, E, T);

	return;
}

static wrapper_initializationMetaheuristic_tabuSearch(int *x, int **n, int E, int T)
{
	TabuList* tl = new_TabuList();
}


