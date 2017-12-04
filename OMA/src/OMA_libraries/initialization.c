/*
 * initialization.c
 *
 *  Created on: 03 dic 2017
 *      Author: Nicola
 */
#include "initialization.h"
#include <stdlib.h>
#include <stdio.h>

void initializationGraphColoring(int *x, int **n, int E, int T)
{
	int i, j;

	unsigned char *timeslots_not_available = calloc(T, sizeof(int)); // boolean array (1 means that the color is not available)

	x[0] = 0; // put exam 0 in timeslot 0
	for(i=1; i<E; i++)
		x[i] = -1; // reset other exams timeslots

	for(i=1; i<E; i++)
	{
		for(j=0; j<E; j++)
			if(n[i][j] != 0 && x[j] != -1) // exam i and j in conflict and exam j color is fixed
				timeslots_not_available[x[j]] = 1; //exam j color is not available for exam i

		for(j=0; j<T; j++)
			if(timeslots_not_available[j] == 0)
				break;
		if(j == T)
		{
			#ifdef DEBUG
				fprintf(stdout, "No feasible solution does exist.\n");
			#endif
			for(j=0; j<E; j++)
				x[j] = -1;
			return;
		}

		x[i] = j; // set exam i color to the first available color found

		for(j=0; j<T; j++)
			timeslots_not_available[j] = 0; //reset color array
	}
	free(timeslots_not_available);
}
