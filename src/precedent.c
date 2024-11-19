/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xpazurm00, Marek Pazúr
 *
 * @file precedent.c
 */

#include <string.h>
#include <ctype.h>

#include "compiler_error.h"
#include "precedent.h"

#define PT_SIZE 7
#define PUSH_SYMBOL(symbol_id) push(&sym_stack, (symbol) {.id = symbol_id, .token = {.id = TOKEN_DEFAULT, .lexeme = {.array = NULL}}, .type = NON_OPERAND})

/* Simple term precedence table
* Priority depends on operator type and its association
* PT contains basic operators, (), identifiers or literals, $ = end marker
* --------------------
* '>' greater priority, invokes reduction
* '<' less priority, adds shift symbol
* '=' equal priority
* 'e' - as for error
* --------------------
*   row 		  col
*[stack_top_term] [next_token]
*/
static int precedence_table[PT_SIZE][PT_SIZE] = {
//       [ +- | */ |  l |  ( |  ) |  i |  $ ]
	/*0*/{ '>', '<', '>', '<', '>', '<', '>'}, //|+-| ARITHMETIC OPERATORS
	/*1*/{ '>', '>', '>', '<', '>', '<', '>'}, //|*/| * / ARITHMETIC OPERATORS
	/*2*/{ '<', '<', 'e', '<', '>', '<', '>'}, //| l| > < => <= == != LOGIC OPERATORS
	/*3*/{ '<', '<', '<', '<', '=', '<', 'e'}, //| (| (
	/*4*/{ '>', '>', '>', 'e', '>', 'e', '>'}, //| )| )
	/*5*/{ '>', '>', '>', 'e', '>', 'e', '>'}, //| i| IDENTIFIER LITERAL_i32 LITERAL_f64 LITERAL_string
	/*6*/{ '<', '<', '<', '<', 'e', '<', 'e'}, //| $| STACK BOTTOM
//       [  0 |  1 |  2 |  3 |  4 |  5 |  6 ]
};

/* Maps symbol into PT */
int pt_map(symbol sym) {
	int index = 0;

	switch (sym.id) {
	case ADD: 				// +
	case SUB:				// -
		index = I_ADD_SUB; 	// I_ADD_SUB = 0
		break;

	case MUL:				// *
	case DIV: 				// /
		index = I_MUL_DIV;	// I_MUL_DIV = 1
		break;

	case EQ: 				// ==
	case NEQ: 				// !=
	case LS: 				// <
	case GR:	 			// >
	case GRE: 				// >=
	case LSE: 				// <=
		index = I_LOGIC;	// I_LOGIC = 2
		break;

	case LBR:				// (
		index = I_L_BRACKET;// I_L_BRACKET = 3
		break;
	case RBR:				// )
		index = I_R_BRACKET;// I_R_BRACKET = 4
		break;

	case I:					// IDENTIFIER, integer literal, float literal, string literal, null
		index = I_ID_LIT;	// I_ID_ILIT = 5
		break;

	default:				// else
		index = I_END_MARKER;// I_END_MARK = 6
		break;
	}

	return index;
}
/* Creates a symbol out of processed token */
symbol token_to_symbol(token_t term) {
	symbol symbol;
	symbol.id = DEFAULT; // uninitialised
	symbol.token = term; // assign processed token
	symbol.type = NON_OPERAND; // operator or end marker

	switch (term.id) {
	/* Arithmetic OP */
	case TOKEN_ADDITION: 			// +
		symbol.id = ADD;
		break;
	case TOKEN_SUBSTRACTION:		// -
		symbol.id = SUB; 			// I_ADD_SUB = 0
		break;
	case TOKEN_MULTIPLICATION:		// *
		symbol.id = MUL;
		break;
	case TOKEN_DIVISION: 			// /
		symbol.id = DIV;			// I_MUL_DIV = 1
		break;
	/* Logic OP */
	case TOKEN_EQUAL: 				// ==
		symbol.id = EQ;
		break;
	case TOKEN_NOT_EQUAL: 			// !=
		symbol.id = NEQ;
		break;
	case TOKEN_LESS: 				// <
		symbol.id = LS;
		break;
	case TOKEN_GREATER:	 			// >
		symbol.id = GR;
		break;
	case TOKEN_GREATER_EQUAL: 		// >=
		symbol.id = GRE;
		break;
	case TOKEN_LESS_EQUAL: 			// <=
		symbol.id = LSE;			// I_LOGIC = 2
		break;

	case TOKEN_BRACKET_ROUND_LEFT:	// (
		symbol.id = LBR;		// I_L_BRACKET = 3
		break;
	case TOKEN_BRACKET_ROUND_RIGHT:	// )
		symbol.id = RBR;		// I_R_BRACKET = 4
		break;
	/* ID and type literals are i symbol */
	case TOKEN_IDENTIFIER:			// IDENTIFIER
		symbol.id = I;
		symbol.type = CONST_VAR_ID;
		break;
	case TOKEN_LITERAL_I32:			// integer literal
		symbol.id = I;
		symbol.type = I32_T;
		break;
	case TOKEN_LITERAL_F64:			// float literal
		symbol.id = I;
		symbol.type = F64_T;
		break;
	case TOKEN_LITERAL_STRING:		// string literal
		symbol.id = I;
		symbol.type = STRING_T;
		break;
	case TOKEN_KW_NULL:
		symbol.id = I;			// I_ID_ILIT = 5
		symbol.type = NULL_T;
		break;

	default:						// else
		symbol.id = END;			// I_END_MARK = 6
		break;
	}

	return symbol;
}

/* Pushes symbol to stack */
void equal(stack_t *stack, symbol next_symbol) {
	push(stack, next_symbol);

	if (error) {
		fprintf(stderr, "error: equal precedence error");
	}
}

/* Inserts shift symbol on the stack */
void shift(stack_t *stack, symbol next_symbol) {
	insert_shift(stack);	// Inserts shift symbol
	push(stack, next_symbol);	// Pushes next symbol

	if (error) {
		fprintf(stderr, "error: equal precedence error");
	}
}

/* Expression reduction + semantic checks */
void reduction(stack_t *stack, int expresion_length) {
	/* E -> i */
	if (expresion_length == 1) {
		symbol E = pop(stack); // literal, identifier
		pop(stack); 		   // pop shift

		E.id = E_OPERAND;
		push(stack, E);
	} else if (expresion_length == 3) {
		/* E1 op E2 OR (E) */
		symbol E2 = pop(stack); // expression
		symbol op = pop(stack); // operand
		symbol E1 = pop(stack); // expression
		pop(stack);				// pop shift

		/* Create new non-term Expression*/
		symbol E = {.id = E_EXP,
		            .token = {.id = TOKEN_DEFAULT, .lexeme = {.array = NULL}},
		            .type = NON_OPERAND
		           };
		/* Semantics check */

		/* Case when given expression uses E -> (E) rule, otherwise consider thee expression as arithmetic or logic */
		if (E1.id == LBR && (op.id == E_OPERAND || op.id == E_EXP) && E2.id == RBR) {// E1 = (, OP = E, E2 = )
			E.type = op.type; // Pass the data type
			push(stack, E);
			return;
		}

		/* If E1 or E2 is not non-term, throw syntax error */
		if ((E1.id != E_OPERAND && E1.id != E_EXP) || (E2.id != E_OPERAND && E2.id != E_EXP)) {
			error = ERR_SYNTAX;
			return;
		}

		/* Operators can't be applied on string, []u8, null type  */
		if ((E1.type == STRING_T || E2.type == STRING_T) || (E1.type == U8_T || E2.type == U8_T) || (E1.type == NULL_T || E2.type == NULL_T)) {
			printf("error: invalid type used in expression!\n");
			error = ERR_TYPE_COMPATABILITY;
			return;
		}

		/* Further operator and type checking */
		/* ARITHMETIC OPERATORS */
		if (op.id == ADD) {

		} else if (op.id == SUB) {

		} else if (op.id == MUL) {

		} else if (op.id == DIV) {
			if (E2.type == I32_T && E2.token.lexeme.array[0] == '0')
			{
				printf("error: division by zero causes undefined behavior!\n");
				error = ERR_SEMANTIC_OTHER;
				return;
			}
		} /* LOGIC OPERATORS */
		else if (op.id == LS) {

		} else if (op.id == GR) {

		} else if (op.id == LSE) {

		} else if (op.id == GRE) {

		} else if (op.id == EQ) {

		} else if (op.id == NEQ) {

		}
		else {
			printf("error: invalid syntax in expression, rule doesn't exist!\n");
			error = ERR_SYNTAX;
		}

		push(stack, E);
	} else {
		printf("error: invalid subexpression!\n");
		error = ERR_SYNTAX;
	}
}

/* Precedent analysis core function */
/* Checks expression in assignment, condition, return */
int precedent(token_id end_marker) {
	stack_t sym_stack;
	init_stack(&sym_stack);
	PUSH_SYMBOL(END);

	token_t token;
	symbol top_term, next_term;
	bool read_enable = true, expr_solved = false; // Enable/Disable read when needed during expression reduction, break cycle if the expression is solved

	top_term = get_topmost_term(&sym_stack); // $ at the bottom of the stack
	next_term = token_to_symbol((token = get_token())); // anything from the input

	while (expr_solved == false) { // Repeat until $ = $ -> stack top = input term
		int precedence = precedence_table[pt_map(top_term)][pt_map(next_term)]; // get term precedence
		
		switch (precedence) {
		case '=':
			/* Pushes input symbol onto stack top*/
			printf("[=]\n");
			equal(&sym_stack, next_term);
			read_enable = true;
			break;
		case '<':
			/* Puts SHIFT symbol after topmost TERM, pushes input symbol onto stack top*/
			printf("[< - SHIFT]\n");
			shift(&sym_stack, next_term);
			read_enable = true;
			break;
		case '>':
			/* Reduces the expression between '<' SHIFT symbol and stack top included using given RULE */
			printf("[> - REDUCTION]\n");
			reduction(&sym_stack, reduction_count(&sym_stack));
			read_enable = false; /* Will reduce until SHIFT or solved expression */
			break;
		case 'e':
			printf("error: invalid expression (empty expression, operator precedence error)\n");
			error = ERR_SYNTAX;
			break;
		default:
			printf("error: invalid expression (precedence unexpected error)\n");
			error = ERR_COMPILER_INTERNAL;
			break;
		}

		if (error) // Error occured
			break;

		top_term = get_topmost_term(&sym_stack);

		if (read_enable)
			next_term = token_to_symbol((token = get_token()));

		if (end_marker == TOKEN_SEMICOLON ? (top_term.id == END && next_term.id == END) : (top_term.id == END && next_term.id == RBR)) // $ = $ -> topmost term in stack = next term, expression is finally solved $E$
			expr_solved = true;
	}


	/* DEBUG functions */
	print_stack_content(&sym_stack);
	free_stack(&sym_stack);
	if (error == 0)
		printf(GREEN("Expression solved!")"\n");
	else printf(RED("Unable to solve given expression!")"\n");

	/* Expression can be solved, but can still be incorrect with missing end marker */
	if (token.id != end_marker) {
		printf("error: missing semicolon or parenthesis at the end of expression!\n");
		error = ERR_SYNTAX;
	}

	return 0;
}


// symbol stack functions
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