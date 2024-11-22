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
#include "syna.h"

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

TSymtable* declaration_var_check(struct TScope cur_scope, char* identifier){
    while(cur_scope.current_scope != NULL){
        if(symtable_search(cur_scope.current_scope, identifier)){
            return cur_scope.current_scope;
        }
        if(cur_scope.parent_scope != NULL){
            cur_scope = *cur_scope.parent_scope;
        }else{
            cur_scope.current_scope = NULL;
        }
    }
    return NULL;
}
