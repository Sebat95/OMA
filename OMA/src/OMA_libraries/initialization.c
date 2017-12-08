/*
 * initialization.c
 *
 *  Created on: 03 dic 2017
 *      Author: Nicola
 */
#define DEBUG_INITIALIZATION

#define RANDOMNESS 0.4
#define TABU_LENGTH 40

#include "initialization.h"
#include "tabu_search.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

// DATA STRUCTURES *************************

typedef struct
{
	int timeslot;
	int conflict;
}Conflict;
// PROTOTYPES (almost all static functions) ***************

static void initializationMetaheuristic_tabuSearch(int *x, int **n, int E, int T);
static int insert_exam_in_better_timeslot(int *x, int **n, int E, int T, TABU tl, Conflict *conflict_for_timeslot, int to_swap);

int compare_conflict_increasing(const void* a, const void* b);
int compare_int_decreasing(const void* a, const void* b);
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
			j = rand() % T;// OPPURE j--;
			//colors++; // VERSIONE CHE USA TIMESLOT NON DISPONIBILIadd a "dummy" timeslot, that I will try to eliminate by means of metaheuristic

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

	clock_t t1, t2;
	srand(time(NULL)); // NON DETERMINISMO: ogni esecuzione è diversa dalle altre
	t1 = clock();
	initializationMetaheuristic_tabuSearch(x, n, E, T);
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
	fprintf(stdout, "TEMPO RICHIESTO: circa %.5f secondi.\n", (t2 - t1) * (1.0 / CLOCKS_PER_SEC));
	return;
}

static void initializationMetaheuristic_tabuSearch(int *x, int **n, int E, int T)
{
	int i, j, found = 0, to_swap, candidate_timeslot = 0, mark[E];
	TABU tl = new_TabuList(TABU_LENGTH);
	Conflict *conflict_for_timeslot = malloc(T * sizeof(Conflict));

	while(1)
	{
#ifdef DEBUG_INITIALIZATION
		for(i=0;i<E;i++) fprintf(stdout, "%2d ", x[i]);
		fprintf(stdout, "\n");
#endif
		// ****CERCO UN ESAME IN CONFLITTO CON UN ALTRO ESAME DELLO STESSO TIMESLOT
		to_swap = rand() % E; // start the scan from a random exam
		for(j=0; j<E; j++) // scan exams in a circular way
		{
			to_swap = (to_swap+1) % E;
			for(i=0; i<E; i++)
				if(x[to_swap] == x[i] && n[to_swap][i])
					break; // to_swap is in conflict with an exam in the same timeslot
			if(i != E) break;
		}
		if(j==E) // feasible initial solution found
			break;

		insert_TabuList(tl, to_swap, x[to_swap]);
		candidate_timeslot = insert_exam_in_better_timeslot(x, n, E, T, tl, conflict_for_timeslot, to_swap);
		// ****CERCO DOVE METTERE L'ESAME CHE HO TOLTO
		//now we look for a timeslot for the exam j, otherwise we put in a not available timeslot
		//to_swap = found;

		/*to_swap = rand() % E;
		for(i=0; i<E; i++)
		{
			to_swap = (to_swap+1) % E;
			if(x[to_swap] == candidate_timeslot)
				for(j=0; j<E; j++)
					if(x[to_swap] == x[j] && n[to_swap][j])
						break; // to_swap is in conflict with an exam in the same timeslot
			if(j == E)
				break;
		}*/

		// TENTATIVO SCEGLIENDO TRA GLI ESAMI CON PIù CONFLITTI NEL CANDIDATE_TIMESLOT
		for(i=0;i<E;i++) mark[i] = 0;
		for(i=0;i<E;i++)
			if(x[i] == candidate_timeslot)
				for(j=0;j<E;j++)
					if(x[j] == candidate_timeslot && n[i][j] && j != to_swap && j!=i)
					{
						mark[i]++;
						mark[j]++;
					}
		qsort(mark, E, sizeof(int), compare_int_decreasing);
		for(i=0;i<E && mark[i] > 0;i++)
		{
			if(rand()/(double)RAND_MAX < RANDOMNESS)
				continue;
			break;
		}
		if(i==E || mark[i] == 0)
			i = 0;
		insert_exam_in_better_timeslot(x, n, E, T, tl, conflict_for_timeslot, i);

		/*// TENTATIVO SCEGLIENDO A CASO IN QUEL TIMESLOT
		i = rand() % E;
		for(j=0; j<E; j++)
			if(x[(i+j)%E] == candidate_timeslot)
				break;
		insert_exam_in_better_timeslot(x, n, E, T, tl, conflict_for_timeslot, (i+j)%E);
		*/
		/*// TENTATIVO COME QUELLO CON MENO CONFLITTI NEGLI ALTRI TIMESLOT
		for(i=0;i<E;i++) mark[i] = 0;
		for(i=0;i<E;i++)
			if(x[i] == candidate_timeslot)
				for(j=0;j<E;j++)
					if(x[j] != candidate_timeslot && n[i][j] && j != to_swap && j!=i)
					{
						mark[i]++; //###@@@!!!!! E SE INVECE SEGNASSI QUANTI CONFLITTI HO NEGLI ALTRI TIMESLOT E PRENDESSI QUELLO CON MENO CONFLITTI?
						mark[j]++;
					}
		qsort(mark, E, sizeof(int), compare_int_decreasing);
		for(i=0;i<E && mark[i] > 0;i++)
		{
			if(rand()/(double)RAND_MAX < RANDOMNESS)
				continue;
			break;
		}
		if(i==E || mark[i] == 0)
			i = 0;
		insert_exam_in_better_timeslot(x, n, E, T, tl, conflict_for_timeslot, mark[i]);
		*/


		/*for(i=0; i<T; i++)
		{
			for(j=0; j<E; j++)
				if(x[j] == i && n[to_swap][j] )
					break;
			if(j==E) // no conflictual exam with to_swap in timeslot i
				break;
		}
		if(i == T) // there are conflictual exams everywhere
		{
			i = rand() % T;
			while(check_TabuList(tl, to_swap, i))
				i = (i+1) % T;
		}
		x[to_swap] = i;
		insert_TabuList(tl, to_swap, i);*/

		//reset
		found = 0;
	}
	free(conflict_for_timeslot);
}
static int insert_exam_in_better_timeslot(int *x, int **n, int E, int T, TABU tl, Conflict *conflict_for_timeslot, int to_swap)
{
	int i, j, found = 0, candidate_timeslot;
	for(i=0; i<T; i++)
	{
		conflict_for_timeslot[i].conflict = 0;
		conflict_for_timeslot[i].timeslot = i;
	}

	// ****CALCOLO PER OGNI TIMESLOT QUANTI CONFLITTI AVREI CON L'ESAME SCELTO e ORDINO QUESTO VETTORE
	for(i=0; i<T; i++) // SI PUO' FARE PIù EFFICIENTEMENTE IN QUALCHE MODO? cosi ogni neighborhood costa O(T*E)
		for(j=0; j<E; j++)
			if(x[j] == i && n[to_swap][j]) // if j-exam is in timeslot T and j-exam and to_swap-exam are conflictual
				conflict_for_timeslot[i].conflict++; // increase counter of conflict present in timeslot T (of course all timeslot will have at least 1 conflict, otherwise we would put to_swap in that timeslot already backward)

	qsort(conflict_for_timeslot, T, sizeof(Conflict), compare_conflict_increasing); // sort the array by decreasing conflict
	// ****CERCO UN ESAME DA TOGLIERE DA UN TIMESLOT PER METTERE L'ESAME SCELTO
	for(i=0; i<T && !found; i++)
	{
		if(rand()/(double)RAND_MAX < RANDOMNESS)
			continue;
		candidate_timeslot = conflict_for_timeslot[i].timeslot; // candidate_timeslot is the candidate_timeslot timeslot
		if(!check_TabuList(tl, to_swap, candidate_timeslot)) // GRASP (greedy randomized) + TabuSearch
			break;
	}
	if(i==T) // always continued, repeat without randomness
	{
		/*for(i=0; i<T && !found; i++)
		{
			candidate_timeslot = conflict_for_timeslot[i].timeslot; // candidate_timeslot is the candidate_timeslot timeslot
			if(check_TabuList(tl, to_swap, candidate_timeslot))
				continue;
			for(j=0; j<E && !found; j++)
				if(x[j] == candidate_timeslot) // if j-exam is in the candidate_timeslot timeslot and the swap is allowed
					found = j;
		}
		if(!found)
			found = rand() % E;*/
		candidate_timeslot = conflict_for_timeslot[0].timeslot;
	}
	//candidate_timeslot is the timeslot where we're putting exam to_swap
	// ****METTO L'ESAME SCELTO ALL'INIZIO NEL TIMESLOT SELEZIONATO
	//insert_TabuList(tl, to_swap, x[to_swap]); LA METTO FUORI DALLA FUNZIONE (cosi non aggiorno la tabulist quando vado ad inserire l'elemento che avevo rimosso)
	x[to_swap] = candidate_timeslot;

	return candidate_timeslot;
}

int compare_conflict_increasing(const void* a, const void* b)
{
	Conflict* A = (Conflict*)a;
	Conflict* B = (Conflict*)b;
	return A->conflict - B->conflict;
}
int compare_int_decreasing(const void* a, const void* b)
{
	return *(int*) b - *(int*) a;
}
