/*
 * tabu_search.c
 *
 *  Created on: 05 dic 2017
 *      Author: Nicola
 */
#define DEBUG_TABU_SEARCH

#include "tabu_search.h"
#include <stdio.h>
#include <stdlib.h>

// DATA STRUCTURES **********************

typedef struct fifo_node_struct
{
	int x, y; // elements that have been swapped
	struct fifo_node_struct* next;
}FIFO_node;

typedef struct fifo_struct
{
	FIFO_node *head;
	int length;
}FIFO; // it is used as a shift queue

typedef struct tabulist_struct
{
	FIFO *fifo_queue; // FIFO of forbidden actions (implementation will depend on who use this library)
	int length; //tabu list length
	//int iteration; // number of times tabu list has been accessed since last time TL length has been increased
	//int iteration_to_increase; // each iteration_to_increase iterations, TL length is increased
	//unsigned char dynamic; // boolean specifying if tabu list length is dynamically changing
}TabuList;

// PROTOTYPES (not already in the header file, thus static) ******************

static FIFO* new_FIFO(int length);
static int match_FIFO(FIFO* fifo, int x, int y);
static void insert_FIFO(FIFO* fifo, int x, int y);
static void free_FIFO(FIFO* fifo);

// DEFINITIONS ************************

TabuList* new_TabuList(int length)//, int iteration, int iteration_to_increase, unsigned char dynamic)
{
	TabuList* tl = malloc(sizeof(TabuList));
	tl->length = length;
	//tl->iteration = iteration;
	//tl->iteration_to_increase = iteration_to_increase;
	//tl->dynamic = dynamic;
	tl->fifo_queue = new_FIFO(length);
	return tl;
}
int check_TabuList(TabuList* tl, int x, int y)
{
	return match_FIFO(tl->fifo_queue, x, y); // if the action is not swap
	//return match_FIFO(tl->fifo_queue, x, y) || match_FIFO(tl->fifo_queue, y, x); // because we're swapping
}
void insert_TabuList(TabuList* tl, int x, int y)
{
	//insert_FIFO(tl->fifo_queue, y, x); IN CERTI CASI BISOGNA METTERE IN ORDINE INVERSO (attualmente non lo uso cosi)
	insert_FIFO(tl->fifo_queue, x, y);
}
void delete_TabuList(TabuList* tl)
{
	free_FIFO(tl->fifo_queue);
	free(tl);
}

// FIFO ***** (POTREI FARE UNA LIBRERIA A PARTE MA VEDIAMO SE EFFETTIVAMENTE LA USEREMMO IN ALTRI MODI)
static FIFO* new_FIFO(int length)
{
	int i;
	FIFO_node *pointer = NULL;
	FIFO *fifo = malloc(sizeof(FIFO));
	fifo->length = length;
	fifo->head = NULL;
	for(i=0; i<length;i++)
	{
		pointer = malloc(sizeof(FIFO_node));
		pointer->x = -1; pointer->y = -1; //null values at the beginning (I could manage not completely full FIFO, but maybe is not more efficient)
		pointer->next = fifo->head;
		fifo->head = pointer;
	}
	return fifo;
}
static int match_FIFO(FIFO* fifo, int x, int y)
{
	FIFO_node *pointer = fifo->head;
	while(pointer != NULL)
	{
		if(pointer->x == x && pointer->y == y)
			return 1; //match found
		pointer = pointer->next;
	}
	return 0; //match not found
}
static void insert_FIFO(FIFO* fifo, int x, int y) // remove the head, insert on tail (it could be the contrary also)
{
	FIFO_node *pointer = fifo->head;
	fifo->head->x = x; fifo->head->y = y;
	while(pointer->next != NULL)
		pointer = pointer->next; // at the end, pointer will point to the last node
	pointer->next = fifo->head;
	fifo->head = fifo->head->next; // the new head is the second node
	pointer->next->next = NULL; //the last head is the tail now, so I update his next
}
static void free_FIFO(FIFO* fifo)
{
	FIFO_node *pointer = fifo->head;
	while(pointer != NULL)
	{
		pointer = pointer->next;
		free(pointer);
	}
	free(fifo);
}
