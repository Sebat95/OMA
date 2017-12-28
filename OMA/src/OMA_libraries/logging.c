/*
 * logging.c
 *
 *  Created on: 08 dic 2017
 *      Author: Samuele
 */
//#define DEBUG_LOGGING


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


void write_back(){
	FILE *fp;
	int i, j;
	char file_mode[2];

	if(w_a==0){ //if it's the first time we are logging then write
		w_a++;
		strcpy(file_mode, "w");
	}
	else
		strcpy(file_mode, "a"); //if not append

	if((fp = fopen("log.txt", file_mode)) == NULL)
	{
		fprintf(stdout, "Error during 'log.txt' manipulation");
		return;
	}

	//write the log buffer on file
	for(i=0; i<counter; i++)
		for(j=0; j<logs[i].dim; j++){
			fprintf(fp, "%s", logs[i].things_to_be_logged[j]);
			#ifdef DEBUG_LOGGING
				printf("%s", logs[i].things_to_be_logged[j]);
			#endif
		}

	fclose(fp);
	counter=0;
}

void logger(char **things, int n){
	//if the log buffer is full then flush it
	if(counter==MAX_PENDING_LOGS)
		write_back();

	logs[counter].things_to_be_logged=things;
	logs[counter].dim=n;
	counter++;
}


void start_timer(){
	begin=clock();
}

void stop_timer(){
	end=clock();

	char **log;

	//if (begin == 0) return; //if there wes not a start_timer before

	//prepare a log
	log=(char **)malloc(2*sizeof(char*));
	log[0]=(char *)malloc(1000*sizeof(char));
	log[1]=(char *)malloc(1000*sizeof(char));
	sprintf(log[0], "Total Time Elapsed: %ld\n",  (end - begin));
	sprintf(log[1], "Total Clock Cycles Needed: %.3f\n", (double) ((end - begin) / CLOCKS_PER_SEC));
	#ifdef DEBUG_LOGGING
		printf("%s\n", log[0]);
		printf("%s\n", log[1]);
	#endif

	logger(log, 2);
}

void loggerSTR(char *thing, int len){
	char **log;

	//prepare a log
	log=(char **)malloc(1*sizeof(char*));
	log[0]=(char *)malloc(len*sizeof(char));
	memcpy(log[0], thing, len*sizeof(char));
	#ifdef DEBUG_LOGGING
		printf("%s\n", log[0]);
	#endif

	logger(log, 1);
}

void instant_wb(char **things, int n){
	logger(things, n);
	write_back();
}
