/*
 * queue_char.h
 *
 *  Created on: Feb 23, 2021
 *      Author: tymbys
 */

#ifndef INC_QUEUE_CHAR_H_
#define INC_QUEUE_CHAR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct queue_char {
    char a[1000];
    //простым условием head == tail
    int head;    //Индекс первого элемента.
    int tail;    //Индекс элемента, следующего за последним.
};

void queue_char_init(struct queue_char *qchar);
void char_push(struct queue_char *qchar, char x);
char char_pop(struct queue_char *qchar);
bool queue_char_is_empty(struct queue_char *qchar);


#endif /* INC_QUEUE_CHAR_H_ */
