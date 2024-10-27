/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file token.c
 */
#include <stdio.h>

#include "token.h"
#include "compiler_error.h"

/* prints information about processed token */
void print_token(token_t token) {
    char *token_info[] = {
    "DEFAULT",
    "ERROR",
    /* Variable or function identifier */
    "TOKEN_IDENTIFIER",

    /* Keywords */
    "TOKEN_KW_CONST",
    "TOKEN_KW_ELSE",
    "TOKEN_KW_FN",
    "TOKEN_KW_IF",
    "TOKEN_KW_I32",
    "TOKEN_KW_F64",
    "TOKEN_KW_NULL",
    "TOKEN_KW_PUB",
    "TOKEN_KW_RETURN",
    "TOKEN_KW_U8",
    "TOKEN_KW_VAR",
    "TOKEN_KW_VOID",
    "TOKEN_KW_WHILE",

    /* Brackets  */
    "TOKEN_BRACKET_ROUND_LEFT",
    "TOKEN_BRACKET_ROUND_RIGHT",
    "TOKEN_BRACKET_CURLY_LEFT",
    "TOKEN_BRACKET_CURLY_RIGHT",
    "TOKEN_BRACKET_SQUARE_LEFT",
    "TOKEN_BRACKET_SQUARE_RIGHT",

    /* Binary operators */
    "TOKEN_ADDITION",
    "TOKEN_SUBSTRACTION",
    "TOKEN_MULTIPLICATION",
    "TOKEN_DIVISION",
    
    "TOKEN_EQUAL",
    "TOKEN_NOT_EQUAL",
    "TOKEN_LESS",
    "TOKEN_GREATER",
    "TOKEN_GREATER_EQUAL",
    "TOKEN_LESS_EQUAL",

    "TOKEN_ASSIGNMENT",

    /* Literals (Operands) */
    "TOKEN_LITERAL_I32",
    "TOKEN_LITERAL_F64",
    "TOKEN_LITERAL_STRING",

    /* Other tokens */
    "TOKEN_EOL",
    "TOKEN_EOF",
    "TOKEN_ACCESS_OPERATOR",
    "TOKEN_PROLOG",
    "TOKEN_BACKSLASH",
    "TOKEN_COLON",
    "TOKEN_SEMICOLON",
    "TOKEN_DISCARD_RESULT",
    "TOKEN_NULL",
    "TOKEN_PIPE",
    "TOKEN_OPTIONAL_TYPE_NULL",
    "TOKEN_COMMA"};
    
    printf(WHITE_BOLD("TOKEN: %s"), token_info[token.id]);
    
    if(token.lexeme.length > 0 && token.lexeme.array != NULL){
        printf(WHITE_BOLD(" <LEXEME>: "));
        printf("\033[1;33m'");
        d_array_print(&token.lexeme);
        printf("'\033[0;37m");
    }

    //putchar('\n');
}