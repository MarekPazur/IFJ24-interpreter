/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include "compiler_error.h"

int main(void) {

	char c = 0;

	while(c != EOF) {
		c = getc(stdin);
		printf("%c",c);
	}

	return 0;
}