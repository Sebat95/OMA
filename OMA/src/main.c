/*
 * main.c
 *
 *  Created on: 02 dic 2017
 *      Author: Nicola
 */
//#define DEBUG_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "OMA_libraries\initialization.h"
#include "OMA_libraries\method2.h"

void setup(char *instance_name, int *T_P, int *E_P, int *S_P, int ***n_P, int **x_P);

int main(int argc, char* argv[]) // argv[1] = "instanceXX", argv[2] = "X" total time to execute in seconds,
								 // argv[3]= "XXXXX.txt" parameters file
{
	// CONSTANTS
	const char* INSTANCE_PATH = "instances/";
	char instance_name[50];
	strcpy(instance_name, INSTANCE_PATH);
	strcat(instance_name, argv[1]);

	// DATA STRUCTURE
	int T, E, S; // number of timeslots, exams and students
	int **n; // n[i,j] is the number of students enrolled in exams i and j (conflictual)

	// DECISION VARIABLES
	int *x; // x[i] = timeslot of exam i+1

	//time to run in minutes
	int ex_time;
	double tm=clock()/CLOCKS_PER_SEC;

	if(argc>4)
	{
		fprintf(stderr, "Too much arguments on the command line!\n");
		return -1;
	}
	ex_time=atoi(argv[2]);
	//argv[3] init file

	if((ex_time) < ((clock()/CLOCKS_PER_SEC)-tm))
		return 0;

	//setup data structures
	setup(instance_name, &T, &E, &S, &n, &x); // read instance file and setup data structures

	if((ex_time) < ((clock()/CLOCKS_PER_SEC)-tm))
		return 0;

	initialization(x, n, E, T); // find an initial solution

	if((ex_time) < ((clock()/CLOCKS_PER_SEC)-tm))
		return 0;

	//last argument equal NULL if we want no to initialize the parameters by means of a configuration file
	optimizationMethod2(x, T, E, S, n, argv[1], (double)ex_time - ((clock()/(double)CLOCKS_PER_SEC)-tm), argv[3]);

	return 0;
}

void setup(char *instance_name, int *T_P, int *E_P, int *S_P, int ***conflictual_students_P, int **x_P)
{
	int i, j, k;
	int **enrolled_stud;
	char line[100];
	FILE *fp;
	static const long max_len = 99 + 1;
	char buf[max_len + 1];
	char *last_newline, *last_line;
	int index;

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
	if((fp = fopen(strcat(line, ".exm"), "rb")) == NULL)
	{
		fprintf(stdout, "Error: file %s.exm not found.", instance_name);
		return;
	}
	//read that many bytes from the end of the file
	fseek(fp, -max_len, SEEK_END);
	fread(buf, max_len-1, 1, fp);

	//add the null terminator
	buf[max_len-1] = '\0';

	//find the last newline character
	last_newline = strrchr(buf, '\n');
	//extract its index
	index = (int) abs(buf-last_newline);
	//if index is too close to the end skip it (account for eventual extra newline at the end)
	while(abs(max_len-index)<5){
		buf[index]='\0';
		last_newline = strrchr(buf, '\n');
		index = (int) abs(buf-last_newline);
	}

	//cut the very last line, right after the last newline
	last_line = last_newline+1;

	//extract the needed data
	sscanf(last_line, "%d %*d", E_P);

	fclose(fp);

	*x_P = malloc(*E_P * sizeof(int));

	// .stu
	strcpy(line, instance_name);
	if((fp = fopen(strcat(line, ".stu"), "rb")) == NULL)
	{
		fprintf(stdout, "Error: file %s.stu not found.", instance_name);
		return;
	}

	//read that many bytes from the end of the file
	fseek(fp, -max_len, SEEK_END);
	fread(buf, max_len-1, 1, fp);

	//add the null terminator
	buf[max_len-1] = '\0';

	//add the null terminator
		buf[max_len-1] = '\0';

	//find the last newline character
	last_newline = strrchr(buf, '\n');
	//extract its index
	index = (int) abs(buf-last_newline);
	//if index is too close to the end skip it (account for eventual extra newline at the end)
	while(abs(max_len-index)<5){
		buf[index]='\0';
		last_newline = strrchr(buf, '\n');
		index = (int) abs(buf-last_newline);
	}

	//cut the very last line, right after the last newline
	last_line = last_newline+1;

	//extract the needed data
	sscanf(last_line, "s%d %*d", S_P);

	enrolled_stud = malloc(*S_P * sizeof(int*));
	for(i=0; i<*S_P; i++)
		enrolled_stud[i] = calloc(*E_P, sizeof(int));

	fp = freopen(line, "r", fp);
	while(fgets(line, 99, fp) != NULL)
	{
		sscanf(line, "s%d %d\n", &i, &j);
		enrolled_stud[i-1][j-1]++;
	}
	fclose(fp);

	*conflictual_students_P = malloc(*E_P * sizeof(int*));
	for(i=0; i<*E_P; i++)
		(*conflictual_students_P)[i] = calloc(*E_P, sizeof(int));
	for(k=0; k<*S_P; k++)
	{
		for(i=0; i<*E_P; i++)
		{
			for(j=i+1; j<*E_P; j++)
			{
				if(enrolled_stud[k][i] >= 1 && enrolled_stud[k][j] >= 1)
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

	for(i=0; i<*S_P; i++)
		free(enrolled_stud[i]); // not useful anymore
	free(enrolled_stud);
	return;
}
