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

typedef enum token_id {
	TOKEN_DEFAULT,			// Default state
	TOKEN_ERROR,			// Invalid token

	/* Variable or function identifier */
	TOKEN_IDENTIFIER,		// Sequence of alphanumerical characters + '_', must begin with <Aa-Zz> or '_'

	/* Keywords */
	TOKEN_KW_CONST,			// 'const' non modifiable
	TOKEN_KW_ELSE,			// 'else'
	TOKEN_KW_FN,			// 'fn'
	TOKEN_KW_IF,			// 'if'
	TOKEN_KW_I32,			// 'i32'
	TOKEN_KW_F64,			// 'f64'
	TOKEN_KW_NULL,			// 'null'
	TOKEN_KW_PUB,			// 'pub'
	TOKEN_KW_RETURN,		// 'return'
	TOKEN_KW_U8,			// 'u8'
	TOKEN_KW_VAR,			// 'var' modifiable
	TOKEN_KW_VOID,			// 'void'
	TOKEN_KW_WHILE,			// 'while'

	/* Brackets  */
	TOKEN_BRACKET_ROUND_LEFT, 		// '('
	TOKEN_BRACKET_ROUND_RIGHT,		// ')'
	TOKEN_BRACKET_CURLY_LEFT, 		// '{'
	TOKEN_BRACKET_CURLY_RIGHT,		// '}'
	TOKEN_BRACKET_SQUARE_LEFT,		// '['
	TOKEN_BRACKET_SQUARE_RIGHT,		// ']'

	/* Binary operators */
	TOKEN_ADDITION,			// '+'
	TOKEN_SUBSTRACTION,		// '-'
	TOKEN_MULTIPLICATION,	// '*'
	TOKEN_DIVISION, 		// '/' 
	
	TOKEN_EQUAL,			// '=='
	TOKEN_NOT_EQUAL,		// '!='
	TOKEN_LESS,				// '<'
	TOKEN_GREATER,			// '>'
	TOKEN_GREATER_EQUAL,	// '>='
	TOKEN_LESS_EQUAL,		// '<='

	TOKEN_ASSIGNMENT,		// '='

	/* Literals (Terms/Operands) */
	TOKEN_LITERAL_I32,		// Integer value '123'
	TOKEN_LITERAL_F64,		// Decimal value '123.456, 123.e1, 123.4e5, 123.e+456, 123.e-456'
	TOKEN_LITERAL_STRING,	// String "Hello world!"

	/* Other tokens */
	TOKEN_EOL,					// EOL \n
	TOKEN_EOF,					// EOF -1
	TOKEN_ACCESS_OPERATOR,		// 'ifj[.]'
	TOKEN_PROLOG,				// '@import' '@' is used for builtin functions
	TOKEN_BACKSLASH,			// '\'
	TOKEN_COLON,				// :
	TOKEN_SEMICOLON,			// ;
	TOKEN_DISCARD_RESULT,		// '_' Special pseudo-variable '_' is used for discarding (declared non-use) the result of an expression or calling a function with a return value
	TOKEN_NULL,					// 'null'
	TOKEN_PIPE,					// |
	TOKEN_OPTIONAL_TYPE_NULL,	// ? '?type', '{?}'
	TOKEN_COMMA 				// ,
} token_id;

/*
* Structure containing data about tokens
*/
typedef struct token {
	token_id id;			// ID of current token (token type)
	dynamic_array lexeme;	// Dyn. array type containing sequence of alphanumerical + '_' characters  (lexeme)
	// Lexeme containts identifiers, values
} token_t;

typedef struct token_buffer t_buf; 
typedef struct t_buf_item t_buf_item;

/**
* Item of the queue
*/
struct t_buf_item {
	token_t token;
	t_buf_item *next;
};

/*
* Token buffer implemented as Queue (FIFO)
*/
struct token_buffer {
	t_buf_item *head;
	t_buf_item *tail;
};

/*
* Prints information about current token (debug info)
* \param token_t token
*/
void print_token(token_t token);

// Token-Queue Buffer (FIFO) functions
/**
 * Initialises token-queue (token buffer).
 * \param t_buf buf (pointer to instance of t_buf)
 */
void init_t_buf (t_buf *buf);

/**
 * Checks if the token-queue is empty.
 * \param t_buf buf (pointer to instance of t_buf)
 * \return True: Queue is empty, False: Queue is NOT empty
 */
int is_empty_t_buf (t_buf *buf);

/**
 * Enqueues given token at the start of token-queue.
 * \param t_buf buf (pointer to instance of t_buf)
 * \param token_t token
 */
void enqueue_t_buf (t_buf *buf, token_t token);

/**
 * Returns and removes token from the beggining of the queue.
 * \param t_buf buf (pointer to instance of t_buf)
 * \return token_t token: enqueued token
 */
token_t get_t_buf (t_buf *buf);

/**
 * Removes every token from the token-queue and releases its memory.
 * \param t_buf buf (pointer to instance of t_buf)
 */
void free_t_buf (t_buf *buf);

/**
 * Prints the content of the token-queue.
 * \param t_buf buf (pointer to instance of t_buf)
 */
void print_t_buf (t_buf *buf);

// Utility functions

/**
 * Concatenates two given strings together. 
 * \param char* lex_prefix: first part of the string
 * \param char* lex_suffix: second part of the string
 * \return : Pointer to new string allocated on heap (dynamic alloc) 
 */
char *func_id_concat(char *lex_prefix, char *lex_suffix);

#endif 