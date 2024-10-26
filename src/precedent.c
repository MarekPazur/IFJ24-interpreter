/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file precedent.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "precedent.h"
#include "token.h"
#include "stack.h"

#define PT_SIZE 7

/* Simple term precedence table */
static int precedence_table[PT_SIZE][PT_SIZE] = {
/* Priority depends on op type and association */
/* PT contains basic operators, (), identifiers or literals, $ = end marker */

//   |+-|*/| l| (| )| i| $|
/*0*/{ 0, 0, 0, 0, 0, 0, 0},// + - ARITHMETIC OPERATORS
/*1*/{ 0, 0, 0, 0, 0, 0, 0},//|*/| * / ARITHMETIC OPERATORS
/*2*/{ 0, 0, 0, 0, 0, 0, 0},//| l| > < => <= == != LOGIC OPERATORS
/*3*/{ 0, 0, 0, 0, 0, 0, 0},//| (| ( 
/*4*/{ 0, 0, 0, 0, 0, 0, 0},//| )| )
/*5*/{ 0, 0, 0, 0, 0, 0, 0},//| i| IDENTIFIER LITERAL_i32 LITERAL_f64 LITERAL_string
/*6*/{ 0, 0, 0, 0, 0, 0, 0},//| $| STACK BOTTOM
//     0  1  2  3  4  5  6
};

/* Mapping function for PT table */
int pt_map(token_t term) {
	int index = 0;

	switch(term.id) {
		case TOKEN_ADDITION: 			// +
		case TOKEN_SUBSTRACTION:		// -
			index = I_ADD_SUB; 			// I_ADD_SUB = 0
			break;

		case TOKEN_MULTIPLICATION:		// *
		case TOKEN_DIVISION: 			// /
			index = I_MUL_DIV;			// I_MUL_DIV = 1
			break;

		case TOKEN_EQUAL: 				// ==
		case TOKEN_NOT_EQUAL: 			// !=	
		case TOKEN_LESS: 				// <
		case TOKEN_GREATER:	 			// >
		case TOKEN_GREATER_EQUAL: 		// >=
		case TOKEN_LESS_EQUAL: 			// <=
			index = I_LOGIC;			// I_LOGIC = 2
			break;

		case TOKEN_BRACKET_ROUND_LEFT:	// (
			index = I_L_BRACKET;		// I_L_BRACKET = 3
			break;
		case TOKEN_BRACKET_ROUND_RIGHT:	// )
			index = I_R_BRACKET;		// I_R_BRACKET = 4
			break;

		case TOKEN_IDENTIFIER:			// IDENTIFIER
		case TOKEN_LITERAL_I32:			// integer literal
		case TOKEN_LITERAL_F64:			// float literal
		case TOKEN_LITERAL_STRING:		// string literal
			index = I_ID_LIT;			// I_ID_ILIT = 5
			break;

		default:						// else
			index = I_END_MARK;			// I_END_MARK = 6
			break;
	}

	return index;
}

/* Precedent analysis main function */
int precedent(void) {

	token_t stack_top, next;
	stack_top.id = TOKEN_MULTIPLICATION;
	next.id = TOKEN_ADDITION;

	int precedence = precedence_table[pt_map(stack_top)][pt_map(next)];

	if(precedence == 0)
		printf("Test successful");

	return 0;
}