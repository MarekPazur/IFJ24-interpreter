/* compiler_error.h */

#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H

typedef enum ERROR_CODE {
	SUCCESS = 0, /* Preklad probehl uspesne */
	ERR_LEXICAL = 1, /* Chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému) */
	ERR_SYNTAX = 2, /* Chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu, chybějící hlavička, atp.) */
	ERR_UNDEFINED_IDENTIFIER = 3, /* Sémantická chyba v programu – nedefinovaná funkce či proměnná */
	ERR_PARAM_TYPE_RETURN_VAL = 4, /* Sémantická chyba v programu – špatný počet/typ parametrů u volání funkce; špatný typ či nepovolené zahození návratové hodnoty z funkce */
	ERR_IDENTIFIER_REDEF_CONST_ASSIGN = 5, /* sémantická chyba v programu – redefinice proměnné nebo funkce; přiřazení do nemodifikovatelné proměnné */
	ERR_RETURN_VALUE_EXPRESSION = 6, /* sémantická chyba v programu – chybějící/přebývající výraz v příkazu návratu z funkce */
	ERR_TYPE_COMPATABILITY = 7, /*  sémantická chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech; nekompatibilní typ výrazu (např. při přiřazení) */
	ERR_UNKNOWN_TYPE = 8, /* Sémantická chyba odvození typu – typ proměnné není uveden a nelze odvodit od použitého výrazu. */
	ERR_UNUSED_VAR = 9, /* Sémantická chyba nevyužité proměnné v jejím rozsahu platnosti; modifikovatelná proměnná bez možnosti změny po její inicializaci */
	ERR_SEMANTIC_OTHER = 10, /* Ostatní sémantické chyby */
	ERR_COMPILER_INTERNAL = 99 /* Interní chyba překladače tj. neovlivněná vstupním programem (např. chyba alokace paměti atd.) */
} error_code;

#define RED_BOLD(msg) "\033[1;31m" msg "\033[0;37m"
#define RED(msg) "\033[0;31m" msg "\033[0;37m"
#define GREEN(msg) "\033[0;32m" msg "\033[0;37m" 

void print_error(error_code err);

#endif 