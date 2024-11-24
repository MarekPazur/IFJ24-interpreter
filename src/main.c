/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xpazurm00, Marek Pazúr
 * @author xglosro00, Robert Glos
 * @author xukropj00, Jan Ukropec
 * @author xtomasp00, Patrik Tomaško
 * 
 * @file main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "compiler_error.h"
#include "lexer.h"
#include "dynamic_array.h"
#include "syna.h"
#include "precedent.h"
#include "binary_tree.h"
#include "token.h"

/**
 *@TODO
 *	semantic analyser
 *	code generator
*/



int main (void) {
	/* Init. of scanner struct */
	init_scanner();

	if (error) {
		print_error(error);
	}

	token_t test = {.id = TOKEN_DEFAULT};
	init_parser(test);

	if (error) {
		print_error(error);
	}

	return error;
}