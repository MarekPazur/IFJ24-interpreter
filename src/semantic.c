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
    //BT_print_tree(*program); //debug print

    /* Get global symtable from Program/Root Node */
    globalSymTable = (*program)->data.nodeData.program.globalSymTable;


    main_function_semantics(globalSymTable);
    check_error();

    /* Check functions with semantic rules */
    TNode* func = (*program)->left;

    while (func) {
        BT_print_node_type(func); // debug print

        FunctionSemantics(func);
        check_error();

        hasReturn = false; //set hasReturn to false for next function
        func = func->left;
    }

    //debug_print_keys((*program)->data.nodeData.program.globalSymTable);
}

void FunctionSemantics(TNode* func) {
    TNode* Command = func->right; // Get first command wrapper of the function

    scope_t function_scope = {.current_scope = func->data.nodeData.function.scope, .parent_scope = NULL};

    /* Once we arrive at a return in CommandSemantics we set the global variable has Return to true*/
    CommandSemantics(Command, &function_scope, func); // Pass first command with functions symtable

    /* Check return statement missing if its not void function */
    if (hasReturn == false && func->data.nodeData.function.type != VOID_TYPE) {
        error = ERR_RETURN_VALUE_EXPRESSION;
        return;
    }
}

void CommandSemantics(TNode* Command, scope_t* current_scope, TNode* func) {

    while (Command) {

        TNode* command_instance = Command->left; // Get real command from the wrapper

        if (command_instance == NULL) {
            printf(RED_BOLD("error")": Empty command in semantic analysis\n");
            error = ERR_COMPILER_INTERNAL;
            return;
        }

        BT_print_node_type(command_instance);
        scope_t *sub_scope; // sub-scope of current scope, used when WHEN,IF,ELSE,{} encountered, since they have its own local symtable

        switch (command_instance->type) {
        case WHILE:
        case IF:
        case ELSE:
        case BODY:
            //ExpressionSemantics(command_instance->left);
            sub_scope = command_instance->data.nodeData.body.current_scope;
            CommandSemantics(command_instance->right, sub_scope, func); // Recursively call so we can return to original node as soon as we explore the branch on the left side caused by a while or if
            check_error();
            break;

        case VAR_DECL:
        case CONST_DECL:
            //ExpressionSemantics(command_instance->left);
            declaration_semantics(command_instance, current_scope);
            check_error();
            break;
        
        case ASSIG:
            assig_check(command_instance);
            check_error();
            break;
        
        case FUNCTION_CALL:
            /* while this approach would probably work to some extent, we need to check other things that I believe we'll be able to check further on function call, arguments, etc.
            if(symtable_search(command.instance.function.id) == false)
                error();*/
            FunctionCallSemantics(command_instance, current_scope, NULL);
            check_error();
            break;

        case RETURN:
            if ( (command_instance->left == NULL && func->data.nodeData.function.type != VOID_TYPE) || (command_instance->left != NULL && func->data.nodeData.function.type == VOID_TYPE) ){
                error = ERR_RETURN_VALUE_EXPRESSION;
                return;
            }
            //ExpressionSemantics(command_instance->left);
            hasReturn = true;
            break;    

        default:
            break;
        }

        Command = Command->right;
    }
    /* Scope ends here  */
    /*if(!check_is_used(current_scope->current_scope)){
        error = ERR_UNUSED_VAR;
        return;
    }*/
    printf("------------\n");debug_print_keys(current_scope->current_scope); // debug print of current scopes variables stored in symtable and their properties
}

/**
* Function call semantics checks (definition, formal parameters)
* TODO CHECK NULLABLE TYPES
*/
void FunctionCallSemantics(TNode *functionCall, scope_t* current_scope, int* type_out) {
    char *function_id = functionCall->data.nodeData.identifier.identifier; // Get function ID
    TData function_data;
    TNode *formal_param = functionCall->right; // Right pointer for some unknown reason

    /* Check if function is defined */
    if (symtable_search(globalSymTable, function_id) == false) {
        error = ERR_UNDEFINED_IDENTIFIER;
        return;
    }

    /* Get functions metadata */
    if (symtable_get_data(globalSymTable, function_id, &function_data) == false) {
        error = ERR_COMPILER_INTERNAL;
        return;
    }

    /** Check function call *formal* parameter(s) (count, type) **/
    /* Check function call *formal* parameters count */
    if (function_data.function.argument_types.length != formal_param_count(formal_param)) {
        error = ERR_PARAM_TYPE_RETURN_VAL;
        return;
    }

    /* TODO Check function call *forma* parameter(s) type */
    char *real_param = function_data.function.argument_types.array; // Get function definition real parameters
    int param_position = 0;

    while (formal_param) {
        char *variable_id, formal_param_type;
        TData var_data;

        if (formal_param->type == VAR_CONST) {
            variable_id = formal_param->data.nodeData.value.identifier;

            TSymtable* local = NULL; // Local symtable where variable 'id' might be defined

            if (id_defined(current_scope, variable_id, &local) == false) {
                error = ERR_UNDEFINED_IDENTIFIER;
                break;
            }

            if (symtable_get_data(local, variable_id, &var_data) == false) {
                error = ERR_COMPILER_INTERNAL;
                break;
            }

            set_to_used(local, variable_id); // Set this variables used property to true

            formal_param_type = get_var_type(var_data.variable.type);
        } else {
            formal_param_type = get_literal_type(formal_param->type);
        }

         if (formal_param_type != real_param[param_position]) {
             error = ERR_PARAM_TYPE_RETURN_VAL;
            break; 
        }        

        ++param_position;
        formal_param = formal_param->right;
    }

    if (type_out) // If asked for type, return called functions return type
        *type_out = function_data.function.return_type;

    check_error();
}

void assig_check(TNode* command_instance){
    TData function_data;
    
    if (command_instance->left->type == FUNCTION_CALL) {
        if ( command_instance->data.nodeData.identifier.is_disposeable ) {
            /* Get functions metadata */
            if (symtable_get_data(globalSymTable, command_instance->left->data.nodeData.identifier.identifier, &function_data) == false) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }
                    
            if ( function_data.function.return_type == VOID_T ) {
                error = ERR_PARAM_TYPE_RETURN_VAL;
                return;
            }
        }
    }
}

/**
* Main function semantic checks 
*/
void main_function_semantics(TSymtable* globalSymTable) {
    /* Check existence of main function */
    if(symtable_search(globalSymTable, "main") == false) {
        error = ERR_UNDEFINED_IDENTIFIER;
        return;
    }

    TData function_data;
    /* Get main function metadata */
    if (symtable_get_data(globalSymTable, "main", &function_data) == false) {
        error = ERR_COMPILER_INTERNAL;
        return;
    }

    /* Check defined Parameters and Return value */
    if (function_data.function.argument_types.length != 0 || function_data.function.return_type != VOID_T) {
        error = ERR_PARAM_TYPE_RETURN_VAL;
        return;
    }
}

/**
* Declaration: var/const 'id' = semantic checks
*/
void declaration_semantics(TNode* declaration, scope_t* current_scope) {
    char *variable_id = declaration->data.nodeData.identifier.identifier; // Get LHS var id
    TData var_data;
    int datatype = 0;

    if (symtable_get_data(current_scope->current_scope, variable_id, &var_data) == false) { // Get LHS var 'id' metadata from current symtable
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    
    if (declaration->left->type == FUNCTION_CALL) { // var/const 'id' (:type) = function(param_list);
        FunctionCallSemantics(declaration->left, current_scope, &datatype);

    check_error();

    /* Variable type resolution */
    if (var_data.variable.type == UNKNOWN_T) { // Type was unknown (var/const without specified type)
        var_data.variable.type = datatype;

        if (symtable_insert(current_scope->current_scope, variable_id, var_data) == false) {
            error = ERR_COMPILER_INTERNAL;
            return;
        }
    } 

    if ((int) var_data.variable.type != datatype) {
        error = ERR_PARAM_TYPE_RETURN_VAL;
        return;
    }
    } else
        expression_semantics(declaration->left, current_scope, &datatype);
        
    printf("%d", datatype);

    check_error();

}

/**
* Expression semantic checks
*/
void expression_semantics(TNode *expression, scope_t* scope, int* type_out) {
    if (expression == NULL)
        return;

    /* POST-ORDER-TRAVERSAL */
    expression_semantics(expression->left, scope, type_out);
    check_error();
    expression_semantics(expression->right, scope, type_out);
    check_error();

    BT_print_node_type(expression);

    switch(expression->type) {
        /* LITERALS */
        case INT: // I32 LITERAL
            *type_out = INTEGER_T;
            break;
        case FL:  // F64 LITERAL
            *type_out = FLOAT_T;
            break;
        case U8:  // INVALID LITERAL TYPES
        case STR:
            error = ERR_TYPE_COMPATABILITY;
            break;

        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
            break;

        case OP_DIV:
            break;

        case OP_EQ:
            break;

        case OP_NEQ:
        case OP_GT:
        case OP_LS:
        case OP_GTE:
        case OP_LSE:
            break;    

        default:
            break;        
    }

}

/* Helper functions */

/* Semantic analysis function for Syntax analysis */
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

/** 
* Returns count of formal parameters in function call
* fun(a,b,c) ---> 3 
*/
int formal_param_count(TNode *formal_param) {
    int param_count = 0;

    while(formal_param) {
        ++param_count;
        formal_param = formal_param->right;
    }

    return param_count;
}

/**
* Checks if var/const is defined in given scope or its inherited (parent) scope(s)
* \param out_sym reference to scope the variable is in if it exists
* returns true or false depending on its definition
*/
bool id_defined(struct TScope* scope, char* identifier, TSymtable** out_sym) {
    while(scope) {
         if (symtable_search(scope->current_scope, identifier)) {
            *out_sym = scope->current_scope;
            return true;
         }

         scope = scope->parent_scope;
    }

    *out_sym = NULL;

    return false;
}/**
 * @brief This function initializes the parser
 *
 * @param token, will be used to store the current token
 */

/**
* Returns variables data type in char format 'i','u','f' 
*/
char get_var_type(Type type) {
    char formal_type = 0;

    switch (type) {
        case INTEGER_T:
            formal_type = 'i';
            break;
        case FLOAT_T:
            formal_type = 'f';
            break;
        case U8_SLICE_T:
            formal_type = 'u';
            break;

        default:
            formal_type = 0;
            break; 
    } 

    return formal_type;
}

/**
* Returns literal data type in char format 'i','u','f','s'
*/
char get_literal_type(int type) {
    char formal_type = 0;

    switch (type) {
        case INT:
            formal_type = 'i';
            break;
        case FL:
            formal_type = 'f';
            break;
        case U8:
            formal_type = 'u';
            break;
        case STR:
            formal_type = 's';
            break;

        default:
            formal_type = 0;
            break; 
    } 

    return formal_type;
}

/**
* Sets given variable property is_used to true
*/
void set_to_used(TSymtable* symtable, char* identifier) {
    TData data;

    if (symtable_get_data(symtable, identifier, &data) == false) {
        error = ERR_COMPILER_INTERNAL;
        return;
    }

    data.variable.is_used = true;

    if (symtable_insert(symtable, identifier, data) == false) {
        error = ERR_COMPILER_INTERNAL;
        return;
    }
}