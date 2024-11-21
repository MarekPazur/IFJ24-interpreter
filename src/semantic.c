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

/**
* Functions for Semantic Analysis on assembled Abstract Syntax Tree
*/

void semantic_analysis(TBinaryTree* AST) {
	if (AST->root == NULL) {
		printf(RED_BOLD("error")": AST is NULL\n");
		error = ERR_COMPILER_INTERNAL;
	}

	printf("Hello semantic world!\n");

	AST = AST;
}
