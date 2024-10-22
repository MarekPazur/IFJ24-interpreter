/** 
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 * 
 * @author xpazurm00, Marek Pazúr
 * 
 * @file precedent.h
 */

#ifndef PRECEDENT_H
#define PRECEDENT_H

#include "token.h"

int pt_map(token_t stack_top, token_t token_next);

int precedent(void);

#endif