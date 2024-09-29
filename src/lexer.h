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
	STATE_EQUAL_ASSIGN,
	STATE_NOT_EQUAL,
	STATE_LESS_LEQ,
	STATE_GREATER_GREQ
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

int is_keyword(char *lexeme);

void ignore_comment();

void print_token(token_t token);

bool is_identifier(char c);

#endif