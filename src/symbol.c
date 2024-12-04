/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xpazurm00, Marek Pazúr
 *
 * @file symbol.c
 */
#include <stdio.h>
#include "symbol.h"
#include "compiler_error.h"

/* Prints information about symbol structure and its content */
void print_symbol_info(symbol symbol) {
    char *symbol_to_string[] = {
        "DEFAULT - uninitialised symbol!",
        "ERROR - invalid symbol or internal error occured!",
        /* Terms */
        YELLOW_BOLD("+ ADD"),                           // + addition
        YELLOW_BOLD("- SUB"),                           // - substraction
        YELLOW_BOLD("* MUL"),                           // * multiplication
        YELLOW_BOLD("/ DIV"),                           // / division
        YELLOW_BOLD("< LS"),                            // < less
        YELLOW_BOLD("> GR"),                            // > greater
        YELLOW_BOLD("<= LSE"),                          // <= less-equal
        YELLOW_BOLD(">= GRE"),                          // >= greater-equal
        YELLOW_BOLD("== EQ"),                           // == equal
        YELLOW_BOLD("!= NEQ"),                          // != inequal
        YELLOW_BOLD("( LBR"),                           // ( left bracket
        YELLOW_BOLD(") RBR"),                           // ) right bracket
        YELLOW_BOLD("i I (id, i32, f64, string, null)"),// IDENTIFIER,LITERAL_i32,LITERAL_f64,LITERAL_string,null
        /* Non-terms */
        "< SHIFT",                                      // '<' SHIFT/REDUCTION
        "E E_EXP",                                      // non-term solved expression
        "E E_OPERAND",                                  // non-term solved operand
        /* $ End marker */
        YELLOW_BOLD("$ END")                            // $ end marker
    };

    char *datatype_to_string[] = {
        "NON_OPERAND",
        "[]u8",     // : []u8 doesn't have literal on its own
        "i32",    // : i32
        "f64",    // : f64
        "string", // : "string"
        "null"  // : null
    };

    printf("%s", symbol_to_string[symbol.id]);

    if (symbol.id == I)
        printf(" -- data type: %s", datatype_to_string[symbol.type]);
}