/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * @author xtomasp00, Patrik Tomaško
 * 
 * @file lexer.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "lexer.h"
#include "compiler_error.h"

scanner_t scanner = {0};

void init_scanner(void) { 
	scanner.p_state = STATE_START;
	scanner.row = 1;
	scanner.col = 0;
	scanner.head_pos = 0;
}

token_t get_token(void) {

	token_t token;
	token.id = TOKEN_DEFAULT;
	d_array_init(&token.lexeme, 16);

	scanner.p_state = STATE_START;

	char c = 0;

	while(true) {

		c = getc(stdin);

		if(c == '\n') {
			++scanner.row;
			scanner.col = 0;
		}

		++scanner.col;
		++scanner.head_pos;

		switch(scanner.p_state) {
			case STATE_START:
				/* Simple states */
				if(c == EOF) {
					token.id = TOKEN_EOF;
					return token;
				}

				if(c == '(') {
					token.id = TOKEN_BRACKET_ROUND_LEFT;
					return token;
				}

				if(c == ')') {
					token.id = TOKEN_BRACKET_ROUND_RIGHT;
					return token;
				}

				if(c == '{') {
					token.id = TOKEN_BRACKET_CURLY_LEFT;
					return token;
				}

				if(c == '}') {
					token.id = TOKEN_BRACKET_CURLY_RIGHT;
					return token;
				}				

				if(c == '+') {
					token.id = TOKEN_ADDITION;
					return token;
				}

				if(c == '-') {
					token.id = TOKEN_SUBSTRACTION;
					return token;
				}

				if(c == '*') {
					token.id = TOKEN_MULTIPLICATION;
					return token;
				}

				if(c == ':') {
					token.id = TOKEN_COLON;
					return token;
				}

				if(c == ';') {
					token.id = TOKEN_SEMICOLON;
					return token;
				}				

				/* Problematic states */
				if(c == '/') { // Division op. or comment
					scanner.p_state = STATE_COMMENT_DIV;
				}	

				if(c == '=') { // = or ==
					scanner.p_state = STATE_EQUAL_ASSIGN;
				}

				if(c == '!') { // Can be only !=
					scanner.p_state = STATE_NOT_EQUAL;
				}

				if(c == '<') { // < or <=
					scanner.p_state = STATE_LESS_LEQ;
				}

				if(c == '>') { // > or >=
					scanner.p_state = STATE_GREATER_GREQ;
				}

				break;

			case STATE_COMMENT_DIV:
				// Single line comment
				if(c == '/'){
					ignore_comment();
					scanner.p_state = STATE_START;
				} 

				else { // division operator
					token.id = TOKEN_DIVISION;

					//scanner.p_state = STATE_START;

					ungetc(c,stdin);	// Necessary to put loaded char back to stream

					return token;
				}
				break;

			case STATE_EQUAL_ASSIGN:
				if(c == '=') {
					token.id = TOKEN_EQUAL;
					//scanner.p_state = STATE_START;
					return token;
				}
				else {
					token.id = TOKEN_ASSIGNMENT;
					//scanner.p_state = STATE_START;
					ungetc(c,stdin);
					return token;
				}
				break;	

			case STATE_NOT_EQUAL:

				if(c == '=') {
					token.id = TOKEN_NOT_EQUAL;
					//scanner.p_state = STATE_START;
					return token;
				} else { // Standalone '!' is not any lexeme
					token.id = TOKEN_ERROR;
					//scanner.p_state = STATE_START;

					error = ERR_LEXICAL;
					ungetc(c, stdin);
					return token;
				}

				break;

			case STATE_LESS_LEQ:
				if(c == '=') {
					token.id = TOKEN_LESS_EQUAL;
					//scanner.p_state = STATE_START;
					return token;
				}
				else {
					token.id = TOKEN_LESS;
					//scanner.p_state = STATE_START;
					ungetc(c,stdin);
					return token;
				}

				break;

			case STATE_GREATER_GREQ:
				if(c == '=') {
					token.id = TOKEN_GREATER_EQUAL;
					//scanner.p_state = STATE_START;
					return token;
				}
				else {
					token.id = TOKEN_GREATER;
					//scanner.p_state = STATE_START;
					ungetc(c,stdin);
					return token;
				}
				break;	

			default:
				token.id = TOKEN_ERROR;
				return token;
				break;
		}

	}

	return token;
}

void ignore_comment() {
	char c;

	while((c = getc(stdin)) != '\n' && c != EOF);

	if(c == EOF) // Necessary to return EOF, since it is standalone token
		ungetc(c,stdin);
}

void print_token(token_t token) {
	char *token_info[] = {
	"DEFAULT",
	"ERROR",
	/* Variable or function identifier */
	"TOKEN_IDENTIFIER",

	/* Keywords */
	"TOKEN_KW_CONST",
	"TOKEN_KW_ELSE",
	"TOKEN_KW_FN",
	"TOKEN_KW_IF",
	"TOKEN_KW_I32",
	"TOKEN_KW_F64",
	"TOKEN_KW_NULL",
	"TOKEN_KW_PUB",
	"TOKEN_KW_RETURN",
	"TOKEN_KW_U8",
	"TOKEN_KW_VAR",
	"TOKEN_KW_VOID",
	"TOKEN_KW_WHILE",

	/* Brackets  */
	"TOKEN_BRACKET_ROUND_LEFT",
	"TOKEN_BRACKET_ROUND_RIGHT",
	"TOKEN_BRACKET_CURLY_LEFT",
	"TOKEN_BRACKET_CURLY_RIGHT",
	"TOKEN_BRACKET_SQUARE_LEFT",
	"TOKEN_BRACKET_SQUARE_RIGHT",

	/* Binary operators */
	"TOKEN_ADDITION",
	"TOKEN_SUBSTRACTION,",
	"TOKEN_MULTIPLICATION",
	"TOKEN_DIVISION",
	
	"TOKEN_EQUAL",
	"TOKEN_NOT_EQUAL",
	"TOKEN_LESS",
	"TOKEN_GREATER",
	"TOKEN_GREATER_EQUAL",
	"TOKEN_LESS_EQUAL",

	"TOKEN_ASSIGNMENT",

	/* Literals (Operands) */
	"TOKEN_LITERAL_I32",
	"TOKEN_LITERAL_F64",
	"TOKEN_LITERAL_STRING",

	/* Other tokens */
	"TOKEN_EOL",
	"TOKEN_EOF",
	"TOKEN_NAMESPACE",
	"TOKEN_PROLOG",
	"TOKEN_BACKSLASH",
	"TOKEN_COLON",
	"TOKEN_SEMICOLON",
	"TOKEN_POINT",
	"TOKEN_NULL",
	"TOKEN_PIPE",
	"TOKEN_OPTIONAL_TYPE",
	"TOKEN_SLICE"};
	printf(WHITE_BOLD("TOKEN: %s\n"), token_info[token.id]);
	//d_array_print(&token.lexeme);
}