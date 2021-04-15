/*
 * utils.h
 *
 *  Created on: Feb 23, 2021
 *      Author: tymbys
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    char val;
    struct node *next;
} node_t;

void enqueue(node_t **head, int val) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->val = val;
    new_node->next = *head;

    *head = new_node;
}

int dequeue(node_t **head) {
    node_t *current, *prev = NULL;
    int retval = -1;

    if (*head == NULL) return -1;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}

void print_list(node_t *head) {
    node_t *current = head;

    while (current != NULL) {
        printf("%d\n", current->val);
        current = current->next;
    }
}



#endif /* INC_UTILS_H_ */
