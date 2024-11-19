/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file precedent.h
 */
#ifndef PRECEDENT_H
#define PRECEDENT_H

#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"
#include "token.h"
#include "lexer.h"

/* ITEM STRUCTURE */
typedef struct item {
    symbol symbol;    // Symbol for precedence
    struct item *next;
} item_t;

/* STACK STRUCTURE */
typedef struct stack {
    struct item *top;
} stack_t;

/* Enumeration of precedence table indexes */
typedef enum pt_index_t {
	/*I as for INDEX*/
	I_ADD_SUB,		// +,-
	I_MUL_DIV,		// *,/
	I_LOGIC,		// <,>,<=,>=,==,!=
	I_L_BRACKET,	// (
	I_R_BRACKET,	// )
	I_ID_LIT,		// IDENTIFIER,LITERAL_i32,LITERAL_f64,LITERAL_string
	I_END_MARKER	// $ STACK BOTTOM
} pt_index_t;

// Precedence analysis functions
/* Maps symbol into table */
int pt_map(symbol term);

/* Converts token to precedence symbol */
symbol token_to_symbol(token_t term);

/* Rules to solve expressions, check semantics */
void reduction(stack_t *stack, int expresion_length);	// > REDUCE
void shift(stack_t *stack, symbol next_symbol);			// < SHIFT
void equal(stack_t *stack, symbol next_symbol);			// = EQUAL

/* Precedent analysis main function */
int precedent(token_id end_marker);


// Symbol stack functions
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