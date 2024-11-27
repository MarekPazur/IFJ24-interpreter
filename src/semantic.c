/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xpazurm00, Marek Pazúr
 *
 * @file semantic.c
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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
        //BT_print_node_type(func); // debug print

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
    check_error();

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
        
            sub_scope = command_instance->data.nodeData.body.current_scope;

            check_head_type(command_instance, sub_scope);
            check_error();
            
            CommandSemantics(command_instance->right, sub_scope, func); // Recursively call so we can return to original node as soon as we explore the branch on the left side caused by a while or if
            check_error();
            break;
            
        case ELSE:
        case BODY:
            sub_scope = command_instance->data.nodeData.body.current_scope;
            CommandSemantics(command_instance->right, sub_scope, func); // Recursively call so we can return to original node as soon as we explore the branch on the left side caused by a while or if
            check_error();
            break;

        case VAR_DECL:
        case CONST_DECL:
            declaration_semantics(command_instance, current_scope);
            check_error();
            break;

        case ASSIG:
            assig_check(command_instance);
            check_error();
            break;

        case FUNCTION_CALL: {
            fun_info info = {.type = UNKNOWN_T, .is_optional_null = false};
            FunctionCallSemantics(command_instance, current_scope, &info);
            check_error();

            if (info.type != VOID_T) {
                printf("error: function return value ignored, all non-void values must be used\n");
                error = ERR_PARAM_TYPE_RETURN_VAL;
                return;
            }
            break;
        }

        case RETURN:
            if ( (command_instance->left == NULL && func->data.nodeData.function.type != VOID_TYPE) || (command_instance->left != NULL && func->data.nodeData.function.type == VOID_TYPE) ) {
                error = ERR_RETURN_VALUE_EXPRESSION;
                return;
            }

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
    printf("------------\n"); debug_print_keys(current_scope->current_scope); // debug print of current scopes variables stored in symtable and their properties
}

/**
* @brief this function checks that there is a bool expression inside the if and while header, also checks that if the header has |this| it is nullable
* @param the node of the if or while
*/
void check_head_type(TNode* body, scope_t *scope){

    expr_info expr_data = {.type = UNKNOWN_T, .is_constant_exp = false, .is_optional_null = false, .optional_null_id = NULL};
    
    expression_semantics(body->left, body->data.nodeData.body.current_scope->parent_scope, &expr_data);
    
    //printf("[condition %s]\n",body->data.nodeData.body.is_nullable ? "optional-null" : "not null");

    if ( body->data.nodeData.body.is_nullable ) {
        if ( !expr_data.is_optional_null ) { // variable is not correct type
            error = ERR_SEMANTIC_OTHER;
            return;
        } else { // if (id) |ID| --> update ID datatype with non-null ids datatype
            //printf("%s",expr_data.optional_null_id);
            TSymtable* local;
            TData null_var_data, not_null_inheritor_data;
            char *variable_id = expr_data.optional_null_id, *not_null_id = body->data.nodeData.body.null_replacement;

            /* get variable data in condition */
            if (id_defined(scope, variable_id, &local) == false) {
                printf("error: var/const %s in condition undefined\n", variable_id);
                error = ERR_UNDEFINED_IDENTIFIER;
                return;
            }
            if (symtable_get_data(local, variable_id, &null_var_data) == false) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }/* |not_null_variable| data*/
            if (id_defined(scope, not_null_id, &local) == false) {
                printf("error: not_null variable undefined\n");
                error = ERR_UNDEFINED_IDENTIFIER;
                return;
            }
            if (symtable_get_data(local, not_null_id, &not_null_inheritor_data) == false) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            /* Assign inherited datatype without null */
            not_null_inheritor_data.variable.type = null_var_data.variable.type;
            /* Update data about not_null constant */
            if (symtable_insert(local, not_null_id, not_null_inheritor_data) == false) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        }
    } else {
        if( expr_data.type != BOOL_T ){
            error = ERR_TYPE_COMPATABILITY;
            printf("error: trying to put non-truth expression in condition statement\n");
            return;
        }
    }
}

/**
* Function call semantics checks (definition, formal parameters)
* TODO CHECK NULLABLE TYPES
*/
void FunctionCallSemantics(TNode *functionCall, scope_t* current_scope, fun_info* info) {
    char *function_id = functionCall->data.nodeData.identifier.identifier; // Get function ID
    TData function_data;
    TNode *formal_param = functionCall->right; // Right pointer for some unknown reason

    /* Check if function is defined */
    if (symtable_search(globalSymTable, function_id) == false) {
        printf("error: function %s undefined\n",function_id);
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
        printf("error: invalid function call parameter count\n");
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
                printf("error: formal parameter %s undefined\n", variable_id);
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
            /* Formal and real params may differentiate, however if its built-in function, it may accept all parameter types - 'a' (ifj.write())
            or it may accept string or []u8 slice - 'n' (ifj.string()) where its neccesary to check if the formal params are of type string or []u8 slice */
            if (real_param[param_position] != 'a' && (real_param[param_position] != 'n' || (formal_param_type != 'u' && formal_param_type != 's'))){
                printf("%s - ",function_id);
                printf("[%c - %c]",formal_param_type, real_param[param_position]);
                printf("error: formal parameter type doesn't match function definition parameter type\n");
                error = ERR_PARAM_TYPE_RETURN_VAL;
                break;
            }
        }

        ++param_position;
        formal_param = formal_param->right;
    }

    if (info) { // If asked for type, return called functions return type
        info->type = function_data.function.return_type;
        info->is_optional_null = function_data.function.is_null_type;
    }

    check_error();
}

void assig_check(TNode* command_instance) {
    TData function_data;

    if (command_instance->left->type == FUNCTION_CALL) {
        if ( command_instance->data.nodeData.identifier.is_disposeable ) {
            /* Get functions metadata */
            if (symtable_get_data(globalSymTable, command_instance->left->data.nodeData.identifier.identifier, &function_data) == false) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }

            if ( function_data.function.return_type == VOID_T ) {
                printf("error: trying to assign void to variable\n");
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
    if (symtable_search(globalSymTable, "main") == false) {
        printf("error: main function undeclared\n");
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
        printf("error: main function cant have parameters and must return void\n");
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
    int datatype = 0, is_optional_null = 0;

    if (symtable_get_data(current_scope->current_scope, variable_id, &var_data) == false) { // Get LHS var 'id' metadata from current symtable
        error = ERR_COMPILER_INTERNAL;
        return;
    }

    if(var_data.variable.is_constant && var_data.variable.value_pointer == NULL) { // const must be either literal or have value of another
        //TODO FIX!!!!!!!!!!!!!!!!!!!!!!!//error = ERR_SYNTAX;
        //return;
    }

    if (declaration->left->type == FUNCTION_CALL) { // var/const 'id' (:type) = function(param_list);
        fun_info info = {.type = UNKNOWN_T, .is_optional_null = false};
        FunctionCallSemantics(declaration->left, current_scope, &info);
        check_error(); 
        datatype = info.type;
        is_optional_null = info.is_optional_null;
    } else {
        expr_info exp_res = {.type = UNKNOWN_T, .is_constant_exp = true, .is_optional_null = false};
        expression_semantics(declaration->left, current_scope, &exp_res);
        check_error();
        datatype = exp_res.type;
        is_optional_null = exp_res.is_optional_null;
    }

    /* Variable type resolution */
    if (var_data.variable.type == UNKNOWN_T) { // Type was unknown (var/const without specified type)
        var_data.variable.type = datatype; // assign result data type from exp/fun call
        var_data.variable.is_null_type = is_optional_null; // is ?type

        if (symtable_insert(current_scope->current_scope, variable_id, var_data) == false) {
            error = ERR_COMPILER_INTERNAL;
            return;
        }
    }

    //TODO rozlisit jestli je to z funkce nebo vyrazu
    if ((int) var_data.variable.type != datatype) {
        printf("error: var/const datatype doesn't match expression/function return type\n");
        error = ERR_PARAM_TYPE_RETURN_VAL;
        return;
    } else {
        if (var_data.variable.is_null_type != is_optional_null) {
            printf("error: trying to assign optional-null type into non-null type\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }
    }

    check_error();
}

/**
* Expression semantic checks
*/
void expression_semantics(TNode *expression, scope_t* scope, expr_info* info) {
    if (expression == NULL)
        return;

    /* POST-ORDER-TRAVERSAL */
    expr_info left = {.type = UNKNOWN_T, .is_constant_exp = true, .is_optional_null = false};
    expr_info right = {.type = UNKNOWN_T, .is_constant_exp = true, .is_optional_null = false};

    expression_semantics(expression->left, scope, &left);
    check_error();

    expression_semantics(expression->right, scope, &right);
    check_error();

    //BT_print_tree(expression);//BT_print_node_type(expression);

    switch (expression->type) {
        /* LITERALS */
        case INT: // I32 LITERAL
            info->type = INTEGER_T;
            break;
        case FL:  // F64 LITERAL
            info->type = FLOAT_T;
            break;
        case NULL_LITERAL:
            info->type = NIL_T;
            break;
        case U8:  // INVALID LITERAL TYPES
        case STR:
            error = ERR_TYPE_COMPATABILITY;
            break;
        /* VAR or CONST */
        case VAR_CONST:
        {
            TSymtable* local;

            char *variable_id = expression->data.nodeData.value.identifier;

            if (id_defined(scope, variable_id, &local) == false) {
                printf("error: var/const %s undefined in this expression\n", variable_id);
                error = ERR_UNDEFINED_IDENTIFIER;
                break;
            }

            if (symtable_get_data(local, variable_id, &info->data) == false) {
                error = ERR_COMPILER_INTERNAL;
                break;
            }

            set_to_used(local, variable_id);

            info->type = info->data.variable.type;

            if (info->data.variable.is_null_type) {
                info->is_optional_null = true;
                info->optional_null_id = variable_id;
            }

            if (!info->data.variable.is_constant) // var encountered
                info->is_constant_exp = false;
        }
        break;

        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
            if (left.is_optional_null || right.is_optional_null) {
                printf(RED_BOLD("error")": type mismatch, trying to use null type in arithmethics\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }

            if (left.type == right.type) { // i32 = i32, f64 = f64
                info->type = left.type;
            } else if ((left.type == INTEGER_T && right.type == FLOAT_T) || (left.type == FLOAT_T && right.type == INTEGER_T)) {
                /* type conversion here */
                if (left.is_constant_exp && right.is_constant_exp) { // made out of literals and constants
                    /* left op literal */
                    if (expression->left->type == INT) {
                        char **literal = &expression->left->data.nodeData.value.literal;

                        *literal = literal_convert_i32_to_f64(*literal);
                        expression->left->type = FL;
                    }
                    /* right op literal */
                    if(expression->right->type == INT) {
                        char **literal = &expression->right->data.nodeData.value.literal;

                        *literal = literal_convert_i32_to_f64(*literal);
                        expression->right->type = FL;
                    }
                    /* left op const */
                    if(expression->left->type == VAR_CONST) {
                        TSymtable* local;
                        TData var_data;
                        char *variable_id = expression->left->data.nodeData.value.identifier;

                        if (id_defined(scope, variable_id, &local) == false) {
                            printf("error: var/const %s undefined in arithmetic expression\n", variable_id);
                            error = ERR_UNDEFINED_IDENTIFIER;
                            break;
                        }

                        if (symtable_get_data(local, variable_id, &var_data) == false) {
                            error = ERR_COMPILER_INTERNAL;
                            break;
                        }

                        if (var_data.variable.type == INTEGER_T) {
                            char *const_value = var_data.variable.value_pointer->data.nodeData.value.literal;

                            const_value = literal_convert_i32_to_f64(const_value);
                            expression->left->type = FL;
                            expression->left->data.nodeData.value.literal = const_value;
                        }
                    }
                    /* right op const */
                    if(expression->right->type == VAR_CONST) {
                        TSymtable* local;
                        TData var_data;
                        char *variable_id = expression->right->data.nodeData.value.identifier;

                        if (id_defined(scope, variable_id, &local) == false) {
                            printf("error: var/const %s undefined in arithmetic expression\n", variable_id);
                            error = ERR_UNDEFINED_IDENTIFIER;
                            break;
                        }

                        if (symtable_get_data(local, variable_id, &var_data) == false) {
                            error = ERR_COMPILER_INTERNAL;
                            break;
                        }

                        if (var_data.variable.type == INTEGER_T) {
                            char *const_value = var_data.variable.value_pointer->data.nodeData.value.literal;

                            const_value = literal_convert_i32_to_f64(const_value);
                            expression->right->type = FL;
                            expression->right->data.nodeData.value.literal = const_value;
                        }
                    }                    

                    info->type = FLOAT_T;
                } else { // var must be the same type
                    printf(RED_BOLD("error")": var type mismatch\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }
            } else { // UNKNOWN_T, NULL_T 
                printf(RED_BOLD("error")": expression type mismatch\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }
            break;

        case OP_DIV:
            if (left.is_optional_null || right.is_optional_null) {
                printf(RED_BOLD("error")": type mismatch, trying to use null type in arithmethics\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }
            // Same type
            if (left.type == right.type) {
                    info->type = left.type;
            } // type mismatch
            else if ((left.type == INTEGER_T && right.type == FLOAT_T) || (left.type == FLOAT_T && right.type == INTEGER_T)) {
                // Cant be var
                if (left.is_constant_exp && right.is_constant_exp) {
                    if (expression->left->type == FL) {
                        char **literal = &expression->left->data.nodeData.value.literal;

                        if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) { 
                            error = ERR_TYPE_COMPATABILITY;
                            return;
                        }

                        expression->left->type = INT;
                    }
                    if (expression->right->type == FL) {
                        char **literal = &expression->right->data.nodeData.value.literal;

                        if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) { 
                            error = ERR_TYPE_COMPATABILITY;
                            return;
                        }

                        expression->right->type = INT;
                    }
                    if (expression->left->type == VAR_CONST) {
                        TSymtable* local;
                        TData var_data;
                        char *variable_id = expression->left->data.nodeData.value.identifier;

                        if (id_defined(scope, variable_id, &local) == false) {
                            printf("error: var/const %s undefined in arithmetic expression\n", variable_id);
                            error = ERR_UNDEFINED_IDENTIFIER;
                            break;
                        }

                        if (symtable_get_data(local, variable_id, &var_data) == false) {
                            error = ERR_COMPILER_INTERNAL;
                            break;
                        }

                        if (var_data.variable.type == FLOAT_T) {
                            char *const_value = var_data.variable.value_pointer->data.nodeData.value.literal;

                            if((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                                error = ERR_TYPE_COMPATABILITY;
                                return;
                            }

                            expression->left->type = INT;
                            expression->left->data.nodeData.value.literal = const_value;
                        }
                    }
                    if (expression->right->type == VAR_CONST) {
                        TSymtable* local;
                        TData var_data;
                        char *variable_id = expression->right->data.nodeData.value.identifier;

                        if (id_defined(scope, variable_id, &local) == false) {
                            printf("error: var/const %s undefined in arithmetic expression\n", variable_id);
                            error = ERR_UNDEFINED_IDENTIFIER;
                            break;
                        }

                        if (symtable_get_data(local, variable_id, &var_data) == false) {
                            error = ERR_COMPILER_INTERNAL;
                            break;
                        }

                        if (var_data.variable.type == FLOAT_T) {
                            char *const_value = var_data.variable.value_pointer->data.nodeData.value.literal;

                            if((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                                error = ERR_TYPE_COMPATABILITY;
                                return;
                            }

                            expression->right->type = INT;
                            expression->right->data.nodeData.value.literal = const_value;
                        }
                    }

                    info->type = INTEGER_T;
                } else { // var must be the same type
                    printf(RED_BOLD("error")": var type mismatch\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

            } else { // UNKNOWN_T, NULL_T 
                printf(RED_BOLD("error")": expression type mismatch\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }
            break;

        case OP_EQ:
            info->type = BOOL_T;
            break;

        case OP_NEQ:
        case OP_GT:
        case OP_LS:
        case OP_GTE:
        case OP_LSE:
            info->type = BOOL_T;
            break;

        default:
            break;
    }
    //BT_print_tree(expression);
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

    while (formal_param) {
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
    while (scope) {
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

/**
* Converts I32 literal/const to F64
*/
char *literal_convert_i32_to_f64(char *literal) {
    size_t new_size = strlen(literal) + 3; // '.''0''\0'

    char *converted = (char*) malloc(sizeof(char)*new_size);
    memset(converted, 0, new_size);

    strcpy(converted, literal);
    strcat(converted, ".0");

    return converted;
}

/**
* Converts F64 literal/const to I32
*/
char *literal_convert_f64_to_i32(char *literal) {
    int i = 0;
    bool zero_decimal = true;

    while(literal[i] != '.') {
        ++i;
    }

    int j = i + 1;

    while(literal[j]) {
        if (literal[j] != '0') {
            zero_decimal = false;
            break;
        }
        ++j;
    }

    if(zero_decimal == false) {
        return NULL;
    }

    memset(literal+i, 0, strlen(literal));

    return literal;
}
