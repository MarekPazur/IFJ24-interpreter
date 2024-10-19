/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xglosro00, Robert Glos
 * @author xukropj00, Jan Ukropec
 * @author xtomasp00, Patrik Tomasko
 * 
 * @file syna.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>


#include "syna.h"
#include "compiler_error.h"
#include "lexer.h"

/**
 * @brief This function initializes the parser
 *
 * @param token, will be used to store the current token
 */
void init_parser(token_t token){
   //malloc for the parser and checking if it went correctly
   Tparser* parser = malloc(sizeof(Tparser));
   if(parser == NULL){
      error = ERR_COMPILER_INTERNAL;
   }
   //malloc and initialisation for the global and local symtables and checking if it went correctly
   parser->global_symtable = symtable_init();
   parser->local_symtable = symtable_init();
   if(parser->global_symtable == NULL || parser->global_symtable == NULL){
      error = ERR_COMPILER_INTERNAL;
   }
   if(error){
      return;
   }
   parser->state = STATE_ROOT;
   parser->current_token = token;
   root_code(parser);
}

/**
 * @brief This function looks for function headers and the import thingy
 *
 * @param parser, holds the current token, symtables and the binary tree
 */
void root_code(Tparser* parser){
   parser->current_token = get_token();
   if(parser->state == STATE_ROOT){
      switch(parser->current_token.id){
          //<root_code> -> import_func | function_header | ""
          case TOKEN_KW_PUB:
              parser->state = STATE_fn;
              function_header(parser);
              break;
          case TOKEN_KW_CONST:
              parser->state = STATE_identifier;
              import_func(parser);
              break;
          case TOKEN_EOF:
              return;
          default:
              error=ERR_SYNTAX;
      }
   }else{
       error = ERR_SYNTAX;
   }
}

/**
 * @brief This function checks that the prolog is written syntactically correct
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void import_func(Tparser* parser){
   parser->current_token = get_token();
   switch(parser->state){
      case STATE_identifier: //checking for this part const ->ifj<- = @import("ifj24.zig");
          if(parser->current_token.id == TOKEN_IDENTIFIER){
              parser->state = STATE_assig;
              import_func(parser);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_identifier_prolog: //checking for this part const ifj = @import(->"ifj24.zig"<-);
          if(parser->current_token.id == TOKEN_LITERAL_STRING){
              parser->state = STATE_rr_bracket;
              import_func(parser);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_assig: //checking for this part const ifj ->=<- @import("ifj24.zig");
          if(parser->current_token.id == TOKEN_ASSIGNMENT){
              parser->state = STATE_prolog;
              import_func(parser);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_prolog: //checking for this part const ifj = ->@import<-("ifj24.zig");
          if(parser->current_token.id == TOKEN_PROLOG){
              parser->state = STATE_lr_bracket;
              import_func(parser);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_lr_bracket: //checking for this part const ifj = @import->(<-"ifj24.zig");
          if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){
              parser->state = STATE_identifier_prolog;
              import_func(parser);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_rr_bracket: //checking for this part const ifj = @import("ifj24.zig"->)<-;
          if(parser->current_token.id == TOKEN_BRACKET_ROUND_RIGHT){
              parser->state = STATE_semicolon;
              import_func(parser);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_semicolon: //checking for this part const ifj = @import("ifj24.zig")->;<-
          if(parser->current_token.id == TOKEN_SEMICOLON){
              parser->state = STATE_ROOT;
              root_code(parser);
              return;
          }
          error = ERR_SYNTAX;
          break;
      default:
          error = ERR_SYNTAX;
   }
}

/**
 * @brief This function checks that a header of a function is syntactically correctly written
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void function_header(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_fn: 
            if(parser->current_token.id == TOKEN_KW_FN){ //checking for pub ->fn<- name() type{
                parser->state = STATE_identifier;
                function_header(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_identifier:
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //checking for pub fn ->name<-() type{
                parser->state = STATE_lr_bracket;
                function_header(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_lr_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){ //checking for pub fn name->(<-) type{
                parser->state = STATE_first_fn_param;
                function_params(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_type:
            switch(parser->current_token.id){
                case TOKEN_KW_I32://checking for pub fn name() ->i32<-{
                case TOKEN_KW_F64://checking for pub fn name() ->f64<-{
                case TOKEN_KW_VOID://checking for pub fn name() ->void<-{
                    parser->state = STATE_open_body_check;
                    function_header(parser);
                    break;
                case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name() ->[<-]u8{
                    parser->state = STATE_ls_bracket;
                    function_header(parser);
                    break;
                default:
                error = ERR_SYNTAX;
                return;
            }
            break;
        case STATE_open_body_check:
            if(parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT){ //checking for pub fn name() type->{<-
                parser->state = STATE_command;
                body(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_ls_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_SQUARE_RIGHT){ //checking for pub fn name() [->]<-u8{
                parser->state = STATE_u8;
                function_header(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_u8:
            if(parser->current_token.id == TOKEN_KW_U8){ //checking for pub fn name() []->u8<-{
                parser->state = STATE_open_body_check;
                function_header(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
            
        default:
            error = ERR_SYNTAX;
    }
}

/**
 * @brief This function checks that a the parameters of a function are written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void function_params(Tparser* parser){
	parser->current_token = get_token();
	switch(parser->state){
	case STATE_first_fn_param:
            switch(parser->current_token.id){
                case TOKEN_BRACKET_ROUND_RIGHT: //checking for pub fn name(->)<- type{
                    parser->state = STATE_type;
                    function_header(parser);
                    break;
                case TOKEN_IDENTIFIER: //checking for pub fn name(->param<- : type) type{
                    parser->state = STATE_colon;
                    function_params(parser);
                    break;
                default:
                error = ERR_SYNTAX;
                return;
            }
            break;
        case STATE_colon:
            if(parser->current_token.id == TOKEN_COLON){ //checking for pub fn name(param ->:<- type) type{
                parser->state = STATE_type;
                function_params(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_type:
            switch(parser->current_token.id){
                case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
                case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
                    parser->state = STATE_coma;
                    function_params(parser);
                    break;
                case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
                    parser->state = STATE_ls_bracket;
                    function_params(parser);
                    break;
                default:
                error = ERR_SYNTAX;
                return;
            }
            break;
        case STATE_identifier:
            switch(parser->current_token.id){
                case TOKEN_BRACKET_ROUND_RIGHT: //checking for pub fn name(param : type ,->)<- type{
                    parser->state = STATE_type;
                    function_header(parser);
                    break;
                case TOKEN_IDENTIFIER: //checking for pub fn name(param : type ,->param<- : type) type{
                    parser->state = STATE_colon;
                    function_params(parser);
                    break;
                default:
                error = ERR_SYNTAX;
                return;
            }
            break;
        case STATE_coma:
            switch(parser->current_token.id){
                case TOKEN_COMMA: //checking for pub fn name(param : type ->,<- type) type{
                    parser->state = STATE_identifier;
                    function_params(parser);
                    break;
                case TOKEN_BRACKET_ROUND_RIGHT: //checking for pub fn name(param : type ->)<- type{
                    parser->state = STATE_type;
                    function_header(parser);
                    break;
                default:
                error = ERR_SYNTAX;
                return;
            }
            break;
        case STATE_ls_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_SQUARE_RIGHT){ //checking for pub fn name(param : [->]<-u8) type{
                parser->state = STATE_u8;
                function_params(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_u8:
            if(parser->current_token.id == TOKEN_KW_U8){ //checking for pub fn name(param : []->u8<-) type{
                parser->state = STATE_coma;
                function_params(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
            
        default:
            error = ERR_SYNTAX;
        }

}

void body(Tparser* parser){
    parser->current_token = get_token();
    if(parser->state == STATE_command){
        switch(parser->current_token.id){
            case TOKEN_KW_IF:
                parser->state = STATE_lr_bracket;
                if_while_header(parser);
                break;
            case TOKEN_KW_WHILE:
                parser->state = STATE_lr_bracket;
                if_while_header(parser);
                break;
            default:
                error = ERR_SYNTAX;
                return;
        }
    }else{
        error = ERR_SYNTAX;
        return;
    }
}

void if_while_header(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_lr_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){ //checking for if ->(<-true_statement) |null_replacement| {
                parser->state = STATE_operand;
                true_statement(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_pipe:
            switch(parser->current_token.id){
                case TOKEN_PIPE: //checking for if (true_statement) ->|<-null_replacement| {
                    parser->state = STATE_operand;
                    null_replacement(parser);
                    break;
                case TOKEN_BRACKET_CURLY_LEFT: //checking for if (true_statement) ->{<-
                    parser->state = STATE_command;
                    body(parser);
                    break;
                default:
                    error = ERR_SYNTAX;
                    return;
            }
            break;
        case STATE_open_body_check:
            if(parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT){ //checking for if (true_statement) |null_replacement| ->{<-
                parser->state = STATE_command;
                body(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        default:
            error = ERR_SYNTAX;
            return;
    }
}

void null_replacement(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_identifier:
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //checking for if/while (true_statement) |->null_replacement<-| {
                parser->state = STATE_pipe;
                true_statement(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_pipe:
            if(parser->current_token.id == TOKEN_PIPE){ //checking for if/while (true_statement) |null_replacement->|<- {
                parser->state = STATE_open_body_check;
                if_while_header(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        default:
            error = ERR_SYNTAX;
            return;
    }
}

void true_statement(Tparser* parser){
    parser->current_token = get_token();
    //TODO
}
