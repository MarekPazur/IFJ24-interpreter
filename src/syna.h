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

typedef struct parser {
    Pfsm_state_syna state;      // Automata state
    token_t current_token;      // processed token

    char *processed_identifier; // function or variable identifier thats being processed

    TSymtable* global_symtable; // global symtable for functions

    struct TScope{
    TSymtable* current_scope;       // current scope parser is in
    struct TScope* parent_scope;        // parent scope of current scope
    }scope;

    TBinaryTree* AST;           // Abstract syntax tree thats being assembled
} Tparser;

// Parser LL analysis functions
void init_parser(token_t token);

void root_code(Tparser* parser, TNode** current_node);

void import_func(Tparser* parser, TNode** current_node);

void function_header(Tparser* parser, TNode** current_node);

void body(Tparser* parser, TNode** current_node);

void if_while_header(Tparser* parser, TNode** current_node, node_type type);

void expression(Tparser* parser, token_id end);

void null_replacement(Tparser* parser, TNode** current_node);

void function_params(Tparser* parser, TNode** current_node);

void var_const_declaration(Tparser* parser, TNode** current_node, node_type type);

void function_call(Tparser* parser, TNode** current_node);

void function_call_params(Tparser* parser, TNode** current_node);

#endif
