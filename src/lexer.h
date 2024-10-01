/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * @author xtomasp00, Patrik Tomaško
 * 
 * @file lexer.h
 */

#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

#include "token.h"

/* Set of states for FSM */
typedef enum fsm_state {
	STATE_START,			// Default state (0)
	STATE_COMMENT_DIV,		// Possible SL/ML comment or division operator
	STATE_KW_IDENT,			// Possible keyword or identifier, can only start with alphabet character 'Aa - Zz' or underscore '_'
	STATE_EQUAL_ASSIGN,		// Possible == or =
	STATE_NOT_EQUAL,		// Possible !=, standalone '!' is invalid (causes lexical error!)
	STATE_LESS_LEQ,			// Possible < or <=
	STATE_GREATER_GREQ,		// Possible > or >=
	STATE_DIGIT_ZERO,			// Number starting with '0' can't have more than one digits
	STATE_DIGIT,				// Possible integer or floating point digit, can be also invalid lexeme (causes lexical error!)
	STATE_FLOATING_POINT,		// number[.]decimal
	STATE_DECIMAL_PART,			// number.[decimal]
	STATE_EXPONENT,				//number[E] or number.[E]
	STATE_EXPONENT_POSITIVE_NEGATIVE,	// number[E+] or number[E-]
	STATE_EXPONENT_FINAL,				//numberEsign[Decimal]
	STATE_UNDERSCORE,					// _
	STATE_PROLOG,						// @import
	STATE_STRING_START,					// "
    STATE_MULTILINE_STRING_START,       // //...
    STATE_NEXT_MULTILINE
} fsm_state;

/* Structure containing data about current configuration of the scanner */
typedef struct scanner {
	fsm_state p_state;		// Present state of FSM
	unsigned int row, col;	// Current row and column in the given file
	size_t head_pos;		// Position of reading head
	token_t current_token;	// Most recent token
} scanner_t;

/* Initialization of scanner*/
void init_scanner(void);

/* Final state machine, fetches token for syntax analyser */
token_t get_token(void);

//int is_keyword(char *lexeme);

/* Ignores every char until newline */
void ignore_comment();

/* Checks if the first character is either an alphabet character or '_' */
bool is_identifier(char c);

/* Prints information about current token (debug info) */
void print_token(token_t token);

#endif