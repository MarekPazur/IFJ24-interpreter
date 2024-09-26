/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file compiler_error.c
 */

#include "compiler_error.h"
#include <stdio.h>

unsigned int error = 0;

static const char *error_msg[] = {
	GREEN("SUCCESS")" -  Preklad probehl uspesne",
	RED("ERR_LEXICAL")" - Chyba v programu v ramci lexikalní analyzy (chybna struktura aktualniho lexemu)",
	RED("ERR_SYNTAX") " - Chyba v programu v ramci syntakticke analyzy (chybna syntaxe programu, chybejici hlavicka, ...)",
	RED("ERR_UNDEFINED_IDENTIFIER") " - Semanticka chyba v programu – nedefinovana funkce ci promenna",
	RED("ERR_PARAM_TYPE_RETURN_VAL") " - Semanticka chyba v programu – spatny pocet/typ parametru u volani funkce; spatny typ ci nepovolene zahozeni navratove hodnoty z funkce",
	RED("ERR_IDENTIFIER_REDEF_CONST_ASSIGN") " - Semanticka chyba v programu – redefinice promenne nebo funkce; prirazeni do nemodifikovatelne promenne",
	RED("ERR_RETURN_VALUE") " - Semanticka chyba v programu – chybejici/prebyvajici vyraz v prikazu navratu z funkce",
	RED("ERR_TYPE_COMPATABILITY") " - Semanticka chyba typove kompatibility v aritmetickych, retezcovych a relacnich vyrazech; nekompatibilni typ vyrazu",
	RED("ERR_UNKNOWN_TYPE") " - Semanticka chyba odvození typu – typ promenne neni uveden a nelze odvodit od pouziteho vyrazu.",
	RED("ERR_UNUSED_VAR") " - Semanticka chyba nevyuzite promenne v jejim rozsahu platnosti; modifikovatelna promenna bez moznosti zmeny po jeji inicializaci",
	RED("ERR_SEMANTIC_OTHER") " - Ostatni semanticke chyby",
	RED("ERR_COMPILER_INTERNAL") " - Interni chyba prekladace (chyba alokace pameti, ...)"
};

void print_error(unsigned int err) {

	unsigned int size = sizeof(error_msg) / sizeof(char *); 

	fprintf(stderr, RED_BOLD("error") "(%02d): %s\n", err, error_msg[err == ERR_COMPILER_INTERNAL ? (size - 1) : err]);
}