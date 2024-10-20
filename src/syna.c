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
              parser->state = STATE_ROOT;
              root_code(parser);
              break;
          case TOKEN_KW_CONST:
              parser->state = STATE_identifier;
              import_func(parser);
              parser->state = STATE_ROOT;
              root_code(parser);
              break;
          case TOKEN_EOF:
              return;
          default:
              error=ERR_SYNTAX;
      }
      return;
   }else{
       error = ERR_SYNTAX;
       return;
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
                parser->state = STATE_type;
                function_header(parser);
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
    return;
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
                parser->state = STATE_possible_qmark;
                function_params(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_possible_qmark:
            switch(parser->current_token.id){ //checking for pub fn name(param : ?type) type{
                case TOKEN_OPTIONAL_TYPE_NULL:
                    parser->state = STATE_type;
                    function_params(parser);
                    break;
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
                    break;
                }
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
        return;

}

/**
 * @brief This function checks that the bodies of functions, whiles, ifs and elses are written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void body(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_command:
            switch(parser->current_token.id){
                case TOKEN_KW_IF:
                    parser->state = STATE_lr_bracket;
                    if_while_header(parser);
                    parser->state = STATE_possible_else;
                    body(parser);
                    break;
                case TOKEN_KW_RETURN:
                    parser->state = STATE_operand;
                    expression(parser, TOKEN_SEMICOLON);
                    if(error){
                    	return;
                    }
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_KW_WHILE:
                    parser->state = STATE_lr_bracket;
                    if_while_header(parser);
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_KW_VAR:
                    parser->state = STATE_identifier;
                    var_const_declaration(parser);
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_KW_CONST:
                    parser->state = STATE_identifier;
                    var_const_declaration(parser);
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_IDENTIFIER:
                    parser->current_token = get_token();
                    if(parser->current_token.id == TOKEN_ASSIGNMENT){
                    	parser->state = STATE_operand;
                    	expression(parser, TOKEN_SEMICOLON);
                    	if(error){
                    		return;
                    	}
                    }
                    else if(parser->current_token.id == TOKEN_ACCESS_OPERATOR){
                    	parser->state = STATE_identifier;
                    	function_call(parser);
                    	parser->current_token = get_token();
                    	if(parser->current_token.id != TOKEN_SEMICOLON){
                    		error=ERR_SYNTAX;
                    		return;
                    	}
                    }
                    else{
                    	error=ERR_SYNTAX;
                    	return;
                    }
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_BRACKET_CURLY_RIGHT:
                    break;
                default:
                    error = ERR_SYNTAX;
                    return;
            }
            break;
        case STATE_possible_else:
            switch(parser->current_token.id){ 
                case TOKEN_KW_ELSE://checking for ..} ->else<- { ..
                    parser->state = STATE_open_else;
                    body(parser);
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_KW_IF:
                    parser->state = STATE_lr_bracket;
                    if_while_header(parser);
                    parser->state = STATE_possible_else;
                    body(parser);
                    break;
                case TOKEN_KW_WHILE:
                    parser->state = STATE_lr_bracket;
                    if_while_header(parser);
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_KW_RETURN:
                    parser->state = STATE_operand;
                    expression(parser, TOKEN_SEMICOLON);
                    if(error){
                    	return;
                    }
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_KW_VAR:
                    parser->state = STATE_identifier;
                    var_const_declaration(parser);
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_KW_CONST:
                    parser->state = STATE_identifier;
                    var_const_declaration(parser);
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_IDENTIFIER:
                    parser->current_token = get_token();
                    if(parser->current_token.id == TOKEN_ASSIGNMENT){
                    	parser->state = STATE_operand;
                    	expression(parser, TOKEN_SEMICOLON);
                    	if(error){
                    		return;
                    	}
                    }
                    else if(parser->current_token.id == TOKEN_ACCESS_OPERATOR){
                    	parser->state = STATE_identifier;
                    	function_call(parser);
                    	parser->current_token = get_token();
                    	if(parser->current_token.id != TOKEN_SEMICOLON){
                    		error=ERR_SYNTAX;
                    		return;
                    	}
                    }
                    else{
                    	error=ERR_SYNTAX;
                    	return;
                    }
                    parser->state = STATE_command;
                    body(parser);
                    break;
                case TOKEN_BRACKET_CURLY_RIGHT:
                    break;
                default:
                    error = ERR_SYNTAX;
                    return;
            }
            break;
        case STATE_open_else:
            if(parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT){ //checking for ..} else ->{<- ..
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
    return;
}

/**
 * @brief This function checks that a header of an if/while statement is written correctly
 *
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 */
void if_while_header(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_lr_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){ //checking for if ->(<-expression) |null_replacement| {
                parser->state = STATE_operand;
                expression(parser, TOKEN_BRACKET_ROUND_RIGHT);
                parser->state = STATE_pipe;
                if_while_header(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_pipe:
            switch(parser->current_token.id){
                case TOKEN_PIPE: //checking for if (true_expresssion) ->|<-null_replacement| {
                    parser->state = STATE_identifier;
                    null_replacement(parser);
                    parser->state = STATE_open_body_check;
                    if_while_header(parser);
                    break;
                case TOKEN_BRACKET_CURLY_LEFT: //checking for if (expression) ->{<-
                    parser->state = STATE_command;
                    body(parser);
                    break;
                default:
                    error = ERR_SYNTAX;
                    return;
            }
            break;
        case STATE_open_body_check:
            if(parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT){ //checking for if (true_expression) |null_replacement| ->{<-
                parser->state=STATE_command;
                body(parser);
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
void null_replacement(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_identifier:
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //checking for if/while (expression) |->null_replacement<-| {
                parser->state = STATE_pipe;
                null_replacement(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_pipe:
            if(parser->current_token.id == TOKEN_PIPE){ //checking for if/while (expression) |null_replacement->|<- {
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
void var_const_declaration(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_identifier:
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //checking for var/const ->name<- = expression;
                parser->state = STATE_assig;
                var_const_declaration(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_assig:
            if(parser->current_token.id == TOKEN_ASSIGNMENT){ //checking for var/const name ->=<- expression;
                parser->state = STATE_operand;
                expression(parser, TOKEN_SEMICOLON);
                if(error){
                	return;
                }
                break;
            }else if (parser->current_token.id == TOKEN_COLON){ //checking for var/const name ->:<- type = expression;
                parser->state = STATE_possible_qmark;
                var_const_declaration(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_possible_qmark:
            switch(parser->current_token.id){ //checking for pub fn name(param : ?type) type{
                case TOKEN_OPTIONAL_TYPE_NULL:
                    parser->state = STATE_type;
                    var_const_declaration(parser);
                    break;
                case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
                case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
                    parser->state = STATE_assig_must;
                    var_const_declaration(parser);
                    break;
                case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
                    parser->state = STATE_ls_bracket;
                    var_const_declaration(parser);
                    break;
                default:
                    error = ERR_SYNTAX;
                    break;
            }
            break;
        case STATE_type:
            switch(parser->current_token.id){ //checking for pub fn name(param : ?type) type{
                case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
                case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
                    parser->state = STATE_assig_must;
                    var_const_declaration(parser);
                    break;
                case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
                    parser->state = STATE_ls_bracket;
                    var_const_declaration(parser);
                    break;
                default:
                    error = ERR_SYNTAX;
                    break;
            }
            break;
        case STATE_assig_must:
            if(parser->current_token.id == TOKEN_ASSIGNMENT){ //checking for var/const name ->=<- expression;
                parser->state = STATE_operand;
                expression(parser, TOKEN_SEMICOLON);
                if(error){
                	return;
                }
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_ls_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_SQUARE_RIGHT){ // var/const name :[->]<-u8 =
                parser->state = STATE_u8;
                var_const_declaration(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_u8:
            if(parser->current_token.id == TOKEN_KW_U8){ //var/const name :[]->u8<- =
                parser->state = STATE_assig_must;
                var_const_declaration(parser);
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
void function_call(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_identifier:
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //..ifj.->something<-(param,param,...)..
                parser->state = STATE_lr_bracket;
                function_call(parser);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_lr_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){ //..something->(<-param,param,...)..
                parser->state = STATE_identifier;
                function_call_params(parser);
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
void function_call_params(Tparser* parser){
    parser->current_token = get_token();
    switch(parser->state){
        case STATE_identifier: //something(->param<-,param,...)..
            switch(parser->current_token.id){
                case TOKEN_BRACKET_ROUND_RIGHT:
                    return;
                case TOKEN_IDENTIFIER:
                    parser->state=STATE_possible_function;
                    function_call_params(parser);
                    break;
                case TOKEN_LITERAL_I32:
                case TOKEN_LITERAL_F64:
                case TOKEN_LITERAL_STRING:
                    parser->state = STATE_coma;
                    function_call_params(parser);
                    break;
                default:
                    error = ERR_SYNTAX;
                    return;
            }
            break;
        case STATE_coma: //something(param->,<-param,...)..
            switch(parser->current_token.id){
                case TOKEN_COMMA:
                    parser->state = STATE_identifier;
                    function_call_params(parser);
                    break;
                case TOKEN_BRACKET_ROUND_RIGHT:
                    return;
                default:
                    error = ERR_SYNTAX;
                    return;
            }
            break;
        case STATE_possible_function: //something(param->.<-something(...),param,...).. || //something(param->(<-...),param,...)..
            switch(parser->current_token.id){
                case TOKEN_BRACKET_ROUND_RIGHT:
                    return;
                case TOKEN_COMMA:
                    parser->state = STATE_identifier;
                    function_call_params(parser);
                    break;
                case TOKEN_ACCESS_OPERATOR:
                    parser->state = STATE_identifier;
                    function_call(parser);
                    break;
                case TOKEN_BRACKET_ROUND_LEFT:
                    parser->state = STATE_identifier;
                    function_call_params(parser);
                    break;
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
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 * @param end, holds the symbol that ends the expression (semicolon or right bracket)
 */
void expression(Tparser* parser, token_id end){
    parser->current_token = get_token();
    switch(parser->state){
    	case STATE_operand: // const a = ->[var/int/..]<-
    		switch(parser->current_token.id){
    			case TOKEN_BRACKET_ROUND_LEFT:
    				expression(parser, TOKEN_BRACKET_ROUND_RIGHT);
    				parser->state = STATE_operator;
    				expression(parser, end);
    				break;
    			case TOKEN_IDENTIFIER:
    				parser->current_token = get_token();
    				if(parser->current_token.id == TOKEN_ACCESS_OPERATOR){
    					parser->state = STATE_identifier;
    					function_call(parser);
    					parser->state = STATE_operator;
    					expression(parser, end);
    					break;
    				}
    				else if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){
    					parser->state = STATE_identifier;
    					function_call_params(parser);
    					parser->state = STATE_operator;
    					expression(parser, end);
    					break;
    				}
    				else{
    					goto operator;
    				}
    				break;
    			case TOKEN_LITERAL_F64: case TOKEN_LITERAL_I32:
    				parser->state = STATE_operator;
    				expression(parser, end);
    				break;
    			case TOKEN_LITERAL_STRING: case TOKEN_KW_NULL:
    				parser->current_token = get_token();
    				if(parser->current_token.id == TOKEN_SEMICOLON){
    					return;
    				}
    				error = ERR_SYNTAX;
    				return;
    			default:
    				error=ERR_SYNTAX;
    				return;
    		}
    		break;
    		
    	case STATE_operator:
    	operator:
    		if((parser->current_token.id <=TOKEN_DIVISION && parser->current_token.id >=TOKEN_ADDITION) || (parser->current_token.id <=TOKEN_LESS_EQUAL && parser->current_token.id >=TOKEN_EQUAL)){ //const a = [var/int/..] ->[operators]<-
    			parser->state = STATE_operand;
    			expression(parser, end);
    			break;
    		}
    		else if(parser->current_token.id == end){ //const a = [var/int/..] [operator]... ->[;) (end of expression)]<-
    			return;
    		}
    		else{
    			error=ERR_SYNTAX;
    			return;
    		}
    	
    	default:
    		error=ERR_SYNTAX;
    		return;
    }
}
