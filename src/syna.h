/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xglosro00, Robert Glos
 * @author xukropj00, Jan Ukropec
 * @author xtomasp00, Patrik Tomasko
 * 
 * @file syna.h
 */
 
#include "token.h"
#include "symtable.h"

 typedef struct parser{
  token_t current_token;
  TSymtable* global_symtable;
  TSymtable* local_symtable;
 } Tparser;
 
 void init_parser(token_t token);
 
 void root_code(Tparser* parser);
