/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "compiler_error.h"
#include "lexer.h"
#include "dynamic_array.h"
#include "syna.h"
#include "precedent.h"

/*
 * @TODO
 * lexical analyser = OK
 * syntax analyser - rekurzivni sestup = OK
 * syntax analyser - precedence
 * symtable - vyvazeny binarni strom = OK
 * obecny binarni strom = OK
 * stack = OK
 * semantic analyser
 * code generator
 */

int main(void) {
	/* Init. of scanner struct */
	init_scanner();

	if(error) {
		print_error(error);
	}

	/* Testing phase, will be rewritten */
	token_t test = {.id = TOKEN_BRACKET_ROUND_RIGHT};
	//init_parser(test);

	precedent(test.id);

	/* Uncomment for lexer debug info */       
/*	while((test = get_token()).id != TOKEN_EOF && test.id != TOKEN_ERROR) {
			print_token(test);
			d_array_free(&test.lexeme);
	}

	print_token(test);

	if(test.id == TOKEN_ERROR) {
			print_token(test);
			d_array_free(&test.lexeme);
		}*/
        
	if(error) {
		print_error(error);
	}

	return error;
}