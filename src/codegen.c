/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file codegen.c
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "compiler_error.h"
#include "binary_tree.h"
#include "codegen.h"

typedef enum frame{
    GLOBAL = 0,
    LOCAL = 1,
    TEMPORARY = 2,
} Frame;

typedef struct term{
    enum _term_type{
        CG_VARIABLE_T,
        CG_INTEGER_T,
        CG_FLOAT_T,
        CG_BOOLEAN_T,
        CG_STRING_T,
        CG_NULL_T
    } type;
    union _term_value{
        char *var_name;
        int int_val;
        double float_val;
        bool bool_val;
        char *string;
    } value;
    Frame frame; // In case of CG_VARIABLE_T (otherwise can be left uninitialized)
} TTerm;

typedef unsigned long long TLabel;

// IFJcode24 GF variable for storing function return values
const TTerm cg_var_retval = {.type = CG_VARIABLE_T, .value.var_name = "retval", .frame = GLOBAL};
// IFJcode24 GF bool variable for storing results of comparisons
const TTerm cg_var_cmp = {.type = CG_VARIABLE_T, .value.var_name = "cmp", .frame = GLOBAL};
// IFJcode24 GF temp variables
const TTerm cg_var_temp = {.type = CG_VARIABLE_T, .value.var_name = "temp", .frame = GLOBAL};
const TTerm cg_var_temp2 = {.type = CG_VARIABLE_T, .value.var_name = "temp2", .frame = GLOBAL};
// Often used literals
const TTerm cg_null_term = {.type = CG_NULL_T};
const TTerm cg_true_term = {.type = CG_BOOLEAN_T, .value.bool_val = true};
const TTerm cg_false_term = {.type = CG_BOOLEAN_T, .value.bool_val = false};
const TTerm cg_zero_int_term = {.type = CG_INTEGER_T, .value.int_val = 0};
const TTerm cg_one_int_term = {.type = CG_INTEGER_T, .value.int_val = 1};
const TTerm cg_empty_string_lit = {.type = CG_STRING_T, .value.string = ""};

// IFJcode24 frame types
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
    error = ERR_COMPILER_INTERNAL;
    return "";
}

// ---------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------- DECLARATIONS --------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

void cg_term(TTerm term);

// CG variables and frames

void cg_create_var(TTerm var);

void cg_init(void);

void cg_create_frame(void);

void cg_push_frame(void);

void cg_pop_frame(void);

void cg_exit(TTerm number);

// Function calling

void cg_call(char* function);

void cg_return(void);

void cg_create_fun(char* function);

// Assigning to variables

void cg_move(TTerm dest, TTerm src);

void cg_set_type_bool(TTerm var);

// Labels

TLabel cg_get_new_label(void);

void cg_create_label(TLabel label_number);

void cg_label(TLabel label_number);

// Operands

void cg_two_operands(TTerm o1, TTerm o2);

void cg_three_operands(TTerm o1, TTerm o2, TTerm o3);

// Comparisons

void cg_equal(TTerm value1, TTerm value2);

void cg_less_than(TTerm value1, TTerm value2);

void cg_greater_than(TTerm value1, TTerm value2);

void cg_eq_stack(void);

void cg_lt_stack(void);

void cg_gt_stack(void);

void cg_lteq_stack(void);

void cg_gteq_stack(void);

// Jumps

void cg_jump(TLabel label);

void cg_jump_eq(TLabel label, TTerm value1, TTerm value2);

void cg_jump_neq(TLabel label, TTerm value1, TTerm value2);

void cg_jump_lt(TLabel label, TTerm value1, TTerm value2);

void cg_jump_gt(TLabel label, TTerm value1, TTerm value2);

void cg_jump_lteq(TLabel label, TTerm value1, TTerm value2);

void cg_jump_gteq(TLabel label, TTerm value1, TTerm value2);

// Arithmetic

void cg_add(TTerm dest, TTerm num1, TTerm num2);

void cg_sub(TTerm dest, TTerm num1, TTerm num2);

void cg_mul(TTerm dest, TTerm num1, TTerm num2);

void cg_fdiv(TTerm dest, TTerm num1, TTerm num2);

void cg_idiv(TTerm dest, TTerm num1, TTerm num2);

// Arithmetic stack

void cg_add_stack(void);

void cg_sub_stack(void);

void cg_mul_stack(void);

void cg_fdiv_stack(void);

void cg_idiv_stack(void);

// Increment/Decrement

void cg_int_var_inc_1(TTerm ivar);

void cg_int_var_dec_1(TTerm ivar);

// String

void cg_concat(TTerm dest, TTerm string1, TTerm string2);

void cg_strlen(TTerm dest, TTerm string);

void cg_getchar(TTerm dest, TTerm string, TTerm position);

void cg_setchar(TTerm src_str, TTerm dest_str, TTerm position);

void cg_stri2int(TTerm dest, TTerm string, TTerm pos);

// Stack

void cg_stack_push(TTerm value);

void cg_stack_pop(TTerm variable);

void cg_stack_clear(void);

// IFJ BUILT-IN

void cg_ifj_readstr(void);

void cg_ifj_readi32(void);

void cg_ifj_readf64(void);

void cg_ifj_write(void);

void cg_ifj_i2f(void);

void cg_ifj_f2i(void);

void cg_ifj_string(void);

void cg_ifj_length(void);

void cg_ifj_concat(void);

void cg_ifj_substring(void);

void cg_ifj_strcmp(void);

void cg_ifj_ord(void);

void cg_ifj_chr(void);

// Codegen

void generate_comment(char* string);

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- DEFINITIONS ----------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

void cg_term(TTerm term){
    char* bool_str;
    switch(term.type){
        case CG_VARIABLE_T:
            printf("%sVAR_%s", get_frame(term.frame), term.value.var_name);
            break;
        case CG_INTEGER_T:
            printf("int@%d", term.value.int_val);
            break;
        case CG_FLOAT_T:
            printf("float@%a", term.value.float_val);
            break;
        case CG_BOOLEAN_T:
            bool_str = term.value.bool_val ? "true" : "false";
            printf("bool@%s", bool_str);
            break;
        case CG_STRING_T:
            printf("string@");
            int i = 0;
            while(term.value.string[i] != '\0'){
                char c = term.value.string[i];
                if(c <= 32 || c == '#' || c == '\\'){ // 000-032: Unprintable characters, 035: '#', 092: '\'
                    printf("\\%03d", c);
                }
                else{
                    putchar(c);
                }
                i++;
            }
            break;
        case CG_NULL_T:
            printf("nil@nil");
            break;
        default:
            error = ERR_COMPILER_INTERNAL;
            break;
    }
}

// CG variables and frames

void cg_create_var(TTerm var){
    if(var.type != CG_VARIABLE_T || var.value.var_name == NULL){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("defvar ");
    cg_term(var);
    putchar('\n');
}

void cg_init(void){
    printf(".IFJcode24\n");
    cg_create_var(cg_var_retval);
    cg_create_var(cg_var_cmp);
    cg_create_var(cg_var_temp);
    cg_create_var(cg_var_temp2);
    cg_set_type_bool(cg_var_cmp);
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

void cg_exit(TTerm number){
    printf("exit ");
    cg_term(number);
    putchar('\n');
}

// Function calling

void cg_print_fun(char* function){
    for(int i = 0; function[i] != '\0'; i++){
        if(function[i] == '.'){
            putchar('-');
        }
        else{
            putchar(function[i]);
        }
    }
}

void cg_call(char* function){
    if(function == NULL){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("call FUN_");
    cg_print_fun(function);
    putchar('\n');
}

void cg_return(void){
    printf("return\n");
}

void cg_create_fun(char* function){
    if(function == NULL){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("label FUN_");
    cg_print_fun(function);
    putchar('\n');
}

void cg_move(TTerm dest, TTerm src){
    if(dest.type != CG_VARIABLE_T){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("move ");
    cg_term(dest);
    putchar(' ');
    cg_term(src);
    putchar('\n');
}

void cg_set_type_bool(TTerm var){
    cg_move(var, cg_false_term);
}

// Labels

TLabel cg_get_new_label(void){
    static TLabel label = 0;
    return label++;
}

void cg_create_label(TLabel label_number){
    printf("label L%llu\n", label_number);
}

void cg_label(TLabel label_number){
    printf("L%llu", label_number);
}

// Comparisons

void cg_two_operands(TTerm o1, TTerm o2){
    putchar(' ');
    cg_term(o1);
    putchar(' ');
    cg_term(o2);
    putchar('\n');
}

void cg_three_operands(TTerm o1, TTerm o2, TTerm o3){
    putchar(' ');
    cg_term(o1);
    cg_two_operands(o2, o3);
}

void cg_equal(TTerm value1, TTerm value2){
    printf("eq");
    cg_three_operands(cg_var_cmp, value1, value2);
}

void cg_less_than(TTerm value1, TTerm value2){
    printf("lt");
    cg_three_operands(cg_var_cmp, value1, value2);
}

void cg_greater_than(TTerm value1, TTerm value2){
    printf("gt");
    cg_three_operands(cg_var_cmp, value1, value2);
}

void cg_eq_stack(void){
    printf("eqs\n");
}

void cg_lt_stack(void){
    printf("lts\n");
}

void cg_gt_stack(void){
    printf("gts\n");
}

void cg_lteq_stack(void){
    cg_stack_pop(cg_var_temp2);
    cg_stack_pop(cg_var_temp);
    cg_stack_push(cg_var_temp);
    cg_stack_push(cg_var_temp2);
    cg_lt_stack();
    cg_stack_push(cg_var_temp);
    cg_stack_push(cg_var_temp2);
    cg_eq_stack();
    printf("ors\n");
}

void cg_gteq_stack(void){
    cg_stack_pop(cg_var_temp2);
    cg_stack_pop(cg_var_temp);
    cg_stack_push(cg_var_temp);
    cg_stack_push(cg_var_temp2);
    cg_gt_stack();
    cg_stack_push(cg_var_temp);
    cg_stack_push(cg_var_temp2);
    cg_eq_stack();
    printf("ors\n");
}

// Jumps

void cg_jump(TLabel label){
    printf("jump ");
    cg_label(label);
    putchar('\n');
}

void cg_jump_eq(TLabel label, TTerm value1, TTerm value2){
    printf("jumpifeq ");
    cg_label(label);
    cg_two_operands(value1, value2);
}

void cg_jump_neq(TLabel label, TTerm value1, TTerm value2){
    printf("jumpifneq ");
    cg_label(label);
    cg_two_operands(value1, value2);
}

void cg_jump_lt(TLabel label, TTerm value1, TTerm value2){
    cg_less_than(value1, value2);
    cg_jump_eq(label, cg_var_cmp, cg_true_term);
}

void cg_jump_gt(TLabel label, TTerm value1, TTerm value2){
    cg_greater_than(value1, value2);
    cg_jump_eq(label, cg_var_cmp, cg_true_term);
}

void cg_jump_lteq(TLabel label, TTerm value1, TTerm value2){
    cg_jump_eq(label, value1, value2);
    cg_jump_lt(label, value1, value2);
}

void cg_jump_gteq(TLabel label, TTerm value1, TTerm value2){
    cg_jump_eq(label, value1, value2);
    cg_jump_gt(label, value1, value2);
}

// Arithmetic

void cg_add(TTerm dest, TTerm num1, TTerm num2){
    printf("add");
    cg_three_operands(dest, num1, num2);
}

void cg_sub(TTerm dest, TTerm num1, TTerm num2){
    printf("sub");
    cg_three_operands(dest, num1, num2);
}

void cg_mul(TTerm dest, TTerm num1, TTerm num2){
    printf("mul");
    cg_three_operands(dest, num1, num2);
}

void cg_fdiv(TTerm dest, TTerm num1, TTerm num2){
    printf("div");
    cg_three_operands(dest, num1, num2);
}

void cg_idiv(TTerm dest, TTerm num1, TTerm num2){
    printf("idiv");
    cg_three_operands(dest, num1, num2);
}

// Arithmetic stack

void cg_add_stack(void){
    printf("adds\n");
}

void cg_sub_stack(void){
    printf("subs\n");
}

void cg_mul_stack(void){
    printf("muls\n");
}

void cg_fdiv_stack(void){
    printf("divs\n");
}

void cg_idiv_stack(void){
    printf("idivs\n");
}

void cg_div_stack(void){
    TTerm float_type = {.type = CG_STRING_T, .value.string = "float"};
    cg_stack_pop(cg_var_temp);
    printf("type ");
    cg_two_operands(cg_var_temp2, cg_var_temp);
    cg_stack_push(cg_var_temp);
    TLabel is_float = cg_get_new_label();
    TLabel end = cg_get_new_label();

    cg_jump_eq(is_float, cg_var_temp2, float_type);
    cg_idiv_stack();
    cg_jump(end);
    cg_create_label(is_float);
    cg_fdiv_stack();
    cg_create_label(end);
}

// Increment/Decrement

void cg_int_var_inc_1(TTerm ivar){
    cg_add(ivar, ivar, cg_one_int_term);
}

void cg_int_var_dec_1(TTerm ivar){
    cg_sub(ivar, ivar, cg_one_int_term);
}

// String

void cg_concat(TTerm dest, TTerm string1, TTerm string2){
    printf("concat");
    cg_three_operands(dest, string1, string2);
}

void cg_strlen(TTerm dest, TTerm string){
    printf("strlen");
    cg_two_operands(dest, string);
}

void cg_getchar(TTerm dest, TTerm string, TTerm position){
    printf("getchar");
    cg_three_operands(dest, string, position);
}

void cg_setchar(TTerm src_str, TTerm dest_str, TTerm position){
    printf("setchar");
    cg_three_operands(src_str, dest_str, position);
}

void cg_stri2int(TTerm dest, TTerm string, TTerm pos){
    printf("stri2int");
    cg_three_operands(dest, string, pos);
}

void cg_int2char(TTerm dest, TTerm num){
    printf("int2char");
    cg_two_operands(dest, num);
}

// Stack

void cg_stack_push(TTerm value){
    printf("pushs ");
    cg_term(value);
    putchar('\n');
}

void cg_stack_pop(TTerm variable){
    printf("pops ");
    cg_term(variable);
    putchar('\n');
}

void cg_stack_clear(void){
    printf("clears\n");
}

// IFJ BUILT-IN

void cg_ifj_readstr(void){
    cg_create_fun("ifj.readstr");
    printf("read ");
    cg_term(cg_var_retval);
    printf(" string\n");
    cg_return();
}

void cg_ifj_readi32(void){
    cg_create_fun("ifj.readi32");
    printf("read ");
    cg_term(cg_var_retval);
    printf(" int\n");
    cg_return();
}

void cg_ifj_readf64(void){
    cg_create_fun("ifj.readf64");
    printf("read ");
    cg_term(cg_var_retval);
    printf(" float\n");
    cg_return();
}

void cg_ifj_write(void){
    cg_create_fun("ifj.write");
    cg_create_frame();
    cg_push_frame();

    TTerm term = {.type = CG_VARIABLE_T, .value.var_name = "term", .frame = LOCAL};
    cg_create_var(term);
    cg_stack_pop(term);

    printf("write ");
    cg_term(term);
    putchar('\n');

    cg_pop_frame();
    cg_return();
}

void cg_ifj_i2f(void){
    cg_create_fun("ifj.i2f");
    cg_create_frame();
    cg_push_frame();

    TTerm term = {.type = CG_VARIABLE_T, .value.var_name = "term", .frame = LOCAL};
    cg_create_var(term);
    cg_stack_pop(term);

    printf("int2float");
    cg_two_operands(cg_var_retval, term);

    cg_pop_frame();
    cg_return();
}

void cg_ifj_f2i(void){
    cg_create_fun("ifj.f2i");
    cg_create_frame();
    cg_push_frame();

    TTerm term = {.type = CG_VARIABLE_T, .value.var_name = "term", .frame = LOCAL};
    cg_create_var(term);
    cg_stack_pop(term);

    printf("float2int");
    cg_two_operands(cg_var_retval, term);

    cg_pop_frame();
    cg_return();
}

void cg_ifj_string(void){
    cg_create_fun("ifj.string");
    cg_create_frame();
    cg_push_frame();

    TTerm term = {.type = CG_VARIABLE_T, .value.var_name = "term", .frame = LOCAL};
    cg_create_var(term);
    cg_stack_pop(term);

    cg_move(cg_var_retval, term);

    cg_pop_frame();
    cg_return();
}

void cg_ifj_length(void){
    cg_create_fun("ifj.length");
    cg_create_frame();
    cg_push_frame();

    TTerm s = {.type = CG_VARIABLE_T, .value.var_name = "s", .frame = LOCAL};
    cg_create_var(s);
    cg_stack_pop(s);

    cg_strlen(cg_var_retval, s);

    cg_pop_frame();
    cg_return();
}

void cg_ifj_concat(void){
    cg_create_fun("ifj.concat");
    cg_create_frame();
    cg_push_frame();
    // Parameters
    TTerm s2 = {.type = CG_VARIABLE_T, .value.var_name = "s2", .frame = LOCAL};
    cg_create_var(s2);
    cg_stack_pop(s2);
    TTerm s1 = {.type = CG_VARIABLE_T, .value.var_name = "s1", .frame = LOCAL};
    cg_create_var(s1);
    cg_stack_pop(s1);
    // Code
    cg_concat(cg_var_retval, s1, s2);
    cg_pop_frame();
    cg_return();
}

void cg_ifj_substring(void){
    cg_create_fun("ifj.substring");
    cg_create_frame();
    cg_push_frame();
    TTerm j = {.type = CG_VARIABLE_T, .value.var_name = "j", .frame = LOCAL};
    cg_create_var(j);
    cg_stack_pop(j);
    TTerm i = {.type = CG_VARIABLE_T, .value.var_name = "i", .frame = LOCAL};
    cg_create_var(i);
    cg_stack_pop(i);
    TTerm s = {.type = CG_VARIABLE_T, .value.var_name = "s", .frame = LOCAL};
    cg_create_var(s);
    cg_stack_pop(s);
    // Label numbers
    TLabel ret_null = cg_get_new_label();
    TLabel finish = cg_get_new_label();
    // Terms
    TTerm str_len = {.type = CG_VARIABLE_T, .value.var_name = "str_len", .frame = LOCAL};
    TTerm character = {.type = CG_VARIABLE_T, .value.var_name = "character", .frame = LOCAL};
    cg_create_var(str_len);
    cg_create_var(character);
    // Code
    // i < 0
    cg_jump_lt(ret_null, i, cg_zero_int_term);
    // j < 0
    cg_jump_lt(ret_null, j, cg_zero_int_term);
    // i > j
    cg_jump_gt(ret_null, i, j);
    // i >= ifj.length(s)
    cg_stack_push(s);
    cg_call("ifj.length");
    cg_move(str_len, cg_var_retval);
    cg_jump_gteq(ret_null, i, str_len);
    // j > ifj.length(s)
    cg_jump_gt(ret_null, j, str_len);

    TTerm i_loc = {.type = CG_VARIABLE_T, .value.var_name = "i_loc", .frame = LOCAL};
    cg_create_var(i_loc);
    cg_move(i_loc, i);

    // Init return value to empty string
    cg_move(cg_var_retval, cg_empty_string_lit);

    // while i < j
    TLabel while_beg = cg_get_new_label();
    TLabel while_end = cg_get_new_label();

    cg_create_label(while_beg);
    cg_jump_gteq(while_end, i_loc, j);

    cg_getchar(character, s, i_loc);
    cg_concat(cg_var_retval, cg_var_retval, character);
    cg_int_var_inc_1(i_loc);

    cg_jump(while_beg);
    cg_create_label(while_end);

    // Return
    cg_jump(finish);
    cg_create_label(ret_null);
    cg_move(cg_var_retval, cg_null_term);
    cg_create_label(finish);

    cg_pop_frame();
    cg_return();
}

void cg_ifj_strcmp(void){
    cg_create_fun("ifj.strcmp");
    cg_create_frame();
    cg_push_frame();
    // Parameters
    TTerm s2 = {.type = CG_VARIABLE_T, .value.var_name = "s2", .frame = LOCAL};
    cg_create_var(s2);
    cg_stack_pop(s2);
    TTerm s1 = {.type = CG_VARIABLE_T, .value.var_name = "s1", .frame = LOCAL};
    cg_create_var(s1);
    cg_stack_pop(s1);

    // Variables
    TTerm s1_len = {.type = CG_VARIABLE_T, .value.var_name = "s1_len", .frame = LOCAL};
    TTerm s2_len = {.type = CG_VARIABLE_T, .value.var_name = "s2_len", .frame = LOCAL};
    TTerm len_min = {.type = CG_VARIABLE_T, .value.var_name = "len_min", .frame = LOCAL};
    TTerm index = {.type = CG_VARIABLE_T, .value.var_name = "index", .frame = LOCAL};
    TTerm char1 = {.type = CG_VARIABLE_T, .value.var_name = "char1", .frame = LOCAL};
    TTerm char2 = {.type = CG_VARIABLE_T, .value.var_name = "char2", .frame = LOCAL};
    cg_create_var(s1_len);
    cg_create_var(s2_len);
    cg_create_var(len_min);
    cg_create_var(index);
    cg_create_var(char1);
    cg_create_var(char2);
    // Labels
    TLabel sel_len_2 = cg_get_new_label();
    TLabel len_sel_end = cg_get_new_label();
    TLabel while_beg = cg_get_new_label();
    TLabel while_end = cg_get_new_label();
    TLabel ret_pos = cg_get_new_label();
    TLabel ret_neg = cg_get_new_label();
    TLabel fun_end = cg_get_new_label();

    // Code
    cg_move(cg_var_retval, cg_zero_int_term);

    cg_strlen(s1_len, s1);
    cg_strlen(s2_len, s2);

    // Selection of shorter length
    cg_jump_gt(sel_len_2, s1_len, s2_len);
    cg_move(len_min, s1_len);
    cg_jump(len_sel_end);
    cg_create_label(sel_len_2);
    cg_move(len_min, s2_len);
    cg_create_label(len_sel_end);

    // Comparison of strings
    // BEG WHILE
    cg_move(index, cg_zero_int_term);
    cg_create_label(while_beg);
    cg_jump_gteq(while_end, index, len_min);
    // WHILE BODY
    cg_getchar(char1, s1, index);
    cg_getchar(char2, s2, index);

    cg_jump_lt(ret_neg, char1, char2);
    cg_jump_gt(ret_pos, char1, char2);

    // END WHILE
    cg_int_var_inc_1(index);
    cg_jump(while_beg);
    cg_create_label(while_end);

    cg_jump_lt(ret_neg, s1_len, s2_len);
    cg_jump_gt(ret_pos, s1_len, s2_len);

    cg_jump(fun_end);
    cg_create_label(ret_pos);
    cg_int_var_inc_1(cg_var_retval);
    cg_jump(fun_end);
    cg_create_label(ret_neg);
    cg_int_var_dec_1(cg_var_retval);
    cg_create_label(fun_end);
    cg_pop_frame();
    cg_return();
}

void cg_ifj_ord(void){
    cg_create_fun("ifj.ord");
    cg_create_frame();
    cg_push_frame();

    TTerm i = {.type = CG_VARIABLE_T, .value.var_name = "i", .frame = LOCAL};
    cg_create_var(i);
    cg_stack_pop(i);
    TTerm s = {.type = CG_VARIABLE_T, .value.var_name = "s", .frame = LOCAL};
    cg_create_var(s);
    cg_stack_pop(s);

    // Variables
    TTerm s_len = {.type = CG_VARIABLE_T, .value.var_name = "s_len", .frame = LOCAL};
    cg_create_var(s_len);
    // Labels
    TLabel fun_end = cg_get_new_label();
    TLabel ret_zer = cg_get_new_label();
    // Code
    cg_strlen(s_len, s);
    // Check bounds
    cg_jump_lt(ret_zer, i, cg_zero_int_term);
    cg_jump_gteq(ret_zer, i, s_len);
    // Find ordinal value
    cg_stri2int(cg_var_retval, s, i);

    cg_jump(fun_end);
    cg_create_label(ret_zer);
    cg_move(cg_var_retval, cg_zero_int_term);
    cg_create_label(fun_end);
    cg_pop_frame();
    cg_return();
}

void cg_ifj_chr(void){
    cg_create_fun("ifj.chr");
    cg_create_frame();
    cg_push_frame();

    TTerm i = {.type = CG_VARIABLE_T, .value.var_name = "i", .frame = LOCAL};
    cg_create_var(i);
    cg_stack_pop(i);

    cg_int2char(cg_var_retval, i);

    cg_pop_frame();
    cg_return();
}

// Variable search tree

typedef struct element{
    char* var_name;
    struct element* right;
    struct element* left;
} TElement;

TElement* var_tree = NULL;

bool insert_in(TElement** element, char* name){
    if(element == NULL){
        error = ERR_COMPILER_INTERNAL;
        return false;
    }
    if(*element == NULL){
        *element = malloc(sizeof(TElement));
        if(*element == NULL){
            error = ERR_COMPILER_INTERNAL;
            return false;
        }
        (*element)->var_name = name;
        (*element)->right = NULL;
        (*element)->left = NULL;
        return true;
    }
    int comp = strcmp(name, (*element)->var_name);
    if(comp < 0){
        return insert_in(&(*element)->left, name);
    }
    if(comp > 0){
        return insert_in(&(*element)->right, name);
    }
    return false;
}

bool insert(char* name){
    return insert_in(&var_tree, name);
}

void dispose_in(TElement* element){
    if(element == NULL){
        return;
    }
    dispose_in(element->left);
    dispose_in(element->right);
    free(element);
}

void dispose(void){
    dispose_in(var_tree);
    var_tree = NULL;
}

// Codegen

void generate_comment(char* string){
    if(string == NULL){
        return;
    }
    printf("# %s\n", string);
}

// Function generation
// Declarations

void generate_function_body(TBinaryTree* tree);

// Definitions

void generate_function_parameters(linked_list_t parameters){
    set_last_llist(&parameters);
    char** param_name_ptr = malloc(sizeof(char*));
    if(param_name_ptr == NULL){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    while(get_value_llist(&parameters, param_name_ptr)){
        TTerm variable = {.type = CG_VARIABLE_T, .value.var_name = *param_name_ptr, .frame = LOCAL};
        cg_create_var(variable);
        cg_stack_pop(variable);
        prev_llist(&parameters);
    }
}

void calculate_expression(TBinaryTree* tree){
    if(BT_has_left(tree)){
        BT_go_left(tree);
        calculate_expression(tree);
        BT_go_parent(tree);
    }
    if(BT_has_right(tree)){
        BT_go_right(tree);
        calculate_expression(tree);
        BT_go_parent(tree);
    }
    node_type type;
    node_data data;
    if(!BT_get_node_type(tree, &type) || !BT_get_data(tree, &data)){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    TTerm term;
    switch(type){
        case INT:
            term.type = CG_INTEGER_T;
            term.value.int_val = (int)strtol(data.nodeData.value.literal, NULL, 0);
             cg_stack_push(term);
             break;
        case FL:
            term.type = CG_FLOAT_T;
            term.value.float_val = strtod(data.nodeData.value.literal, NULL);
            cg_stack_push(term);
            break;
        case STR:
            term.type = CG_STRING_T;
            term.value.string = data.nodeData.value.literal;
            cg_stack_push(term);
            break;
        case NULL_LITERAL:
            term.type = CG_NULL_T;
            cg_stack_push(term);
            break;
        case VAR_CONST:
            term.type = CG_VARIABLE_T;
            term.value.var_name = data.nodeData.value.identifier;
            term.frame = LOCAL;
            cg_stack_push(term);
            break;
        case OP_ADD:
            cg_add_stack();
            break;
        case OP_SUB:
            cg_sub_stack();
            break;
        case OP_MUL:
            cg_mul_stack();
            break;
        case OP_DIV:
            cg_div_stack();
            break;
        case OP_EQ:
            cg_eq_stack();
            break;
        case OP_NEQ:
            cg_eq_stack();
            printf("nots\n");
            break;
        case OP_GT:
            cg_gt_stack();
            break;
        case OP_LS:
            cg_lt_stack();
            break;
        case OP_GTE:
            cg_gteq_stack();
            break;
        case OP_LSE:
            cg_lteq_stack();
            break;
        default:
            error = ERR_COMPILER_INTERNAL;
            break;
    }
}

void generate_return(TBinaryTree* tree){
    if(BT_has_left(tree)){
        BT_go_left(tree);
        cg_stack_clear();
        calculate_expression(tree);
        cg_stack_pop(cg_var_retval);
        BT_go_parent(tree);
    }
    cg_pop_frame();
    cg_return();
}

TTerm node_to_term_rval(TBinaryTree* tree){
    node_type type;
    node_data data;
    TTerm term = cg_null_term;
    if(!BT_get_node_type(tree, &type) || !BT_get_data(tree, &data)){
        error = ERR_COMPILER_INTERNAL;
        return term;
    }
    switch(type){
        case INT:
            term.type = CG_INTEGER_T;
            term.value.int_val = (int) strtol(data.nodeData.value.literal, NULL, 0);
            break;
        case FL:
            term.type = CG_FLOAT_T;
            term.value.float_val = strtod(data.nodeData.value.literal, NULL);
            break;
        case STR:
            term.type = CG_STRING_T;
            term.value.string = data.nodeData.value.literal;
            break;
        case NULL_LITERAL:
            term.type = CG_NULL_T;
            break;
        case VAR_CONST:
            term.type = CG_VARIABLE_T;
            term.value.var_name = data.nodeData.value.identifier;
            term.frame = LOCAL;
            break;
        default:
            error = ERR_COMPILER_INTERNAL;
            break;
    }
    return term;
}

void generate_call(TBinaryTree* tree){
    node_data fun_data;
    if(!BT_get_data(tree, &fun_data)){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    int arg_count = 0;
    while(BT_has_right(tree)){
        BT_go_right(tree);
        cg_stack_push(node_to_term_rval(tree));
        arg_count++;
    }
    while(arg_count > 0){
        BT_go_parent(tree);
        arg_count--;
    }
    cg_call(fun_data.nodeData.identifier.identifier);
}

void generate_var_declaration(TBinaryTree* tree){
    node_data data;
    node_type type;
    if(!BT_get_data(tree, &data)){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    TTerm variable = {.type = CG_VARIABLE_T, .value.var_name = data.nodeData.identifier.identifier, .frame = LOCAL};
    if(insert(variable.value.var_name)){
        cg_create_var(variable);
    }
    if(BT_has_left(tree)){
        BT_go_left(tree);
        if(!BT_get_node_type(tree, &type)){
            error = ERR_COMPILER_INTERNAL;
            return;
        }
        if(type == FUNCTION_CALL){
            generate_call(tree);
            cg_move(variable, cg_var_retval);
        }
        else{
            cg_stack_clear();
            calculate_expression(tree);
            cg_stack_pop(variable);
        }
        BT_go_parent(tree);
    }
}

void generate_assignment(TBinaryTree* tree){
    node_type type;
    node_data data;
    BT_get_data(tree, &data);
    TTerm var = {.type = CG_VARIABLE_T, .value.var_name = data.nodeData.identifier.identifier, .frame = LOCAL};
    BT_go_left(tree);
    BT_get_node_type(tree, &type);
    if(strcmp("_", var.value.var_name) != 0){
        if(type == FUNCTION_CALL){
            generate_call(tree);
            cg_move(var, cg_var_retval);
        }
        else{
            calculate_expression(tree);
            cg_stack_pop(var);
        }
    }
    BT_go_parent(tree);
}

void generate_else(TBinaryTree* tree, TLabel label){
    cg_create_label(label);
    // Enter else node
    BT_go_parent(tree);
    BT_go_right(tree);
    BT_go_left(tree);
    // Else
    generate_function_body(tree);
    // Return to if node
    BT_go_parent(tree);
    BT_go_parent(tree);
    BT_go_left(tree);
}

void generate_if(TBinaryTree* tree){
    // Node
    node_data data;
    BT_get_data(tree, &data);
    // Labels
    TLabel else_label = cg_get_new_label();
    TLabel end_if_label = cg_get_new_label();
    // Expression
    BT_go_left(tree);
    calculate_expression(tree);
    BT_go_parent(tree);
    // Jump
    cg_stack_pop(cg_var_temp);
    if(data.nodeData.body.is_nullable){
        cg_jump_eq(else_label, cg_var_temp, cg_null_term);
        TTerm replacement = {.type = CG_VARIABLE_T, .value.var_name = data.nodeData.body.null_replacement, .frame = LOCAL};
        if(insert(replacement.value.var_name)){
            cg_create_var(replacement);
        }
        cg_move(replacement, cg_var_temp);
    }
    else{
        cg_jump_eq(else_label, cg_var_temp, cg_false_term);
    }
    generate_function_body(tree);
    cg_jump(end_if_label);
    generate_else(tree, else_label);
    cg_create_label(end_if_label);
}

void generate_while_declarations(TBinaryTree* tree){
    if(BT_has_left(tree)){
        BT_go_left(tree);
        node_type type;
        node_data data;
        BT_get_node_type(tree, &type);
        BT_get_data(tree, &data);
        TTerm variable = {.type = CG_VARIABLE_T, .frame = LOCAL};
        switch(type){
            case CONST_DECL:
            case VAR_DECL:
                if(insert(data.nodeData.identifier.identifier)){
                    variable.value.var_name = data.nodeData.identifier.identifier;
                    cg_create_var(variable);
                }
                break;
            case WHILE:
            case IF:
                if(data.nodeData.body.is_nullable){
                    if(insert(data.nodeData.body.null_replacement)){
                        variable.value.var_name = data.nodeData.body.null_replacement;
                        cg_create_var(variable);
                    }
                }
            default:
                break;
        }
        int nestings = 0;
        while(BT_has_right(tree)){
            BT_go_right(tree);
            nestings++;
            generate_while_declarations(tree);
        }
        while(nestings > 0){
            BT_go_parent(tree);
            nestings--;
        }
        BT_go_parent(tree);
    }
}

bool generated = false;
void generate_while(TBinaryTree* tree){
    // Node
    node_data data;
    BT_get_data(tree, &data);
    // Labels
    TLabel while_beg = cg_get_new_label();
    TLabel while_end = cg_get_new_label();
    // Declarations
    bool unlock = false;
    if(!generated){
        BT_go_parent(tree);
        generate_while_declarations(tree);
        BT_go_left(tree);
        unlock = true;
        generated = true;
    }
    // Expression
    cg_create_label(while_beg);
    BT_go_left(tree);
    calculate_expression(tree);
    BT_go_parent(tree);
    // Jump
    cg_stack_pop(cg_var_temp);
    if(data.nodeData.body.is_nullable){
        cg_jump_eq(while_end, cg_var_temp, cg_null_term);
        TTerm replacement = {.type = CG_VARIABLE_T, .value.var_name = data.nodeData.body.null_replacement, .frame = LOCAL};
        cg_move(replacement, cg_var_temp);
    }
    else{
        cg_jump_eq(while_end, cg_var_temp, cg_false_term);
    }
    generate_function_body(tree);
    cg_jump(while_beg);
    cg_create_label(while_end);
    if(unlock){
        generated = false;
    }
}

void generate_command(TBinaryTree* tree){
    if(!BT_has_left(tree)){
        return;
    }
    BT_go_left(tree);
    node_type type;
    if(!BT_get_node_type(tree, &type)){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    switch(type){
        case RETURN:
            generate_return(tree);
            break;
        case CONST_DECL:
        case VAR_DECL:
            generate_var_declaration(tree);
            break;
        case ASSIG:
            generate_assignment(tree);
            break;
        case BODY:
            generate_function_body(tree);
            break;
        case WHILE:
            generate_while(tree);
            break;
        case IF:
            generate_if(tree);
            break;
        case ELSE:
            // Do nothing
            break;
        case FUNCTION_CALL:
            generate_call(tree);
            break;
        default:
            error = ERR_COMPILER_INTERNAL;
            break;
    }
    BT_go_parent(tree);
}

void generate_function_body(TBinaryTree* tree){
    if(!BT_has_right(tree)){
        return;
    }
    BT_go_right(tree);
    generate_command(tree);

    generate_function_body(tree);
    BT_go_parent(tree);
}

void generate_function(TBinaryTree* tree){
    node_data data;
    BT_get_data(tree, &data);
    // Creating function label and memory frame
    generate_comment(data.nodeData.function.identifier); // Comment
    cg_create_fun(data.nodeData.function.identifier);
    cg_create_frame();
    cg_push_frame();
    // Creating variables for the parameters, moving arguments to the variables
    generate_function_parameters(data.nodeData.function.param_identifiers);
    generate_function_body(tree);
    // Creating return value
    generate_return(NULL);
    // Dispose var tree
    dispose();
}

void generate_builtin(void){
    generate_comment("____IFJ BUILT-IN____");
    cg_ifj_readstr();
    cg_ifj_readi32();
    cg_ifj_readf64();
    cg_ifj_write();
    cg_ifj_i2f();
    cg_ifj_f2i();
    cg_ifj_string();
    cg_ifj_length();
    cg_ifj_concat();
    cg_ifj_substring();
    cg_ifj_strcmp();
    cg_ifj_ord();
    cg_ifj_chr();
}

void codegen(TBinaryTree* tree){
    if(tree == NULL){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    BT_assign_parents(tree);
    generate_comment("Init:");
    cg_init();
    generate_comment("Generating program:");
    generate_comment("Main function call:");
    cg_call("main");
    cg_exit(cg_zero_int_term);
    generate_comment("Function definitions:");
    BT_set_root(tree);
    while(BT_has_left(tree)){
        BT_go_left(tree);
        generate_function(tree);
    }
    generate_builtin();
}
