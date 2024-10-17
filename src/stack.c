#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"
#include "token.h"

/* DEBUG STATE */
static bool debug_mode = false; // Using boolean for more clean looking debug function

/* STACK DEBUG TOGGLE */
void s_debug() {
    debug_mode = !debug_mode; // Toggle the debug mode
}

/* Current stack output */
void print_stack_state(struct stack *s) {
    printf("Current stack state: ");
    struct item *current = s->last;
    while (current) {
        printf("%d ", current->token.id); // Print the integer value of the token ID, I couldn't find a way to display it simply as the enum names outside of using a switch
        current = current->next;
    }
    printf("\n");
}

/* STACK INIT FUNCTION */
struct stack* init_stack() {
    return (struct stack *)calloc(1, sizeof(struct stack));
}

/* ITEM INIT FUNCTION */
struct item* init_item(token_t token) {
    struct item *new_item = (struct item *)calloc(1, sizeof(struct item));
    if (new_item) {
        new_item->token = token;
        new_item->next = NULL;
    }
    return new_item;
}

/* STACK PUSH FUNCTION */
void push(struct stack *s, token_t token) {
    struct item *new_item = init_item(token);
    if (new_item) {
        new_item->next = s->last;
        s->last = new_item;

        if (debug_mode) {
            printf("Pushed token ID: %d\n", token.id); // Print the integer value of the pushed token ID
            print_stack_state(s);
        }
    }
}

/* STACK POP FUNCTION */
token_t pop(struct stack *s) {
    if (!s->last) { // Error if stack has no items
        exit(1);
    }
    struct item *top_item = s->last;
    token_t token = top_item->token;
    s->last = top_item->next;
    free(top_item);  // Free item from memory

    // Debug output
    if (debug_mode) {
        printf("Popped token ID: %d\n", token.id); // Print the integer value of the popped token ID
        print_stack_state(s);
    }

    return token; //returns the popped token so there's no reason to save the current token seperately before popping in syntatic analysis
}

/* FREE STACK + ITEMS FUNCTION */
void free_stack(struct stack *s) {
    while (s->last) {
        pop(s);
    }
    free(s);
}
