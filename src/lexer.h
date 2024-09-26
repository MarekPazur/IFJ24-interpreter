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

//@TODO finish and maybe rework
typedef enum token_id {
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
	TOKEN_ROUND_LEFT, 		// (
	TOKEN_ROUND_RIGHT,		// )
	TOKEN_CURLY_LEFT, 		// {
	TOKEN_CURLY_RIGHT,		// }
	TOKEN_SQUARE_LEFT,		// [
	TOKEN_SQUARE_RIGHT,		// ]

	/* Binary operators */
	TOKEN_ADDITION,			// +
	TOKEN_SUBSTRACTION,		// -
	TOKEN_MULTIPLICATION,	// *
	TOKEN_DIVISION_I32, 	// / int division
	TOKEN_DIVISION_F64,		// / double division
	
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
typedef enum p_state {
	STATE_START = 0,
	STATE_B = 1
} p_state;

/* Structure containing data about tokens */
typedef struct token {
	token_id id;
} token_t;

/* Structure containing data about current configuration of the scanner */
typedef struct scanner {
	// todo
	unsigned int row, col;
	size_t head_pos;
	token_t current_token;
} scanner_t;

/* Initialization of scanner*/
int init_scanner(void);

/* Final state machine, fetches token for syntax analyser */
token_t* get_token(void);

void print_token(token_t token);

#endif