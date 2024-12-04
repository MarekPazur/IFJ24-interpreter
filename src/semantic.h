/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xpazurm00, Marek Pazúr
 * 
 * @file semantic.c
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "token.h"
#include "syna.h"
#include "symtable.h"
#include "binary_tree.h"

// Struct with information about expressions
typedef struct {
    int type;	// result type
    TData data;	// data about variable/constant
    bool is_constant_exp; // is result of (sub)expression constant
    bool is_optional_null; // flag set to true if the type contains null 
    char *optional_null_id; // id of the optional null variable used in expression
} expr_info;

// Struct with information about function calls
typedef struct {
    int type;   // function return type
    bool is_optional_null; // flag set to true if the type contains null 
} fun_info;

/**
 * Main function of semantic analysis, checks main function, initiates the traverse of the abstract syntax tree.
 * \param TBinaryTree* AST (pointer to Abstract syntax (binary) Tree)
 */
void semantic_analysis(TBinaryTree* AST);

/**
 * Fetches its scope, initiates semantic analysis of commands in its body, passes its scope to the command semantic function.
 * Does a check on missing/extra return statement.
 * \param TNode* func (pointer to node function instance)
 */
void FunctionSemantics(TNode* func);

/**
 * Iterates through the command branch and performs checks based on the command instance type.
 * Incase the command has its own body, it calls itself recursively and passes the command instances scope as a parameter.
 * At the end, the function checks if every var/const is used, incase of the var also mutated.
 * 
 * \param TNode* Command (pointer to command wrapper node )
 * \param scope_t* current_scope (pointer to structure contaning current local symtable and its parent scope)
 * \param TNode* func (pointer to the statement's parent function)
 */
void CommandSemantics(TNode* Command, scope_t* current_scope, TNode* func);

/**
 * Performs semantic analysis on function call command instance, that consists of:
 * Function definition check (existence of the function)
 * Parameter type and count checking
 * Definition of formal parameters in current scope
 * 
 * \param TNode* Command (pointer to command wrapper node )
 * \param scope_t* current_scope (pointer to structure contaning current local symtable and its parent scope)
 * \param TNode* func (pointer to the statement's parent function)
 */
void FunctionCallSemantics(TNode *functionCall, scope_t* current_scope, fun_info* info);

/**
 * Checks if main function is defined, if it has 0 parameters and void return type.
 * 
 * \param TSymtable* globalSymTable (Global symtable containing information about every function)
 */
void main_function_semantics(TSymtable* globalSymTable);

/**
 * Performs semantic analysis of assignment, which consists of checking function call return type and
 * expression result type compatability with the type of given variable
 * 
 * \param TNode* command_instance (pointer to command instance node)
 * \param scope_t* current_scope (pointer to structure contaning current local symtable and its parent scope)
 */
void assig_check(TNode* command_instance, scope_t *scope);

/**
 * Performs semantic analysis of declaration, which consists of checking function call return type and
 * expression result type compatability with the type of given variable/constant.
 * Also updates the record of a constant in its symtable with the information of it being comp-time or not.
 * 
 * \param TNode* declaration (pointer to declaration node)
 * \param scope_t* current_scope (pointer to structure contaning current local symtable and its parent scope)
 */
void declaration_semantics(TNode* declaration, scope_t* current_scope);

/**
 * Checks if the expression type inside the condition brackets matches the expected type of given if/while command.
 *
 * The if/while conditional command can either accept a truth expression or an expression involving the null type.
 * 
 * If the if/while conditional command accepts expression including null, the record of the constant that inherits
 * the null variables type and value is updated in its local symtable. 
 * 
 * \param TNode* body (pointer to body node)
 * \param scope_t* current_scope (pointer to structure contaning current local symtable and its parent scope)
 */
void check_head_type(TNode* body, scope_t *scope);

/**
 * Checks if the expression type inside the condition brackets matches the expected type of given if/while command.
 *
 * The if/while conditional command can either accept a truth expression or an expression involving the null type.
 * 
 * If the if/while conditional command accepts expression including null, the record of the constant that inherits
 * the null variables type and value is updated in its local symtable. 
 * 
 * \param TNode* expression (pointer to expression node)
 * \param scope_t* current_scope (pointer to data structure contaning current local symtable and its parent scope)
 * \param expr_info* info (pointer to data structure containing the result information of given expression)
 */
void expression_semantics(TNode *expression, scope_t* scope, expr_info* info);

// Helper functions

/**
* Checks if var/const is defined in given scope or its inherited (parent) scope(s)
* \param out_sym reference to scope the variable is in if it exists
* returns true or false depending on its definition
*/
TSymtable* declaration_var_check(struct TScope scope, char* identifier);

/**
* Returns count of formal parameters in function call
* fun(a,b,c) ---> 3
*/
int formal_param_count(TNode *formal_param);

/**
* Checks if var/const is defined in given scope or its inherited (parent) scope(s)
* \param out_sym reference to scope the variable is in if it exists
* returns true or false depending on its definition
*/
bool id_defined(struct TScope* scope, char* identifier,  TSymtable** out_sym);

/**
* Returns variables data type in char format 'i','u','f'
*/
char get_var_type(Type type);

/**
* Returns literal data type in char format 'i','u','f','s'
*/
char get_literal_type(int type);

/**
* Sets given variable property is_used to true
*/
void set_to_used(TSymtable* symtable, char* identifier);

/**
* Converts I32 literal/const to F64
* 
* Returns converted literal
*/
char *literal_convert_i32_to_f64(char *literal);

/**
* Converts F64 literal/const to I32.
* 
* Returns converted literal if it has zero decimal part, else it returs null
*/
char *literal_convert_f64_to_i32(char *literal);

/**
* Allocates new memory space and copies given literal into it.
* 
* Returns allocated string 
*/
char *copy_literal(char *literal);

// Data fetch functions

/**
* Returns return type of given function in integer format
*/
int get_func_type(TSymtable *globalSymTable, char *function_id);

/**
* Fetches data about given var/constant
*/
TData get_const_var_data(struct TScope* scope, char *variable_id);

#endif
