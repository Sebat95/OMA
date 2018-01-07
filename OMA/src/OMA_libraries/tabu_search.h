#ifndef SRC_OMA_LIBRARIES_TABU_SEARCH_H_
#define SRC_OMA_LIBRARIES_TABU_SEARCH_H_

// LENGTH = 0 not controlled, too heavy, never passed directly in the first place

typedef struct tabulist_struct* TABU;

// TYPE FIELD: 0 = single exam move, 1 = whole set of exam in a certain timeslot moved

TABU new_TabuList(int length, int min_length, int max_length);//, int iteration, int iteration_to_increase, unsigned char dynamic);
int check_TabuList(TABU tl, int x, int y, int type); // return 0 if the action is allowed
void insert_TabuList(TABU tl, int x, int y, int type);
void delete_TabuList(TABU tl); // free the heap
int increase_TabuList(TABU tl); // increase tabu length by 1
int decrease_TabuList(TABU tl); // decrease tabu length by 1
int update_TabuList(TABU tl, int length); // set tabu length to length

#endif /* SRC_OMA_LIBRARIES_TABU_SEARCH_H_ */
