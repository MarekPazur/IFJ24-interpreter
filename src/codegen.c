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

// Return value global frame variable
const TTerm cg_var_retval = {.type = VARIABLE_T, .value.var_name = "input", .frame = GLOBAL};

// IFJ24code frame types
#define GF "GF@"
#define LF "LF@"
#define TF "TF@"

/**
 * Function for getting strings with memory frames
 * \param frame Constant value of a memory frame
 * \return String with a corresponding frame and @ eg. "'frame type'F@"
 */
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

void cg_term(TTerm term){
    char* bool_str;
    switch(term.type){
        case VARIABLE_T:
            printf("%s%s", get_frame(term.frame), term.value.var_name);
            break;
        case INTEGER_T:
            printf("int@%d", term.value.int_val);
            break;
        case FLOAT_T:
            printf("float@%a", term.value.float_val);
            break;
        case BOOLEAN_T:
            bool_str = term.value.bool_val ? "true" : "false";
            printf("bool@%s", bool_str);
            break;
        case STRING_T:
            printf("string@");
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
            break;
        case NULL_T:
            printf("nil@nil");
            break;
        default:
            error = ERR_COMPILER_INTERNAL;
            break;
    }
}

void cg_init(void){
    printf(".IFJcode24\n");
    cg_create_var(cg_var_retval);
}

void cg_create_var(TTerm var){
    if(var.type != VARIABLE_T || var.value.var_name == NULL){
        //set_error(ERR_COMPILER_INTERNAL);
        return;
    }
    printf("defvar %s%s\n", get_frame(var.frame), var.value.var_name);
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
        //set_error(ERR_COMPILER_INTERNAL);
        return;
    }
    printf("call %s\n", label);
}

void cg_return(void){
    printf("return\n");
}

void cg_move(TTerm var, TTerm symb){
    if(var.type != VARIABLE_T){
        //set_error(ERR_COMPILER_INTERNAL);
        return;
    }
    printf("move ");
    cg_term(var);
    putchar(' ');
    cg_term(symb);
    putchar('\n');
}

// IFJ BUILT-IN

void cg_ifj_readstr(void){
    printf("read ");
    cg_term(cg_var_retval);
    printf(" string\n");
}

void cg_ifj_readi32(void){
    printf("read %s%s int\n", get_frame(cg_var_retval.frame), cg_var_retval.value.var_name);
}

void cg_ifj_readf64(void){
    printf("read %s%s float\n", get_frame(cg_var_retval.frame), cg_var_retval.value.var_name);
}

void cg_ifj_write(TTerm term){
    printf("write ");
    cg_term(term);
    putchar('\n');
}

void cg_ifj_i2f(TTerm term){
    if(term.type != INTEGER_T){
        //set error
        return;
    }
    printf("int2float ");
    cg_term(cg_var_retval);
    putchar(' ');
    cg_term(term);
    putchar('\n');
}

void cg_ifj_f2i(TTerm term){
    if(term.type != FLOAT_T){
        //set error
        return;
    }
    printf("float2int ");
    cg_term(cg_var_retval);
    putchar(' ');
    cg_term(term);
    putchar('\n');
}

