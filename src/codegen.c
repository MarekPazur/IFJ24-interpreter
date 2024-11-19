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
const TTerm cg_var_retval = {.type = VARIABLE_T, .value.var_name = "retval", .frame = GLOBAL};
// Global terms
const TTerm cg_null_term = {.type = NULL_T};
const TTerm cg_true_term = {.type = BOOLEAN_T, .value.bool_val = true};
const TTerm cg_zero_int_term = {.type = INTEGER_T, .value.int_val = 0};

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

// CG variables and frames

void cg_init(void){
    printf(".IFJcode24\n");
    cg_create_var(cg_var_retval);
}

void cg_create_var(TTerm var){
    if(var.type != VARIABLE_T || var.value.var_name == NULL){
        error = ERR_COMPILER_INTERNAL;
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

// Function calling

void cg_call(char* label){
    if(label == NULL){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("call %s\n", label);
}

void cg_return(void){
    printf("return\n");
}

void cg_move(TTerm dest, TTerm src){
    if(dest.type != VARIABLE_T){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("move ");
    cg_term(dest);
    putchar(' ');
    cg_term(src);
    putchar('\n');
}

// Labels

ull cg_get_new_label_number(void){
    ull label = 0;
    return label++;
}

void cg_create_label(ull label_number){
    printf("label L%llu\n", label_number);
}

void cg_label(ull label_number){
    printf("L%llu\n", label_number);
}

// Comparisons

void cg_two_operands(TTerm o1, TTerm o2){
    cg_term(o1);
    putchar(' ');
    cg_term(o2);
    putchar('\n');
}

void cg_three_operands(TTerm o1, TTerm o2, TTerm o3){
    cg_term(o1);
    putchar(' ');
    cg_two_operands(o2, o3);
}

void cg_equal(TTerm result_var, TTerm value1, TTerm value2){
    printf("eq ");
    cg_three_operands(result_var, value1, value2);
}

void cg_less_than(TTerm result_var, TTerm value1, TTerm value2){
    printf("lt ");
    cg_three_operands(result_var, value1, value2);
}

void cg_greater_than(TTerm result_var, TTerm value1, TTerm value2){
    printf("gt ");
    cg_three_operands(result_var, value1, value2);
}

// Jumps

void cg_jump(ull label){
    printf("jump ");
    cg_label(label);
    putchar('\n');
}

void cg_jump_eq(ull label, TTerm value1, TTerm value2){
    printf("jumpifeq ");
    cg_label(label);
    cg_two_operands(value1, value2);
}

void cg_jump_not_neq(ull label, TTerm value1, TTerm value2){
    printf("jumpifneq ");
    cg_label(label);
    cg_two_operands(value1, value2);
}

void cg_jump_lt(ull label, TTerm value1, TTerm value2){
    cg_less_than(cg_var_retval, value1, value2);
    cg_jump_eq(label, cg_var_retval, cg_true_term);
}

void cg_jump_gt(ull label, TTerm value1, TTerm value2){
    cg_greater_than(cg_var_retval, value1, value2);
    cg_jump_eq(label, cg_var_retval, cg_true_term);
}

void cg_jump_lteq(ull label, TTerm value1, TTerm value2){
    cg_jump_eq(label, value1, value2);
    cg_jump_lt(label, value1, value2);
}

void cg_jump_gteq(ull label, TTerm value1, TTerm value2){
    cg_jump_eq(label, value1, value2);
    cg_jump_gt(label, value1, value2);
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
    if(term.type != VARIABLE_T && term.type != INTEGER_T){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("int2float ");
    cg_term(cg_var_retval);
    putchar(' ');
    cg_term(term);
    putchar('\n');
}

void cg_ifj_f2i(TTerm term){
    if(term.type != VARIABLE_T && term.type != FLOAT_T){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("float2int ");
    cg_term(cg_var_retval);
    putchar(' ');
    cg_term(term);
    putchar('\n');
}

void cg_ifj_string(TTerm term){
    if(term.type != VARIABLE_T && term.type != STRING_T){
        error = ERR_COMPILER_INTERNAL;
    }
    cg_move(cg_var_retval, term);
}

void cg_ifj_length(TTerm s){
    if(s.type != VARIABLE_T){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("strlen ");
    cg_term(cg_var_retval);
    putchar(' ');
    cg_term(s);
    putchar('\n');
}

void cg_ifj_concat(TTerm s1, TTerm s2){
    if(s1.type != VARIABLE_T || s2.type != VARIABLE_T){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("concat ");
    cg_term(cg_var_retval);
    putchar(' ');
    cg_term(s1);
    putchar(' ');
    cg_term(s2);
    putchar('\n');
}

// TODO:
void cg_ifj_substring(TTerm s, TTerm i, TTerm j){
    cg_create_frame();
    cg_push_frame();

    // Label numbers
    ull ret_null = cg_get_new_label_number();
    ull finish = cg_get_new_label_number();
    // Terms
    TTerm str_len = {.type = VARIABLE_T, .value.var_name = "str_len", .frame = LOCAL};
    cg_create_var(str_len);
    // Code
    // i < 0
    cg_jump_lt(ret_null, i, cg_zero_int_term);
    // j < 0
    cg_jump_lt(ret_null, j, cg_zero_int_term);
    // i > j
    cg_jump_gt(ret_null, i, j);
    // i >= ifj.length(s)
    cg_ifj_length(s);
    cg_move(str_len, cg_var_retval);
    cg_jump_gteq(ret_null, i, str_len);
    // j > ifj.length(s)
    cg_jump_gt(ret_null, j, str_len);

    cg_create_label(ret_null);

    cg_move(cg_var_retval, cg_null_term);
    cg_create_label(finish);

    cg_pop_frame();
}

