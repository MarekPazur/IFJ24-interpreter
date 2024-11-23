/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xpazurm00, Marek Pazúr
 * 
 * @file semantic.c
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "token.h"
#include "symtable.h"
#include "binary_tree.h"

void semantic_analysis(TBinaryTree* AST);

void FunctionSemantics(TNode* func);

void CommandSemantics(TNode* Command);

TSymtable* declaration_var_check(struct TScope scope, char* identifier);



#endif
