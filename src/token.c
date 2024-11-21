/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file token.h
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "token.h"
#include "compiler_error.h"
#include "lexer.h"

/**
* Initialises token buffer
*/
void init_t_buf (t_buf *buf) {
	buf->head = NULL; // Start of the queue
	buf->tail = NULL; // End of the queue
}

/**
* Returns true if empty
*/
int is_empty_t_buf (t_buf *buf) {
	if (buf->head)
		return 0;
	else
		return 1;
}

/**
* Enqueues given token to the queue
*/
void enqueue_t_buf (t_buf *buf, token_t token) {
	t_buf_item *new;

	if ((new = (t_buf_item*) malloc(sizeof(t_buf_item))) == NULL) {
		printf(RED_BOLD("error")": Token buffer resource allocation failure\n");
		error = ERR_COMPILER_INTERNAL;
		return;
	}

	*new = (t_buf_item) {
		.token = token,
		.next = NULL
	};

	if (buf->head == NULL) {
		buf->tail = new;
		buf->head = buf->tail;
	} else {
		buf->tail->next = new;
		buf->tail = buf->tail->next;
	}
}

/*
* Removes first token in the queue, shifts queue to the next token
*/
token_t get_t_buf (t_buf *buf) {
	token_t token;

	if (buf == NULL || buf->head == NULL) {
		printf(RED_BOLD("error")": Token buffer is empty or invalid pointer to buffer\n");
		error = ERR_COMPILER_INTERNAL;
		return (token_t) {.id = TOKEN_ERROR};
	}

	t_buf_item* ptr = buf->head;

	token = ptr->token;

	buf->head = ptr->next; // next or NULL

	free(ptr);

	return token;
}

/**
* Frees token queue buffer
*/
void free_t_buf (t_buf *buf) {
	t_buf_item* ptr = buf->head, *tmp;

	while (ptr) {
		tmp = ptr->next; // next or NULL

		free(ptr);		// Free current item

		ptr = tmp;		// Go next
	}

	buf->head = NULL;
	buf->tail = NULL;
}

/*
* Prints the content of the queue
*/
void print_t_buf (t_buf *buf) {
	t_buf_item* ptr = buf->head;

	if (ptr == NULL)
		printf("Queue is empty!\n");
	else
		printf("Queue content:\n");

	while(ptr) {
		print_token(ptr->token);
		putchar('\n');
		ptr = ptr->next;
	}
}

/**
* Concatenates two strings with '.' between: ifj, function --> 'ifj.function'
* Used for functions composed of two parts with namespace/access operator between
*/
char *func_id_concat(char *lex_prefix, char *lex_suffix) {
	if (lex_prefix == NULL || lex_suffix == NULL)
		return NULL;

	char *new_lexeme = NULL;
	size_t size = strlen(lex_prefix) + strlen(".") +strlen(lex_suffix);

	if ((new_lexeme = (char*) malloc(size + 1)) == NULL) {
		error = ERR_COMPILER_INTERNAL;
		printf(RED("error")": Lexeme concatenation resource allocation failure\n");
		return NULL;
	}

	strcpy(new_lexeme, lex_prefix);
	strcat(new_lexeme, ".");
	strcat(new_lexeme, lex_suffix);

	return new_lexeme;
}