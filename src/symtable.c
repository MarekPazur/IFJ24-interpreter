/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file symtable.c
 */

// INCLUDES

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "symtable.h"
#include "binary_tree.h"

// STRUCTURES

typedef struct sym_node SymNode;

struct sym_node{
    TKey key;
    TData data;
    SymNode* left;
    SymNode* right;
};

struct symtable{
    SymNode* root;
};

// DATA OPERATION DECLARATIONS

static bool key_is_equal(TKey key_1, TKey key_2);

static bool key_is_less(TKey key_1, TKey key_2);

//static bool key_is_greater(TKey key_1, TKey key_2);

// BST OPERATION DECLARATIONS

static SymNode* create_node_sym(TKey key, TData data);

static bool bst_insert_node(SymNode** root, TKey key, TData data);

static bool bst_seek_node(SymNode* root, TKey key, TData* data_out);

static void bst_free_nodes(SymNode* root);

static void delete_node(SymNode** root);

static void bst_delete_node(SymNode** root, TKey key);

//static int bst_get_height(SymNode* root);

static bool bst_is_height_balanced(SymNode* root, int* height);

static int bst_get_weight(SymNode* root);

static SymNode** bst_to_array(SymNode* root, int* len);

static SymNode* array_to_balanced_bst(SymNode* node_arr[], unsigned int len);

static bool bst_balance(SymNode** root);

// SYMTABLE OPERATIONS DEFINITIONS
TSymtable* symtable_init(void){
    TSymtable* new_symtable = malloc(sizeof(TSymtable));
    if(new_symtable != NULL) {
        new_symtable->root = NULL;
    }
    return new_symtable;
}

bool symtable_insert(TSymtable* symtable, TKey key, TData data){
    if(symtable == NULL || key == NULL){
        return false;
    }
    if(!bst_insert_node(&(symtable->root), key, data)){
        return false;
    }
    return bst_balance(&(symtable->root));
}

bool symtable_get_data(TSymtable* symtable, TKey key, TData* data_out){
    if(symtable == NULL || key == NULL || data_out == NULL){
        return false;
    }
    return bst_seek_node(symtable->root, key, data_out);
}

bool symtable_search(TSymtable* symtable, TKey key){
    if(symtable == NULL || key == NULL){
        return false;
    }
    return bst_seek_node(symtable->root, key, NULL);
}

bool symtable_delete(TSymtable* symtable, TKey key){
    if(symtable == NULL || key == NULL){
        return false;
    }
    bst_delete_node(&(symtable->root), key);
    return bst_balance(&(symtable->root));
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
    if(strcmp(key_1, key_2) == 0){
        return true;
    }
    return false;
}

static bool key_is_less(TKey key_1, TKey key_2){
    if(strcmp(key_1, key_2) < 0){
        return true;
    }
    return false;
}

//static bool key_is_greater(TKey key_1, TKey key_2){
//    if(strcmp(key_1, key_2) > 0){
//        return true;
//    }
//    return false;
//}

// BST OPERATION DEFINITIONS

static SymNode* create_node_sym(TKey key, TData data){
    if(key == NULL){
        return NULL;
    }
    SymNode* new_node = malloc(sizeof(SymNode));
    if(new_node == NULL){
        return NULL;
    }
    int length = (int)(strlen(key) + 1);
    new_node->key = calloc(length, sizeof(char));
    if(new_node->key == NULL){
        free(new_node);
        return NULL;
    }
    strcpy(new_node->key, key);
    new_node->data = data;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

static bool bst_insert_node(SymNode** root, TKey key, TData data){
    if(root == NULL || key == NULL){
        return false;
    }
    SymNode* temp = *(root);
    if(temp == NULL){
        *root = create_node_sym(key, data);
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
    else {
        return bst_insert_node(&(temp->right), key, data);
    }
}

static bool bst_seek_node(SymNode* root, TKey key, TData* data_out){
    if(root == NULL || key == NULL){
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

static void bst_free_nodes(SymNode* root){
    if(root == NULL){
        return;
    }
    bst_free_nodes(root->left);
    bst_free_nodes(root->right);
    free(root->key);
    free(root);
}

static void delete_node(SymNode** root){
    if(root == NULL || *root == NULL){
        return;
    }
    SymNode* temp = *root;
    if(temp->left == NULL){
        *root = temp->right;
        free(temp->key);
        free(temp);
        return;
    }
    if(temp->right == NULL){
        *root = temp->left;
        free(temp->key);
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

static void bst_delete_node(SymNode** root, TKey key){
    if(root == NULL || *root == NULL || key == NULL){
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

//static int bst_get_height(SymNode* root){
//    if(root == NULL){
//        return 0;
//    }
//    int left_height = bst_get_height(root->left);
//    int right_height = bst_get_height(root->right);
//    if(left_height > right_height){
//        return left_height + 1;
//    }
//    else{
//        return right_height + 1;
//    }
//}

static int bst_get_weight(SymNode* root){
    if(root == NULL){
        return 0;
    }
    return bst_get_weight(root->left) + bst_get_weight(root->right) + 1;
}

static bool bst_to_array_inner(SymNode* root, SymNode *arr[], unsigned int* pos){
    if(root == NULL){
        return true;
    }
    if(arr == NULL || pos == NULL){
        return false;
    }
    if(!bst_to_array_inner(root->left, arr, pos)){
        return false;
    }
    SymNode* node = create_node_sym(root->key, root->data);
    if(node == NULL){
        return false;
    }
    arr[*pos] = node;
    *pos += 1;
    if(!bst_to_array_inner(root->right, arr, pos)){
        return false;
    }
    return true;
}

static SymNode** bst_to_array(SymNode* root, int* len){
    if(root == NULL || len == NULL){
        return NULL;
    }
    *len = bst_get_weight(root);
    if(len == 0){
        return NULL;
    }
    SymNode** arr = calloc(*len, sizeof(SymNode*));
    if(arr == NULL){
        *len = -1;
        return NULL;
    }
    unsigned int pos = 0;
    if(!bst_to_array_inner(root, arr, &pos)){
        *len = -1;
        for(int i = 0; i < *len; i++){
            if(arr[i] != NULL){
                free(arr[i]);
            }
        }
        free(arr);
        return NULL;
    }
    return arr;
}

static SymNode* array_to_balanced_bst_inner(SymNode *node_arr[], int beg, int end){
    if(node_arr == NULL){
        return NULL;
    }
    if(beg > end){
        return NULL;
    }
    int mid = (beg + end) / 2;
    SymNode* node = node_arr[mid];
    node->left = array_to_balanced_bst_inner(node_arr, beg, mid - 1);
    node->right = array_to_balanced_bst_inner(node_arr, mid + 1, end);
    return node;
}

static SymNode* array_to_balanced_bst(SymNode* node_arr[], unsigned int len){
    if(node_arr == NULL || len == 0){
        return NULL;
    }
    return array_to_balanced_bst_inner(node_arr, 0, len - 1);
}

static bool bst_is_height_balanced(SymNode* root, int* height){
    if(root == NULL) {
        if(height != NULL){
            *height = 0;
        }
        return true;
    }
    int left_height = 0, right_height = 0;
    if(!bst_is_height_balanced(root->left, &left_height)){
        return false;
    }
    if(!bst_is_height_balanced(root->right, &right_height)){
        return false;
    }
    if(abs(left_height - right_height) > 1){
        return false;
    }
    if(height != NULL){
        *height += 1;
    }
    return true;
}

static bool bst_balance(SymNode** root){
    if(root == NULL){
        return false;
    }
    if(!bst_is_height_balanced(*root, NULL)){
        int len = 0;
        SymNode** arr = bst_to_array(*root, &len);
        if(arr == NULL){
            return false;
        }
        SymNode* new_root = array_to_balanced_bst(arr, len);
        SymNode* prev_root = *root;
        free(arr);
        if(new_root == NULL){
            return false;
        }
        (*root) = new_root;
        bst_free_nodes(prev_root);
    }
    return true;
}

// DEBUG FUNCTIONS

// DEBUG FUNCTION
void bst_print_keys(SymNode* root){
    TData data;
    if(root == NULL){
        return;
    }
    bst_print_keys(root->left);
    printf("Key: %s\n", root->key);
    data = root->data;
    
    //used for debuging the global symtable
    /*printf("is_null_type: %d\n", data.function.is_null_type);
    printf("argument_types: %s\n", data.function.argument_types.array);
    printf("return_type: %d\n", data.function.return_type);*/
    
    //used for debuging local symtables
    printf("is_null_type: %d\n", data.variable.is_null_type);
    printf("is_constant: %d\n", data.variable.is_constant);
    printf("is_used: %d\n", data.variable.is_used);
    printf("comp_runtime: %d\n", data.variable.comp_runtime);
    printf("type: %d\n\n", data.variable.type);
    if ( data.variable.value_pointer != NULL )
        printf("The value is %s \n\n", data.variable.value_pointer->data.nodeData.value.literal);
    bst_print_keys(root->right);
}

// DEBUG FUNCTION
void debug_print_keys(TSymtable* symtable){
    if(symtable == NULL){
        return;
    }
    bst_print_keys(symtable->root);
}

/**
 * @brief This function is used to itterate through a symtable and check wether all it's elements are used
 *
 * @param root, the root of the symtable
 */
bool check_is_used_iter(SymNode* root){
    if ( root == NULL ) {
        return true;
    }
    if (!check_is_used_iter(root->left))
        return false;
    
    if( !root->data.variable.is_used ){
        return false;
    }
    if ( (!root->data.variable.is_constant) && (!root->data.variable.is_mutated)) {
        return false;
    }

    if (!check_is_used_iter(root->right))
        return false;
    return true;
}

/**
 * @brief This function checks that all the variables in a given are used
 * The symtable given to the function has to be local, it calls the check_is_used_iter function to check the elements of the symtable
 *
 * @param symtable, has the instances to check
 * @return True if there was no element with the flas is_used set to false, returns false otherwise
 */
bool check_is_used(TSymtable* symtable){
    if( symtable == NULL ){
        return true;
    }
    if(!check_is_used_iter(symtable->root))
        return false;
    return true;
}
