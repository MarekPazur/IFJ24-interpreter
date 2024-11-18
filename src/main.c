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
#include "binary_tree.h"

int main(void) {
	//TODO
	//lexical analyser
	//syntax analyser - rekurzivni sestup?
	//symtable - vyvazeny binarni strom?
	//semantic analyser
	//code generator

	/* Init. of scanner struct */
	init_scanner();

	if(error) {
		print_error(error);
	}

	/* Testing phase, will be rewritten */
	//token_t test;
	//init_parser(test);
      
	TBinaryTree *AST = BT_init();
	BT_insert_root(AST, RETURN);

	TNode** tree = &(AST->root);

	printf("%d",(*tree)->type);



	BT_free_nodes(AST);

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
