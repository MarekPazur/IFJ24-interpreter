/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * @author xglosro00, Robert Glos
 * 
 * @file main.c
 */
#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>

#include "symbol.h"

/* ITEM STRUCTURE */
typedef struct item {
    symbol symbol;    // Symbol for precedence
    struct item *next;
} item_t;

/* STACK STRUCTURE */
typedef struct stack {
    struct item *top;
} stack_t;

/* Initializes stack */
void init_stack(stack_t* stack);

/* Allocates new item */
struct item* alloc_item(symbol symbol);

/* Pushes item on top of the stack */
void push(struct stack *s, symbol symbol);

/* Pops item out of top of the stack */
symbol pop(struct stack *s);

/* Returns top item without popping it from the stack */
symbol get_top(struct stack *s);

/* Returns TOPMOST terminal item */
symbol get_topmost_term(struct stack *s);

/* Returns the number of symbols to be reduced */
int reduction_count(struct stack *s);

/* Inserts shift symbol after topmost terminal */
void insert_shift(struct stack *s);

/* Frees stack content */
void free_stack(struct stack *s);

/* Prints the content of the stack, used for debugging */
void print_stack_content(stack_t *stack);

#endif
