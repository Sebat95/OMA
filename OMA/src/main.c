/*
 * main.c
 *
 *  Created on: 02 dic 2017
 *      Author: Nicola
 */
#define DEBUG_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OMA_libraries\method1.h"

void setup(char *instance_name, int *T_P, int *E_P, int *S_P, int ***n_P, int **x_P, int **students_per_exam_P, int ***conflictual_students_P);

int main(int argc, char* argv[]) // argv[1] = "instanceXX"
{
	// CONSTANTS
	const char* INSTANCE_PATH = "instances/";
	char instance_name[50];
	strcpy(instance_name, INSTANCE_PATH);
	strcat(instance_name, argv[1]);

	// DATA STRUCTURE
	int T, E, S; // number of timeslots, exams and students
	int **n; // n[i,j] is the number of students enrolled in exams i and j (conflictual)

	int *students_per_exam; // number of students enrolled in each exam
	int **conflictual_students; // ExE matrix which specifies the number of conflictual students between two exams
	/**********************************
		 * NOTA: dai dati che ci da Manerba, potremmo salvare anche a che esami è iscritto
		 * ogni studente, usando quest'informazione per migliorare la soluzione iniziale/generazione del neighborhood

			EDIT: IMPLEMENTATO con conflictual_students, ma è computazionalmente costoso (SE NON LO USIAMO DOBBIAMO TOGLIERLO)
	 */

	// DECISION VARIABLES
	int *x; // x[i] = timeslot of exam i-1

	setup(instance_name, &T, &E, &S, &n, &x, &students_per_exam, &conflictual_students);

	solveMethod1(x, T, E, S, conflictual_students, students_per_exam, conflictual_students, instance_name);

	return 0;
}

void setup(char *instance_name, int *T_P, int *E_P, int *S_P, int ***n_P, int **x_P, int **students_per_exam_P, int ***conflictual_students_P)
{
	int i, j, k, S;
	int **enrolled_stud;
	char line[100];
	FILE *fp;

	// .slo
	strcpy(line, instance_name);
	if((fp = fopen(strcat(line, ".slo"), "r")) == NULL)
	{
		fprintf(stdout, "Error: file %s.slo not found.", instance_name);
		return;
	}
	fscanf(fp, "%d", T_P);
	fclose(fp);

	// .exm
	strcpy(line, instance_name);
	if((fp = fopen(strcat(line, ".exm"), "r")) == NULL)
	{
		fprintf(stdout, "Error: file %s.exm not found.", instance_name);
		return;
	}
	i = 0;
	while(fgets(line, 99, fp) != NULL)
		i++;
	if (!(strcmp(line, "\n"))) //avoid reading extra the line at the end of every .exm file
		i--;
	*E_P = i;
	*students_per_exam_P = malloc(*E_P * sizeof(int));
	rewind(fp);
	while(fgets(line, 99, fp) != NULL)
	{
		sscanf(line, "%4d %d", &i, &j);
		(*students_per_exam_P)[i-1] = j;
	}
	fclose(fp);

	*x_P = malloc(*E_P * sizeof(int));

	// .stu
	strcpy(line, instance_name);
	if((fp = fopen(strcat(line, ".stu"), "r")) == NULL)
	{
		fprintf(stdout, "Error: file %s.stu not found.", instance_name);
		return;
	}
	while(fgets(line, 99, fp) != NULL)
	{
		sscanf(line, "s%d %*d", &S);
	}
	enrolled_stud = malloc(S * sizeof(int*));
	for(i=0; i<S; i++)
		enrolled_stud[i] = calloc(*E_P, sizeof(int));
	rewind(fp);
	while(fgets(line, 99, fp) != NULL)
	{
		sscanf(line, "s%d %d\n", &i, &j);
		enrolled_stud[i-1][j-1]++;
	}
	fclose(fp);

	*conflictual_students_P = malloc(*E_P * sizeof(int*));
	for(i=0; i<*E_P; i++)
		(*conflictual_students_P)[i] = calloc(*E_P, sizeof(int));
	for(i=0; i<*E_P; i++)
	{
		for(j=i+1; j<*E_P; j++)
		{
			for(k=0; k<S-1; k++)
			{
				if(enrolled_stud[k][i] == 1 && enrolled_stud[k][j] == 1)
				{
					(*conflictual_students_P)[i][j]++;
					(*conflictual_students_P)[j][i]++;
				}
			}
		}
	}
#ifdef DEBUG_MAIN
	fprintf(stdout, "Number of exams: %d\n", *E_P);
	fprintf(stdout, "Number of timeslots: %d\n\n", *T_P);
	fprintf(stdout, "Students per exam:\n");
	for(i=0; i<*E_P; i++)
		fprintf(stdout, "Exam %d: students %d\n", i+1, (*students_per_exam_P)[i]);
	fprintf(stdout, "\nConflictual students:\n");
	for(i=0; i<*E_P; i++)
	{
		fprintf(stdout, "Exam %d:", i+1);
		for(j=0; j<*E_P; j++)
			fprintf(stdout, "%d ", (*conflictual_students_P)[i][j]);
		fprintf(stdout, "\n");
	}


#endif

	for(i=0; i<S; i++)
		free(enrolled_stud[i]); // not useful anymore
	free(enrolled_stud);
	return;
}
