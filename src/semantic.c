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
#include "codegen.h"

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
    //BT_print_tree(*program); // debug print of the AST before semantic checks

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
    //BT_print_tree(*program); // Final tree passed to code generator
    //debug_print_keys((*program)->data.nodeData.program.globalSymTable);
    codegen(AST);
    check_error();
}

/**
* Iterates through every function in AST
*/
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

/**
* Iterates and uses recursion to do semantic checks on every command/statement used in given function, also takes in mind scopes of variables/constants
*/
void CommandSemantics(TNode* Command, scope_t* current_scope, TNode* func) {

    while (Command) {

        TNode* command_instance = Command->left; // Get real command from the wrapper

        if (command_instance == NULL) {
            printf(RED_BOLD("error")": Empty command in semantic analysis\n");
            error = ERR_COMPILER_INTERNAL;
            return;
        }

        //BT_print_node_type(command_instance);
        scope_t *sub_scope; // sub-scope of current scope, used when WHEN,IF,ELSE,{} encountered, since they have its own local symtable

        switch (command_instance->type) {
        case WHILE:
        case IF:

            sub_scope = command_instance->data.nodeData.body.current_scope;

            check_head_type(command_instance, sub_scope); // Checks expression and its type inside condition '(expr)'
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
            assig_check(command_instance, current_scope);
            check_error();
            break;

        case FUNCTION_CALL: {
            fun_info info = {.type = UNKNOWN_T, .is_optional_null = false};
            FunctionCallSemantics(command_instance, current_scope, &info);
            check_error();

            if (info.type != VOID_T) { // If the functions return type is NOT void, the return value cannot be ignored.
                printf("error: function return value ignored, all non-void values must be used\n");
                error = ERR_PARAM_TYPE_RETURN_VAL;
                return;
            }
            break;
        }

        case RETURN: {
            TNode *expression = command_instance->left; // Get expression from left child-node of the command
            /* Checks if the expression in a return statement is missing or is there an extra expression when the function has void return type */
            if ( (expression == NULL && func->data.nodeData.function.type != VOID_TYPE) || (expression != NULL && func->data.nodeData.function.type == VOID_TYPE) ) {
                error = ERR_RETURN_VALUE_EXPRESSION;
                return;
            }

            if (expression) { // Function is not void, check type of return expression compared to functions return type
                expr_info expr_data = {.type = UNKNOWN_T, .is_constant_exp = false, .is_optional_null = false, .optional_null_id = NULL};
                expression_semantics(expression, current_scope, &expr_data);
                check_error();

                if (expression) {
                    int function_return_type = get_func_type(globalSymTable, func->data.nodeData.function.identifier);
                    check_error();

                    if (expr_data.type != function_return_type) {
                        error = ERR_PARAM_TYPE_RETURN_VAL;
                        return;
                    }
                }
            }

            hasReturn = true; // Set the flag to true, since the function has atleast one return statement/command
            break;
        }

        default:
            break;
        }

        Command = Command->right; // Move to the next command wrapper
    }
    /* Scope ends here  */
    if (!check_is_used(current_scope->current_scope)) { // Check if every const is used, var used and mutated
        error = ERR_UNUSED_VAR;
        return;
    }
    //printf("------------\n"); debug_print_keys(current_scope->current_scope); // debug print of current scopes variables stored in symtable and their properties
}

/**
* @brief this function checks that there is a bool expression inside the if and while header, also checks that if the header has |this| it is nullable
* @param the node of the if or while
*/
void check_head_type(TNode* body, scope_t *scope) {

    expr_info expr_data = {.type = UNKNOWN_T, .is_constant_exp = false, .is_optional_null = false, .optional_null_id = NULL};

    expression_semantics(body->left, body->data.nodeData.body.current_scope->parent_scope, &expr_data); // Evaluate the expression in the condition first
    check_error();

    //printf("[condition %s]\n",body->data.nodeData.body.is_nullable ? "optional-null" : "not null");

    if ( body->data.nodeData.body.is_nullable ) {
        if ( !expr_data.is_optional_null ) { // variable is not correct type
            error = ERR_TYPE_COMPATABILITY;
            return;
        } else { // if (id) |ID| --> update ID datatype with non-null ids datatype
            //printf("%s",expr_data.optional_null_id);
            TSymtable* local;
            TData null_var_data, not_null_inheritor_data;
            char *variable_id = expr_data.optional_null_id, *not_null_id = body->data.nodeData.body.null_replacement; // ID of the variable in condition brackets and ID of the optional replacement const in vertical bars

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
    } else { /* Condition is not optional null AND expression must be of truth expression type else its error */
        if ( expr_data.type != BOOL_T ) {
            error = ERR_TYPE_COMPATABILITY;
            printf("error: trying to put non-truth expression in condition statement\n");
            return;
        }
    }
}

/**
* Function call semantics checks (definition, formal parameters)
*/
void FunctionCallSemantics(TNode *functionCall, scope_t* current_scope, fun_info* info) {
    char *function_id = functionCall->data.nodeData.identifier.identifier; // Get function ID
    TData function_data;
    TNode *formal_param = functionCall->right; // Right pointer for some unknown reason

    /* Check if function is defined */
    if (symtable_search(globalSymTable, function_id) == false) {
        printf("error: function %s undefined\n", function_id);
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
            if (real_param[param_position] != 'a' && (real_param[param_position] != 'n' || (formal_param_type != 'u' && formal_param_type != 's'))) {
                printf("%s - ", function_id);
                printf("[%c - %c]", formal_param_type, real_param[param_position]);
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

/**
* Assigment semantic checks: throw away, assigment type match
*/
void assig_check(TNode* command_instance, scope_t *scope) {
    TData function_data, var_data;
    char *variable_id = command_instance->data.nodeData.identifier.identifier;
    bool throw_away = command_instance->data.nodeData.identifier.is_disposeable;

    /* Fetches data about variable, checks its existence, if its not '_' */
    if (throw_away == false) // Must check this, else it would throw error, since '_' is not registered in any symtable
        var_data =  get_const_var_data(scope, variable_id);

    check_error();

    if (command_instance->left->type == FUNCTION_CALL) { // Variable is being assigned function call return value
        TNode* function = command_instance->left;
        char* function_id = function->data.nodeData.identifier.identifier;

        /* Get functions metadata */
        if (symtable_get_data(globalSymTable, function_id, &function_data) == false) {
            error = ERR_COMPILER_INTERNAL;
            return;
        }

        fun_info info = {.type = UNKNOWN_T, .is_optional_null = false};
        FunctionCallSemantics(function, scope, &info);
        check_error();

        if (throw_away) { // function call is assigned to "_" special variable, so the type check can be skipped
            if ( function_data.function.return_type == VOID_T ) {
                printf("error: trying to assign void type\n");
                error = ERR_PARAM_TYPE_RETURN_VAL;
                return;
            }
        } else {
            if ((int) var_data.variable.type != info.type) { // types dont match
                error = ERR_TYPE_COMPATABILITY;
                printf("error: assignment type mismatch caused by function %s\n", function_id);
                return;
            } else {
                if ((var_data.variable.is_null_type == false) && info.is_optional_null) { // types can be the same but differ in null includement
                    error = ERR_TYPE_COMPATABILITY;
                    printf("error: trying to assign optional-null type into non-null variable, type mismatch caused by function %s\n", function_id);
                    return;
                }
            }
        }
    } else {
        TNode* expression = command_instance->left;
        expr_info expr_data = {.type = UNKNOWN_T, .is_constant_exp = false, .is_optional_null = false, .optional_null_id = NULL};

        expression_semantics(expression, scope, &expr_data);
        check_error();

        if (throw_away == false) {
            if ((int) var_data.variable.type != expr_data.type) { // type of a != b
                if ((var_data.variable.is_null_type == false) || (expr_data.type != NIL_T)) { // only acceptable if a is ?type and b is null, else error
                    error = ERR_TYPE_COMPATABILITY;
                    printf("error: assignment type mismatch\n");
                    return;
                }
            } else {
                if ((var_data.variable.is_null_type == false) && expr_data.is_optional_null) { // types can be the same but differ in null includement
                    error = ERR_TYPE_COMPATABILITY;
                    printf("error: trying to assign optional-null type into non-null variable, type mismatch\n");
                    return;
                }
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

    /* const comptime check */
    if (var_data.variable.is_constant) {
        TNode* rhs = var_data.variable.value_pointer;

        if (rhs && (rhs->type != FUNCTION_CALL))
            var_data.variable.comp_runtime = true;
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

        if (datatype == NIL_T || datatype == STR_T) {
            printf("error: unknown type - type of the variable is not specified and cannot be inferred from the expression used\n");
            error = ERR_UNKNOWN_TYPE;
            return;
        }

        var_data.variable.type = datatype; // assign result data type from exp/fun call
        var_data.variable.is_null_type = is_optional_null; // is ?type
    }

    if (symtable_insert(current_scope->current_scope, variable_id, var_data) == false) {
        error = ERR_COMPILER_INTERNAL;
        return;
    }

    if ((int) var_data.variable.type != datatype) { // a = b inequal types
        if ((var_data.variable.is_null_type == false) || (datatype != NIL_T)) { // only acceptable if a is ?type and b is null
            printf("error: var/const datatype doesn't match expression/function return type\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }
    } else {
        if ((var_data.variable.is_null_type == false) && is_optional_null) {
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
    case U8:  // INVALID LITERAL TYPE
        error = ERR_TYPE_COMPATABILITY;
        break;
    case STR: // STR LITERAL
        info->type = STR_T;
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

    /* +,-,* implicit conversion of: f64 const & literal with zero decimal part -> i32 literal, i32 literal -> f64 literal */
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    {
        if (left.is_optional_null || right.is_optional_null) { // Unacceptable to have ?type in arithmetics
            printf(RED_BOLD("error")": type mismatch, trying to use optional null type in arithmethics\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }

        if ((left.type == INTEGER_T && right.type == INTEGER_T) || ((left.type == FLOAT_T && right.type == FLOAT_T))) { // i32 = i32, f64 = f64
            info->type = left.type;
        }
        else if ((left.type == INTEGER_T && right.type == FLOAT_T) || (left.type == FLOAT_T && right.type == INTEGER_T)) { // i32 op f64, f64 op i32
            /* type conversion here */
            if (left.is_constant_exp || right.is_constant_exp) { //TODO CHECK POTENTIAL ERROR One side has to be non variable

                if (expression->left->type == INT) { /* left op is i32 literal */
                    char **literal = &expression->left->data.nodeData.value.literal;

                    *literal = literal_convert_i32_to_f64(*literal);
                    expression->left->type = FL;
                    info->type = FLOAT_T; // Succesful conversion of i32 literal, result of binary +-* operation is float type
                }
                else if (expression->right->type == INT) { /* right op is i32 literal */
                    char **literal = &expression->right->data.nodeData.value.literal;

                    *literal = literal_convert_i32_to_f64(*literal);
                    expression->right->type = FL;
                    info->type = FLOAT_T; // Succesful conversion of i32 literal, result of binary +-* operation is float type
                }
                else if (expression->left->type == FL && expression->right->type != INT) { // Convert lhs f64 literal to i32 if it has zero-decimal part and rhs is NOT i32 literal (its const, op result, variable, ...) 
                    char **literal = &expression->left->data.nodeData.value.literal;

                    if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) { // NULL => float has non zero decimal part, so it cannot be converted
                        printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    expression->left->type = INT; // binary op result type = i32, changed in AST
                    info->type = INTEGER_T; // Succesful conversion of i32 literal, result of binary +-* operation is integer type
                }
                else if (expression->right->type == FL && expression->left->type != INT) { // Convert rhs f64 literal to i32 if it has zero-decimal part and lhs is NOT i32 literal (its const, op result, variable, ...) 
                   char **literal = &expression->right->data.nodeData.value.literal;

                    if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) {  // NULL => float has non zero decimal part, so it cannot be converted
                        printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    expression->right->type = INT; // binary op result type = i32, changed in AST
                    info->type = INTEGER_T; // Succesful conversion of i32 literal, result of binary +-* operation is integer type
                }
                else if (expression->left->type == VAR_CONST && left.type == FLOAT_T && expression->right->type != INT) { // Convert lhs f64 comp-time const to i32 if it has zero-decimal part and rhs is NOT i32 literal (its const, op result, variable, ...) 
                    TData var_data;
                    char *variable_id = expression->left->data.nodeData.value.identifier;

                    var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                    check_error();


                    if (var_data.variable.is_constant == false) { // float operator is var, which cannot be implicitly converted
                        printf("error: expression - cannot convert var in division\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                        printf("error: expression - cannot convert const in division (constant value is unknown at compile time)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                    if ((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                        printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    expression->left->type = INT;
                    expression->left->data.nodeData.value.literal = const_value;
                    info->type = INTEGER_T; // Succesful conversion of f64 const, result of binary +-* operation is integer type
                }
                else if (expression->right->type == VAR_CONST && right.type == FLOAT_T && expression->left->type != INT) { // Convert rhs f64 comp-time const to i32 if it has zero-decimal part and lhs is NOT i32 literal (its const, op result, variable, ...) 
                    TData var_data;
                    char *variable_id = expression->right->data.nodeData.value.identifier;

                    var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                    check_error();

                    if (var_data.variable.is_constant == false) {
                        printf("error: expression - cannot convert var in division\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    if (var_data.variable.comp_runtime == false) {
                        printf("error: expression - cannot convert const in division (constant value is unknown at compile time)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal);

                    if ((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                        printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    expression->right->type = INT;
                    expression->right->data.nodeData.value.literal = const_value;
                    info->type = INTEGER_T; // Succesful conversion of f64 const, result of binary +-* operation is integer type
                }
                else { // No i32 literal / f64 const expression on either side to be converted, conversion error (i32/f64 is var, expression or operation result)
                    printf(RED_BOLD("error")": arithmetic (+ - *) expression error, cannot convert non-i32 literal\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }
            } else { // var must be the same type
                printf(RED_BOLD("error")": var type mismatch\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }
        } else { // invalid data types in arithmetics: UNKNOWN_T, NULL_T, STR_T
            printf(RED_BOLD("error")": expression type mismatch\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }

        if (!left.is_constant_exp || !right.is_constant_exp) { // Result contains var => not a constant expression, unknown at comp-time
            info->is_constant_exp = false;
        }
    }
    break;

    /* / implicit conversion of: f64 const & literal with zero decimal part -> i32 literal */
    case OP_DIV:
    {
        if (left.is_optional_null || right.is_optional_null) {
            printf(RED_BOLD("error")": type mismatch, trying to use null type in arithmethics\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }
        if ((left.type == INTEGER_T && right.type == INTEGER_T) || ((left.type == FLOAT_T && right.type == FLOAT_T))) { // i32 = i32, f64 = f64
            info->type = left.type; // Types equal on both sides
        }
        else if ((left.type == INTEGER_T && right.type == FLOAT_T) || (left.type == FLOAT_T && right.type == INTEGER_T)) {
            if (left.is_constant_exp || right.is_constant_exp) {  //TODO CHECK POTENTIAL ERROR One side has to be non var

                if (expression->left->type == FL) { // left operand = f64 literal
                    char **literal = &expression->left->data.nodeData.value.literal;

                    if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) { // NULL => float has non zero decimal part, so it cannot be converted
                        printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    expression->left->type = INT; // binary op result type = i32, changed in AST
                }
                else if (expression->right->type == FL) { // right operand = f64 literal
                    char **literal = &expression->right->data.nodeData.value.literal;

                    if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) {  // NULL => float has non zero decimal part, so it cannot be converted
                        printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    expression->right->type = INT; // binary op result type = i32, changed in AST
                }
                else if (expression->left->type == VAR_CONST && left.type == FLOAT_T) {
                    TData var_data;
                    char *variable_id = expression->left->data.nodeData.value.identifier;

                    var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                    check_error();


                    if (var_data.variable.is_constant == false) { // float operator is var, which cannot be implicitly converted
                        printf("error: expression - cannot convert var in division\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                        printf("error: expression - cannot convert const in division (constant value is unknown at compile time)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                    if ((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                        printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    expression->left->type = INT;
                    expression->left->data.nodeData.value.literal = const_value;

                }
                else if (expression->right->type == VAR_CONST && right.type == FLOAT_T) {
                    TData var_data;
                    char *variable_id = expression->right->data.nodeData.value.identifier;

                    var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                    check_error();


                    if (var_data.variable.is_constant == false) {
                        printf("error: expression - cannot convert var in division\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    if (var_data.variable.comp_runtime == false) {
                        printf("error: expression - cannot convert const in division (constant value is unknown at compile time)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal);

                    if ((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                        printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                        error = ERR_TYPE_COMPATABILITY;
                        return;
                    }

                    expression->right->type = INT;
                    expression->right->data.nodeData.value.literal = const_value;

                } else { // No f64 literal/const expression on either side to be converted, conversion error (f64 op is either var, expression or operation result)
                    printf(RED_BOLD("error")": arithmetic (/) expression error, cannot convert non-f64 literal/ non-const expression \n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                info->type = INTEGER_T;
            } else { // var must be the same type, this type mismatch is not acceptable
                printf(RED_BOLD("error")": var type mismatch\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }
        } else { // UNKNOWN_T, NULL_T, STR_T
            printf(RED_BOLD("error")": expression type mismatch\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }

        if (!left.is_constant_exp || !right.is_constant_exp) { // Result contains var => not a constant expression, unknown at comp-time
            info->is_constant_exp = false;
        }
    }
    break;

    /* ==,!= implicit conversion of: f64 const & literal with zero decimal part -> i32 literal, i32 const & literal -> f64 literal */
    case OP_EQ:
    case OP_NEQ:
    {
        if (left.type == STR_T || right.type  == STR_T) {
            printf("error: string in relation operators expression (==, !=)\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }
        if (left.type != right.type) {
            if (!left.is_constant_exp && !right.is_constant_exp) { // two variables with different types => error
                printf("error: var type mismatch in relation operators expression (==, !=)\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }

            if (expression->left->type == INT) { // lhs i32 literal => convert to f64 literal
                char **literal = &expression->left->data.nodeData.value.literal;

                *literal = literal_convert_i32_to_f64(*literal);
                expression->left->type = FL;
            }
            else if (expression->right->type == INT) { // rhs i32 literal => convert to f64 literal
                char **literal = &expression->right->data.nodeData.value.literal;

                *literal = literal_convert_i32_to_f64(*literal);
                expression->right->type = FL;
            }
            else if (expression->left->type == VAR_CONST && left.type == INTEGER_T && left.is_constant_exp) { // lhs i32 const => convert to f64 literal
                TData var_data;
                char *variable_id = expression->left->data.nodeData.value.identifier;

                var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                check_error();

                if (var_data.variable.is_constant == false) { // i32 operator is var, which cannot be implicitly converted
                    printf("error: expression - cannot convert var in (==, !=)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                    printf("error: expression - cannot convert const in (==, !=) (constant value is unknown at compile time)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                const_value = literal_convert_i32_to_f64(const_value);

                expression->left->type = FL;
                expression->left->data.nodeData.value.literal = const_value;
            }
            else if (expression->right->type == VAR_CONST && right.type == INTEGER_T && right.is_constant_exp) { // rhs i32 const => convert to f64 literal
                TData var_data;
                char *variable_id = expression->right->data.nodeData.value.identifier;

                var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                check_error();

                if (var_data.variable.is_constant == false) { // i32 operator is var, which cannot be implicitly converted
                    printf("error: expression - cannot convert var in (==, !=)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                    printf("error: expression - cannot convert const in (==, !=) (constant value is unknown at compile time)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                const_value = literal_convert_i32_to_f64(const_value);

                expression->right->type = FL;
                expression->right->data.nodeData.value.literal = const_value;
            }
            else if (expression->left->type == FL && (!right.is_constant_exp || expression->right->type != INT || expression->right->type != VAR_CONST)) { // lhs f64 literal, rhs is not i32 literal or const (op result, variable)
                char **literal = &expression->left->data.nodeData.value.literal;

                if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) { // NULL => float has non zero decimal part, so it cannot be converted
                    printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                expression->left->type = INT; // binary op result type = i32, changed in AST
            }
            else if (expression->right->type == FL && (!left.is_constant_exp || expression->left->type != INT || expression->left->type != VAR_CONST)) { // rhs f64 literal, lhs is not i32 literal or const (op result, variable)
                char **literal = &expression->right->data.nodeData.value.literal;

                if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) { // NULL => float has non zero decimal part, so it cannot be converted
                    printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                expression->right->type = INT; // binary op result type = i32, changed in AST
            }
            else if ((expression->left->type == VAR_CONST && left.is_constant_exp) && (!right.is_constant_exp || expression->right->type != INT || expression->right->type != VAR_CONST)) { // lhs f64 literal, rhs is not i32 literal or const (op result, variable)
                TData var_data;
                char *variable_id = expression->left->data.nodeData.value.identifier;

                var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                check_error();


                if (var_data.variable.is_constant == false) { // float operator is var, which cannot be implicitly converted
                    printf("error: expression - cannot convert var in  relation operators (==, !=)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                    printf("error: expression - cannot convert const in relation operators (==, !=) (constant value is unknown at compile time)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                /*ERROR!!!!!!!!!!!!!!! NUTNO ZKOPIROVAT NE UKRAST POZOR POZOR POZOR NUTNO OPRAVIT VSUDE*/
                char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                if ((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                    printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                expression->left->type = INT;
                expression->left->data.nodeData.value.literal = const_value;
            }
            else if ((expression->right->type == VAR_CONST && right.is_constant_exp) && (!left.is_constant_exp || expression->left->type != INT || expression->left->type != VAR_CONST)) { // rhs f64 literal, lhs is not i32 literal or const (op result, variable)
                TData var_data;
                char *variable_id = expression->right->data.nodeData.value.identifier;

                var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                check_error();

                if (var_data.variable.is_constant == false) { // float operator is var, which cannot be implicitly converted
                    printf("error: expression - cannot convert var in  relation operators (==, !=)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                    printf("error: expression - cannot convert const in relation operators (==, !=) (constant value is unknown at compile time)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                if ((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                    printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                expression->right->type = INT;
                expression->right->data.nodeData.value.literal = const_value;
            }
            else {
                if (left.type != NIL_T || right.type != NIL_T) { // Any type can be compared to 'null'
                    printf("error: relation operators (==, !=) type mismatch\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }
            }
        }

        if (!left.is_constant_exp || !right.is_constant_exp) { // Result contains var => not a constant expression, unknown at comp-time
            info->is_constant_exp = false;
        }

        info->type = BOOL_T;
    }
    break;

    /* >, <, >=, <= implicit conversion of: f64 const & literal with zero decimal part -> i32 literal, i32 const & literal -> f64 literal */
    case OP_GT:
    case OP_LS:
    case OP_GTE:
    case OP_LSE:
    {
        if (left.type == STR_T || right.type  == STR_T) {
            printf("error: string type in relation operators expression\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }
        if (left.type == NIL_T || right.type == NIL_T) {
            printf("error: null in relation operators expression\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }
        if (left.is_optional_null || right.is_optional_null) {
            printf(RED_BOLD("error")": type mismatch, trying to use optional null type in relation operators expression\n");
            error = ERR_TYPE_COMPATABILITY;
            return;
        }
        if (left.type != right.type) {
            if (!left.is_constant_exp && !right.is_constant_exp) { // two variables with different types => error
                printf("error: var type mismatch in relation operators expression (==, !=)\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }

            if (expression->left->type == INT) { // lhs i32 literal => convert to f64 literal
                char **literal = &expression->left->data.nodeData.value.literal;

                *literal = literal_convert_i32_to_f64(*literal);
                expression->left->type = FL;
            }
            else if (expression->right->type == INT) { // rhs i32 literal => convert to f64 literal
                char **literal = &expression->right->data.nodeData.value.literal;

                *literal = literal_convert_i32_to_f64(*literal);
                expression->right->type = FL;
            }
            else if (expression->left->type == VAR_CONST && left.type == INTEGER_T && left.is_constant_exp) { // lhs i32 const => convert to f64 literal
                TData var_data;
                char *variable_id = expression->left->data.nodeData.value.identifier;

                var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                check_error();

                if (var_data.variable.is_constant == false) { // i32 operator is var, which cannot be implicitly converted
                    printf("error: expression - cannot convert var in (==, !=)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                    printf("error: expression - cannot convert const in (==, !=) (constant value is unknown at compile time)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                const_value = literal_convert_i32_to_f64(const_value);

                expression->left->type = FL;
                expression->left->data.nodeData.value.literal = const_value;
            }
            else if (expression->right->type == VAR_CONST && right.type == INTEGER_T && right.is_constant_exp) { // rhs i32 const => convert to f64 literal
                TData var_data;
                char *variable_id = expression->right->data.nodeData.value.identifier;

                var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                check_error();

                if (var_data.variable.is_constant == false) { // i32 operator is var, which cannot be implicitly converted
                    printf("error: expression - cannot convert var in (==, !=)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                    printf("error: expression - cannot convert const in (==, !=) (constant value is unknown at compile time)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                const_value = literal_convert_i32_to_f64(const_value);

                expression->right->type = FL;
                expression->right->data.nodeData.value.literal = const_value;
            }
            else if (expression->left->type == FL && (!right.is_constant_exp || expression->right->type != INT || expression->right->type != VAR_CONST)) { // lhs f64 literal, rhs is not i32 literal or const (op result, variable)
                char **literal = &expression->left->data.nodeData.value.literal;

                if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) { // NULL => float has non zero decimal part, so it cannot be converted
                    printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                expression->left->type = INT; // binary op result type = i32, changed in AST
            }
            else if (expression->right->type == FL && (!left.is_constant_exp || expression->left->type != INT || expression->left->type != VAR_CONST)) { // rhs f64 literal, lhs is not i32 literal or const (op result, variable)
                char **literal = &expression->right->data.nodeData.value.literal;

                if ((*literal = literal_convert_f64_to_i32(*literal)) == NULL) { // NULL => float has non zero decimal part, so it cannot be converted
                    printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                expression->right->type = INT; // binary op result type = i32, changed in AST
            }
            else if ((expression->left->type == VAR_CONST && left.is_constant_exp) && (!right.is_constant_exp || expression->right->type != INT || expression->right->type != VAR_CONST)) { // lhs f64 literal, rhs is not i32 literal or const (op result, variable)
                TData var_data;
                char *variable_id = expression->left->data.nodeData.value.identifier;

                var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                check_error();


                if (var_data.variable.is_constant == false) { // float operator is var, which cannot be implicitly converted
                    printf("error: expression - cannot convert var in  relation operators (==, !=)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                    printf("error: expression - cannot convert const in relation operators (==, !=) (constant value is unknown at compile time)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                /*ERROR!!!!!!!!!!!!!!! NUTNO ZKOPIROVAT NE UKRAST POZOR POZOR POZOR NUTNO OPRAVIT VSUDE*/
                char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                if ((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                    printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                expression->left->type = INT;
                expression->left->data.nodeData.value.literal = const_value;
            }
            else if ((expression->right->type == VAR_CONST && right.is_constant_exp) && (!left.is_constant_exp || expression->left->type != INT || expression->left->type != VAR_CONST)) { // rhs f64 literal, lhs is not i32 literal or const (op result, variable)
                TData var_data;
                char *variable_id = expression->right->data.nodeData.value.identifier;

                var_data = get_const_var_data(scope, variable_id); // fetch var/const data
                check_error();

                if (var_data.variable.is_constant == false) { // float operator is var, which cannot be implicitly converted
                    printf("error: expression - cannot convert var in  relation operators (==, !=)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                if (var_data.variable.comp_runtime == false) { // const value is unknown at the time of compilation
                    printf("error: expression - cannot convert const in relation operators (==, !=) (constant value is unknown at compile time)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                char *const_value = copy_literal(var_data.variable.value_pointer->data.nodeData.value.literal); // Extract the literal to be converted

                if ((const_value = literal_convert_f64_to_i32(const_value)) == NULL) {
                    printf("error: expression - cannot do implicit conversion of f64 value (non-zero decimal part)\n");
                    error = ERR_TYPE_COMPATABILITY;
                    return;
                }

                expression->right->type = INT;
                expression->right->data.nodeData.value.literal = const_value;
            }
            else {
                printf("error: relation operators (==, !=) type mismatch\n");
                error = ERR_TYPE_COMPATABILITY;
                return;
            }
        }

        if (!left.is_constant_exp || !right.is_constant_exp) { // Result contains var => not a constant expression, unknown at comp-time
            info->is_constant_exp = false;
        }

        info->type = BOOL_T;
    }
    break;

    default:
        break;
    }
    //BT_print_tree(expression);
}

/* Helper functions */

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
}

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

    char *converted = (char*) malloc(sizeof(char) * new_size);
    memset(converted, 0, new_size);

    strcpy(converted, literal);
    strcat(converted, ".0");

    return converted;
}

/**
* Converts F64 literal/const to I32
*/
char *literal_convert_f64_to_i32(char *literal) {
    if (literal == NULL)
        return NULL;

    int i = 0;
    bool zero_decimal = true;

    while (literal[i] != '.' && literal[i] != '\0') {
        ++i;
    }

    if (literal[i] != '.') { //1e2 format, no '.' part
        printf("error: missing '.' decimal separator --> %s\n", literal);
        return NULL;
    }

    int j = i + 1;

    while (literal[j]) {
        if (literal[j] != '0') {
            zero_decimal = false;
            break;
        }
        ++j;
    }

    if (zero_decimal == false) {
        printf("error: f2i conversion, float number has non-zero decimal part --> %s\n", literal);
        return NULL;
    }

    literal[i] = '\0';

    return literal;
}

/**
* Returns return type of given function in integer format
*/
int get_func_type(TSymtable *globalSymTable, char *function_id) {
    TData function_data;

    /* Check if function is defined */
    if (symtable_search(globalSymTable, function_id) == false) {
        printf("error: function %s undefined\n", function_id);
        error = ERR_UNDEFINED_IDENTIFIER;
        return -1;
    }

    /* Get functions metadata */
    if (symtable_get_data(globalSymTable, function_id, &function_data) == false) {
        error = ERR_COMPILER_INTERNAL;
        return -1;
    }

    return function_data.function.return_type;
}

/**
* Fetches data about given var/constant
*/
TData get_const_var_data(struct TScope* scope, char *variable_id) {
    TData variable_data;

    memset(&variable_data, 0, sizeof(variable_data));

    TSymtable* local_st;

    if (id_defined(scope, variable_id, &local_st) == false) {
        printf("error: var/const %s undefined\n", variable_id);
        error = ERR_UNDEFINED_IDENTIFIER;
        return variable_data;
    }

    /* Get var/const metadata */
    if (symtable_get_data(local_st, variable_id, &variable_data) == false) {
        error = ERR_COMPILER_INTERNAL;
        return variable_data;
    }

    return variable_data;
}

char *copy_literal(char *literal) {
    size_t size = strlen(literal) + 1;

    char *copy = (char*) malloc(size);

    if (copy == NULL) {
        error = ERR_COMPILER_INTERNAL;
        return NULL;
    }

    memset(copy, 0, size);

    strcpy(copy, literal);

    return copy;
}