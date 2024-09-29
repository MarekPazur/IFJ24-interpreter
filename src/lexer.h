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

#include "dynamic_array.h"

//@TODO finish and maybe rework
typedef enum token_id {
	TOKEN_DEFAULT,
	TOKEN_ERROR,

	/* Variable or function identifier */
	TOKEN_IDENTIFIER,		// sequence of alphanumerical characters + '_'

	/* Keywords */
	TOKEN_KW_CONST,			// const
	TOKEN_KW_ELSE,			// else
	TOKEN_KW_FN,			// fn
	TOKEN_KW_IF,			// if
	TOKEN_KW_I32,			// i32
	TOKEN_KW_F64,			// f64
	TOKEN_KW_NULL,			// null
	TOKEN_KW_PUB,			// pub
	TOKEN_KW_RETURN,		// return
	TOKEN_KW_U8,			// u8
	TOKEN_KW_VAR,			// var
	TOKEN_KW_VOID,			// void
	TOKEN_KW_WHILE,			// while

	/* Brackets  */
	TOKEN_BRACKET_ROUND_LEFT, 		// (
	TOKEN_BRACKET_ROUND_RIGHT,		// )
	TOKEN_BRACKET_CURLY_LEFT, 		// {
	TOKEN_BRACKET_CURLY_RIGHT,		// }
	TOKEN_BRACKET_SQUARE_LEFT,		// [
	TOKEN_BRACKET_SQUARE_RIGHT,		// ]

	/* Binary operators */
	TOKEN_ADDITION,			// +
	TOKEN_SUBSTRACTION,		// -
	TOKEN_MULTIPLICATION,	// *
	TOKEN_DIVISION, 	// / int division
	
	TOKEN_EQUAL,			// ==
	TOKEN_NOT_EQUAL,		// !=
	TOKEN_LESS,				// <
	TOKEN_GREATER,			// >
	TOKEN_GREATER_EQUAL,	// >=
	TOKEN_LESS_EQUAL,		// <=

	TOKEN_ASSIGNMENT,		// = 

	/* Literals (Operands) */
	TOKEN_LITERAL_I32,		// 123
	TOKEN_LITERAL_F64,		// 123.456
	TOKEN_LITERAL_STRING,	// "Hello world!"

	/* Other tokens */
	TOKEN_EOL,				// EOL \n
	TOKEN_EOF,				// EOF -1
	TOKEN_NAMESPACE,		// 'ifj.'
	TOKEN_PROLOG,			// '@' 
	TOKEN_BACKSLASH,		// '\'
	TOKEN_COLON,			// :
	TOKEN_SEMICOLON,		// ;
	TOKEN_POINT,			// .
	TOKEN_NULL,				// 'null'
	TOKEN_PIPE,				// |
	TOKEN_OPTIONAL_TYPE,	// ?
	TOKEN_SLICE 			// []
} token_id;

/* Set of states for FSM */
typedef enum fsm_state {
	STATE_START,			// Default state (0)
	//STATE_COMMENT_DIV,		// Possible SL/ML comment or division operator
	//STATE_KW_IDENT,			// Possible keyword or identifier, can only start with alphabet character 'Aa - Zz' or underscore '_' 
	//STATE_WHITESPACE,		// Whitespace character
	//STATE_EOF,				// End of file

} fsm_state;

/* Structure containing data about tokens */
typedef struct token {
	token_id id;			// ID of current token
	dynamic_array lexeme;	// Dyn. array containing sequence of alphanumerical + '_' characters  (lexeme)
} token_t;

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

void print_token(token_t token);

#endif