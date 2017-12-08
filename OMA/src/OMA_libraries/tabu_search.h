/*
 * tabu_search.h
 *
 *  Created on: 05 dic 2017
 *      Author: Nicola
 */

#ifndef SRC_OMA_LIBRARIES_TABU_SEARCH_H_
#define SRC_OMA_LIBRARIES_TABU_SEARCH_H_

typedef struct tabulist_struct* TABU;

TABU new_TabuList(int length);//, int iteration, int iteration_to_increase, unsigned char dynamic);
int check_TabuList(TABU tl, int x, int y); // return 0 if the action is allowed
void insert_TabuList(TABU tl, int x, int y);
void delete_TabuList(TABU tl); // free the heap

#endif /* SRC_OMA_LIBRARIES_TABU_SEARCH_H_ */
