/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * @author xtomasp00, Patrik Tomaško
 * 
 * @file lexer.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "compiler_error.h"

#define NOF_KEY_WORDS 13
#define BACKSLASH_SIGNS 5

scanner_t scanner = {0};

const char *keywords[] = {"const", "else", "fn", "if", "i32", "f64", "null", "pub", "return", "u8", "var", "void", "while"};

char special_chars[] = {'"', 'n', 'r', 't', '\\'};
char special_chars_backslash[] = {'\"', '\n', '\r', '\t', '\\'};

void init_scanner(void) { 
    scanner.p_state = STATE_START;
    scanner.row = 1;
    scanner.col = 0;
    scanner.head_pos = 0;
}

bool valid_hex(char a){
    return (a >= '0' && a <= '9') || (a >= 'A' && a <= 'F') || (a >= 'a' && a <= 'f');
}

token_t get_token(void) {
    token_t token; /* New token is created every function call */
    token.id = TOKEN_DEFAULT; /* Default placeholder */
    d_array_init(&token.lexeme, 16); /* Each token has its own array of chars (string), necessary for keyword/identifier tokens */

    scanner.p_state = STATE_START; 

    int c = 0;

    while(true) {

        c = getc(stdin);

        if(c == '\n') { /* row&column calculation */
            ++scanner.row;
            scanner.col = 0;
        }
        ++scanner.col;
        ++scanner.head_pos;

        switch(scanner.p_state) {
            case STATE_START:
                /* Simple states */
                if (isspace(c)) {
                    scanner.p_state = STATE_START;
                }  else if (c == EOF) {
                    token.id = TOKEN_EOF;
                    return token;
                } else if (c == '(') {
                    token.id = TOKEN_BRACKET_ROUND_LEFT;
                    return token;
                } else if (c == ')') {
                    token.id = TOKEN_BRACKET_ROUND_RIGHT;
                    return token;
                } else if (c == '{') {
                    token.id = TOKEN_BRACKET_CURLY_LEFT;
                    return token;
                } else if (c == '}') {
                    token.id = TOKEN_BRACKET_CURLY_RIGHT;
                    return token;
                } else if (c == '[') {
                    token.id = TOKEN_BRACKET_SQUARE_LEFT;
                    return token;
                } else if (c == ']') {
                    token.id = TOKEN_BRACKET_SQUARE_RIGHT;
                    return token;
                } else if (c == '+') {
                    token.id = TOKEN_ADDITION;
                    return token;
                } else if (c == '-') {
                    token.id = TOKEN_SUBSTRACTION;
                    return token;
                } else if (c == '*') {
                    token.id = TOKEN_MULTIPLICATION;
                    return token;
                } else if (c == '.') {
                    token.id = TOKEN_ACCESS_OPERATOR;
                    return token;
                } else if (c == '?') {
                    token.id = TOKEN_OPTIONAL_TYPE_NULL;
                    return token;
                } else if (c == ':') {
                    token.id = TOKEN_COLON;
                    return token;
                } else if (c == '|') {
                    token.id = TOKEN_PIPE;
                    return token;
                } else if (c == ',') {
                    token.id = TOKEN_COMMA;
                    return token;
                } else if (c == ';') {
                    token.id = TOKEN_SEMICOLON;
                    return token;
                }

                    /* Complex states */
                else if (c == '/') { // Division op. or comment
                    scanner.p_state = STATE_COMMENT_DIV;
                } else if (c == '=') { // = or ==
                    scanner.p_state = STATE_EQUAL_ASSIGN;
                } else if (c == '!') { // Can be only !=
                    scanner.p_state = STATE_NOT_EQUAL;
                } else if (c == '<') { // < or <=
                    scanner.p_state = STATE_LESS_LEQ;
                } else if (c == '>') { // > or >=
                    scanner.p_state = STATE_GREATER_GREQ;
                } else if (c == '0') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_DIGIT_ZERO;
                }

                    /* Literals */
                else if (isdigit(c)) {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_DIGIT;
                } else if (c == '"') {
                    token.id = TOKEN_LITERAL_STRING;
                    scanner.p_state = STATE_STRING_START;
                } else if (c == '\\') {
                    c = getc(stdin);
                    if (c == '\\') {
                        token.id = TOKEN_LITERAL_STRING;
                        scanner.p_state = STATE_MULTILINE_STRING_START;
                    } else {
                        token.id = TOKEN_ERROR;
                        error = ERR_LEXICAL;
                        return token;
                    }
                }

                    /* Identifiers or keywords */
                else if (c == '@') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_PROLOG;
                } else if (c == '_') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_UNDERSCORE;
                } else if (is_identifier(c)) {
                    d_array_append(&token.lexeme, c);
                    token.id = TOKEN_IDENTIFIER;
                    scanner.p_state = STATE_KW_IDENT;
                } else {
                    token.id = TOKEN_ERROR;
                    error = ERR_LEXICAL;
                    return token;
                }

                break;

                /* Comment */
            case STATE_COMMENT_DIV:
                // Single line comment
                if (c == '/') {
                    ignore_comment();
                    scanner.p_state = STATE_START;
                } else { // division operator
                    token.id = TOKEN_DIVISION;

                    //scanner.p_state = STATE_START;

                    ungetc(c, stdin);    // Necessary to put loaded char back to stream

                    return token;
                }
                break;

                /* Operators */
            case STATE_EQUAL_ASSIGN:
                if (c == '=') {
                    token.id = TOKEN_EQUAL;
                    //scanner.p_state = STATE_START;
                    return token;
                } else {
                    token.id = TOKEN_ASSIGNMENT;
                    //scanner.p_state = STATE_START;
                    ungetc(c, stdin);
                    return token;
                }
                break;

            case STATE_NOT_EQUAL:

                if (c == '=') {
                    token.id = TOKEN_NOT_EQUAL;
                    //scanner.p_state = STATE_START;
                    return token;
                } else { // Standalone '!' is not any lexeme
                    token.id = TOKEN_ERROR;
                    //scanner.p_state = STATE_START;

                    error = ERR_LEXICAL;
                    ungetc(c, stdin);
                    return token;
                }

                break;

            case STATE_LESS_LEQ:
                if (c == '=') {
                    token.id = TOKEN_LESS_EQUAL;
                    //scanner.p_state = STATE_START;
                    return token;
                } else {
                    token.id = TOKEN_LESS;
                    //scanner.p_state = STATE_START;
                    ungetc(c, stdin);
                    return token;
                }

                break;

            case STATE_GREATER_GREQ:
                if (c == '=') {
                    token.id = TOKEN_GREATER_EQUAL;
                    //scanner.p_state = STATE_START;
                    return token;
                } else {
                    token.id = TOKEN_GREATER;
                    //scanner.p_state = STATE_START;
                    ungetc(c, stdin);
                    return token;
                }
                break;

                /* Special */
            case STATE_UNDERSCORE:
                if (isalnum(c) || c == '_') {
                    d_array_append(&token.lexeme, c);
                    token.id = TOKEN_IDENTIFIER;
                    scanner.p_state = STATE_KW_IDENT;
                } else {
                    token.id = TOKEN_DISCARD_RESULT;
                    ungetc(c, stdin);
                    return token;
                }
                break;

                /* Identifier or keyword */
            case STATE_KW_IDENT:
                if (is_identifier(c) || isdigit(c)) {
                    d_array_append(&token.lexeme, c);
                } else {
                    //scanner.p_state = STATE_START;
                    ungetc(c, stdin);
                    d_array_append(&token.lexeme, '\0');

                    for (int i = 0; i < NOF_KEY_WORDS; i++) {
                        if (!strcmp(token.lexeme.array, keywords[i])) {
                            token.id = i + 3; // 3 is the offset of the tokens enum to the keywords
                        }
                    }

                    d_array_remove(&token.lexeme, token.lexeme.length - 1);
                    return token;
                }
                break;

                /* Digits - integer or double */
            case STATE_DIGIT_ZERO:
                if (isdigit(c)) {
                    token.id = TOKEN_ERROR;
                    error = ERR_LEXICAL;
                    fprintf(stderr, RED_BOLD("error")": A non-zero literal must not start with a 0\n");
                    ungetc(c, stdin);
                    return token;
                } else if (c == '.') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_FLOATING_POINT;
                } else if (c == 'e') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_EXPONENT;
                } else {
                    token.id = TOKEN_LITERAL_I32;
                    ungetc(c, stdin);
                    return token;
                }
                break;

            case STATE_DIGIT:
                if (isdigit(c)) {
                    d_array_append(&token.lexeme, c);
                } else if (c == '.') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_FLOATING_POINT;
                } else if (c == 'e' || c == 'E') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_EXPONENT;
                }
                else {
                    token.id = TOKEN_LITERAL_I32;
                    ungetc(c, stdin);
                    return token;
                }
                break;

            case STATE_FLOATING_POINT:
                if (isdigit(c)) {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_DECIMAL_PART;
                } else if (c == 'e' || c == 'E') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_EXPONENT;
                } else {
                    token.id = TOKEN_ERROR;
                    error = ERR_LEXICAL;
                    fprintf(stderr, RED_BOLD("error")": invalid decimal base\n");
                    ungetc(c, stdin);
                    return token;
                }
                break;

            case STATE_DECIMAL_PART:
                if (isdigit(c)) {
                    d_array_append(&token.lexeme, c);
                } else if (c == 'e' || c == 'E') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_EXPONENT;
                } else {
                    token.id = TOKEN_LITERAL_F64;
                    ungetc(c, stdin);
                    return token;
                }
                break;

            case STATE_EXPONENT:
                if (isdigit(c)) {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_EXPONENT_FINAL;
                } else if (c == '+' || c == '-') {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_EXPONENT_POSITIVE_NEGATIVE;
                } else {
                    token.id = TOKEN_ERROR;
                    error = ERR_LEXICAL;
                    fprintf(stderr, RED_BOLD("error")": invalid decimal base\n");
                    ungetc(c, stdin);
                    return token;
                }
                break;

            case STATE_EXPONENT_POSITIVE_NEGATIVE:
                if (isdigit(c)) {
                    d_array_append(&token.lexeme, c);
                    scanner.p_state = STATE_EXPONENT_FINAL;
                } else {
                    token.id = TOKEN_ERROR;
                    error = ERR_LEXICAL;
                    fprintf(stderr, RED_BOLD("error")": invalid decimal base\n");
                    ungetc(c, stdin);
                    return token;
                }
                break;

            case STATE_EXPONENT_FINAL:
                if (isdigit(c)) {
                    d_array_append(&token.lexeme, c);
                } else {
                    token.id = TOKEN_LITERAL_F64;
                    ungetc(c, stdin);
                    return token;
                }
                break;

                /* Prolog - builtin functions*/
            case STATE_PROLOG:
                if (isalpha(c)) {
                    d_array_append(&token.lexeme, c);
                } else {
                    ungetc(c, stdin);

                    d_array_append(&token.lexeme, '\0');

                    if (!strcmp(token.lexeme.array, "@import")) {
                        token.id = TOKEN_PROLOG;
                    } else {
                        token.id = TOKEN_ERROR;
                        error = ERR_LEXICAL;
                        fprintf(stderr, RED_BOLD("error")": invalid builtin function\n");
                    }

                    d_array_remove(&token.lexeme, token.lexeme.length - 1);

                    return token;
                }
                break;

            case STATE_STRING_START:
                if (c == '"') {
                    return token;
                } else if (c == '\\') {
                    c = getc(stdin);
                    int initial_array_size = token.lexeme.length;
                    for (int i = 0; i < BACKSLASH_SIGNS; i++) {
                        if (special_chars[i] == c) {
                            d_array_append(&token.lexeme, special_chars_backslash[i]);
                            break;
                        } else if ('x' == c) {
                            char hex_chars[3] = {0};
                            for (int hex_number = 0; hex_number < 2; hex_number++) {
                                c = getc(stdin);
                                if (valid_hex(c)) {
                                    hex_chars[hex_number] = c;
                                } else {
                                    token.id = TOKEN_ERROR;
                                    return token;
                                }
                            }
                            unsigned int value;
                            sscanf(hex_chars, "%2x", &value);  // Convert hex string to integer
                            d_array_append(&token.lexeme, (char)value);
                            break;
                        }
                    }
                    if (initial_array_size == token.lexeme.length) {
                        token.id = TOKEN_ERROR;
                        return token;
                    }
                } else if (c >= 32 && c <= 126) {
                    d_array_append(&token.lexeme, c);
                } else {
                    token.id = TOKEN_ERROR;
                    printf("440");
                    error = ERR_LEXICAL;
                    return token;
                }
                break;

            case STATE_MULTILINE_STRING_START:
                if (c == '\\') {
                    d_array_append(&token.lexeme, '\\');
                    c = getc(stdin);
                    int initial_array_size = token.lexeme.length;
                    for (int i = 0; i < BACKSLASH_SIGNS; i++) {
                        if (special_chars[i] == c) {
                            d_array_append(&token.lexeme, c);
                            break;
                        } else if ('x' == c) {
                            d_array_append(&token.lexeme, c);
                            for (int hex_number = 0; hex_number < 2; hex_number++) {
                                c = getc(stdin);
                                if (valid_hex(c)) {
                                    d_array_append(&token.lexeme, c);
                                } else {
                                    token.id = TOKEN_ERROR;
                                    return token;
                                }
                            }
                            break;
                        }
                    }
                    if (initial_array_size == token.lexeme.length) {
                        token.id = TOKEN_ERROR;
                        return token;
                    }
                } else if (c >= 32 && c <= 126) {
                    d_array_append(&token.lexeme, c);
                } else if (c == '\n') {
                    scanner.p_state = STATE_NEXT_MULTILINE;
                }    
                  else if (c == EOF) {
                    scanner.p_state = STATE_NEXT_MULTILINE;
                } else {
                    token.id = TOKEN_ERROR;
                    return token;
                }
                break;

            case STATE_NEXT_MULTILINE:
                while(isspace(c)){
                    c = getc(stdin);
                }
                if(c == '\\'){
                    c = getc(stdin);
                    if(c == '\\'){
                        d_array_append(&token.lexeme, '\n');
                        scanner.p_state = STATE_MULTILINE_STRING_START;
                    }
                } else if(!isspace(c) || c == EOF){
                    ungetc(c, stdin);
                    return token;
                } else {
                    token.id = TOKEN_ERROR;
                    return token;
                }
                break;

            default:
                token.id = TOKEN_ERROR;
                return token;
                break;
        }
    }
    return token;
}

void ignore_comment() {
    char c;

    while((c = getc(stdin)) != '\n' && c != EOF);

    if(c == EOF) // Necessary to return EOF, since it is standalone token
        ungetc(c,stdin);
}

/**
 * @brief This function determines wether the character could be a part of an identifier
 *
 * @param c character to be examined
 *
 * @return returns true if the sign could be a part of the identifier, false otherwise
 */
bool is_identifier(char c){
    if(isalpha(c) || c == '_'){
        return true;
    }
    return false;
}

/**
 *  Prints information about processed token
 */
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