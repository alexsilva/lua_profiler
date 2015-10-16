/* ED4: pilha3.c */

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

void init(STACK *p) {
    *p = NULL;
}

void push(STACK *stack, STACK_RECORD stack_record) {
    STACK q;
    q = (STACK) malloc(sizeof(STACK_RECORD));
    *q = stack_record;
    q->prox = *stack;
    *stack = q;
}

STACK_RECORD pop(STACK *stack) {
    STACK_RECORD stack_record;
    STACK q;

    stack_record = **stack;
    q = *stack;
    *stack = (*stack)->prox;
    free(q);
    return stack_record;
}

int empty(STACK *stack) {
    return *stack == NULL;
}
