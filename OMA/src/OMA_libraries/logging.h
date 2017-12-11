/*
 * logging.h
 *
 *  Created on: 08 dic 2017
 *      Author: Samuele
 */

#ifndef SRC_OMA_LIBRARIES_LOGGING_H_
#define SRC_OMA_LIBRARIES_LOGGING_H_

//put them to compute time&CLKs of a certain piece of code and write it in a log
//ONE TIMER MAXIMUM
void start_timer();
void stop_timer();
//ask to log something; n is the number of string passed with things
void logger(char **things, int n);
//ask to log and directly write it on the file log.txt
void instant_wb(char **things, int n);
//flush the log buffer on file
void write_back();

#endif /* SRC_OMA_LIBRARIES_LOGGING_H_ */
