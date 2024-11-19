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
	/*
		TODO
		lexical analyser
		syntax analyser - rekurzivni sestup?
		symtable - vyvazeny binarni strom?
		semantic analyser
		code generator
	*/

	/* Init. of scanner struct */
	init_scanner();

	if (error) {
		print_error(error);
	}

	/* Testing phase, will be rewritten */
	//token_t test;
	//init_parser(test);

/*	TBinaryTree *AST = BT_init();
	TNode** tree = &(AST->root);

	BT_insert_root(AST, PROGRAM);

	BT_set_root(AST);
	BT_insert_right(AST, PROLOGUE);
	BT_insert_left(AST, FN);

	BT_go_left(AST);
	BT_insert_left(AST, FN);
	BT_go_left(AST);
	BT_insert_left(AST, FN);

	BT_set_root(AST);
	BT_go_right(AST);
	BT_insert_right(AST, PROLOGUE);

	BT_set_root(AST);
	BT_go_left(AST);
	BT_insert_right(AST, COMMAND);
	BT_go_right(AST);
	BT_insert_left(AST, WHILE);
	BT_insert_right(AST, COMMAND);
	BT_go_right(AST);
	BT_insert_right(AST, COMMAND);
	BT_go_right(AST);
	BT_insert_right(AST, COMMAND);
	BT_go_right(AST);
	BT_insert_left(AST, BODY);
	BT_insert_right(AST, COMMAND);

	BT_set_root(AST);
	BT_go_left(AST);
	BT_go_left(AST);
	BT_insert_right(AST, COMMAND);

	// Print the tree
	BT_print_tree(*tree);
	BT_free_nodes(AST);*/
	
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

	if (error) {
		print_error(error);
	}

	return error;
}
