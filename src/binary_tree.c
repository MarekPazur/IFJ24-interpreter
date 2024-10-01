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

typedef void (*Operation)(Node* node);

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

static bool add_node(Node** root, int key, void* data){
    if(root == NULL){
        return false;
    }
    Node* temp = *root;
    if(temp == NULL){
        *root = node_init(key, data);
        if(*root == NULL){
            return false;
        }
        return true;
    }
    if(key == temp->key){
        return false;
    }
    if(key < temp->key){
        return add_node(&(temp->left), key, data);
    }
    else{
        return add_node(&(temp->right), key, data);
    }
}

static void in_order(Node* node, Operation operation){
    if(node == NULL || operation == NULL){
        return;
    }
    in_order(node->left, operation);
    operation(node);
    in_order(node->right, operation);
}

// Global functions

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

bool BST_add_node(BST* bst, int key, void* data){
    if(bst == NULL){
        return false;
    }
    return add_node(&(bst->root), key, data);
}

void BST_in_order(BST* bst, Operation operation){
    if(bst == NULL || operation == NULL){
        return;
    }
    in_order(bst->root, operation);
}
