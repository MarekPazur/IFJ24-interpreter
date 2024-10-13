#ifndef STACK_H // Resolves issues regarding inclusion in multiple source files
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include "token.h"

/* ITEM STRUCTURE */
struct item {
    token_t token;
    struct item *next;
};

/* STACK STRUCTURE */
struct stack {
    struct item *last;
};

/* FUNCTION PROTOTYPES */
struct stack* init_stack();
struct item* init_item(token_t token);
void push(struct stack *s, token_t token);
token_t pop(struct stack *s);
void free_stack(struct stack *s);

#endif
