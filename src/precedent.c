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

#include "compiler_error.h"
#include "precedent.h"
#include "token.h"
#include "stack.h"

#define PT_SIZE 7

/*@TODO
*
* FUNCTION IN EXPRESSIONS, FIX BRACKETS
*
*/

/* Simple term precedence table
* Priority depends on operator type and its association
* PT contains basic operators, (), identifiers or literals, $ = end marker
* --------------------
* '>' greater priority, invokes reduction
* '<' less priority, adds shift symbol
* '=' equal priority
* 'e' - as for error
* --------------------
*   row 		  col
*[stack_top_term] [next_token]
*/
static int precedence_table[PT_SIZE][PT_SIZE] = {
//   [ +- | */ |  l |  ( |  ) |  i |  $ ]
	/*0*/{ '>', '<', '>', '<', '>', '<', '>'}, //|+-| ARITHMETIC OPERATORS
	/*1*/{ '>', '>', '>', '<', '>', '<', '>'}, //|*/| * / ARITHMETIC OPERATORS
	/*2*/{ '<', '<', '>', '<', '>', '<', '>'}, //| l| > < => <= == != LOGIC OPERATORS
	/*3*/{ '<', '<', '<', '<', '=', '<', 'e'}, //| (| (
	/*4*/{ '>', '>', '>', 'e', '>', 'e', '>'}, //| )| )
	/*5*/{ '>', '>', '>', 'e', '>', 'e', '>'}, //| i| IDENTIFIER LITERAL_i32 LITERAL_f64 LITERAL_string
	/*6*/{ '<', '<', '<', '<', 'e', '<', 'e'}, //| $| STACK BOTTOM
//   [  0 |  1 |  2 |  3 |  4 |  5 |  6 ]
};

/* Maps symbol into PT */
int pt_map(symbol sym) {
	int index = 0;

	switch (sym.id) {
	case ADD: 				// +
	case SUB:				// -
		index = I_ADD_SUB; 	// I_ADD_SUB = 0
		break;

	case MUL:				// *
	case DIV: 				// /
		index = I_MUL_DIV;	// I_MUL_DIV = 1
		break;

	case EQ: 				// ==
	case NEQ: 				// !=
	case LS: 				// <
	case GR:	 			// >
	case GRE: 				// >=
	case LSE: 				// <=
		index = I_LOGIC;	// I_LOGIC = 2
		break;

	case LBR:				// (
		index = I_L_BRACKET;// I_L_BRACKET = 3
		break;
	case RBR:				// )
		index = I_R_BRACKET;// I_R_BRACKET = 4
		break;

	case I:					// IDENTIFIER, integer literal, float literal, string literal, null
		index = I_ID_LIT;	// I_ID_ILIT = 5
		break;

	default:				// else
		index = I_END_MARKER;// I_END_MARK = 6
		break;
	}

	return index;
}
/* Creates symbol out of processed token */
symbol token_to_symbol(token_t term) {
	symbol symbol;
	symbol.id = DEFAULT; // uninitialised
	symbol.token = term; // assign processed token
	symbol.type = NON_OPERAND; // operator or end marker

	switch (term.id) {
	/* Arithmetic OP */
	case TOKEN_ADDITION: 			// +
		symbol.id = ADD;
		break;
	case TOKEN_SUBSTRACTION:		// -
		symbol.id = SUB; 			// I_ADD_SUB = 0
		break;
	case TOKEN_MULTIPLICATION:		// *
		symbol.id = MUL;
		break;
	case TOKEN_DIVISION: 			// /
		symbol.id = DIV;			// I_MUL_DIV = 1
		break;
	/* Logic OP */
	case TOKEN_EQUAL: 				// ==
		symbol.id = EQ;
		break;
	case TOKEN_NOT_EQUAL: 			// !=
		symbol.id = NEQ;
		break;
	case TOKEN_LESS: 				// <
		symbol.id = LS;
		break;
	case TOKEN_GREATER:	 			// >
		symbol.id = GR;
		break;
	case TOKEN_GREATER_EQUAL: 		// >=
		symbol.id = GRE;
		break;
	case TOKEN_LESS_EQUAL: 			// <=
		symbol.id = LSE;			// I_LOGIC = 2
		break;

	case TOKEN_BRACKET_ROUND_LEFT:	// (
		symbol.id = LBR;		// I_L_BRACKET = 3
		break;
	case TOKEN_BRACKET_ROUND_RIGHT:	// )
		symbol.id = RBR;		// I_R_BRACKET = 4
		break;
	/* ID and type literals are i symbol */
	case TOKEN_IDENTIFIER:			// IDENTIFIER
		symbol.id = I;
		//symbol.type = symtable_get(token.lexeme).type
		break;
	case TOKEN_LITERAL_I32:			// integer literal
		symbol.id = I;
		symbol.type = I32;
		break;
	case TOKEN_LITERAL_F64:			// float literal
		symbol.id = I;
		symbol.type = F64;
		break;
	case TOKEN_LITERAL_STRING:		// string literal
		symbol.id = I;
		symbol.type = STRING;
		break;
	case TOKEN_KW_NULL:
		symbol.id = I;			// I_ID_ILIT = 5
		symbol.type = NULL_T;
		break;

	default:						// else
		symbol.id = END;			// I_END_MARK = 6
		break;
	}

	return symbol;
}

void equal(stack_t *stack, symbol next_symbol) {
	push(stack, next_symbol);

	if (error) {
		fprintf(stderr, "error: equal precedence error");
	}
}

void shift(stack_t *stack, symbol next_symbol) {
	insert_shift(stack);	// Inserts shift symbol
	push(stack, next_symbol);	// Pushes next symbol

	if (error) {
		fprintf(stderr, "error: equal precedence error");
	}
}

/* Expression reduction with semantic checks */
/*@TODO semantics*/
void reduction(stack_t *stack, int expresion_length) {
	/* E -> i */
	if (expresion_length == 1) {
		symbol E = pop(stack); // literal, identifier
		pop(stack); 		   // pop shift

		E.id = E_OPERAND;
		push(stack, E);
	} else if (expresion_length == 3) {
		/* E1 op E2 OR (E) */
		symbol E2 = pop(stack); // expression
		symbol op = pop(stack); // operand
		symbol E1 = pop(stack); // expression
		pop(stack);				// pop shift

		/* Create new non-term */
		symbol E = {.id = E_EXP,
		            .token = {.id = TOKEN_DEFAULT, .lexeme = {.array = NULL}},
		            .type = NON_OPERAND
		           };
		/* Semantics check */

		/* Case when given expression uses E -> (E) rule, otherwise consider thee expression as arithmetic or logic */
		if (E1.id == LBR && (op.id == E_OPERAND || op.id == E_EXP) && E2.id == RBR) {// E1 = (, OP = E, E2 = )
			E.type = op.type; // Pass the data type
			push(stack, E);
			return;
		}

		/* If E1 or E2 is not non-term, throw syntax error */
		if ((E1.id != E_OPERAND && E1.id != E_EXP) || (E2.id != E_OPERAND && E2.id != E_EXP)) {
			error = ERR_SYNTAX;
			return;
		}

		/* Operators can't be applied on STRING literals or []u8 type (slice) */
		if ((E1.type == STRING || E2.type == STRING) || (E1.type == U8 || E2.type == U8)) {
			printf("error: string literal or []u8 type used in expression!\n");
			error = ERR_TYPE_COMPATABILITY;
			return;
		}

		/* Further operator and type checking */
		/* ARITHMETIC OPERATORS */
		if (op.id == ADD) {

		} else if (op.id == SUB) {

		} else if (op.id == MUL) {

		} else if (op.id == DIV) {
			if (E2.type == I32 && E2.token.lexeme.array[0] == '0')
			{
				printf("error: division by zero causes undefined behavior!\n");
				error = ERR_SEMANTIC_OTHER;
				return;
			}
		} /* LOGIC OPERATORS */
		else if (op.id == LS) {

		} else if (op.id == GR) {

		} else if (op.id == LSE) {

		} else if (op.id == GRE) {

		} else if (op.id == EQ) {

		} else if (op.id == NEQ) {

		}
		else {
			printf("error: invalid syntax in expression, rule doesn't exist!\n");
			error = ERR_SYNTAX;
		}

		push(stack, E);
	} else {
		printf("error: invalid subexpression!\n");
		error = ERR_SYNTAX;
	}
}

#define PUSH_DEBUG(symbol_id) push(&sym_stack, (symbol) {.id = symbol_id, .token = {.id = TOKEN_DEFAULT, .lexeme = {.array = NULL}}, .type = NON_OPERAND})

/* Precedent analysis main function */
int precedent(void) {
	/*@TODO
	 *symbol stack OK
	 *shift OK
	 *reduce OK
	 *semantics
	 *binary tree + codegen */
	stack_t sym_stack; // Stack of symbols
	init_stack(&sym_stack);
	PUSH_DEBUG(END);	//PUSH_DEBUG(R);PUSH_DEBUG(LBR);PUSH_DEBUG(E_OPERAND);/*PUSH_DEBUG(END);PUSH_DEBUG(R);PUSH_DEBUG(LBR);PUSH_DEBUG(R);PUSH_DEBUG(E_OPERAND);PUSH_DEBUG(MUL);PUSH_DEBUG(E_OPERAND);*/
	token_t token;	// Input (next) term
	symbol top_term, next_term;	// Tokens in symbol form
	bool read_enable = true, expr_solved = false; // Enable/Disable read when needed during expression reduction, break cycle if the expression is solved

	top_term = get_topmost_term(&sym_stack); // $ at the bottom of the stack
	next_term = token_to_symbol((token = get_token())); // anything from the input

	while (expr_solved == false) { // Repeat until $ = $ -> stack top = input term
		int precedence = precedence_table[pt_map(top_term)][pt_map(next_term)]; // term precedence
		
		switch (precedence) {
		case '=':
			/* Pushes input symbol onto stack top*/
			printf("[=]\n");
			equal(&sym_stack, next_term);
			read_enable = true;
			break;
		case '<':
			/* Puts SHIFT symbol after topmost TERM, pushes input symbol onto stack top*/
			printf("[< - SHIFT]\n");
			shift(&sym_stack, next_term);
			read_enable = true;
			break;
		case '>':
			/* Reduces the expression between '<' SHIFT symbol and stack top included using given RULE */
			printf("[> - REDUCTION]\n");
			reduction(&sym_stack, reduction_count(&sym_stack));
			read_enable = false; /* Will reduce until SHIFT or solved expression */
			break;
		case 'e':
			//print_symbol_info(top_term);print_symbol_info(next_term);print_stack_content(&sym_stack);
			printf("error: invalid expression (empty expression, operator precedence error)\n");print_token(token);
			error = ERR_SYNTAX;
			break;
		default:
			printf("error: invalid expression (precedence unexpected error)\n");
			error = ERR_COMPILER_INTERNAL;
			break;
		}

		if (error) // Error occured
			break;

		top_term = get_topmost_term(&sym_stack);

		if (read_enable)
			next_term = token_to_symbol((token = get_token()));

		if ((top_term.id == END && next_term.id == END)) // $ = $ -> topmost term in stack = next term, expression is finally solved $E$
			expr_solved = true;
	}


	//print_token(token);
	print_stack_content(&sym_stack);
	free_stack(&sym_stack);
	//print_stack_content(&sym_stack);

	if (error == 0)
		printf(GREEN("Expression solved!")"\n");
	else printf(RED("Unable to solve given expression!")"\n");

	return 0;
}