/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xglosro00, Robert Glos
 * @author xukropj00, Jan Ukropec
 * @author xtomasp00, Patrik Tomaško
 * @author xpazurm00, Marek Pazúr
 *
 * @file syna.h
 */

#ifndef SYNA_H
#define SYNA_H

#include "token.h"
#include "symtable.h"
#include "binary_tree.h"

typedef enum fsm_state_syna {
    STATE_ROOT, //expecting either a function header or import
    STATE_identifier, //expecting identifier
    STATE_assig, //expecting =
    STATE_prolog, //expectin @import
    STATE_lr_bracket, //expecting (
    STATE_rr_bracket, //expecting )
    STATE_semicolon, //expecting ;
    STATE_identifier_prolog, //@import(this part)
    STATE_fn, //expecting keyword fn
    STATE_type, //data type
    STATE_open_body_check, //the beginning { of the body of function/if/else/while
    STATE_next_command, //expecting either next command or } end of body
    STATE_first_fn_param, //expecting either ) or the first param of fn
    STATE_colon, //expecting :
    STATE_ls_bracket, //expecting ]
    STATE_u8, //expecting u8
    STATE_type_fn_param, //expecting a type of a parameter of a function
    STATE_rs_bracket_fn_param, //expecting the ] of []u8 in a parameter of a function
    STATE_u8_fn_param, //expectin u8 keyword in a parameter of a function
    STATE_coma, //expectin ,
    STATE_identifier_fn_param, //expecting an identifier of a parameter of a function (not first)
    STATE_command, //expecting commands in the body of a function/while/if/else
    STATE_operand, //expecting an operand in a true or a regular statement
    STATE_operator, //expecting an operator in a true or a regular statement
    STATE_pipe, //expecting this |
    STATE_possible_else, //  else
    STATE_open_else, // { after else
    STATE_possible_function,
    STATE_possible_qmark, //?
    STATE_assig_must
} Pfsm_state_syna;


typedef struct TScope{
        TSymtable* current_scope;       // current scope parser is in
        struct TScope* parent_scope;    // parent scope of current scope
} scope_t;

typedef struct parser {
    Pfsm_state_syna state;      // Automata state
    token_t current_token;      // processed token

    char *processed_identifier; // function or variable identifier thats being processed

    TSymtable* global_symtable; // global symtable for functions

    scope_t scope; // current scope parser is in, parent scope of current scope

    TBinaryTree* AST;           // Abstract syntax tree thats being assembled
} Tparser;

/**
 * @brief This function creates a new symtable when entering a new sub body and sets the parser scope struct to have the new body symtable as the current_scope and the previous in the parent_scope
 *
 * @param parser, where the scope will be changed
 */
void enter_sub_body(Tparser* parser);

/**
 * @brief This function leaves sets the current_scope back to its parent and the parent_scope to the parent of the parent
 *
 * @param parser, where the scope will be changed
 */
void leave_sub_body(Tparser* parser);

/**
 * @brief This function a data structure for the symtable to insert a variable/constant
 *
 * @param nullable, signifies wether the returned data should have the nullable bool set to true or false
 * @param constant, signifies wether the returned data should have the constant bool set to true or false
 * @param type, represents the type to be set in the returned data
 *
 * @return returns the struct TData, that will be inserted into the symtable as a variable/constant
 */
TData declaration_data(bool nullable, bool constant, Type type);

/**
 * @brief This function a data structure for the symtable to insert a function, the data are set to default values
 *
 * @param scope, the scope of the symtables that will be set to the TData the will be returned and inserted into the symtable
 *
 * @return returns the struct TData, that will be inserted into the symtable as a function
 */
TData blank_function(TSymtable* scope);

/**
 * @brief This function initialises and runs the syntax analysis
 *
 * @param token, holds the current token to parse
 */
void init_parser(token_t token);

/**
 * @brief This function checks for a function header, prologue constant or the EOF
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void root_code(Tparser* parser, TNode** current_node);

/**
 * @brief This function checks the correct syntax of the prologue constant
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void import_func(Tparser* parser, TNode** current_node);

/**
 * @brief This function checks the correct syntax of function header and then calls the body function to check the correct syntax of the body contents
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void function_header(Tparser* parser, TNode** current_node);

/**
 * @brief This function checks the correct syntax of the commands inside a body of a function, if, else, while and body with no header
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void body(Tparser* parser, TNode** current_node);

/**
 * @brief This function checks the correct syntax of the header of while and if and calls the body function afterwards to check the contents of the if and while bodies
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void if_while_header(Tparser* parser, TNode** current_node, node_type type);

/**
 * @brief This function checks the correct syntax an expression
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param end, holds the token by which the expression should end
 * @param tree is the node onto which the newly created subtree will be appended
 * @param allow_empty represents the possibility of the expression to be empty
 */
void expression(Tparser* parser, token_id end, TNode **tree, bool allow_empty);

/**
 * @brief This function checks the correct syntax and the possibility of the |null_replacement| section in the while and if headers
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void null_replacement(Tparser* parser, TNode** current_node);

/**
 * @brief This function checks the correct syntax of the parameters of a function inside it's header
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void function_params(Tparser* parser, TNode** current_node);

/**
 * @brief This function checks the correct syntax of the declarations of constants and variables
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void var_const_declaration(Tparser* parser, TNode** current_node, node_type type);

/**
 * @brief This function checks the correct syntax of calling a function
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void function_call(Tparser* parser, TNode** current_node);

/**
 * @brief This function checks the correct syntax of the parameters of a called function
 *
 * @param parser, holds the current token and other structs to ensure the correct creation of the AST and creation and insertion into the symtables
 * @param current_node is the node onto which the newly created subtree will be appended
 */
void function_call_params(Tparser* parser, TNode** current_node);

/**
 * @brief This function fills the global symtable with the built-in ifj.name functions
 *
 * @param global_symtable where the function will be inserted
 */
void populate_builtin_functions(TSymtable* global_symtable);

#endif
