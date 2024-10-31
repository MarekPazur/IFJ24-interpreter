/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file codegen.c
 */

#include <stdio.h>
#include <stdbool.h>

#include "compiler_error.h"
#include "codegen.h"

// IFJ24code frame types
#define GF "GF@"
#define LF "LF@"
#define TF "TF@"

const char* get_frame(Frame frame){
    switch(frame){
        case GLOBAL:
            return GF;
        case LOCAL:
            return LF;
        case TEMPORARY:
            return TF;
    }
    return "";
}

void cg_init(void){
    printf(".IFJcode24\n");
}

void cg_create_var(char* var_name){
    if(var_name == NULL){
        print_error(ERR_COMPILER_INTERNAL);
        return;
    }
    printf("defvar %s%s\n", get_frame(LOCAL), var_name);
}

void cg_create_frame(void){
    printf("createframe\n");
}

void cg_push_frame(void){
    printf("pushframe\n");
}

void cg_pop_frame(void){
    printf("popframe\n");
}

void cg_call(char* label){
    if(label == NULL){
        print_error(ERR_COMPILER_INTERNAL);
        return;
    }
    printf("call %s\n", label);
}

void cg_return(void){
    printf("return\n");
}

// IFJ BUILT-IN

void cg_ifj_readstr(char* var_name){
    if(var_name == NULL){
        print_error(ERR_COMPILER_INTERNAL);
        return;
    }
    printf("read %s%s string\n", get_frame(LOCAL), var_name);
}

void cg_ifj_readi32(char* var_name){
    if(var_name == NULL){
        print_error(ERR_COMPILER_INTERNAL);
        return;
    }
    printf("read %s%s int\n", get_frame(LOCAL), var_name);
}

void cg_ifj_readf64(char* var_name){
    if(var_name == NULL){
        print_error(ERR_COMPILER_INTERNAL);
        return;
    }
    printf("read %s%s float\n", get_frame(LOCAL), var_name);
}

void cg_ifj_write(TTerm term){
    char* bool_str;
    switch(term.type){
        case VARIABLE_T:
            printf("write %s%s\n", get_frame(LOCAL), term.value.var_name);
            break;
        case INTEGER_T:
            printf("write int@%d\n", term.value.int_val);
            break;
        case FLOAT_T:
            printf("write float@%a\n", term.value.float_val);
            break;
        case BOOLEAN_T:
            bool_str = term.value.bool_val ? "true" : "false";
            printf("write bool@%s\n", bool_str);
            break;
        case STRING_T:
            printf("write string@");
            int i = 0;
            while(term.value.string[i] != '\0'){
                char c = term.value.string[i];
                if(c < 32 || c == '#' || c == '\\'){ // 000-032: Unprintable characters, 035: '#', 092: '\'
                    printf("\\%03d", c);
                }
                else{
                    putchar(c);
                }
                i++;
            }
            putchar('\n');
            break;
        case NULL_T:
            printf("write string@null\n");
            break;
        default:
            print_error(ERR_COMPILER_INTERNAL);
            break;
    }
}

