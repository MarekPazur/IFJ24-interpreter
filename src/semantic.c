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

/* Utility macros */
#define check_error() do { if (error) return; } while (0)

/* Global variables */
bool hasReturn = false;
TSymtable *globalSymTable;

/* Functions for Semantic Analysis on assembled Abstract Syntax Tree  */

/* Main semantic analysis function */
void semantic_analysis(TBinaryTree* AST) {
    check_error();

    if (AST->root == NULL) {
        printf(RED_BOLD("error")": AST is NULL\n");
        error = ERR_COMPILER_INTERNAL;
        return;
    }

    /* Program/Root Node (Starting point of the program) */
    TNode** program = &(AST->root);
    BT_print_tree(*program); //debug print

    /* Get global symtable from Program/Root Node */
    globalSymTable = (*program)->data.nodeData.program.globalSymTable;

    /* Check functions with semantic rules */
    TNode* func = (*program)->left;

    while (func) {
        BT_print_node_type(func); // debug print

        FunctionSemantics(func);
        check_error();

        hasReturn = false; //set hasReturn to false for next function
        func = func->left;
    }

    //no other controls here most likely as all can be done before
    //debug_print_keys((*program)->data.nodeData.program.globalSymTable);
}

void FunctionSemantics(TNode* func) {
    TNode* Command = func->right; // Get first command wrapper of the function

    /* Once we arrive at a return in CommandSemantics we set the global variable has Return to true*/
    CommandSemantics(Command);
}

void CommandSemantics(TNode* Command) {
    while (Command) {

        TNode* command_instance = Command->left; // Get real command from the wrapper

        if (command_instance == NULL) {
            printf(RED_BOLD("error")": Empty command in semantic analysis\n");
            error = ERR_COMPILER_INTERNAL;
            return;
        }

        BT_print_node_type(command_instance);

        switch (command_instance->type) {
        case WHILE:
        case IF:
        case ELSE:
            //ExpressionSemantics(command_instance->left);
            CommandSemantics(command_instance->right); // Recursively call so we can return to original node as soon as we explore the branch on the left side caused by a while or if
            check_error();
            break;

        case VAR_DECL:
        case CONST_DECL:
            //ExpressionSemantics(command_instance->left);

            if (command_instance->left->type == FUNCTION_CALL)
                FunctionCallSemantics(command_instance->left);

            check_error();
            break;

        case FUNCTION_CALL:
            /* while this approach would probably work to some extent, we need to check other things that I believe we'll be able to check further on function call, arguments, etc.
            if(symtable_search(command.instance.function.id) == false)
                error();*/
            FunctionCallSemantics(command_instance);
            check_error();
            break;


        default:
            break;
            /*case ...:
                break;*/
        }


        Command = Command->right;
    }
}

/**
* Function call semantics
*/
void FunctionCallSemantics(TNode *functionCall) {
    char *function_id = functionCall->data.nodeData.identifier.identifier;

    TNode *formal_param = functionCall->right; //right pointer for some unknown reason

    printf("[%s]", function_id);

    int i = 1;
    while (formal_param) {
        char *content;

        if (formal_param->type == INT || formal_param->type == FL || formal_param->type == STR) {
            content = formal_param->data.nodeData.value.literal;
        }

        if (formal_param->type == VAR_CONST) {
            content = formal_param->data.nodeData.value.identifier;
        }


        printf("param %d: %s", i++, content);

        formal_param = formal_param->right;
    }

    check_error();
}

/**
* Function to check declaration of variables, has to be done during syntax analysis so completely seperate from the rest of this script
*/
TSymtable* declaration_var_check(struct TScope cur_scope, char* identifier) {
    while (cur_scope.current_scope != NULL) {
        if (symtable_search(cur_scope.current_scope, identifier)) {
            return cur_scope.current_scope;
        }
        if (cur_scope.parent_scope != NULL) {
            cur_scope = *cur_scope.parent_scope;
        } else {
            cur_scope.current_scope = NULL;
        }
    }
    return NULL;
}
