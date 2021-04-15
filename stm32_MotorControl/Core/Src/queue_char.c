/*
 * queue_char.c
 *
 *  Created on: Feb 23, 2021
 *      Author: tymbys
 */
#include "queue_char.h"

void queue_char_init(struct queue_char *qchar) {
    if(qchar == NULL) {
        printf("Error: %s:%d", __FILE__, __FILE__);
        return;
    }

    qchar->head = 0;
    qchar->tail = 0;
}

void char_push(struct queue_char *qchar, char x) {
    if(qchar == NULL) {
        printf("Error: %s:%d", __FILE__, __FILE__);
        return;
    }

    qchar->a[qchar->tail] = x;
    qchar->tail++;
}

char char_pop(struct queue_char *qchar) {
    if(qchar == NULL){
        printf("Error: %s:%d", __FILE__, __FILE__);
        return -1;
    }

    if (qchar->head != qchar->tail) {
        qchar->head++;
        return qchar->a[qchar->head - 1];
    } else {
        //Ошибка, попытка извлечь элемент из пустой очереди.
    }
}

bool queue_char_is_empty(struct queue_char *qchar) {
    return qchar->head == qchar->tail;
}


