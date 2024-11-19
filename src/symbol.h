/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file symbol.h
 */
#ifndef SYMBOL_H
#define SYMBOL_H

#include "token.h"

/* Enumeration of specific symbols */
typedef enum symbol_t {
	DEFAULT,// uninitialised symbol
	ERROR,	// invalid symbol, internal error occured
	ADD,	// + addition
	SUB,	// - substraction
	MUL,	// * multiplication
	DIV,	// / division
	LS,		// < less
	GR,		// > greater
	LSE,	// <= less-equal
	GRE,	// >= greater-equal
	EQ,		// == equal
	NEQ,	// != inequal
	LBR,	// ( left bracket 
	RBR,	// ) right bracket
	I,		// IDENTIFIER,LITERAL_i32,LITERAL_f64,LITERAL_string
	R,		// '<' REDUCTION
	E_EXP,	// non-term solved expression
	E_OPERAND,// non-term solved operand
	END		// $ end marker
} symbol_t;

/* Enumeration of datatypes for literals and variables */
typedef enum datatype {
	NON_OPERAND,
	CONST_VAR_ID,
	U8_T,		// : []u8 slice type
	I32_T,	// : i32
	F64_T,	// : f64
	STRING_T, // : "string"
	NULL_T	// : null
} datatype_t;

/* Struct containing information about symbol thats being processed */
typedef struct symbol {
	symbol_t id;
	token_t token;
	datatype_t type;
} symbol;

/* prints information about symbol */
void print_symbol_info(symbol symbol);

#endif