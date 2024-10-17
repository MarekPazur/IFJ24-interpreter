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
   switch(parser->current_token.id){
      //<root_code> -> import | function_header | ""
      case TOKEN_EOF:
          return;
      default:
          error=ERR_SYNTAX;
          return;
   }
}
