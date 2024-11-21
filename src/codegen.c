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

typedef enum frame{
    GLOBAL = 0,
    LOCAL = 1,
    TEMPORARY = 2,
} Frame;

typedef struct term{
    enum _term_type{
        VARIABLE_T,
        INTEGER_T,
        FLOAT_T,
        BOOLEAN_T,
        STRING_T,
        NULL_T
    } type;
    union _term_value{
        char *var_name;
        int int_val;
        double float_val;
        bool bool_val;
        char *string;
    } value;
    Frame frame; // In case of VARIABLE_T (otherwise can be left uninitialized)
} TTerm;

typedef unsigned long long TLabel;

// Return value global frame variable
const TTerm cg_var_retval = {.type = VARIABLE_T, .value.var_name = "retval", .frame = GLOBAL};
const TTerm cg_var_cmp = {.type = VARIABLE_T, .value.var_name = "cmp", .frame = GLOBAL};
// Global terms
const TTerm cg_null_term = {.type = NULL_T};
const TTerm cg_true_term = {.type = BOOLEAN_T, .value.bool_val = true};
const TTerm cg_zero_int_term = {.type = INTEGER_T, .value.int_val = 0};
const TTerm cg_one_int_term = {.type = INTEGER_T, .value.int_val = 1};
const TTerm cg_empty_string_lit = {.type = STRING_T, .value.string = ""};
const TTerm cg_bool_false_term = {.type = BOOLEAN_T, .value.bool_val = false};

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

// Function calling

void cg_call(char* label);

void cg_return(void);

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

// Jumps

void cg_jump(TLabel label);

void cg_jump_eq(TLabel label, TTerm value1, TTerm value2);

void cg_jump_not_neq(TLabel label, TTerm value1, TTerm value2);

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

// Increment/Decrement

void cg_int_var_inc_1(TTerm ivar);

void cg_int_var_dec_1(TTerm ivar);

// String

void cg_concat(TTerm dest, TTerm string1, TTerm string2);

void cg_strlen(TTerm dest, TTerm string);

void cg_getchar(TTerm dest, TTerm string, TTerm position);

void cg_setchar(TTerm src_str, TTerm dest_str, TTerm position);

// IFJ BUILT-IN

void cg_ifj_readstr(void);

void cg_ifj_readi32(void);

void cg_ifj_readf64(void);

void cg_ifj_write(TTerm term);

void cg_ifj_i2f(TTerm term);

void cg_ifj_f2i(TTerm term);

void cg_ifj_string(TTerm term);

void cg_ifj_length(TTerm s);

void cg_ifj_concat(TTerm s1, TTerm s2);

void cg_ifj_substring(TTerm s, TTerm i, TTerm j);

// Codegen

void codegen(void);

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- DEFINITIONS ----------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

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

void cg_create_var(TTerm var){
    if(var.type != VARIABLE_T || var.value.var_name == NULL){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("defvar %s%s\n", get_frame(var.frame), var.value.var_name);
}

void cg_init(void){
    printf(".IFJcode24\n");
    cg_create_var(cg_var_retval);
    cg_create_var(cg_var_cmp);
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

void cg_set_type_bool(TTerm var){
    cg_move(var, cg_bool_false_term);
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

void cg_jump_not_neq(TLabel label, TTerm value1, TTerm value2){
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
    printf("int2float");
    cg_two_operands(cg_var_retval, term);
}

void cg_ifj_f2i(TTerm term){
    if(term.type != VARIABLE_T && term.type != FLOAT_T){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    printf("float2int");
    cg_two_operands(cg_var_retval, term);
}

void cg_ifj_string(TTerm term){
    if(term.type != VARIABLE_T && term.type != STRING_T){
        error = ERR_COMPILER_INTERNAL;
    }
    cg_move(cg_var_retval, term);
}

void cg_ifj_length(TTerm s){
    cg_strlen(cg_var_retval, s);
}

void cg_ifj_concat(TTerm s1, TTerm s2){
    if(s1.type != VARIABLE_T || s2.type != VARIABLE_T){
        error = ERR_COMPILER_INTERNAL;
        return;
    }
    cg_concat(cg_var_retval, s1, s2);
}

void cg_ifj_substring(TTerm s, TTerm i, TTerm j){
    cg_create_frame();
    cg_push_frame();

    // Label numbers
    TLabel ret_null = cg_get_new_label();
    TLabel finish = cg_get_new_label();
    // Terms
    TTerm str_len = {.type = VARIABLE_T, .value.var_name = "str_len", .frame = LOCAL};
    TTerm character = {.type = VARIABLE_T, .value.var_name = "character", .frame = LOCAL};
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
    cg_ifj_length(s);
    cg_move(str_len, cg_var_retval);
    cg_jump_gteq(ret_null, i, str_len);
    // j > ifj.length(s)
    cg_jump_gt(ret_null, j, str_len);

    TTerm i_loc = {.type = VARIABLE_T, .value.var_name = "i_loc", .frame = LOCAL};
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
}

// Codegen

void codegen(void){
    cg_init();
    cg_create_frame();
    cg_push_frame();

    TTerm s = {.type = STRING_T, .value.string = "HelloWorld!"};
//    TTerm enter = {.type = STRING_T, .value.string = "\n"};
//    TTerm s1 = {.type = VARIABLE_T, .value.var_name = "sj", .frame = GLOBAL};
//    TTerm s2 = {.type = VARIABLE_T, .value.var_name = "sd", .frame = GLOBAL};
//    TTerm var = {.type = VARIABLE_T, .value.var_name = "i", .frame = GLOBAL};

    TTerm i = {.type = INTEGER_T, .value.int_val = 1};
    TTerm j = {.type = INTEGER_T, .value.int_val = 5};

    cg_ifj_substring(s, i, j);

    cg_ifj_write(cg_var_retval);

    cg_pop_frame();
}
