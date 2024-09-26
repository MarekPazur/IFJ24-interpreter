/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file lexer.h
 */

#ifndef LEXER_H
#define LEXER_H

//todo
typedef enum token_id {
	TOKEN_A = 1,
	TOKEN_B = 2
} token_id;

/* Structure containing data about tokens */
typedef struct token {
	token_id id;
} token_t;

/* Structure containing data about current configuration of the scanner */
typedef struct scanner {
	unsigned int row, col;
	// todo
} scanner_t;

/* Initialization of scanner*/
int init_scanner(void);

/* Final state machine, fetches token for syntax analyser */
token_t* get_token(void);

#endif