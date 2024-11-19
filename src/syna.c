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
#include "compiler_error.h"
#include "lexer.h"


 TData current_data;
 TKey current_key;
 TData local_data;
 TKey local_key;
 
void wipe_data(TData* data){
	data->is_null=false;
	data->is_constant=false;
	data->value.int_val = 0;
	data->value.float_val = 0.0;
	data->value.string_val = NULL;
	data->value.bool_val = NULL;
	d_array_free(&data->value.argument_types);
	data->type = UNDEFINED;
}

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
      return;
   }

   //initialising the AST
   parser->AST = BT_init();
   BT_insert_root(parser->AST, PROGRAM);
   parser->AST->root->data.nodeData.program.globalSymTable = parser->global_symtable;
   //malloc and initialisation for the global and local symtables and checking if it went correctly
   parser->global_symtable = symtable_init();
   parser->local_symtable = symtable_init();

   if(parser->global_symtable == NULL || parser->global_symtable == NULL){
      error = ERR_COMPILER_INTERNAL;
      return;
   }

   parser->state = STATE_ROOT; // delete later

   parser->current_token = token;

   TNode** root = &(parser->AST->root);

   root_code(parser, root);
   
   /*printf("Left of root %d", parser->AST->root->left->type);
   printf("Function name starts with %c", *parser->AST->root->left->data.nodeData.function.identifier);
   printf("Function type is %d", parser->AST->root->left->data.nodeData.function.type);
   printf("Right of root %d", parser->AST->root->right->type);*/
   //printf("\n global");
   //debug_print_keys(parser->global_symtable);
}

/**
 * @brief This function looks for function headers and the import thingy
 *
 * @param parser, holds the current token, symtables and the binary tree
 */
void root_code(Tparser* parser, TNode** current_node){

   if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
    return;
   
   if(parser->state == STATE_ROOT){
      switch(parser->current_token.id){
          //<root_code> -> import_func | function_header | ""
          case TOKEN_KW_PUB:
              d_array_init(&current_data.value.argument_types, 1); //init new argument type dynamic array for each new function
              parser->state = STATE_fn;
              current_data.type = FUNCTION;
              function_header(parser, &(*current_node)->left);
              
              //Symtable handling of local symtable
              //printf("\nlocal");
              //debug_print_keys(parser->local_symtable);
              symtable_free(parser->local_symtable);
              parser->local_symtable = symtable_init();
              
              //insertion of function into global symtable
              symtable_insert(parser->global_symtable, current_key, current_data);
              //d_array_print(&current_data.value.argument_types); debug print for function types
              
              //wipe current global data
              wipe_data(&current_data);
              parser->state = STATE_ROOT;

              if(error)
                return;

              root_code(parser, &(*current_node)->left);

              if(error)
                return;

              break;
          case TOKEN_KW_CONST:
              parser->state = STATE_identifier;
              current_data.is_constant = true;
              import_func(parser, &(parser->AST->root->right));
	      symtable_insert(parser->global_symtable,current_key,current_data);
              if(error)
                return;
	      wipe_data(&current_data);
              parser->state = STATE_ROOT;
              root_code(parser, current_node);

              if(error)
                return;

              break;

          case TOKEN_EOF:
              BT_print_tree(parser->AST->root);
              return; 

          default:
              error = ERR_SYNTAX;
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
void import_func(Tparser* parser, TNode** current_node){
   if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
    return;

   switch(parser->state){
      case STATE_identifier: //checking for this part const ->ifj<- = @import("ifj24.zig");
          if(parser->current_token.id == TOKEN_IDENTIFIER){
              (*current_node) = create_node(PROLOGUE);
              current_data.type = IMPORT;
              current_key = parser->current_token.lexeme.array;
              parser->state = STATE_assig;
              import_func(parser, current_node);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_identifier_prolog: //checking for this part const ifj = @import(->"ifj24.zig"<-);
          if(parser->current_token.id == TOKEN_LITERAL_STRING){
              current_data.value.string_val = parser->current_token.lexeme.array;
              parser->state = STATE_rr_bracket;
              import_func(parser, current_node);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_assig: //checking for this part const ifj ->=<- @import("ifj24.zig");
          if(parser->current_token.id == TOKEN_ASSIGNMENT){
              parser->state = STATE_prolog;
              import_func(parser, current_node);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_prolog: //checking for this part const ifj = ->@import<-("ifj24.zig");
          if(parser->current_token.id == TOKEN_PROLOG){
              parser->state = STATE_lr_bracket;
              import_func(parser, current_node);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_lr_bracket: //checking for this part const ifj = @import->(<-"ifj24.zig");
          if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){
              parser->state = STATE_identifier_prolog;
              import_func(parser, current_node);
              break;
          }
          error = ERR_SYNTAX;
          break;
      case STATE_rr_bracket: //checking for this part const ifj = @import("ifj24.zig"->)<-;
          if(parser->current_token.id == TOKEN_BRACKET_ROUND_RIGHT){
              parser->state = STATE_semicolon;
              import_func(parser, current_node);
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
void function_header(Tparser* parser, TNode** current_node){
    int has_qmark = 0;
    if (error) return;

    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch(parser->state){
        case STATE_fn: 
            if(parser->current_token.id == TOKEN_KW_FN){ //checking for pub ->fn<- name() type{
                (*current_node) = create_node(FN);
                (*current_node)->data.nodeData.function.scope = parser->local_symtable;
                parser->state = STATE_identifier;
                function_header(parser, current_node);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_identifier:
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //checking for pub fn ->name<-() type{
                (*current_node)->data.nodeData.function.identifier = parser->current_token.lexeme.array;
            	current_key = parser->current_token.lexeme.array;
                parser->state = STATE_lr_bracket;
                function_header(parser, current_node);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_lr_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){ //checking for pub fn name->(<-) type{
                parser->state = STATE_first_fn_param;
                function_params(parser);
                parser->state = STATE_type;
                function_header(parser, current_node);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_type:
            if(parser->current_token.id == TOKEN_OPTIONAL_TYPE_NULL){
                if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                    return;
                has_qmark = 1;
            }
            switch(parser->current_token.id){
                case TOKEN_KW_I32://checking for pub fn name() ->i32<-{
                    (*current_node)->data.nodeData.function.type = I32;
                    parser->state = STATE_open_body_check;
                    function_header(parser, current_node);
                    break;
                case TOKEN_KW_F64://checking for pub fn name() ->f64<-{
                    (*current_node)->data.nodeData.function.type = F64;
                    parser->state = STATE_open_body_check;
                    function_header(parser, current_node);
                    break;
                case TOKEN_KW_VOID://checking for pub fn name() ->void<-{
                    if (has_qmark != 1){
                        (*current_node)->data.nodeData.function.type = VOID_TYPE;
                        parser->state = STATE_open_body_check;
                        function_header(parser, current_node);
                        break;
                    }else{
                        error = ERR_SYNTAX;
                        return;
                    }
                case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name() ->[<-]u8{
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
            if(parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT){ //checking for pub fn name() type->{<-
                parser->state = STATE_command;
                body(parser, &(*current_node)->right);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_ls_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_SQUARE_RIGHT){ //checking for pub fn name() [->]<-u8{
                parser->state = STATE_u8;
                function_header(parser, current_node);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_u8:
            if(parser->current_token.id == TOKEN_KW_U8){ //checking for pub fn name() []->u8<-{
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
void function_params(Tparser* parser){
    if (error) return;


   if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
    return;
	switch(parser->state){
	case STATE_first_fn_param:
            switch(parser->current_token.id){
                case TOKEN_BRACKET_ROUND_RIGHT: //checking for pub fn name(->)<- type{
                    parser->state = STATE_type;
                    break;
                case TOKEN_IDENTIFIER: //checking for pub fn name(->param<- : type) type{
                    local_key = parser->current_token.lexeme.array;
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
                    local_data.is_null=true;
                    parser->state = STATE_type;
                    function_params(parser);
                    break;
                case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
                    d_array_append(&current_data.value.argument_types, 'i');
                    local_data.type = INTEGER;
                    parser->state = STATE_coma;
                    function_params(parser);
                    break;
                case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
                    d_array_append(&current_data.value.argument_types, 'f');
                    local_data.type=FLOAT;
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
                    d_array_append(&current_data.value.argument_types, 'i');
                    local_data.type = INTEGER;
                    parser->state = STATE_coma;
                    function_params(parser);
                    break;
                case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
                    d_array_append(&current_data.value.argument_types, 'f');
                    local_data.type=FLOAT;
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
                    local_key = parser->current_token.lexeme.array;
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
                    symtable_insert(parser->local_symtable, local_key, local_data);
                    wipe_data(&local_data);
                    parser->state = STATE_identifier;
                    function_params(parser);
                    break;
                case TOKEN_BRACKET_ROUND_RIGHT: //checking for pub fn name(param : type ->)<- type{7
                    symtable_insert(parser->local_symtable, local_key, local_data);
                    wipe_data(&local_data);
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
            	d_array_append(&current_data.value.argument_types, 's');
                local_data.type = STRING;
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
void body(Tparser* parser, TNode** current_node){
    char* temp_identifier;
    if (error) return;

    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch(parser->state){
        case STATE_command:
            (*current_node) = create_node(COMMAND);
            switch(parser->current_token.id){
                case TOKEN_KW_IF:
                    parser->state = STATE_lr_bracket;
                    if_while_header(parser, &(*current_node)->left, IF);
                    if (error) return;
                    parser->state = STATE_possible_else;
                    body(parser, &(*current_node)->right);
                    break;
                case TOKEN_DISCARD_RESULT:
                    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;
                    if(parser->current_token.id != TOKEN_ASSIGNMENT){
                        error = ERR_SYNTAX;
                        return;
                    }
                    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;
                    if(parser->current_token.id != TOKEN_IDENTIFIER){
                        error = ERR_SYNTAX;
                        return;
                    }
                    temp_identifier = parser->current_token.lexeme.array;
                    (*current_node)->left = create_node(FUNCTION_CALL);
                    (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;
                    if(parser->current_token.id == TOKEN_ACCESS_OPERATOR){
                        (*current_node)->left = create_node(FUNCTION_CALL);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                    	parser->state = STATE_identifier;
                    	function_call(parser, &(*current_node)->left);
                        if(error)
                            return;
                    	if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                            return;
                    	if(parser->current_token.id != TOKEN_SEMICOLON){
                    		error=ERR_SYNTAX;
                    		return;
                    	}
                    }
                    else if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){
                        (*current_node)->left = create_node(FUNCTION_CALL);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                        parser->state = STATE_identifier;
                        function_call_params(parser, &(*current_node)->left->right);
                        if(error)
                            return;
                    	if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                            return;
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
                    body(parser, &(*current_node)->right);
                    if (error) return;
                    break;
                case TOKEN_KW_RETURN:
                    (*current_node)->left = create_node(RETURN);
                    //TODO give the expression function the current_node->left->left to make the expression there
                    parser->state = STATE_operand;
                    expression(parser, TOKEN_SEMICOLON);
                    if(error){
                    	return;
                    }
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->right);
                    break;
                case TOKEN_BRACKET_CURLY_LEFT:
                    //TODO add the local symtable for the headerless body
                    (*current_node)->left = create_node(BODY);
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->left->right);
                    if (error) return;
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->right);
                    break;
                case TOKEN_KW_WHILE:
                    parser->state = STATE_lr_bracket;
                    if_while_header(parser, &(*current_node)->left, WHILE);
                    if (error) return;
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->right);
                    break;
                case TOKEN_KW_VAR:
                    parser->state = STATE_identifier;
                    var_const_declaration(parser, &(*current_node)->left, VAR_DECL);
                    symtable_insert(parser->local_symtable, local_key, local_data);
                    wipe_data(&local_data);
                    if (error) return;
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->right);
                    if(error)
                        return;
                    break;
                case TOKEN_KW_CONST:
                    local_data.is_constant=true;
                    parser->state = STATE_identifier;
                    var_const_declaration(parser, &(*current_node)->left, CONST_DECL);
                    symtable_insert(parser->local_symtable, local_key, local_data);
                    wipe_data(&local_data);
                    if (error) return;
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->right);
                    if(error)
                        return;
                    break;
                case TOKEN_IDENTIFIER:
                    //TODO add the expression
                    temp_identifier = parser->current_token.lexeme.array;
                    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;
                    if(parser->current_token.id == TOKEN_ASSIGNMENT){
                        (*current_node)->left = create_node(ASSIG);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                    	parser->state = STATE_operand;

                    	expression(parser, TOKEN_SEMICOLON);
                    	//set data type for local_databased on expression result
                    	if(error){
                    		return;
                    	}
                    }

                    else if(parser->current_token.id == TOKEN_ACCESS_OPERATOR){
                        (*current_node)->left = create_node(FUNCTION_CALL);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                    	parser->state = STATE_identifier;
                    	function_call(parser, &(*current_node)->left);
                        if(error)
                            return;
                    	if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                            return;
                    	if(parser->current_token.id != TOKEN_SEMICOLON){
                    		error=ERR_SYNTAX;
                    		return;
                    	}
                    }
                    else if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){
                        (*current_node)->left = create_node(FUNCTION_CALL);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                        parser->state = STATE_identifier;
                        function_call_params(parser, &(*current_node)->left->right);
                        if(error)
                            return;
                    	if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                            return;
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
            switch(parser->current_token.id){ 
                case TOKEN_BRACKET_CURLY_LEFT:
                    //TODO add the local symtable for the headerless body
                    (*current_node)->left = create_node(BODY);
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->left->right);
                    if (error) return;
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->right);
                    if (error) return;
                    break;
                case TOKEN_DISCARD_RESULT:
                    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;
                    if(parser->current_token.id != TOKEN_ASSIGNMENT){
                        error = ERR_SYNTAX;
                        return;
                    }
                    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;
                    if(parser->current_token.id != TOKEN_IDENTIFIER){
                        error = ERR_SYNTAX;
                        return;
                    }
                    temp_identifier = parser->current_token.lexeme.array;
                    (*current_node)->left = create_node(FUNCTION_CALL);
                    (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;
                    if(parser->current_token.id == TOKEN_ACCESS_OPERATOR){
                        (*current_node)->left = create_node(FUNCTION_CALL);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                    	parser->state = STATE_identifier;
                    	function_call(parser, &(*current_node)->left);
                        if(error)
                            return;
                    	if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                            return;
                    	if(parser->current_token.id != TOKEN_SEMICOLON){
                    		error=ERR_SYNTAX;
                    		return;
                    	}
                    }
                    else if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){
                        (*current_node)->left = create_node(FUNCTION_CALL);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                        parser->state = STATE_identifier;
                        function_call_params(parser, &(*current_node)->left->right);
                        if(error)
                            return;
                    	if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                            return;
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
                    body(parser, &(*current_node)->right);
                    if (error) return;
                    break;
                case TOKEN_KW_ELSE://checking for ..} ->else<- { ..
                    parser->state = STATE_open_else;
                    (*current_node)->left = create_node(ELSE);
                    body(parser, &(*current_node)->left->right);
                    if (error) return;
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->right);
                    if (error) return;
                    break;
                case TOKEN_KW_IF:
                    parser->state = STATE_lr_bracket;
                    if_while_header(parser, &(*current_node)->left, IF);
                    if (error) return;
                    parser->state = STATE_possible_else;
                    body(parser, &(*current_node)->right);
                    if (error) return;
                    break;
                case TOKEN_KW_WHILE:
                    parser->state = STATE_lr_bracket;
                    if_while_header(parser, &(*current_node)->left, WHILE);
                    if (error) return;
                    parser->state = STATE_command;
                    body(parser, &(*current_node)->right);
                    if (error) return;
                    break;
                case TOKEN_KW_RETURN:
                    (*current_node)->left = create_node(RETURN);
                    //TODO give the expression function the current_node->left->left to make the expression there
                    parser->state = STATE_operand;
                    expression(parser, TOKEN_SEMICOLON);
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
                    local_data.is_constant = true;
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
                    temp_identifier = parser->current_token.lexeme.array;
                    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;

                    if(parser->current_token.id == TOKEN_ASSIGNMENT){
                        (*current_node)->left = create_node(ASSIG);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                    	parser->state = STATE_operand;
                    	expression(parser, TOKEN_SEMICOLON);
                        if (error) return;
                    }
                    else if(parser->current_token.id == TOKEN_ACCESS_OPERATOR){
                        (*current_node)->left = create_node(FUNCTION_CALL);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                    	parser->state = STATE_identifier;
                    	function_call(parser, &(*current_node)->left);
                        if (error) return;

                    	if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                            return;
                    	if(parser->current_token.id != TOKEN_SEMICOLON){
                    		error=ERR_SYNTAX;
                    		return;
                    	}
                    }
                    else if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){
                        (*current_node)->left = create_node(FUNCTION_CALL);
                        (*current_node)->left->data.nodeData.identifier.identifier = temp_identifier;
                        parser->state = STATE_identifier;
                        function_call_params(parser, &(*current_node)->left->right);
                        if(error)
                            return;
                    	if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                            return;
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
            if(parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT){ //checking for ..} else ->{<- ..
                (*current_node) = create_node(ELSE);
                //TODO add the local symtable which Robert didn't do
                parser->state = STATE_command;
                body(parser, current_node);
                if (error) return;
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
 //TODO local symtable for the if and whiles, and then add it into the ABT structures
void if_while_header(Tparser* parser, TNode** current_node, node_type type){
    if (error) return;
    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch(parser->state){
        case STATE_lr_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){ //checking for if ->(<-expression) |null_replacement| {
                (*current_node) = create_node(type);
                parser->state = STATE_operand;
                expression(parser, TOKEN_BRACKET_ROUND_RIGHT);
                parser->state = STATE_pipe;
                if_while_header(parser, current_node, type);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_pipe:
            switch(parser->current_token.id){
                case TOKEN_PIPE: //checking for if (true_expresssion) ->|<-null_replacement| {
                    parser->state = STATE_identifier;
                    null_replacement(parser, &(*current_node)->left->left);
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
            if(parser->current_token.id == TOKEN_BRACKET_CURLY_LEFT){ //checking for if (true_expression) |null_replacement| ->{<-
                parser->state=STATE_command;
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
void null_replacement(Tparser* parser, TNode** current_node){
    if (error) return;

    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
    return;

    switch(parser->state){
        case STATE_identifier:
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //checking for if/while (expression) |->null_replacement<-| {
                (*current_node) = create_node(NULL_REPLACEMENT);
                (*current_node)->data.nodeData.identifier.identifier = parser->current_token.lexeme.array;
                parser->state = STATE_pipe;
                null_replacement(parser, current_node);
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
void var_const_declaration(Tparser* parser, TNode** current_node, node_type type){
    if (error) return;

    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;

    switch(parser->state){
        case STATE_identifier:
            local_key=parser->current_token.lexeme.array;
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //checking for var/const ->name<- = expression;
                (*current_node) = create_node(type);
                (*current_node)->data.nodeData.identifier.identifier = local_key;
                parser->state = STATE_assig;
                var_const_declaration(parser, current_node, type);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_assig:
            if(parser->current_token.id == TOKEN_ASSIGNMENT){ //checking for var/const name ->=<- expression;
                parser->state = STATE_operand;
                expression(parser, TOKEN_SEMICOLON);
                //something to match the data type of the final expression result
                if(error){
                	return;
                }
                break;
            }else if (parser->current_token.id == TOKEN_COLON){ //checking for var/const name ->:<- type = expression;
                parser->state = STATE_possible_qmark;
                var_const_declaration(parser, current_node, type);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_possible_qmark:
            switch(parser->current_token.id){ //checking for pub fn name(param : ?type) type{
                case TOKEN_OPTIONAL_TYPE_NULL:
                    local_data.is_null = true;
                    parser->state = STATE_type;
                    var_const_declaration(parser, current_node, type);
                    break;
                case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
                case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
                    parser->state = STATE_assig_must;
                    var_const_declaration(parser, current_node, type);
                    break;
                case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
                    parser->state = STATE_ls_bracket;
                    var_const_declaration(parser, current_node, type);
                    break;
                default:
                    error = ERR_SYNTAX;
                    break;
            }
            break;
        case STATE_type:
            switch(parser->current_token.id){ //checking for pub fn name(param : ?type) type{
                case TOKEN_KW_I32: //checking for pub fn name(param : ->i32<-) type{
                    local_data.type = INTEGER;
                    parser->state = STATE_assig_must;
                    function_params(parser);
                    break;
                case TOKEN_KW_F64: //checking for pub fn name(param : ->f64<-) type{
                    local_data.type = FLOAT;
                    parser->state = STATE_assig_must;
                    var_const_declaration(parser, current_node, type);
                    break;
                case TOKEN_BRACKET_SQUARE_LEFT: //checking for pub fn name(param : ->[<-]u8) type{
                    parser->state = STATE_ls_bracket;
                    var_const_declaration(parser, current_node, type);
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
                var_const_declaration(parser, current_node, type);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_u8:
            if(parser->current_token.id == TOKEN_KW_U8){ //var/const name :[]->u8<- =
            	local_data.type = STRING;
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
void function_call(Tparser* parser, TNode** current_node){
    if (error) return;

    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;
    
    switch(parser->state){
        case STATE_identifier:
            if(parser->current_token.id == TOKEN_IDENTIFIER){ //..ifj.->something<-(param,param,...)..
                parser->state = STATE_lr_bracket;
                function_call(parser, current_node);
                break;
            }
            error = ERR_SYNTAX;
            break;
        case STATE_lr_bracket:
            if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){ //..something->(<-param,param,...)..
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
void function_call_params(Tparser* parser, TNode** current_node){
    if (error) return;

    if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
        return;
    
    switch(parser->state){
        case STATE_identifier: //something(->param<-,param,...)..
            switch(parser->current_token.id){
                case TOKEN_BRACKET_ROUND_RIGHT:
                    return;
                case TOKEN_IDENTIFIER:
                    *current_node = create_node(VAR_CONST);
                    (*current_node)->data.nodeData.value.identifier = parser->current_token.lexeme.array;
                    parser->state=STATE_coma;
                    function_call_params(parser, &(*current_node)->right);
                    break;
                case TOKEN_LITERAL_I32:
                    *current_node = create_node(INT);
                    (*current_node)->data.nodeData.value.integer_value = parser->current_token.value.i32;
                    parser->state = STATE_coma;
                    function_call_params(parser, &(*current_node)->right);
                    break;
                case TOKEN_LITERAL_F64:
                    *current_node = create_node(FL);
                    (*current_node)->data.nodeData.value.float_value = parser->current_token.value.f64;
                    parser->state = STATE_coma;
                    function_call_params(parser, &(*current_node)->right);
                    break;
                case TOKEN_LITERAL_STRING:
                    *current_node = create_node(STR);
                    (*current_node)->data.nodeData.value.literal = parser->current_token.lexeme.array;
                    parser->state = STATE_coma;
                    function_call_params(parser, &(*current_node)->right);
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
 * @param parser, holds the current token, symtables, binary tree and the current state of the FSM
 * @param end, holds the symbol that ends the expression (semicolon or right bracket)
 */
void expression(Tparser* parser, token_id end){
    if (error) return;


    if((parser->current_token = get_token()).id == TOKEN_ERROR) { // Token is invalid
        return;}

    switch(parser->state){
    	case STATE_operand: // const a = ->[var/int/..]<-
    		switch(parser->current_token.id){
    			case TOKEN_BRACKET_ROUND_LEFT:
    				expression(parser, TOKEN_BRACKET_ROUND_RIGHT);
    				parser->state = STATE_operator;
    				expression(parser, end);
    				break;
    			case TOKEN_IDENTIFIER:
    				if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                    return;
    				if(parser->current_token.id == TOKEN_ACCESS_OPERATOR){
    					parser->state = STATE_identifier;
    					function_call(parser, NULL);
    					parser->state = STATE_operator;
    					expression(parser, end);
    					break;
    				}
    				else if(parser->current_token.id == TOKEN_BRACKET_ROUND_LEFT){
    					parser->state = STATE_identifier;
    					function_call_params(parser, NULL);
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
    				if((parser->current_token = get_token()).id == TOKEN_ERROR) // Token is invalid
                        return;
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
