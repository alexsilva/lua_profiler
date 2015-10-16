/* ED4: pilha3.h */

#include <time.h>
#include "measure.h"

typedef struct no STACK_RECORD;

struct no {
    Meta *meta;
    STACK_RECORD *prox;
    int index;
};

typedef STACK_RECORD *STACK;

void init(STACK *p);

void push(STACK *stack, STACK_RECORD stack_record);

STACK_RECORD pop(STACK *stack);

int empty(STACK *stack);

