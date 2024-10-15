#include "stack.h"
#include "token.h"
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
    }
}

/* STACK POP FUNCTION */
token_t pop(struct stack *s) {
    if (!s->last) { //Error if stack has no items, probably won't be an issue
        exit(1);
    }
    struct item *top_item = s->last;
    token_t token = top_item->token;
    s->last = top_item->next;
    free(top_item);  // Free item from memory
    return token;
}

/* FREE STACK + ITEMS FUNCTION */
void free_stack(struct stack *s) {
    while (s->last) {
        pop(s);
    }
    free(s);
}
