#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "compiler_error.h"
#include "stack.h"
#include "token.h"

/* STACK DEBUG TOGGLE */
static bool debug_mode = false; // Using boolean for more clean looking debug function

/* STACK INITIALISATION */
void init_stack(stack_t* stack) {
    stack->top = NULL;
}

/* CREATE AND INIT NEW ITEM */
struct item* alloc_item(symbol symbol) {
    struct item *new_item = (struct item *) malloc(sizeof(struct item));

    if (new_item) {
        new_item->symbol = symbol;
        new_item->next = NULL;
    }

    return new_item;
}

/* STACK PUSH PROCEDURE */
void push(struct stack *s, symbol symbol) {
    struct item *new_item = alloc_item(symbol);

    if (new_item) { // Succesful allocation
        new_item->next = s->top;
        s->top = new_item;

        if (debug_mode) {
            printf("[STACK INFO]: Pushed symbol: ");
            print_symbol_info(symbol);
            //print_stack_state(s);
        }
    } else {
        fprintf(stderr, "error: stack item allocation has failed!\n");
        error = ERR_COMPILER_INTERNAL;
    }
}

/* STACK POP FUNCTION */
symbol pop(struct stack *s) {
    symbol symbol;

    if (s->top != NULL) {
        struct item *top_item = s->top;
        symbol = top_item->symbol;

        s->top = top_item->next; // NULL or next item

        free(top_item);  // Free item from memory

        // Debug output
        if (debug_mode) {
            printf("[STACK INFO]: Popped token ID: ");
            print_symbol_info(symbol);
            //print_stack_state(s);
        }

        return symbol;
    }

    /* Trying to pop from an empty stack, throws error */
    fprintf(stderr, "error: trying to pop an empty stack!\n");
    symbol.id = ERROR;
    error = ERR_COMPILER_INTERNAL;
    return symbol;
}

/* Returns top item without removing it */
symbol get_top(struct stack *s) {
    return s->top->symbol;
}

symbol get_topmost_term(struct stack *s) {
    item_t *topmost = s->top;

    symbol sym;
    sym.id = ERROR;

    while (topmost != NULL) {
        sym = topmost->symbol;
        /* symbol = < (reduce) OR symbol = E */
        if (sym.id == R || sym.id == E_EXP || sym.id == E_OPERAND) {
            topmost = topmost->next; // if sym is either < or E, go further
        } else break;
    }

    if (sym.id == ERROR) {
        fprintf(stderr, "error: There is no terminal in this stack!\n");
        error = ERR_COMPILER_INTERNAL;
    }

    if (debug_mode) {
        printf("TOPMOST TERMINAL IS --> ");
        print_symbol_info(sym);
    }

    return sym;
}

/* Count of symbols to be reduced, can be either 1 or 3 */
int reduction_count(struct stack *s) {
    int count = 0;

    item_t *top = s->top;
    symbol sym;

    while (top != NULL) {
        sym = top->symbol;

        if (sym.id != R) {
            top = top->next;
            ++count;
        } else break;
    }

    if (count != 1 && count != 3) {
        fprintf(stderr, "error: Expression error, invalid count of symbols to reduce!\n");
        return -1;
    }

    return count;
}
/* Inserts '<' shift into stack */
void insert_shift(struct stack *s) {
    /* Shift is inserted BEFORE top non-terminal and AFTER topmost terminal */
    /* example: '$<(E' --> '$<(<E*' */
    // Shift symbol to be inserted
    symbol shift = {
        .id = R, // '<'
        .token = {.id = TOKEN_DEFAULT, .lexeme = {.array = NULL}},
        .type = NON_OPERAND
    };
    /* If top item is non-terminal */
    if (get_top(s).id != get_topmost_term(s).id) { // stack top symbol is topmost terminal
        item_t *item = alloc_item(shift);
        if (item) {
            /* Insert < between T and E (T<E) */
            item->next = s->top->next; //T <-- '<'
            s->top->next = item;       //T <-- '<' <-- E
        } else {
            /* malloc error */
            fprintf(stderr, "error: stack item allocation has failed in insert_shift!\n");
            error = ERR_COMPILER_INTERNAL;
        }
    } else push(s, shift);// If top item is terminal
}

/* Frees stack content from memory */
void free_stack(struct stack *s) {
    while (s->top) {
        pop(s);
    }
}

/* Prints stack content */
void print_stack_content(struct stack *s) {
    printf("-----------------------------\n"
        "[STACK INFO]: Stack content:\n");

    struct item *top_item = s->top;

    if (top_item == NULL) {
        printf("Stack is empty!\n");
    } else {
        while (top_item) {
            print_symbol_info(top_item->symbol);
            top_item = top_item->next;
            putchar('\n');
        }
        putchar('\n');
    }
}