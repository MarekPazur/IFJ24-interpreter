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

int main(void) {
	//TODO
	//lexical analyser
	//syntax analyser - rekurzivni sestup?
	//symtable - vyvazeny binarni strom?
	//semantic analyser
	//code generator

	/* Init. of scanner struct */
	init_scanner();

	if(error)
		print_error(error);

	/* Testing phase, will be rewritten */
	token_t test;
	init_parser(test);
        
	/*while((test = get_token()).id != TOKEN_EOF && test.id != TOKEN_ERROR) {
			print_token(test);
			d_array_free(&test.lexeme);
		}

	if(test.id == TOKEN_ERROR) {
			print_token(test);
			d_array_free(&test.lexeme);
		}*/
        
	
	if(error)
		print_error(error);

	return error;
}
