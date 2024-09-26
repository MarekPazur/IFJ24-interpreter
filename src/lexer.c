/* lexer.c */

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "compiler_error.h"

token_t Token;

token_t* get_token(void) {
	//todo	
	printf("token\n");

	return &Token;
}
