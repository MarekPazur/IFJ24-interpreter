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
#include "syna.h"
#include "symtable.h"
#include "binary_tree.h"

void semantic_analysis(TBinaryTree* AST);

void FunctionSemantics(TNode* func);

void CommandSemantics(TNode* Command, scope_t* current_scope);

void FunctionCallSemantics(TNode *functionCall, scope_t* current_scope);

void main_function_check(TSymtable* globalSymTable);

TSymtable* declaration_var_check(struct TScope scope, char* identifier);

// Helper functions
int formal_param_count(TNode *formal_param);
bool id_defined(struct TScope* scope, char* identifier,  TSymtable** out_sym);
char get_var_type(Type type);
char get_literal_type(int type);

#endif
