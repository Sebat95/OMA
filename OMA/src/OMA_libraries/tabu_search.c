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
	int type; // 0 = single move, 1 = group move
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
	//int iteration; // number of times tabu list has been accessed since last time TL length has been increased (potrei demandare la variazione di lunghezza al chiamante, dando un'apposita funzione per allungare/ridurre la tabu list)
	//int iteration_to_increase; // each iteration_to_increase iterations, TL length is increased
	//unsigned char dynamic; // boolean specifying if tabu list length is dynamically changing
	int min_length;
	int max_length;
}TabuList;

// PROTOTYPES (not already in the header file, thus static) ******************

static FIFO* new_FIFO(int length);
static int match_FIFO(FIFO* fifo, int x, int y, int type);
static void insert_FIFO(FIFO* fifo, int x, int y, int type);
static void free_FIFO(FIFO* fifo);
static void decrease_FIFO(FIFO* fifo);
static void increase_FIFO(FIFO* fifo);
static void resetNode(FIFO_node *pointer);

// DEFINITIONS ************************

TabuList* new_TabuList(int length, int min_length, int max_length)//, int iteration, int iteration_to_increase, unsigned char dynamic)
{
	TabuList* tl = malloc(sizeof(TabuList));
	tl->length = length;
	tl->min_length = min_length;
	tl->max_length = max_length;
	//tl->iteration = iteration;
	//tl->iteration_to_increase = iteration_to_increase;
	//tl->dynamic = dynamic;
	tl->fifo_queue = new_FIFO(length);
	return tl;
}
int check_TabuList(TabuList* tl, int x, int y, int type)
{
	return match_FIFO(tl->fifo_queue, x, y, type); // if the action is not swap
	//return match_FIFO(tl->fifo_queue, x, y) || match_FIFO(tl->fifo_queue, y, x); // because we're swapping
}
void insert_TabuList(TabuList* tl, int x, int y, int type)
{
	//insert_FIFO(tl->fifo_queue, y, x); IN CERTI CASI BISOGNA METTERE IN ORDINE INVERSO (attualmente non lo uso cosi) (in questi casi, potrei invertire l'ordine direttamente quando chiamo la insert)
	insert_FIFO(tl->fifo_queue, x, y, type);
}
void delete_TabuList(TabuList* tl)
{
	//free_FIFO(tl->fifo_queue);
	free(tl);
}
int increase_TabuList(TabuList* tl)
{
	if(tl->length < tl->max_length)
	{
		tl->length++;
		increase_FIFO(tl->fifo_queue);
	}
	return tl->length;
}
int decrease_TabuList(TabuList* tl)
{
	if(tl->length > tl->min_length)
	{
		tl->length--;
		decrease_FIFO(tl->fifo_queue);
	}
	return tl->length;
}
int update_TabuList(TabuList* tl, int length)
{
	while(length > tl->length && tl->length < tl->max_length)
	{
		increase_TabuList(tl);
	}
	while(length < tl->length && tl->length > tl->min_length)
	{
		decrease_TabuList(tl);
	}
	return tl->length;
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
		resetNode(pointer);
		pointer->next = fifo->head;
		fifo->head = pointer;
	}
	return fifo;
}
static int match_FIFO(FIFO* fifo, int x, int y, int type)
{
	FIFO_node *pointer = fifo->head;
	while(pointer != NULL)
	{
		if(pointer->x == x && pointer->y == y && pointer->type == type)
			return 1; //match found
		pointer = pointer->next;
	}
	return 0; //match not found
}
static void insert_FIFO(FIFO* fifo, int x, int y, int type) // remove the head, insert on tail (it could be the contrary also)
{
	FIFO_node *pointer = fifo->head;
	fifo->head->x = x; fifo->head->y = y; fifo->head->type = type; // remove the head and use the allocated node to insert on tail
	while(pointer->next != NULL)
		pointer = pointer->next; // at the end, pointer will point to the last node
	pointer->next = fifo->head; // insert the ex-head on tail
	fifo->head = fifo->head->next; // the new head is the second node
	pointer->next->next = NULL; //the ex-head is the tail now, so I put his next to NULL
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
static void resetNode(FIFO_node *pointer)
{
	pointer->x = -1; pointer->y = -1; pointer->type = -1;//null values at the beginning (I could manage not completely full FIFO, but maybe is not more efficient)
}
static void increase_FIFO(FIFO* fifo)
{
	FIFO_node *newNode = malloc(sizeof(FIFO_node));
	resetNode(newNode);
	newNode->next = fifo->head; // the new node is the head now, thus will be popped during the next insert
	fifo->head = newNode;
}
static void decrease_FIFO(FIFO* fifo)
{
	FIFO_node *to_remove = fifo->head; // the oldest move (the head) will be removed
	fifo->head = fifo->head->next; // the new head is the second node
	free(to_remove);
}
