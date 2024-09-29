/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file token.h
 */

#ifndef TOKEN_H
#define TOKEN_H

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

/* Structure containing data about tokens */
typedef struct token {
	token_id id;			// ID of current token
	dynamic_array lexeme;	// Dyn. array containing sequence of alphanumerical + '_' characters  (lexeme)
} token_t;

#endif 