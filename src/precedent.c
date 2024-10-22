/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file precedent.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "precedent.h"
#include "token.h"

#define PT_SIZE 13

int precedence_table[PT_SIZE][PT_SIZE] = {
/* Priority depends on op type and association */
/* PT contains basic operators, $ = end marker */

//   +  -  *  / == !=  <  > >= <= (  )  $
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// +
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// -
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// *
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// /
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// ==
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// !=
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// <
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// >
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// >=
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// <=
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// (
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},// )
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}// $
};

int pt_map(token_t stack_top, token_t token_next) {
	int col = stack_top.id;
	int row = token_next.id;

	col++;row++; /*DELETE LATER, C Compiler will cry if deleted now*/

	return 0;
}

int precedent(void) { 
	return 0;
}