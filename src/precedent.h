/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file precedent.h
 */

#ifndef PRECEDENT_H
#define PRECEDENT_H

#include "token.h"

typedef enum pt_index_t {
	/*I as for INDEX*/
	I_ADD_SUB,		// +,-
	I_MUL_DIV,		// *,/
	I_LOGIC,		// <,>,<=,>=,==,!=
	I_L_BRACKET,	// (
	I_R_BRACKET,	// )
	I_ID_LIT,		// IDENTIFIER,LITERAL_i32,LITERAL_f64,LITERAL_string
	I_END_MARK		// $ STACK BOTTOM
} pt_index_t;

int pt_map(token_t term);

int precedent(void);

#endif