/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xpazurm00, Marek Pazúr
 *
 * @file precedent.c
 */

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "compiler_error.h"

#include "precedent.h"
#include "semantic.h"
#include "token.h"
#include "binary_tree.h"
#include "syna.h"

#define PT_SIZE 7

#define OPERATOR 1
#define OPERAND 2

#define PUSH_SYMBOL(symbol_id) push(&sym_stack, (symbol) {.id = symbol_id, .token = {.id = TOKEN_DEFAULT, .lexeme = {.array = NULL}}, .type = NON_OPERAND})

bool precedence_debug = false;
int relative_op_count = 0;

struct TScope current_symtable_scope;

TData retrieved_data;

TSymtable* identifier_residence;

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
		relative_op_count++;
		break;
	case TOKEN_NOT_EQUAL: 			// !=
		symbol.id = NEQ;
		relative_op_count++;
		break;
	case TOKEN_LESS: 				// <
		symbol.id = LS;
		relative_op_count++;
		break;
	case TOKEN_GREATER:	 			// >
		symbol.id = GR;
		relative_op_count++;
		break;
	case TOKEN_GREATER_EQUAL: 		// >=
		symbol.id = GRE;
		relative_op_count++;
		break;
	case TOKEN_LESS_EQUAL: 			// <=
		symbol.id = LSE;
		relative_op_count++;			// I_LOGIC = 2
		break;

	case TOKEN_BRACKET_ROUND_LEFT:	// (
		symbol.id = LBR;		// I_L_BRACKET = 3
		break;
	case TOKEN_BRACKET_ROUND_RIGHT:	// )
		symbol.id = RBR;		// I_R_BRACKET = 4
		break;
	/* ID and type literals are i symbol */
	case TOKEN_IDENTIFIER:			// IDENTIFIER
	
	        //Checking the existance of the variable and changing it's is_used value to true
	        
	        if((identifier_residence = declaration_var_check(current_symtable_scope, term.lexeme.array)) == NULL){
                    error = ERR_UNDEFINED_IDENTIFIER;
                    return symbol;
                }
                
                symtable_get_data(identifier_residence, term.lexeme.array, &retrieved_data);
                
                retrieved_data.variable.is_used = true;
                
                if(!symtable_insert(identifier_residence, term.lexeme.array, retrieved_data)){
                    error = ERR_COMPILER_INTERNAL;
                    return symbol;
                }
	
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

/* Gets node type for AST */
int get_node_type(int term, int select) {
	node_type type;

	if (select == OPERAND)
		switch (term) {
		case I32_T:
			type = INT;
			break;
		case STRING_T:
			type = STR;
			break;
		case F64_T:
			type = FL;
			break;
		case NULL_T:
			type = NULL_LITERAL;
			break;
		default:
			type = VAR_CONST;
			break;
		}
	else if (select == OPERATOR)
		switch (term) {
		case ADD:
			type = OP_ADD;
			break;
		case SUB:
			type = OP_SUB;
			break;
		case MUL:
			type = OP_MUL;
			break;
		case DIV:
			type = OP_DIV;
			break;
		case EQ:
			type = OP_EQ;
			break;
		case NEQ:
			type = OP_NEQ;
			break;
		case GR:
			type = OP_GT;
			break;
		case GRE:
			type = OP_GTE;
			break;
		case LS:
			type = OP_LS;
			break;
		case LSE:
			type = OP_LSE;
			break;
		default:
			break;
		}

	return type;
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

/* Expression reduction */
void reduction(stack_t *stack, int expresion_length) {
	/* E -> i */
	if (expresion_length == 1) {
		symbol E = pop(stack); // literal, identifier
		pop(stack); 		   // pop shift

		/* Create operand node */
		E.node = create_node(get_node_type(E.type, OPERAND));

		if (E.node->type == VAR_CONST)
			E.node->data.nodeData.value.identifier = E.token.lexeme.array;
		else
			E.node->data.nodeData.value.literal = E.token.lexeme.array;

		E.id = E_OPERAND;
		push(stack, E);
	}
	/* E1 op E2 OR (E) */
	else if (expresion_length == 3) {

		symbol E2 = pop(stack); // LHS: expression, can be bracket
		symbol op = pop(stack); // MID: operand, expression if its in (E) form
		symbol E1 = pop(stack); // RHS: expression, can be bracket
		pop(stack);				// pop shift <

		/* Create new non-term E for processed expression */
		symbol E = {.id = E_EXP,
		            .token = {.id = TOKEN_DEFAULT, .lexeme = {.array = NULL}},
		            .type = NON_OPERAND,
		            .node = NULL
		           };

		/* Case when given expression uses E -> (E) rule, otherwise consider thee expression as arithmetic or logic */
		if (E1.id == LBR && (op.id == E_OPERAND || op.id == E_EXP) && E2.id == RBR) {// E1 = (, OP = E, E2 = )
			E.type = op.type; // Pass the data type
			E.node = op.node; // Propagate node
			push(stack, E);

			return;
		}

		/* Else Create operator node, set operands as children */
		E.node = create_node(get_node_type(op.id, OPERATOR));
		E.node->left = E1.node;
		E.node->right = E2.node;

		/* If E1 or E2 is not non-term, throw syntax error */
		if ((E1.id != E_OPERAND && E1.id != E_EXP) || (E2.id != E_OPERAND && E2.id != E_EXP)) {
			error = ERR_SYNTAX;
			return;
		}

		/* Operators can't be applied on string, []u8, null type  */
/*		if ((E1.type == STRING_T || E2.type == STRING_T) || (E1.type == U8_T || E2.type == U8_T) || (E1.type == NULL_T || E2.type == NULL_T)) {
			printf("error: invalid type used in expression!\n");
			error = ERR_TYPE_COMPATABILITY;
			return;
		}*/

		push(stack, E);
	} else {
		printf("error: invalid subexpression!\n");
		error = ERR_SYNTAX;
	}
}

/** Private function that wraps basic get_token function
* Fetches a token from the buffer or from input
* If the token buffer is not empty it retrieves a token from it
* Otherwise it fetches a token from input
*/
token_t fetch_token(t_buf* token_buffer) {
	/* If the token buffer has tokens, get them from the buffer*/
	if (!is_empty_t_buf(token_buffer))
		return get_t_buf(token_buffer);

	/* If the token buffer is empty, get tokens from input */
	return get_token();
}

/* Precedent analysis core function */
/* Checks expression in assignment, condition, return */
TNode* precedent(t_buf* token_buffer, token_id end_marker, struct TScope cur_scope) {
	relative_op_count = 0;
    current_symtable_scope = cur_scope;
	stack_t sym_stack;
	init_stack(&sym_stack);
	PUSH_SYMBOL(END);

	token_t token;
	symbol top_term, next_term;
	bool read_enable = true, expr_solved = false; // Enable/Disable read when needed during expression reduction, break cycle if the expression is solved

	top_term = get_topmost_term(&sym_stack); // $ at the bottom of the stack
	next_term = token_to_symbol((token = fetch_token(token_buffer))); // anything from the input
	
	if(error)
	    return NULL;

	while (expr_solved == false) { // Repeat until $ = $ -> stack top = input term
		int precedence = precedence_table[pt_map(top_term)][pt_map(next_term)]; // get term precedence

		switch (precedence) {
		case '=':
			/* Pushes input symbol onto stack top*/
			if (precedence_debug)
				printf("[=]\n");

			equal(&sym_stack, next_term);
			read_enable = true;
			break;
		case '<':
			/* Puts SHIFT symbol after topmost TERM, pushes input symbol onto stack top*/
			if (precedence_debug)
				printf("[< - SHIFT]\n");
			
			shift(&sym_stack, next_term);
			read_enable = true;
			break;
		case '>':
			/* Reduces the expression between '<' SHIFT symbol and stack top included using given RULE */
			if (precedence_debug)
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

		if (read_enable) {
			next_term = token_to_symbol((token = fetch_token(token_buffer)));
			
			if (relative_op_count > 1) {
				printf("error: too many relative operators\n");
				error = ERR_SYNTAX;
			}
		}
			
		if (error) {
			break;
		}

		if (end_marker == TOKEN_SEMICOLON ? (top_term.id == END && next_term.id == END) : (top_term.id == END && next_term.id == RBR)) // $ = $ -> topmost term in stack = next term, expression is finally solved $E$
			expr_solved = true;
	}


	if (error) {
		printf(RED("error")": Expression syntax error\n");
		return NULL;
	}

	/* Final expression tree, to calculate use POST-ORDER TRAVERSAL */
	TNode *expression_root = get_top(&sym_stack).node;  	

	/* DEBUG functions */
	if (precedence_debug) {
		print_stack_content(&sym_stack);
	}

	free_stack(&sym_stack);	
	
	//BT_print_tree(expression_root);

	/* Expression can be solved, but can still be incorrect with missing end marker */
	if (token.id != end_marker) {
		printf(RED("error")": missing semicolon or parenthesis at the end of expression!\n");
		error = ERR_SYNTAX;
	}

	return expression_root;
}


// Symbol stack functions
/**
 * Stack debug toggle
 */
static bool debug_mode = false; // Using boolean for more clean looking debug function

/**
 * Initialises the stack
 */
void init_stack(stack_t* stack) {
	stack->top = NULL;
}

/**
 * Allocates new item and initialises it
 */
struct item* alloc_item(symbol symbol) {
	struct item *new_item = (struct item *) malloc(sizeof(struct item));

	if (new_item) {
		new_item->symbol = symbol;
		new_item->next = NULL;
	}

	return new_item;
}

/**
 * Pushes given item on the top of the stack
 */
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

/**
 * Removes item from the top of the stack and returns it
 */
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

/**
 * Returns top item without removing it
 */
symbol get_top(struct stack *s) {
	return s->top->symbol;
}

/**
 * Returns topmost TERM item, doesn't have to be on top of the stack!
 */
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

/**
 * Finds the number of characters to determine the type of expression reduction
 * Can be 1 or 3: E->i, E->(E), E-> E op E
 */
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

/**
 * Inserts shift symbol BEFORE the topmost TERM
 */
void insert_shift(struct stack *s) {
	/* Shift is inserted BEFORE top non-term and AFTER topmost term */
	/* example: '$<(E' --> '$<(<E*' */
	// Shift symbol to be inserted
	symbol shift = {
		.id = R, // '<'
		.token = {.id = TOKEN_DEFAULT, .lexeme = {.array = NULL}},
		.type = NON_OPERAND
	};
	/* If top item is non-term */
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

/**
 * Clears stack content, frees allocated memory
 */
void free_stack(struct stack *s) {
	while (s->top) {
		pop(s);
	}
}

/**
 * Debug function, prints stack content at the time
 */
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
