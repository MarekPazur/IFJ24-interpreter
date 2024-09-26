/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file lexer.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "compiler_error.h"

scanner_t scanner = {0};
token_t token = {0};

int init_scanner(void) { 
	//todo
	scanner.row = 1;
	scanner.col = 0;

	return error; 
}

token_t* get_token(void) {
	//todo, testing	
	printf("token\n");

	char c = 0;

	while(c != EOF) {
		c = getc(stdin);

		if(c == '\n')
		{
			++scanner.row;
			scanner.col = 0;
		} else ++scanner.col;

		printf("%c",c);
	}

	printf(WHITE_BOLD("[%i:%i]"),scanner.row, scanner.col);

	return &token;
}
