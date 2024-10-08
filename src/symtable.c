// INCLUDES

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "symtable.h"

// STRUCTURES

typedef struct node TNode;

struct node{
    TKey key;
    TData data;
    TNode* left;
    TNode* right;
};

struct symtable{
    TNode* root;
};

// DATA OPERATION DECLARATIONS

static bool key_is_equal(TKey key_1, TKey key_2);

static bool key_is_less(TKey key_1, TKey key_2);

//static bool key_is_greater(TKey key_1, TKey key_2);

// BST OPERATION DECLARATIONS

static TNode* create_node(TKey key, TData data);

static bool bst_insert_node(TNode** root, TKey key, TData data);

static bool bst_seek_node(TNode* root, TKey key, TData* data_out);

static void bst_free_nodes(TNode* root);

static void delete_node(TNode** root);

static void bst_delete_node(TNode** root, TKey key);

// SYMTABLE OPERATIONS DEFINITIONS

TSymtable* symtable_init(void){
    TSymtable* new_symtable = malloc(sizeof(TSymtable));
    if(new_symtable != NULL) {
        new_symtable->root = NULL;
    }
    return new_symtable;
}

bool symtable_insert(TSymtable* symtable, TKey key, TData data){
    if(symtable == NULL){
        return false;
    }
    return bst_insert_node(&(symtable->root), key, data);
}

bool symtable_get_data(TSymtable* symtable, TKey key, TData* data_out){
    if(symtable == NULL || data_out == NULL){
        return false;
    }
    return bst_seek_node(symtable->root, key, data_out);
}

bool symtable_search(TSymtable* symtable, TKey key){
    if(symtable == NULL){
        return false;
    }
    return bst_seek_node(symtable->root, key, NULL);
}

void symtable_delete(TSymtable* symtable, TKey key){
    if(symtable == NULL){
        return;
    }
    bst_delete_node(&(symtable->root), key);
}

void symtable_free(TSymtable* symtable){
    if(symtable == NULL){
        return;
    }
    bst_free_nodes(symtable->root);
    free(symtable);
}

// DATA OPERATION DEFINITIONS

static bool key_is_equal(TKey key_1, TKey key_2){
    if(key_1 == key_2){
        return true;
    }
    return false;
}

static bool key_is_less(TKey key_1, TKey key_2){
    if(key_1 < key_2){
        return true;
    }
    return false;
}

//static bool key_is_greater(TKey key_1, TKey key_2){
//    if(key_1 > key_2){
//        return true;
//    }
//    return false;
//}

// BST OPERATION DEFINITIONS

static TNode* create_node(TKey key, TData data){
    TNode* new_node = malloc(sizeof(TNode));
    if(new_node != NULL){
        new_node->key = key;
        new_node->data = data;
        new_node->left = NULL;
        new_node->right = NULL;
    }
    return new_node;
}

static bool bst_insert_node(TNode** root, TKey key, TData data){
    if(root == NULL){
        return false;
    }
    TNode* temp = *(root);
    if(temp == NULL){
        *root = create_node(key, data);
        if(*root == NULL){
            return false;
        }
        return true;
    }
    if(key_is_equal(key, temp->key)) {
        temp->data = data;
        return true;
    }
    if(key_is_less(key, temp->key)){
        return bst_insert_node(&(temp->left), key, data);
    }
    else{
        return bst_insert_node(&(temp->right), key, data);
    }
}

static bool bst_seek_node(TNode* root, TKey key, TData* data_out){
    if(root == NULL){
        return false;
    }
    if(key_is_equal(key, root->key)){
        if(data_out != NULL) {
            *data_out = root->data;
        }
        return true;
    }
    if(key_is_less(key, root->key)){
        return bst_seek_node(root->left, key, data_out);
    }
    else{
        return bst_seek_node(root->right, key, data_out);
    }
}

static void bst_free_nodes(TNode* root){
    if(root == NULL){
        return;
    }
    bst_free_nodes(root->left);
    bst_free_nodes(root->right);
    free(root);
}

static void delete_node(TNode** root){
    if(root == NULL || *root == NULL){
        return;
    }
    TNode* temp = *root;
    if(temp->left == NULL){
        *root = temp->right;
        free(temp);
        return;
    }
    if(temp->right == NULL){
        *root = temp->left;
        free(temp);
        return;
    }
    if(temp->left->right == NULL){
        temp->key = temp->left->key;
        temp->data = temp->left->data;
        delete_node(&(temp->left));
        return;
    }
    temp = temp->left;
    while (temp->right->right != NULL){
        temp = temp->right;
    }
    (*root)->key = temp->right->key;
    (*root)->data = temp->right->data;
    delete_node(&(temp->right));
}

static void bst_delete_node(TNode** root, TKey key){
    if(root == NULL || *root == NULL){
        return;
    }
    if(key_is_equal(key, (*root)->key)){
        delete_node(root);
    }
    else if(key_is_less(key, (*root)->key)){
        bst_delete_node(&((*root)->left), key);
    }
    else{
        bst_delete_node(&((*root)->right), key);
    }
}

// DEBUG FUNCTIONS

// DEBUG FUNCTION
void bst_print_keys(TNode* root){
    if(root == NULL){
        return;
    }
    bst_print_keys(root->left);
    printf("Key: %d\n", root->key);
    bst_print_keys(root->right);
}

// DEBUG FUNCTION
void debug_print_keys(TSymtable* symtable){
    if(symtable == NULL){
        return;
    }
    bst_print_keys(symtable->root);
}