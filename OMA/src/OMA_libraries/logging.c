/*
 * logging.c
 *
 *  Created on: 08 dic 2017
 *      Author: Samuele
 */
#define DEBUG_LOGGING


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PENDING_LOGS 5

typedef struct single_log
{
		char **things_to_be_logged;
		int dim;
}log;

clock_t begin = 0;
clock_t end = 0;
log logs[MAX_PENDING_LOGS];
int counter=0;
int w_a=0; //write or append


static void write_back(){
	FILE *fp;
	int i, j;
	char file_mode[2];

	if(w_a==0){
		w_a++;
		strcpy(file_mode, "w");
	}
	else
		strcpy(file_mode, "a");

	if((fp = fopen("log.txt", file_mode)) == NULL)
	{
		fprintf(stdout, "Error during 'log.txt' creation");
		return;
	}

	for(i=0; i<counter; i++){
		for(j=0; j<logs[i].dim; j++){
			fprintf(fp, "%s", logs[i].things_to_be_logged[j]);
			#ifdef DEBUG_LOGGING
				printf("%s", logs[i].things_to_be_logged[j]);
			#endif
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
	counter=0;
}

void logger(char **things, int n){

	if(counter==MAX_PENDING_LOGS)
		write_back();

	logs[counter].things_to_be_logged=things;
	logs[counter].dim=n;
	counter++;
}


void start_timer(){
	begin=(double)clock();
	#ifdef DEBUG_LOGGING
		printf("%f\n", (double) begin);
	#endif
}

void stop_timer(){
	end=(double)clock();

	char log[70], tot_time[10], tot_clock_cycles[10];

	if (begin == 0) return;

	snprintf(tot_time, 10, "%.3f", (double) (end - begin));
	snprintf(tot_clock_cycles, 10, "%.3f", (double) ((end - begin) / CLOCKS_PER_SEC));


	sprintf(log, "Total Time Elapsed: %s\nTotal Clock Cycles Needed: %s\n", tot_time, tot_clock_cycles);
	#ifdef DEBUG_LOGGING
		printf("%s\n", log);
	#endif

	logger(&log, 1);
}

void instant_wb(char **things, int n){
	logger(things, n);
	write_back();
}
