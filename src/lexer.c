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

/**
 * @brief This function determines wether the character could be a part of an identifier
 *
 * @param c character to be examined
 *
 * @return returns true if the sign could be a part of the identifier, false otherwise
 */
bool is_identifier(char c){
    if(isalnum(c) || c == '_'){
        return true;
    }
    return false;
}

token_t get_token(void) {

	token_t token;
	token.id = TOKEN_DEFAULT;
	d_array_init(&token.lexeme, 16);

	//print_token(token);
	//print_error(error);

	int c = 0;

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
                if(is_identifier(c)){
                    d_array_append(&token.lexeme, c);
                    token.id = TOKEN_IDENTIFIER;
                    scanner.p_state = STATE_KW_IDENT;
                }
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

				break;
            case STATE_KW_IDENT:
                if(is_identifier(c)){
                    d_array_append(&token.lexeme, c);
                }else{
                    scanner.p_state = STATE_START;
                    ungetc(c, stdin);
                    d_array_print(&token.lexeme);
                    return token;
                }
                break;

			default:
				token.id = TOKEN_ERROR;
				return token;
		}

	}

	return token;
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