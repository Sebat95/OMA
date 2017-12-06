/*
 * method1.c
 *
 *  Created on: 02 dic 2017
 *      Author: Nicola
 */
#define DEBUG_METHOD1

#include "method1.h"
#include "initialization.h"
#include <stdio.h>

void solveMethod1(int *x, int T, int E, int S, int **n, int *students_per_exam, int **conflictual_students, char *instance_name)
{
	initialization(x, n, E, T);

#ifdef DEBUG_METHOD1
		int i;
		fprintf(stdout, "\nInitial solution:\n");
		for(i=0; i<E; i++)
			fprintf(stdout, "x[%d] = %d\n", i, x[i]);
#endif
}

//LE ALTRE FUNZIONI LE METTO STATIC
