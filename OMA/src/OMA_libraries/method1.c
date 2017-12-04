/*
 * method1.c
 *
 *  Created on: 02 dic 2017
 *      Author: Nicola
 */
#define PRINT_DEBUG_INFO

#include "method1.h"
#include "initialization.h"
#include <stdio.h>

void solveMethod1(int *x, int T, int E, int S, int **n, int *students_per_exam, int **conflictual_students, char *instance_name)
{
	fprintf(stdout, "ciao0"); // ma che cazzo di problemi ha sto coso?
	initializationGraphColoring(x, n, E, T);
	printf("ciao");
#ifdef PRINT_DEBUG_INFO
		int i;
		fprintf(stdout, "Initial solution:\n");
		for(i=0; i<E; i++)
			fprintf(stdout, "x[%d] = %d\n", i, x[i]);
#endif
		printf("ciao2");
}

//LE ALTRE FUNZIONI LE METTO STATIC
