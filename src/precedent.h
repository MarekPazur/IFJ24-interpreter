/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file precedent.h
 */
#ifndef PRECEDENT_H
#define PRECEDENT_H

#include "stack.h"
#include "symbol.h"
#include "token.h"
#include "lexer.h"

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

/* Maps symbol into table */
int pt_map(symbol term);

/* Converts token to precedence symbol */
symbol token_to_symbol(token_t term);

/* Uses rules to reduce terms */
void reduction(stack_t *stack, int expresion_length);	// > REDUCE
void shift(stack_t *stack, symbol next_symbol);			// < SHIFT
void equal(stack_t *stack, symbol next_symbol);			// = EQUAL

/* Precedent analysis main function */
int precedent(void);

#endif