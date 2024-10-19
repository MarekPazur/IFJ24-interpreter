/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file binary_tree.h
 */

#include <stdlib.h>
#include <stdbool.h>
#include "binary_tree.h"

typedef struct node TNode;
struct node{
    TNode* parent;
    TNode* left;
    TNode* right;
    TDataBT data;
};

struct binary_tree{
    TNode* root;
    TNode* active;
};

// Binary tree private functions

/**
 * Creates new node and initializes it
 * \param data
 * \return New node | NULL in case of a memory allocation error
 */
TNode* create_node(TDataBT data){
    TNode* new_node = malloc(sizeof(TNode));
    if(new_node != NULL){
        new_node->parent = NULL;
        new_node->left = NULL;
        new_node->right = NULL;
        new_node->data = data;
    }
    return new_node;
}

/**
 * Frees all nodes
 * \param root Root node
 */
void free_nodes(TNode* root){
    if(root == NULL){
        return;
    }
    free_nodes(root->left);
    free_nodes(root->right);
    free(root);
}

// Binary tree public functions

TBinaryTree* BT_init(void){
    TBinaryTree* new_tree = malloc(sizeof(TBinaryTree));
    if(new_tree != NULL){
        new_tree->root = NULL;
        new_tree->active = NULL;
    }
    return new_tree;
}

bool BT_is_active(TBinaryTree* BT){
    if(BT == NULL){
        return false;
    }
    return (BT->active != NULL);
}

void BT_set_root(TBinaryTree* BT){
    if(BT == NULL){
        return;
    }
    BT->active = BT->root;
}

bool BT_has_parent(TBinaryTree* BT){
    if(!BT_is_active(BT)){
        return false;
    }
    return (BT->active->parent != NULL);
}

bool BT_has_left(TBinaryTree* BT){
    if(!BT_is_active(BT)){
        return false;
    }
    return (BT->active->left != NULL);
}

bool BT_has_right(TBinaryTree* BT){
    if(!BT_is_active(BT)){
        return false;
    }
    return (BT->active->right != NULL);
}

void BT_go_parent(TBinaryTree* BT){
    if(BT_is_active(BT)){
        BT->active = BT->active->parent;
    }
}

void BT_go_left(TBinaryTree* BT){
    if(BT_is_active(BT)){
        BT->active = BT->active->left;
    }
}

void BT_go_right(TBinaryTree* BT){
    if(BT_is_active(BT)){
        BT->active = BT->active->right;
    }
}

bool BT_insert_left(TBinaryTree* BT, TDataBT data){
    if(!BT_is_active(BT)){
        return false;
    }
    if(BT_has_left(BT)){
        return false;
    }
    TNode* new_node = create_node(data);
    if(new_node == NULL){
        return false;
    }
    new_node->parent = BT->active;
    BT->active->left = new_node;
    return true;
}

bool BT_insert_right(TBinaryTree* BT, TDataBT data){
    if(!BT_is_active(BT)){
        return false;
    }
    if(BT_has_right(BT)){
        return false;
    }
    TNode* new_node = create_node(data);
    if(new_node == NULL){
        return false;
    }
    new_node->parent = BT->active;
    BT->active->right = new_node;
    return true;
}

bool BT_has_root(TBinaryTree* BT){
    if(BT == NULL){
        return false;
    }
    return (BT->root != NULL);
}

bool BT_insert_root(TBinaryTree* BT, TDataBT data){
    if(BT == NULL){
        return false;
    }
    if(BT_has_root(BT)){
        return false;
    }
    TNode* new_node = create_node(data);
    if(new_node == NULL){
        return false;
    }
    BT->root = new_node;
    return true;
}

void BT_free_nodes(TBinaryTree* BT){
    free_nodes(BT->root);
    BT->root = NULL;
    BT->active = NULL;
}

void BT_free_left_subtree(TBinaryTree* BT){
    if(BT_is_active(BT)){
        free_nodes(BT->active->left);
        BT->active->left = NULL;
    }
}

void BT_free_right_subtree(TBinaryTree* BT){
    if(BT_is_active(BT)){
        free_nodes(BT->active->right);
        BT->active->right = NULL;
    }
}

void BT_free_active_tree(TBinaryTree* BT){
    if(BT_is_active(BT)){
        TNode* parent = BT->active->parent;
        if(parent != NULL){
            if(parent->left == BT->active){
                parent->left = NULL;
            }
            else{
                parent->right = NULL;
            }
        }
        free_nodes(BT->active);
        BT->active = NULL;
    }
}

bool BT_get_data(TBinaryTree* BT, TDataBT* data_out){
    if(!BT_is_active(BT)){
        return false;
    }
    if(data_out == NULL){
        return false;
    }
    *data_out = BT->active->data;
    return true;
}

bool BT_get_data_left(TBinaryTree* BT, TDataBT* data_out){
    if(!BT_has_left(BT)){
        return false;
    }
    if(data_out == NULL){
        return false;
    }
    *data_out = BT->active->left->data;
    return true;
}

bool BT_get_data_right(TBinaryTree* BT, TDataBT* data_out){
    if(!BT_has_right(BT)){
        return false;
    }
    if(data_out == NULL){
        return false;
    }
    *data_out = BT->active->right->data;
    return true;
}

bool BT_get_data_parent(TBinaryTree* BT, TDataBT* data_out){
    if(!BT_has_parent(BT)){
        return false;
    }
    if(data_out == NULL){
        return false;
    }
    *data_out = BT->active->parent->data;
    return true;
}
