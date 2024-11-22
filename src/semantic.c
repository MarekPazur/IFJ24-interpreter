/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xpazurm00, Marek Pazúr
 * 
 * @file semantic.c
 */

#include <stdbool.h>
#include <stdio.h>

#include "semantic.h"
#include "compiler_error.h"
#include "binary_tree.h"
#include "symtable.h"

/**
* Functions for Semantic Analysis on assembled Abstract Syntax Tree
*/

void semantic_analysis(TBinaryTree* AST) {
	if (AST->root == NULL) {
		printf(RED_BOLD("error")": AST is NULL\n");
		error = ERR_COMPILER_INTERNAL;
	}

	/* Program/Root Node (Starting point of the program) */
	TNode** program = &(AST->root);

	/* Lze jde zkontrolovat, jestli globalni symtable obsahuje main */

	BT_print_node_type(*program);
	debug_print_keys((*program)->data.nodeData.program.globalSymTable);
}
