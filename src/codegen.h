/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file codegen.h
 */

#ifndef CODEGEN_H
#define CODEGEN_H

typedef enum frame{
    GLOBAL = 0,
    LOCAL = 1,
    TEMPORARY = 2,
} Frame;

//typedef TTerm_type;

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
} TTerm;

//

void cg_init(void);

void cg_create_var(char* var_name);

void cg_create_frame(void);

void cg_push_frame(void);

// IFJ BUILT-IN

void cg_ifj_readstr(char* var_name);

void cg_ifj_write(TTerm term);

#endif
