/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xglosro00, Robert Glos
 * @author xukropj00, Jan Ukropec
 * 
 * @file main.c
 */

/* gradually complete tokens from list
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

*/
