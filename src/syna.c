/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xglosro00, Robert Glos
 * @author xukropj00, Jan Ukropec
 * @author xtomasp00, Patrik Tomaško
 * @author xpazurm00, Marek Pazúr
 *
 * @file syna.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "syna.h"
#include "token.h"
#include "precedent.h"
#include "compiler_error.h"
#include "lexer.h"
#include "semantic.h"

/**
 * @brief This function a data structure for the symtable to insert a variable/constant
 *
 * @param parser
 */
 TData declaration_data(bool nullable, bool constant, Type type){
 
    TData symtable_data;
    
    symtable_data.variable.is_null_type = nullable;
    symtable_data.variable.is_constant = constant;
    symtable_data.variable.is_used = false;
    symtable_data.variable.is_mutated = false;
    symtable_data.variable.comp_runtime = false;
    symtable_data.variable.value_pointer = NULL;
    symtable_data.variable.type = type;
    
    return symtable_data;
 }
 
 TData blank_function(TSymtable* scope){
 
    TData function_data;
    function_data.function.is_null_type = false;
    d_array_init((&function_data.function.argument_types), 2);
    function_data.function.return_type = UNKNOWN_T;
    function_data.function.function_scope = scope;
    
    return function_data;
 }

/**
 * @brief This function creates a new symtable when entering a new sub body and sets the parser scope struct to have the new body symtable as the current_scope and the previous in the parent_scope
 *
 * @param parser
 */
void enter_sub_body(Tparser* parser) {
    struct TScope* new_scope = (struct TScope*)malloc(sizeof(struct TScope));
    if (!new_scope) {
        error = ERR_COMPILER_INTERNAL;
        return ;
    }

    new_scope->current_scope = parser->scope.current_scope;
    new_scope->parent_scope = parser->scope.parent_scope;
    
    parser->scope.parent_scope = new_scope;
    parser->scope.current_scope = symtable_init();
    
    if (parser->scope.current_scope == NULL) {
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    
    if (parser->scope.current_scope == NULL) {
        error = ERR_COMPILER_INTERNAL;
    }

    return ;
}

/**
 * @brief This function leaves sets the current_scope back to its parent and the parent_scope to the parent of the parent
 *
 * @param parser
 */
 void leave_sub_body(Tparser* parser){
 
    //debug_print_keys(parser->scope.current_scope);
 
    if(parser->scope.parent_scope == NULL){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    
    parser->scope = *(parser->scope.parent_scope);
 }

/**
 * @brief This function initializes the parser
 *
 * @param token, will be used to store the current token
 */
void init_parser(token_t token) {
    //malloc for the parser and checking if it went correctly
    Tparser* parser = malloc(sizeof(Tparser));

    if (parser == NULL) {
        error = ERR_COMPILER_INTERNAL;
        return;
    }

    // Malloc and initialisation for the global symtable and checking if it went correctly
    parser->global_symtable = symtable_init();

    if (parser->global_symtable == NULL) {
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    
    // Inserting the built-in functions into the global symtable
    populate_builtin_functions(parser->global_symtable);
    if(error)
        return;

    // Initialising the AST
    parser->AST = BT_init();
    BT_insert_root(parser->AST, PROGRAM);
    parser->AST->root->data.nodeData.program.globalSymTable = parser->global_symtable;

    // Pointer to current scope were in, function local scope or its children blocks (while, if-else, headless body)
    parser->scope.current_scope = NULL;
    parser->scope.parent_scope = NULL;

    // Initial automata state
    parser->state = STATE_ROOT;

    // Processed token
    parser->current_token = token;

    TNode** root = &(parser->AST->root);
    root_code(parser, root);

    // debug functions
    //BT_print_tree(parser->AST->root);
    //debug_print_keys(parser->global_symtable);

    /* SEMANTIC ANALYSIS */
    semantic_analysis(parser->AST);
}

/**
 * @brief This function looks for function headers and the import thingy
 *
 * @param parser, holds the current token, symtables and the binary tree
 */
void root_code(Tparser* parser, TNode** current_node) {

    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    if (parser->state == STATE_ROOT) {
        switch (parser->current_token.id) {
        //<root_code> -> import_func | function_header | ""
        case TOKEN_KW_PUB:
            parser->state = STATE_fn;

            function_header(parser, &(*current_node)->left);
            //debug_print_keys(parser->scope.current_scope);

            if (error)
                return;

            parser->state = STATE_ROOT;
            root_code(parser, &(*current_node)->left);

            if (error)
                return;

            break;
        case TOKEN_KW_CONST:
            parser->state = STATE_identifier;

            import_func(parser, &(parser->AST->root->right));

            if (error)
                return;

            parser->state = STATE_ROOT;
            root_code(parser, current_node);

            if (error)
                return;

            break;

        case TOKEN_EOF:
            return;

        default:
            error = ERR_SYNTAX;
        }
        return;
    } else {
        error = ERR_SYNTAX;
        return;
    }
}

/**
 * @brief This function checks that the prolog is written syntactically correct
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void import_func(Tparser* parser, TNode** current_node) {
    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch (parser->state) {
    case STATE_identifier: //checking for this part const ->ifj<- = @import("ifj24.zig");
        if (parser->current_token.id == TOKEN_IDENTIFIER) {
            (*current_node) = create_node(PROLOGUE);

            parser->state = STATE_assig;
            import_func(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_identifier_prolog: //checking for this part const ifj = @import(->"ifj24.zig"<-);
        if (parser->current_token.id == TOKEN_LITERAL_STRING) {
            parser->state = STATE_rr_bracket;
            import_func(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_assig: //checking for this part const ifj ->=<- @import("ifj24.zig");
        if (parser->current_token.id == TOKEN_ASSIGNMENT) {
            parser->state = STATE_prolog;
            import_func(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_prolog: //checking for this part const ifj = ->@import<-("ifj24.zig");
        if (parser->current_token.id == TOKEN_PROLOG) {
            parser->state = STATE_lr_bracket;
            import_func(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_lr_bracket: //checking for this part const ifj = @import->(<-"ifj24.zig");
        if (parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT) {
            parser->state = STATE_identifier_prolog;
            import_func(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_rr_bracket: //checking for this part const ifj = @import("ifj24.zig"->)<-;
        if (parser->current_token.id == TOKEN_BRACKET_ROUND_RIGHT) {
            parser->state = STATE_semicolon;
            import_func(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_semicolon: //checking for this part const ifj = @import("ifj24.zig")->;<-
        if (parser->current_token.id == TOKEN_SEMICOLON) {
            return;
        }
        error = ERR_SYNTAX;
        break;
    default:
        error = ERR_SYNTAX;
    }
    return;
}

/**
 * @brief This function checks that a header of a function is syntactically correctly written
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void function_header(Tparser* parser, TNode** current_node) {
    
    if (error) return;
    
    TData param_data;
    int has_qmark = 0;
    linked_list_t llist;

    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch (parser->state) {
    case STATE_fn:
        if (parser->current_token.id == TOKEN_KW_FN) { //checking for pub ->fn<- name() type{
            (*current_node) = create_node(FN);

            parser->state = STATE_identifier;
            function_header(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_identifier:
        if (parser->current_token.id == TOKEN_IDENTIFIER) { //checking for pub fn ->name<-() type{
        
            if (symtable_search(parser->global_symtable, parser->current_token.lexeme.array)) {
                error = ERR_IDENTIFIER_REDEF_CONST_ASSIGN;
                return;
            }
        
            parser->scope.current_scope = symtable_init();
            
            if (parser->scope.current_scope == NULL) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            parser->processed_identifier = parser->current_token.lexeme.array;
        
            (*current_node)->data.nodeData.function.identifier = parser->current_token.lexeme.array;
            (*current_node)->data.nodeData.function.scope = parser->scope.current_scope;
            
            TData function_data = blank_function(parser->scope.current_scope);
            if(!symtable_insert(parser->global_symtable, parser->current_token.lexeme.array, function_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
          
            parser->state = STATE_lr_bracket;
            function_header(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_lr_bracket:
        if (parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT) { //checking for pub fn name->(<-) type{
        
            init_llist(&llist);
            (*current_node)->data.nodeData.function.param_identifiers = llist;
        
            parser->state = STATE_first_fn_param;
            function_params(parser, current_node);
            
            parser->state = STATE_type;
            function_header(parser, current_node);
            break;
            
        }
        error = ERR_SYNTAX;
        break;
    case STATE_type:
    
        parser->processed_identifier = (*current_node)->data.nodeData.function.identifier;
        
        if (parser->current_token.id == TOKEN_OPTIONAL_TYPE_NULL) {
            if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                return;
            has_qmark = 1;
        }
        switch (parser->current_token.id) {
        case TOKEN_KW_I32://checking for pub fn name() ->i32<-{
        
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.function.return_type = INTEGER_T;
            if(has_qmark == 1)
                param_data.function.is_null_type = true;
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            (*current_node)->data.nodeData.function.type = I32;
            parser->state = STATE_open_body_check;
            function_header(parser, current_node);
            break;
            
        case TOKEN_KW_F64://checking for pub fn name() ->f64<-{
        
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.function.return_type = FLOAT_T;
            if(has_qmark == 1)
                param_data.function.is_null_type = true;
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            (*current_node)->data.nodeData.function.type = F64;
            parser->state = STATE_open_body_check;
            function_header(parser, current_node);
            break;
            
        case TOKEN_KW_VOID://checking for pub fn name() ->void<-{
            if (has_qmark != 1) {
            
                if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                    error = ERR_COMPILER_INTERNAL;
                    return;
                }
                param_data.function.return_type = VOID_T;
                if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                    error = ERR_COMPILER_INTERNAL;
                    return;
                }
            
                (*current_node)->data.nodeData.function.type = VOID_TYPE;
                parser->state = STATE_open_body_check;
                function_header(parser, current_node);
                break;
                
            } else {
                error = ERR_SYNTAX;
                return;
            }
        case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name() ->[<-]u8{
            
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.function.return_type = U8_SLICE_T;
            if(has_qmark == 1)
                param_data.function.is_null_type = true;
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            (*current_node)->data.nodeData.function.type = U8_SLICE;
            parser->state = STATE_ls_bracket;
            function_header(parser, current_node);
            break;
            
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_open_body_check:
        if (parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT) { //checking for pub fn name() type->{<-
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_ls_bracket:
        if (parser->current_token.id == TOKEN_BRACKET_SQUARE_RIGHT) { //checking for pub fn name() [->]<-u8{
            parser->state = STATE_u8;
            function_header(parser, current_node);

            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_u8:
        if (parser->current_token.id == TOKEN_KW_U8) { //checking for pub fn name() []->u8<-{
            parser->state = STATE_open_body_check;

            function_header(parser, current_node);

            break;
        }
        error = ERR_SYNTAX;
        break;

    default:
        error = ERR_SYNTAX;
    }
    return;
}

/**
 * @brief This function checks that a the parameters of a function are written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void function_params(Tparser* parser, TNode** current_node) {
    if (error) return;
    
    TData param_data;

    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;
    switch (parser->state) {
    case STATE_first_fn_param:
        
        switch (parser->current_token.id) {
        case TOKEN_BRACKET_ROUND_RIGHT: //checking for pub fn name(->)<- type{
            parser->state = STATE_type;
            break;
        case TOKEN_IDENTIFIER: //checking for pub fn name(->param<- : type) type{
            
            parser->processed_identifier = parser->current_token.lexeme.array;
            
            if(!insert_llist(&(*current_node)->data.nodeData.function.param_identifiers, parser->processed_identifier)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            param_data = declaration_data(false, true, UNKNOWN_T);
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            parser->state = STATE_colon;
            function_params(parser, current_node);
            break;
            
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_colon:
        if (parser->current_token.id == TOKEN_COLON) { //checking for pub fn name(param ->:<- type) type{
            parser->state = STATE_possible_qmark;
            function_params(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_possible_qmark:
        switch (parser->current_token.id) { //checking for pub fn name(param : ?type) type{
        case TOKEN_OPTIONAL_TYPE_NULL:
            parser->state = STATE_type;
            function_params(parser, current_node);
            break;
        case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
            
            if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.variable.type = INTEGER_T;
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            d_array_append(&param_data.function.argument_types, 'i');
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            parser->state = STATE_coma;
            function_params(parser, current_node);
            break;
            
        case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
        
            if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.variable.type = FLOAT_T;
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            d_array_append(&param_data.function.argument_types, 'f');
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_coma;
            function_params(parser, current_node);
            break;
            
        case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
          
            if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.variable.type = U8_SLICE_T;
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            d_array_append(&param_data.function.argument_types, 'u');
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_ls_bracket;
            function_params(parser, current_node);
            break;
            
        default:
            error = ERR_SYNTAX;
            break;
        }
        break;
    case STATE_type:
        switch (parser->current_token.id) {
        case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
          
            if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.variable.type = INTEGER_T;
            param_data.variable.is_null_type = true;
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            d_array_append(&param_data.function.argument_types, 'i');
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
          
            parser->state = STATE_coma;
            function_params(parser, current_node);
            break;
            
        case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
        
            if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.variable.type = FLOAT_T;
            param_data.variable.is_null_type = true;
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            d_array_append(&param_data.function.argument_types, 'f');
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_coma;
            function_params(parser, current_node);
            break;
            
        case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
        
            if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            param_data.variable.type = U8_SLICE_T;
            param_data.variable.is_null_type = true;
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            if(!symtable_get_data(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, &param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            d_array_append(&param_data.function.argument_types, 'u');
            if(!symtable_insert(parser->global_symtable, (*current_node)->data.nodeData.function.identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_ls_bracket;
            function_params(parser, current_node);
            break;
            
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_identifier:
        switch (parser->current_token.id) {
        case TOKEN_BRACKET_ROUND_RIGHT: //checking for pub fn name(param : type ,->)<- type{
            break;
        case TOKEN_IDENTIFIER: //checking for pub fn name(param : type ,->param<- : type) type{
        
            parser->processed_identifier = parser->current_token.lexeme.array;
            
            if(!insert_llist(&(*current_node)->data.nodeData.function.param_identifiers, parser->processed_identifier)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            TData param_data;
            param_data = declaration_data(false, true, UNKNOWN_T);
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, param_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_colon;
            function_params(parser, current_node);
            break;
            
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_coma:
        switch (parser->current_token.id) {
        case TOKEN_COMMA: //checking for pub fn name(param : type ->,<- type) type{
            parser->state = STATE_identifier;
            function_params(parser, current_node);
            break;
        case TOKEN_BRACKET_ROUND_RIGHT: //checking for pub fn name(param : type ->)<- type{
            break;
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_ls_bracket:
        if (parser->current_token.id == TOKEN_BRACKET_SQUARE_RIGHT) { //checking for pub fn name(param : [->]<-u8) type{
            parser->state = STATE_u8;
            function_params(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_u8:
        if (parser->current_token.id == TOKEN_KW_U8) { //checking for pub fn name(param : []->u8<-) type{
            parser->state = STATE_coma;
            function_params(parser, current_node);
            break;
        }
        error = ERR_SYNTAX;
        break;

    default:
        error = ERR_SYNTAX;
    }

    return;
}

/**
 * @brief This function checks that the bodies of functions, whiles, ifs and elses are written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void body(Tparser* parser, TNode** current_node) {

    TData retrieved_data;
    
    TSymtable* identifier_residence;

    if (error) return;

    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch (parser->state) {
    case STATE_command:
        (*current_node) = create_node(COMMAND);
        switch (parser->current_token.id) {
        case TOKEN_KW_IF:
        
            parser->state = STATE_lr_bracket;
            if_while_header(parser, &(*current_node)->left, IF);
            if (error) return;
            
            leave_sub_body(parser);
            if (error) return;
            
            parser->state = STATE_possible_else;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
            
        case TOKEN_DISCARD_RESULT:
            if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                return;
            if (parser->current_token.id != TOKEN_ASSIGNMENT) {
                error = ERR_SYNTAX;
                return;
            } else {
            
                (*current_node)->left = create_node(ASSIG);
                (*current_node)->left->data.nodeData.identifier.identifier = "_";
                (*current_node)->left->data.nodeData.identifier.is_disposeable = true;
                parser->state = STATE_operand;

                /* Expression */
                expression(parser, TOKEN_SEMICOLON, &(*current_node)->left->left, false);

                //set data type for local_databased on expression result
                if (error) {
                    return;
                }
            }
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
        case TOKEN_KW_RETURN:
            (*current_node)->left = create_node(RETURN);

            parser->state = STATE_operand;

            /* Expression */
            expression(parser, TOKEN_SEMICOLON, &(*current_node)->left->left, true);

            if (error) {
                return;
            }

            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            break;
        case TOKEN_BRACKET_CURLY_LEFT:
        
            enter_sub_body(parser);
            if (error) return;

            (*current_node)->left = create_node(BODY);

            (*current_node)->left->data.nodeData.body.current_scope = (scope_t*) malloc(sizeof(scope_t));

            if ((*current_node)->left->data.nodeData.body.current_scope == NULL) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }

            *(*current_node)->left->data.nodeData.body.current_scope = parser->scope;
           
            parser->state = STATE_command;
            body(parser, &(*current_node)->left->right);
            if (error) return;
            
            leave_sub_body(parser);
            if (error) return;
            
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            break;
        case TOKEN_KW_WHILE:
        
            parser->state = STATE_lr_bracket;
            if_while_header(parser, &(*current_node)->left, WHILE);
            if (error) return;
            
            leave_sub_body(parser);
            if (error) return;
            
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
            
        case TOKEN_KW_VAR:
        
            parser->state = STATE_identifier;
            var_const_declaration(parser, &(*current_node)->left, VAR_DECL);
            if (error) return;
            
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error)
                return;
            break;
            
        case TOKEN_KW_CONST:
        
            parser->state = STATE_identifier;
            var_const_declaration(parser, &(*current_node)->left, CONST_DECL);
            if (error) return;
            
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error)
                return;
            break;
            
        case TOKEN_IDENTIFIER:
            //TODO add the expression
            parser->processed_identifier = parser->current_token.lexeme.array;
            if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                return;
            if (parser->current_token.id == TOKEN_ASSIGNMENT) {
            
                if((identifier_residence = declaration_var_check(parser->scope, parser->processed_identifier)) == NULL){
                    error = ERR_UNDEFINED_IDENTIFIER;
                    return;
                }
                
                symtable_get_data(identifier_residence, parser->processed_identifier, &retrieved_data);
                if( retrieved_data.variable.is_constant ){
                    error = ERR_IDENTIFIER_REDEF_CONST_ASSIGN;
                    return;
                }
                
                retrieved_data.variable.is_used = true;
                retrieved_data.variable.is_mutated = true;
                
                if(!symtable_insert(identifier_residence, parser->processed_identifier, retrieved_data)){
                    error = ERR_COMPILER_INTERNAL;
                    return;
                }
            
                (*current_node)->left = create_node(ASSIG);
                (*current_node)->left->data.nodeData.identifier.is_disposeable = false;
                (*current_node)->left->data.nodeData.identifier.identifier = parser->processed_identifier;
                parser->state = STATE_operand;

                /* Expression */
                expression(parser, TOKEN_SEMICOLON, &(*current_node)->left->left, false);

                //set data type for local_databased on expression result
                if (error) {
                    return;
                }
            }

            else if (parser->current_token.id == TOKEN_ACCESS_OPERATOR) {
                (*current_node)->left = create_node(FUNCTION_CALL);
                
                parser->state = STATE_identifier;
                function_call(parser, &(*current_node)->left);
                
                if (error)
                    return;
                    
                if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                    return;

                
                if (parser->current_token.id != TOKEN_SEMICOLON) {
                    error = ERR_SYNTAX;
                    return;
                }
            }
            else if (parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT) {
                (*current_node)->left = create_node(FUNCTION_CALL);
                (*current_node)->left->data.nodeData.identifier.identifier = parser->processed_identifier;
                parser->state = STATE_identifier;
                function_call_params(parser, &(*current_node)->left->right);
                if (error)
                    return;
                if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                    return;
                if (parser->current_token.id != TOKEN_SEMICOLON) {
                    error = ERR_SYNTAX;
                    return;
                }
            }
            else {
                error = ERR_SYNTAX;
                return;
            }
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
        case TOKEN_BRACKET_CURLY_RIGHT:
            free(*current_node);
            *current_node = NULL;
            break;
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_possible_else:
        (*current_node) = create_node(COMMAND);
        switch (parser->current_token.id) {
        case TOKEN_BRACKET_CURLY_LEFT:
        
            enter_sub_body(parser);
            if (error) return;

            (*current_node)->left = create_node(ELSE);

            (*current_node)->left->data.nodeData.body.current_scope = (scope_t*) malloc(sizeof(scope_t));

            if ((*current_node)->left->data.nodeData.body.current_scope == NULL) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }

            *(*current_node)->left->data.nodeData.body.current_scope = parser->scope;
            
            parser->state = STATE_command;
            body(parser, &(*current_node)->left->right);
            if (error) return;
            
            leave_sub_body(parser);
            if (error) return;
            
            
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
        case TOKEN_DISCARD_RESULT:
            if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                return;
            if (parser->current_token.id != TOKEN_ASSIGNMENT) {
                error = ERR_SYNTAX;
                return;
            } else {
            
                (*current_node)->left = create_node(ASSIG);
                (*current_node)->left->data.nodeData.identifier.is_disposeable = true;
                (*current_node)->left->data.nodeData.identifier.identifier = "_";
                parser->state = STATE_operand;

                /* Expression */
                expression(parser, TOKEN_SEMICOLON, &(*current_node)->left->left, false);

                //set data type for local_databased on expression result
                if (error) {
                    return;
                }
            }
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
        case TOKEN_KW_ELSE://checking for ..} ->else<- { ..
        
            enter_sub_body(parser);
            if (error) return;
        
            (*current_node)->left = create_node(ELSE);
            
            (*current_node)->left->data.nodeData.body.current_scope = (scope_t*) malloc(sizeof(scope_t));

            if ((*current_node)->left->data.nodeData.body.current_scope == NULL) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }

            *(*current_node)->left->data.nodeData.body.current_scope = parser->scope;

            parser->state = STATE_open_else;
            body(parser, &(*current_node)->left->right);
            if (error) return;
            
            leave_sub_body(parser);
            if (error) return;
            
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
            
        case TOKEN_KW_IF:
        
            parser->state = STATE_lr_bracket;
            if_while_header(parser, &(*current_node)->left, IF);
            if (error) return;
            
            leave_sub_body(parser);
            if (error) return;
            
            parser->state = STATE_possible_else;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
            
        case TOKEN_KW_WHILE:
            parser->state = STATE_lr_bracket;
            if_while_header(parser, &(*current_node)->left, WHILE);
            if (error) return;
            
            leave_sub_body(parser);
            if (error) return;
            
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
            
        case TOKEN_KW_RETURN:
            (*current_node)->left = create_node(RETURN);
            //TODO give the expression function the current_node->left->left to make the expression there
            parser->state = STATE_operand;

            /* Expression */
            expression(parser, TOKEN_SEMICOLON, &(*current_node)->left->left, true);
            if (error) return;

            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
        case TOKEN_KW_VAR:
            parser->state = STATE_identifier;
            var_const_declaration(parser, &(*current_node)->left, VAR_DECL);
            if (error) return;
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
        case TOKEN_KW_CONST:
            parser->state = STATE_identifier;
            var_const_declaration(parser, &(*current_node)->left, CONST_DECL);
            if (error) return;
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
        case TOKEN_IDENTIFIER:
            //TODO add the expression
            //probably set key here, depends on how we decide to approach this
            parser->processed_identifier = parser->current_token.lexeme.array;
            if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                return;

            if (parser->current_token.id == TOKEN_ASSIGNMENT) {
            
                if((identifier_residence = declaration_var_check(parser->scope, parser->processed_identifier)) == NULL){
                    error = ERR_UNDEFINED_IDENTIFIER;
                    return;
                }
                
                symtable_get_data(identifier_residence, parser->processed_identifier, &retrieved_data);
                if( retrieved_data.variable.is_constant ){
                    error = ERR_IDENTIFIER_REDEF_CONST_ASSIGN;
                    return;
                }
                
                retrieved_data.variable.is_used = true;
                retrieved_data.variable.is_mutated = true;
                
                if(!symtable_insert(identifier_residence, parser->processed_identifier, retrieved_data)){
                    error = ERR_COMPILER_INTERNAL;
                    return;
                }
            
                (*current_node)->left = create_node(ASSIG);
                (*current_node)->left->data.nodeData.identifier.is_disposeable = false;
                (*current_node)->left->data.nodeData.identifier.identifier = parser->processed_identifier;

                parser->state = STATE_operand;
                /* Expression */
                expression(parser, TOKEN_SEMICOLON, &(*current_node)->left->left, false);
                if (error) return;
            }
            else if (parser->current_token.id == TOKEN_ACCESS_OPERATOR) {
                (*current_node)->left = create_node(FUNCTION_CALL);
                parser->state = STATE_identifier;
                function_call(parser, &(*current_node)->left);
                if (error) return;

                if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                    return;
                if (parser->current_token.id != TOKEN_SEMICOLON) {
                    error = ERR_SYNTAX;
                    return;
                }
            }
            else if (parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT) {
                (*current_node)->left = create_node(FUNCTION_CALL);
                (*current_node)->left->data.nodeData.identifier.identifier = parser->processed_identifier;
                parser->state = STATE_identifier;
                function_call_params(parser, &(*current_node)->left->right);
                if (error)
                    return;
                if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                    return;
                if (parser->current_token.id != TOKEN_SEMICOLON) {
                    error = ERR_SYNTAX;
                    return;
                }
            }
            else {
                error = ERR_SYNTAX;
                return;
            }
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            if (error) return;
            break;
        case TOKEN_BRACKET_CURLY_RIGHT:
            free(*current_node);
            *current_node = NULL;
            break;
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_open_else:
            parser->state = STATE_command;
            body(parser, current_node);
            if (error) return;
            break;
    default:
        error = ERR_SYNTAX;
        return;
    }
    return;
}

/**
 * @brief This function checks that a header of an if/while statement is written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void if_while_header(Tparser* parser, TNode** current_node, node_type type) {
    if (error) return;
    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch (parser->state) {
    case STATE_lr_bracket:
        if (parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT) { //checking for if ->(<-expression) |null_replacement| {

            enter_sub_body(parser);
            if (error) return;
        
            (*current_node) = create_node(type);

            (*current_node)->data.nodeData.body.current_scope = (scope_t*) malloc(sizeof(scope_t));

            if ((*current_node)->data.nodeData.body.current_scope == NULL) {
                error = ERR_COMPILER_INTERNAL;
                return;
            }

            *(*current_node)->data.nodeData.body.current_scope = parser->scope;
            parser->state = STATE_operand;

            /* Expression */
            expression(parser, TOKEN_BRACKET_ROUND_RIGHT, &(*current_node)->left, false);
            if (error) return;

            parser->state = STATE_pipe;
            if_while_header(parser, current_node, type);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_pipe:
        switch (parser->current_token.id) {
        case TOKEN_PIPE: //checking for if (true_expresssion) ->|<-null_replacement| {
        
            parser->state = STATE_identifier;
            null_replacement(parser, current_node);
            
            parser->state = STATE_open_body_check;
            if_while_header(parser, current_node, type);
            break;
            
        case TOKEN_BRACKET_CURLY_LEFT: //checking for if (expression) ->{<-
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            break;
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_open_body_check:
        if (parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT) { //checking for if (true_expression) |null_replacement| ->{<-
            parser->state = STATE_command;
            body(parser, &(*current_node)->right);
            break;
        }
        error = ERR_SYNTAX;
        break;
    default:
        error = ERR_SYNTAX;
        return;
    }

    return;
}

/**
 * @brief This function checks that the null replacement part in the head of a while or if is written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void null_replacement(Tparser* parser, TNode** current_node) {
    if (error) return;
    
    TData nonull_data;

    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch (parser->state) {
    case STATE_identifier:
        if (parser->current_token.id == TOKEN_IDENTIFIER) { //checking for if/while (expression) |->null_replacement<-| {
        
            nonull_data = declaration_data(false,true,UNKNOWN_T);
            if(!symtable_insert(parser->scope.current_scope, parser->current_token.lexeme.array, nonull_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            (*current_node)->data.nodeData.body.is_nullable = true;
            (*current_node)->data.nodeData.body.null_replacement = parser->current_token.lexeme.array;
            
            parser->state = STATE_pipe;
            null_replacement(parser, current_node);
            break;
            
        }
        error = ERR_SYNTAX;
        break;
    case STATE_pipe:
        if (parser->current_token.id == TOKEN_PIPE) { //checking for if/while (expression) |null_replacement->|<- {
            break;
        }
        error = ERR_SYNTAX;
        break;
    default:
        error = ERR_SYNTAX;
        return;
    }
    return;
}

/**
 * @brief This function checks that variable and constant declaration is written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void var_const_declaration(Tparser* parser, TNode** current_node, node_type type) {
    TData retrieved_data;
    
    TNode* value_pointer;
    
    TSymtable* identifier_residence;
    
    if (error) return;

    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    TData symtable_data;
    bool constant;
    if(type == VAR_DECL){
        constant = false;
    }else{
        constant = true;
    }
    
    switch (parser->state) {
    case STATE_identifier:
        if (parser->current_token.id == TOKEN_IDENTIFIER) { //checking for var/const ->name<- = expression;
            
            if (declaration_var_check(parser->scope, parser->current_token.lexeme.array) != NULL) {
                error = ERR_IDENTIFIER_REDEF_CONST_ASSIGN;
                return;
            }
        
            (*current_node) = create_node(type);
            (*current_node)->data.nodeData.identifier.identifier = parser->current_token.lexeme.array;
            
            parser->processed_identifier = parser->current_token.lexeme.array;
            
            parser->state = STATE_assig;
            var_const_declaration(parser, current_node, type);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_assig:
        if (parser->current_token.id == TOKEN_ASSIGNMENT) { //checking for var/const name ->=<- expression;
        
            symtable_data = declaration_data(false, constant, UNKNOWN_T);
            
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, symtable_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
                
            parser->state = STATE_operand;

            /* Expression */
            expression(parser, TOKEN_SEMICOLON, &(*current_node)->left, false);
            //something to match the data type of the final expression result
            if (error) {
                return;
            }
            if ( (*current_node)->left->left == NULL && (*current_node)->left->right == NULL ) {
                printf("Entered here");
                if ( (*current_node)->type == CONST_DECL ) {
                    
                    if ( (*current_node)->left->type == VAR_CONST ) {
                        if((identifier_residence = declaration_var_check(parser->scope, (*current_node)->left->data.nodeData.value.identifier)) == NULL){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                        
                        if(!symtable_get_data(identifier_residence, (*current_node)->left->data.nodeData.value.identifier, &retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                        
                        if ( !retrieved_data.variable.is_constant ) {
                            break;
                        }
                        
                        value_pointer = retrieved_data.variable.value_pointer;
                        
                        if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                        
                        retrieved_data.variable.value_pointer = value_pointer;
                        
                        if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                    }else{
                        if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                        
                        retrieved_data.variable.value_pointer = (*current_node)->left;
                        
                        if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                    }
                }
            }
            break;
        } else if (parser->current_token.id == TOKEN_COLON) { //checking for var/const name ->:<- type = expression;
            parser->state = STATE_possible_qmark;
            var_const_declaration(parser, current_node, type);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_possible_qmark:
            
        switch (parser->current_token.id) {
        case TOKEN_OPTIONAL_TYPE_NULL:  //checking for pub fn name(param : ->?<-type) type{
        
            parser->state = STATE_type;
            var_const_declaration(parser, current_node, type);
            break;
            
        case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type
            
            symtable_data = declaration_data(false, constant, INTEGER_T);
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, symtable_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            
            parser->state = STATE_assig_must;
            var_const_declaration(parser, current_node, type);
            break;
            
        case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
        
            symtable_data = declaration_data(false, constant, FLOAT_T);
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, symtable_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_assig_must;
            var_const_declaration(parser, current_node, type);
            break;
            
        case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
        
            symtable_data = declaration_data(false, constant, U8_SLICE_T);
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, symtable_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_ls_bracket;
            var_const_declaration(parser, current_node, type);
            break;
            
        default:
            error = ERR_SYNTAX;
            break;
        }
        break;
    case STATE_type:
    
        switch (parser->current_token.id) { //checking for pub fn name(param : ?type) type{
        case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
        
            symtable_data = declaration_data(true, constant, INTEGER_T);
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, symtable_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_assig_must;
            var_const_declaration(parser, current_node, type);
            break;
            
        case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
        
            symtable_data = declaration_data(true, constant, FLOAT_T);
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, symtable_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_assig_must;
            var_const_declaration(parser, current_node, type);
            break;
            
        case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
        
            symtable_data = declaration_data(true, constant, U8_SLICE_T);
            if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, symtable_data)){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
        
            parser->state = STATE_ls_bracket;
            var_const_declaration(parser, current_node, type);
            break;
            
        default:
            error = ERR_SYNTAX;
            break;
        }
        break;
    case STATE_assig_must:
        if (parser->current_token.id == TOKEN_ASSIGNMENT) { //checking for var/const name ->=<- expression;
            parser->state = STATE_operand;

            /* Expression */
            expression(parser, TOKEN_SEMICOLON, &(*current_node)->left, false);

            if (error) {
                return;
            }
          
            if ( (*current_node)->left->left == NULL && (*current_node)->left->right == NULL ) {
            
                if ( (*current_node)->type == CONST_DECL ) {
                    
                    if ( (*current_node)->left->type == VAR_CONST ) {
                        if((identifier_residence = declaration_var_check(parser->scope, (*current_node)->left->data.nodeData.value.identifier)) == NULL){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                        
                        if(!symtable_get_data(identifier_residence, (*current_node)->left->data.nodeData.value.identifier, &retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                        
                        if ( !retrieved_data.variable.is_constant ) {
                            break;
                        }
                        
                        value_pointer = retrieved_data.variable.value_pointer;
                        
                        if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                        
                        retrieved_data.variable.value_pointer = value_pointer;
                        
                        if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                    }else{
                        if(!symtable_get_data(parser->scope.current_scope, parser->processed_identifier, &retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                        
                        retrieved_data.variable.value_pointer = (*current_node)->left;
                        
                        if(!symtable_insert(parser->scope.current_scope, parser->processed_identifier, retrieved_data)){
                            error = ERR_COMPILER_INTERNAL;
                            return;
                        }
                    }
                }
            }
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_ls_bracket:
        if (parser->current_token.id == TOKEN_BRACKET_SQUARE_RIGHT) { // var/const name :[->]<-u8 =
            parser->state = STATE_u8;
            var_const_declaration(parser, current_node, type);
            break;
        }
        error = ERR_SYNTAX;
        break;
    case STATE_u8:
        if (parser->current_token.id == TOKEN_KW_U8) { //var/const name :[]->u8<- =
            parser->state = STATE_assig_must;
            var_const_declaration(parser, current_node, type);
            break;
        }
        error = ERR_SYNTAX;
        return;
    default:
        error = ERR_SYNTAX;
        return;
    }
    return;
}

/**
 * @brief This function checks that a calling of a function is written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void function_call(Tparser* parser, TNode** current_node) {
    if (error) return;

    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch (parser->state) {
    case STATE_identifier:
        if (parser->current_token.id == TOKEN_IDENTIFIER) { //..ifj.->something<-(param,param,...)..
        
            (*current_node)->data.nodeData.identifier.identifier = func_id_concat(parser->processed_identifier, parser->current_token.lexeme.array);
        
            parser->state = STATE_lr_bracket;
            function_call(parser, current_node);
            break;
            
        }
        error = ERR_SYNTAX;
        break;
    case STATE_lr_bracket:  // F(......
        if (parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT) { //..something->(<-param,param,...)..
            parser->state = STATE_identifier;
            function_call_params(parser, &(*current_node)->right);
            break;
        }
        error = ERR_SYNTAX;
        break;
    default:
        error = ERR_SYNTAX;
        break;
        return;
    }
}

/**
 * @brief This function checks that the parameters of called function are written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void function_call_params(Tparser* parser, TNode** current_node) {
    if (error) return;

    if ((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch (parser->state) {
    case STATE_identifier: //something(->param<-,param,...)..
        switch (parser->current_token.id) {
        case TOKEN_BRACKET_ROUND_RIGHT:
            return;
        case TOKEN_IDENTIFIER:
            *current_node = create_node(VAR_CONST);
            (*current_node)->data.nodeData.value.identifier = parser->current_token.lexeme.array;
            parser->state = STATE_coma;
            function_call_params(parser, &(*current_node)->right);
            break;
        case TOKEN_LITERAL_I32:
            *current_node = create_node(INT);
            (*current_node)->data.nodeData.value.literal = parser->current_token.lexeme.array;
            parser->state = STATE_coma;
            function_call_params(parser, &(*current_node)->right);
            break;
        case TOKEN_LITERAL_F64:
            *current_node = create_node(FL);
            (*current_node)->data.nodeData.value.literal = parser->current_token.lexeme.array;
            parser->state = STATE_coma;
            function_call_params(parser, &(*current_node)->right);
            break;
        case TOKEN_LITERAL_STRING:
            *current_node = create_node(STR);
            (*current_node)->data.nodeData.value.literal = parser->current_token.lexeme.array;
            parser->state = STATE_coma;
            function_call_params(parser, &(*current_node)->right);
            break;
        case TOKEN_KW_NULL:
            *current_node = create_node(NULL_LITERAL);
            parser->state = STATE_coma;
            function_call_params(parser, &(*current_node)->right);
            break;
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    case STATE_coma: //something(param->,<-param,...)..
        switch (parser->current_token.id) {
        case TOKEN_COMMA:
            parser->state = STATE_identifier;
            function_call_params(parser, current_node);
            break;
        case TOKEN_BRACKET_ROUND_RIGHT:
            return;
        default:
            error = ERR_SYNTAX;
            return;
        }
        break;
    default:
        error = ERR_SYNTAX;
        return;
    }

    return;
}

/**
 * @brief This function checks that all expressions are written correctly
 *
 * @Author xpazurm00, Marek Pazúr
 * 
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 * @param end, holds the symbol that ends the expression (semicolon or right bracket)
 * @param tree, pointer to tree branch pointer (Where will be new node placed to)
 * @param allow_empty, allows empty expression
 */

// Custom macro for getting new token from the input
#define GET_TOKEN() do { \
    if (((parser)->current_token = get_token()).id == TOKEN_ERROR) { \
        return; \
    } \
} while (0)

void expression(Tparser* parser, token_id end, TNode **current_node, bool allow_empty) {
    if (error)
        return;
    // Potential issues:
    // var x = expr; <-- expr not epsilon (empty expression)
    // var x = fun(param_list); OR var x = 'IFJ''.'FUN'();
    // while-if-else (expr) <-- expr not epsilon (empty expression)
    // return (expr); <-- expr can be epsilon (empty expression)

    t_buf t_buffer; // Token Queue buffer
    init_t_buf(&t_buffer);

    GET_TOKEN(); // Read first token
    enqueue_t_buf(&t_buffer, parser->current_token); // Enqueue it into the buffer

    /* Empty expression */
    if (allow_empty && parser->current_token.id == TOKEN_SEMICOLON) { // return ';' <---
        return;
    } 
    /* Distinction between Expression and Function Call */
    if (parser->current_token.id == TOKEN_IDENTIFIER) { // First token is 'identifier' <---
        char *id = parser->current_token.lexeme.array; // Store identifier/namespace of the function

        GET_TOKEN();    // Read second token
        enqueue_t_buf(&t_buffer, parser->current_token); // Enqueue it into the buffer incase its expression

        if (parser->current_token.id  == TOKEN_BRACKET_ROUND_LEFT) { // Second token is lbr '(' <---
            (*current_node) = create_node(FUNCTION_CALL); // Create function call node
            (*current_node)->data.nodeData.identifier.identifier = id; // Assign function ID to the node property

            parser->state = STATE_identifier; // ???
            function_call_params(parser, &(*current_node)->right); // Parameter S.A., TODO decide on left or right pointer?
            
            GET_TOKEN(); // Read third and last token, to ensure the statement is followed by corresponding end marker, no need to enqueue

            if(parser->current_token.id != TOKEN_SEMICOLON) {// Statement must be followed by a Semicolon at the end
                error = ERR_SYNTAX;
                return;
            }

        } else if (parser->current_token.id  == TOKEN_ACCESS_OPERATOR) { // Second token is namespace '.' (access operator) ---> 'IFJ''.''ID'

            GET_TOKEN(); // Read third token, must be ID, no need to enqueue
            char *id_2 = parser->current_token.lexeme.array; // Store second part of the function identifier

            if (parser->current_token.id != TOKEN_IDENTIFIER) { // ifj.'id' namespace must be followed by identifier
                error = ERR_SYNTAX;
                return;
            }

            GET_TOKEN(); // Read fourth token, must be left bracket

            if (parser->current_token.id != TOKEN_BRACKET_ROUND_LEFT) { // ifj.id'(' namespace must be followed by left bracket
                error = ERR_SYNTAX;
                return;
            }

            (*current_node) = create_node(FUNCTION_CALL); // Create function call node

            char *full_id = func_id_concat(id, id_2); // Unite namespace and function id into 'namespace.id'

            if (full_id == NULL) {
                error = ERR_SYNTAX;
                return;
            }

            (*current_node)->data.nodeData.identifier.identifier = full_id; // Assign function ID to the node property

            parser->state = STATE_identifier; // ???
            function_call_params(parser, &(*current_node)->right); // Parameter S.A., TODO decide on left or right pointer?
            
            GET_TOKEN(); // Read third and last token, to ensure the statement is followed by corresponding end marker, no need to enqueue

            if(parser->current_token.id != TOKEN_SEMICOLON) {// Statement must be followed by a Semicolon at the end
                error = ERR_SYNTAX;
                return;
            }

        } else { // First token was ID, but Second wasn't left bracket or '.', so its an expression, not a function --> pass it to P.A.
            (*current_node) = precedent(&t_buffer, end, parser->scope); // buffered tokens passed so they dont get lost
        }

    } else { // First token is NOT ID --> expression, at this point, Empty expression is Invalid
        (*current_node) = precedent(&t_buffer, end, parser->scope); // call precedence analysis for expression syntax analysis
    }
}


/**
 * @brief This function adds the built-in functions into the symtable it is given
 *
 * @Author xtomasp00, Patrik Tomasko
 * 
 * @param global_symtable, pointer to the symtable which is to get filled by the built-in function
 *
 * @return true if the symtable was filled succesfully, false if there was some error like seqfault
 */
void populate_builtin_functions(TSymtable* global_symtable){

    char* function_names[] = {"ifj.readstr", "ifj.readi32", "ifj.readf64", "ifj.write", "ifj.i2f", "ifj.f2i", "ifj.string", "ifj.length", "ifj.concat", "ifj.substring", "pub fn ifj.strcmp", "ifj.ord", "ifj.chr"};
    char function_input[] = {'n','i','f','n','u','u', 'u','u', 'i', 'i', 'u', 'u', 'u', 'i', 'i'};
    int param_ammounts[] = {0,0,0,1,1,1,1,1,2,3,2,2,1};
    Type return_types[] = {U8_SLICE_T, INTEGER_T, FLOAT_T, VOID_T, FLOAT_T, INTEGER_T, U8_SLICE_T, INTEGER_T, U8_SLICE_T, U8_SLICE_T, INTEGER_T, INTEGER_T, U8_SLICE_T};
    bool is_nullable[] = {true, true, true, false, false, false, false, false, false, true, false, false, false};
    
    int param_type_index = 0;
    
    for(int index = 0; index < 13; index++){
    
        TData function_data;
        function_data.function.is_null_type = is_nullable[index];
        d_array_init((&function_data.function.argument_types), 2);
        for(int index_adder = 0; index_adder < param_ammounts[index]; index_adder++){
            if((param_type_index++)>14){
                error = ERR_COMPILER_INTERNAL;
                return;
            }
            d_array_append(&function_data.function.argument_types, function_input[param_type_index]);
        }
        function_data.function.return_type = return_types[index];
        function_data.function.function_scope = NULL;
        
        if(!symtable_insert(global_symtable, function_names[index], function_data)){
            error = ERR_COMPILER_INTERNAL;
            return;
        }
        
    }
    
    return;
}
