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

//Global variable 
bool hasReturn = false;

//main semantic analysis function
void semantic_analysis(TBinaryTree* AST) {
	if (AST->root == NULL) {
		printf(RED_BOLD("error")": AST is NULL\n");
		error = ERR_COMPILER_INTERNAL;
	}

	/* Program/Root Node (Starting point of the program) */
	TNode** program = &(AST->root);

	//MainFunctionCheck();
	TNode* func = (*program)->left;

    	while(func) {
        	FunctionSemantics(func);
        	hasReturn = false; //set hasReturn to false for next function
        	func = func->left;
    	}
    	
	//no other controls here most likely as all can be done before
	
	//BT_print_node_type(*program);
	//debug_print_keys((*program)->data.nodeData.program.globalSymTable);
}

void FunctionSemantics(TNode* func) {
    TNode* Command = func->right;
    
    /* Once we arrive at a return in CommandSemantics we set the global variable hasReturn to true*/
    CommandSemantics(Command);
}

void CommandSemantics(TNode* Command) {
        while(Command) {
        BT_print_tree(Command);

        TNode* command_instance = Command->left;
        switch(command_instance->type) {
            case WHILE:
            case IF:
            case ELSE:
                //ExpressionSemantics(command_instance->left);
                CommandSemantics(command_instance->right); // Recursively call so we can return to original node as soon as we explore the branch on the left side caused by a while or if
                
                break;

            case VAR_DECL:
            case CONST_DECL:
                //ExpressionSemantics(command_instance->left);
                break;

            case FUNCTION_CALL:
                /* while this approach would probably work to some extent, we need to check other things that I believe we'll be able to check further on function call, arguments, etc.
                if(symtable_search(command.instance.function.id) == false)
                    error();*/
                break;
	    default:
	    	break;
            /*case ...:
                break;*/
        }


        Command = Command->right;
    }
}
/*
function to check declaration of variables, has to be done during syntax analysis so completely seperate from the rest of this script
*/
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
