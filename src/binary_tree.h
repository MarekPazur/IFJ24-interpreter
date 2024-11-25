/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file binary_tree.h
 */

#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include "symtable.h"

typedef struct binary_tree TBinaryTree;
typedef struct node TNode;
typedef struct node_data node_data;

/***************************/
/*		AST utilities	   */
/***************************/
typedef enum node_type {
	PROGRAM,			// Root node

	PROLOGUE,			// Import
	FN,					// Functions

	COMMAND,			// Wrapper for statements
	METADATA,			// 
	NULL_REPLACEMENT,	// |not_null_variable|
	
	VAR_DECL,			// declaration of a variable
	CONST_DECL,			// declaration of a constant
	
	ASSIG,				// assignment node

	BODY,				// "headless" body {}
	WHILE,				// while loop
	IF,					// if condition
	ELSE,				// else 
	
	INT,				// integer i32 datatype LITERAL
	FL,					// float f64 datatype	LITERAL
	U8,					// U8 SLICE
	STR,				// string datatype		LITERAL
	VAR_CONST,			// variable/constant, u8[] can be only in this form, not on its own

	EXPRESSION,			// node containing expression
	OP_ADD,				// +
	OP_SUB,				// -
	OP_MUL,				// *
	OP_DIV,				// /
	OP_EQ,				// ==
	OP_NEQ,				// !=
	OP_GT,				// >
	OP_LS,				// <
	OP_GTE,				// >=
	OP_LSE,				// <=

	FUNCTION_CALL,		// foo(param_list)

	RETURN 				// return (expression)
} node_type;

typedef enum return_type {
	VOID_TYPE,
	I32,
	F64,
	U8_SLICE,
	NULL_TYPE
} return_type;

/* LLIST STRUCTURE */
typedef struct linked_list {
    struct item_ll *first;
    struct item_ll *last;
    struct item_ll *active;
} linked_list_t;

/* ITEM STRUCTURE */
typedef struct item_ll {
    char* identifier;    // Symbol for precedence
    struct item_ll *next;
    struct item_ll *prev;
} item_ll_t;

// Llist functions
/* Initializes llist */
void init_llist(linked_list_t* llist);

/* Inserts an item into the llist */
bool insert_llist(linked_list_t* llist, char* inserted);

/* Sets the active item to the first */
bool set_first_llist(linked_list_t* llist);

/* Sets the active item to the last */
bool set_last_llist(linked_list_t* llist);

/* Sets the active item to the next item */
bool next_llist(linked_list_t* llist);

/* Sets the active item to the previous item */
bool prev_llist(linked_list_t* llist);

/* Sets the active item to the next item */
bool get_value_llist(linked_list_t* llist, char** value);

/* Frees all the items of the llist */
void free_llist(linked_list_t* llist);

/*************************/
/*      BINARY TREE      */
/*************************/

struct node_data {
    union {
    	/* ROOT/PROGRAM */
        struct {
            TSymtable *globalSymTable;
        } program;

        /* FUNCTION DEFINITION */
        struct {
            char *identifier;
            return_type type;
            TSymtable *scope;
            linked_list_t param_identifiers;
        } function;

        /* WHILE-IF-ELSE-BODY */
        struct {
            struct TScope *current_scope;
            bool is_nullable;
            char* null_replacement;
        } body;
        
        /* FUN. CALL, VAR/CONST DECL., ASSIGNMENT */
        struct {
            char *identifier;
            bool is_disposeable;
        } identifier;

        /* LITERALS OR USAGE OF VARIABLE AS VALUE */
        struct {
            char *literal;
            char *identifier;
        } value;
    } nodeData;
};

struct node{
    TNode* parent;
    TNode* left;
    TNode* right;

    node_type type;
    node_data data;
};

struct binary_tree{
    TNode* root;
    TNode* active;
};

/**
 * Allocates memory for binary tree and initializes it
 * \return New binary tree | NULL in case of a memory allocation error
 */
TBinaryTree* BT_init(void);

TNode* create_node(node_type type);

/**
 * Checks if binary tree is active. Use everytime before using operation with an active node.
 * \param BT Binary tree
 * \return True: BT is active, False: BT is not active | BT is NULL
 */
bool BT_is_active(TBinaryTree* BT);

/**
 * Set the active node to the root node
 * \param BT Binary tree
 */
void BT_set_root(TBinaryTree* BT);

/**
 * Checks if the active has a parent node
 * \return True: It does have., False: It does not have. | BT is NULL or not active.
 */
bool BT_has_parent(TBinaryTree* BT);

/**
 * Checks if the active has a right node
 * \return True: It does have., False: It does not have. | BT is NULL or not active.
 */
bool BT_has_left(TBinaryTree* BT);

/**
 * Checks if the active has a left node
 * \return True: It does have., False: It does not have. | BT is NULL or not active.
 */
bool BT_has_right(TBinaryTree* BT);

/**
 * Sets active node to its parent if BT is active
 * \param BT Binary tree
 */
void BT_go_parent(TBinaryTree* BT);

/**
 * Sets active node to its left child if BT is active
 * \param BT Binary tree
 */
void BT_go_left(TBinaryTree* BT);

/**
 * Sets active node to its right child if BT is active
 * \param BT Binary tree
 */
void BT_go_right(TBinaryTree* BT);

/**
 * Inserts node as a left child of the active node.
 * \param BT Binary tree
 * \param data
 * \return True: Succes, False: Memory allocation error | BT is NULL | BT is not active | Active node has a left node
 */
bool BT_insert_left(TBinaryTree* BT, node_type type);

/**
 * Inserts node as a right child of the active node.
 * \param BT Binary tree
 * \param data
 * \return True: Succes, False: Memory allocation error | BT is NULL | BT is not active | Active node has a right node
 */
bool BT_insert_right(TBinaryTree* BT, node_type type);

/**
 * Checks if the binary tree has a root.
 * \param BT Binary tree
 * \return True: It does have. False: It does not have. | BT is NULL.
 */
bool BT_has_root(TBinaryTree* BT);

/**
 * Inserts a root node into a tree if it already does not have one.
 * \param BT Binary tree
 * \param data
 * \return True: Success, False: Memory allocation error | BT is NULL | BT already has a root node
 */
bool BT_insert_root(TBinaryTree* BT, node_type type);

/**
 * Frees all nodes
 * \param BT
 */
void BT_free_nodes(TBinaryTree* BT);

/**
 * Frees the left subtree of the active node
 * \param BT Binary tree
 */
void BT_free_left_subtree(TBinaryTree* BT);

/**
 * Frees the right subtree of the active node
 * \param BT Binary tree
 */
void BT_free_right_subtree(TBinaryTree* BT);

/**
 * Frees left and right subtrees of the active node and the active node too
 * \param BT Binary tree
 */
void BT_free_active_tree(TBinaryTree* BT);

/**
 *
 * \param BT Binary tree
 * \param[out] data_out Data output variable
 * \return True: Success, False: NULL pointer argument | Binary tree is not active
 */
bool BT_get_data(TBinaryTree* BT, node_data* data_out);

/**
 *
 * \param BT Binary tree
 * \param[out] type Type output variable
 * \return True: Success, False: NULL pointer argument | Binary tree is not active
 */
bool BT_get_node_type(TBinaryTree* BT, node_type* type);

/**
 *
 * \param BT Binary tree
 * \param[out] data_out Data output variable
 * \return True: Success, False: NULL pointer argument | Binary tree is not active | Active node does not have a left node
 */
bool BT_get_data_left(TBinaryTree* BT, node_data* data_out);

/**
 *
 * \param BT Binary tree
 * \param[out] data_out Data output variable
 * \return True: Success, False: NULL pointer argument | Binary tree is not active | Active node does not have a right node
 */
bool BT_get_data_right(TBinaryTree* BT, node_data* data_out);

/**
 *
 * \param BT Binary tree
 * \param[out] data_out Data output variable
 * \return True: Success, False: NULL pointer argument | Binary tree is not active | Active node does not have a parent node
 */
bool BT_get_data_parent(TBinaryTree* BT, node_data* data_out);

/**
 * Prints given tree (subtree)
 * \param tree TNode Pointer to tree
*/
void BT_print_tree(TNode *tree);

/**
 * Prints given nodes Type
 * \param Tnode pointer to given node
*/
void BT_print_node_type (TNode *node);

#endif
