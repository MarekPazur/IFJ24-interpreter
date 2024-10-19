/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file binary_tree.h
 */

#ifndef BINARY_TREE_H
#define BINARY_TREE_H

typedef struct binary_tree TBinaryTree;

typedef struct data_bt{
    int temp;
} TDataBT;

/**
 * Allocates memory for binary tree and initializes it
 * \return New binary tree | NULL in case of a memory allocation error
 */
TBinaryTree* BT_init(void);

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
bool BT_insert_left(TBinaryTree* BT, TDataBT data);

/**
 * Inserts node as a right child of the active node.
 * \param BT Binary tree
 * \param data
 * \return True: Succes, False: Memory allocation error | BT is NULL | BT is not active | Active node has a right node
 */
bool BT_insert_right(TBinaryTree* BT, TDataBT data);

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
bool BT_insert_root(TBinaryTree* BT, TDataBT data);

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
bool BT_get_data(TBinaryTree* BT, TDataBT* data_out);

/**
 *
 * \param BT Binary tree
 * \param[out] data_out Data output variable
 * \return True: Success, False: NULL pointer argument | Binary tree is not active | Active node does not have a left node
 */
bool BT_get_data_left(TBinaryTree* BT, TDataBT* data_out);

/**
 *
 * \param BT Binary tree
 * \param[out] data_out Data output variable
 * \return True: Success, False: NULL pointer argument | Binary tree is not active | Active node does not have a right node
 */
bool BT_get_data_right(TBinaryTree* BT, TDataBT* data_out);

/**
 *
 * \param BT Binary tree
 * \param[out] data_out Data output variable
 * \return True: Success, False: NULL pointer argument | Binary tree is not active | Active node does not have a parent node
 */
bool BT_get_data_parent(TBinaryTree* BT, TDataBT* data_out);

#endif
