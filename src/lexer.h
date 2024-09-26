/* lexer.h */

#ifndef LEXER_H
#define LEXER_H

typedef enum token_id {
	TOKEN_TEST = 1
} token_id;

/* structure containing data about tokens */
typedef struct token {
	token_id id;
} token_t;

/* Final state machine, fetches token for syntax analyser */
token_t* get_token(void);

#endif