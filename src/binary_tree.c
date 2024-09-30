#include <stdbool.h>
#include <stdlib.h>

#include "binary_tree.h"

typedef struct _node Node;
struct _node{
    int key;
    void* data;
    Node* left;
    Node* right;
};

struct _BST{
    Node* root;
};

// Static functions

static Node* node_init(int key, void* data){
    Node* new_node = malloc(sizeof(Node));
    if(new_node == NULL){
        return NULL;
    }
    new_node->key = key;
    new_node->data = data;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

static void free_nodes(Node** node){
    if(node == NULL || *node == NULL) {
        return;
    }
    free_nodes(&((*node)->left));
    free_nodes(&((*node)->right));
    free(*node);
}

// Module functions

BST* BST_init(){
    BST* new_bst = malloc(sizeof(BST));
    if(new_bst == NULL){
        return NULL;
    }
    new_bst->root = NULL;
    return new_bst;
}

void BST_free(BST* bst){
    if(bst == NULL){
        return;
    }
    if(bst->root != NULL){
        free_nodes(&(bst->root));
    }
    free(bst);
}

//bool BST_add_node(BST* bst, int key, void* data){
//    if(bst == NULL){
//        return false;
//    }
//    if(bst->root == NULL){
//        Node* new_node = node_init(key, data);
//        if(new_node == NULL){
//            return false;
//        }
//        else{
//            bst->root = new_node;
//            return true;
//        }
//    }
//    while(){
//
//    }
//}