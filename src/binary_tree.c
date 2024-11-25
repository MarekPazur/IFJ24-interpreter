/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file binary_tree.h
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "binary_tree.h"
#include "compiler_error.h"

// Binary tree private functions

/**
 * Creates new node and initializes it
 * \param data
 * \return New node | NULL in case of a memory allocation error
 */
TNode* create_node(node_type type) {
    TNode* new_node = malloc(sizeof(TNode));
    if(new_node != NULL){
        new_node->parent = NULL;
        new_node->left = NULL;
        new_node->right = NULL;
        new_node->type = type;
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

bool BT_insert_left(TBinaryTree* BT, node_type type){
    if(!BT_is_active(BT)){
        return false;
    }
    if(BT_has_left(BT)){
        return false;
    }
    TNode* new_node = create_node(type);
    if(new_node == NULL){
        return false;
    }
    new_node->parent = BT->active;
    BT->active->left = new_node;
    return true;
}

bool BT_insert_right(TBinaryTree* BT, node_type type){
    if(!BT_is_active(BT)){
        return false;
    }
    if(BT_has_right(BT)){
        return false;
    }
    TNode* new_node = create_node(type);
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

bool BT_insert_root(TBinaryTree* BT, node_type type){
    if(BT == NULL){
        return false;
    }
    if(BT_has_root(BT)){
        return false;
    }
    TNode* new_node = create_node(type);
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

bool BT_get_data(TBinaryTree* BT, node_data* data_out){
    if(!BT_is_active(BT)){
        return false;
    }
    if(data_out == NULL){
        return false;
    }
    *data_out = BT->active->data;
    return true;
}

bool BT_get_data_left(TBinaryTree* BT, node_data* data_out){
    if(!BT_has_left(BT)){
        return false;
    }
    if(data_out == NULL){
        return false;
    }
    *data_out = BT->active->left->data;
    return true;
}

bool BT_get_data_right(TBinaryTree* BT, node_data* data_out){
    if(!BT_has_right(BT)){
        return false;
    }
    if(data_out == NULL){
        return false;
    }
    *data_out = BT->active->right->data;
    return true;
}

bool BT_get_data_parent(TBinaryTree* BT, node_data* data_out){
    if(!BT_has_parent(BT)){
        return false;
    }
    if(data_out == NULL){
        return false;
    }
    *data_out = BT->active->parent->data;
    return true;
}


// Binary tree debug functions

const char *node_t_string[] = {
    "ROOT",

    "PROLOGUE",
    "FUNCTION",

    "COMMAND",
    "METADATA",
    "NULL REPLACEMENT",

    "VAR DECLARATION",
    "CONST DECLARATION",

    "ASSIGNMENT",

    "BODY",
    "WHILE",
    "IF",
    "ELSE",

    "I32",
    "F64",
    "U8[]",
    "STR",
    "NULL",
    "VAR/CONST",

    "EXPR",
    "+",
    "-",
    "*",
    "/",
    "==",
    "!=",
    ">",
    "<",
    ">=",
    "<=",

    "FUNCTION CALL",

    "RETURN"
};

typedef enum side {
    ROOT,
    LEFT_SIDE,
    RIGHT_SIDE
} side_t;

/*
* Creates new line out of previous and string suffix to be added
*/
char *new_line(char *line, char *str) {
    char *new;
    size_t new_size = strlen(line) + strlen(str) + 1; /* Size of previous line + string suffix + terminator */

    if((new = (char *) malloc(new_size)) == NULL) {
        fprintf(stderr,"Tree print function resource allocation failure");
        error =  ERR_COMPILER_INTERNAL;
        return NULL;
    }; 
    
    strcpy(new, line);
    strcat(new, str);
    
    return new;
}

/*
* Returns index of appropriate node type according to its children
*/
int get_node_shape(TNode *tree) {
    int node_type;
    if (tree->right && tree->left)
        node_type = 0;
    else if (tree->right && tree->left == NULL)
        node_type = 1;
    else if (tree->right == NULL && tree->left)
        node_type = 2;
    else node_type = 3;

    return node_type;
}

/*
* Prints the subtrees using inverse in-order traversal (Right subtree first)
*/
void BT_print_subtree(TNode *tree, char *line, side_t side) {
    /* Empty node */
    if (tree == NULL)
        return;

    /* Append branch and space to current line (Creates two variants, selects one later) */
    char *current_branch = new_line(line,  "|    ");
    char *current_space = new_line(line, "     ");

    /* Prints branch when entering left-side node */
    if (side == LEFT_SIDE) {
        printf("%s\n", current_branch);
    }

    /* Traverses through right subtree first, selects appropriate new line to be passed to recursive call of this function */
    BT_print_subtree(tree->right, side == LEFT_SIDE ? current_branch : current_space, RIGHT_SIDE);

    /* Shape of node according to its children */
    char* node_shape[] = {           /*   Node has:   */
        "\033[0;94m┼\033[0;37m",    /* Both children */
        "\033[0;94m┴\033[0;37m",    /* Right children*/
        "\033[0;94m┬\033[0;37m",    /* Left children */
        "\033[0;92m>\033[0;37m"     /* Leaf node     */
    };

    /* Prints node content with its edge and shape */
    printf("%s%s----%s", line, (side == ROOT ? "\033[0;91m~\033[0;37m" : (side == RIGHT_SIDE ? "┌" : "└")), node_shape[get_node_shape(tree)]);
    
    /* Select correct node content do print */
    char *content = NULL;

    /* Values in expressions, function call parameters... */
    if (tree->type == INT || tree->type == FL || tree->type == STR) {
        content = tree->data.nodeData.value.literal;
    }

    if (tree->type == VAR_CONST) {
        content = tree->data.nodeData.value.identifier;
    }

    /* Function declaration */
    if (tree->type == FN)
        content = tree->data.nodeData.function.identifier;

    /* Function call, variable declaration or assignment */
    if (tree->type == FUNCTION_CALL || tree->type == VAR_DECL || tree->type == CONST_DECL || tree->type == ASSIG)
        content = tree->data.nodeData.identifier.identifier;

    printf("\033[1;33m %s %s\033[0;37m\n", node_t_string[tree->type], (content ? content : ""));

    /* Traverse through left subtree */
    BT_print_subtree(tree->left, side == RIGHT_SIDE ? current_branch : current_space, LEFT_SIDE);

    /* Prints branch after return from previous right-side node (backstep) */
    if (side == RIGHT_SIDE) {
        printf("%s\n", current_branch);
    }

    /* Frees resources used for lines after its printed */
    free(current_branch);
    free(current_space);
}

/* 
* Debug function for printing binary tree
*/
void BT_print_tree(TNode *tree) {
    printf("    \033[4;37mAbstract syntax tree for IFJ24:\033[0;37m\n\n");

    /* Prints tree if its not empty, else returns */
    if (tree)
        BT_print_subtree(tree, "", ROOT);
    else
        printf("Tree is NULL!\n");
}

void init_llist(linked_list_t* llist){
    llist->first = NULL;
    llist->active = NULL;
}

bool insert_llist(linked_list_t* llist, char* inserted){
    item_ll_t* new_item = (item_ll_t*)malloc(sizeof(item_ll_t));
    if (new_item == NULL) {
        return false;
    }

    new_item->identifier = inserted;
    if (!new_item->identifier) {
        free(new_item);
        return false;
    }
    new_item->next = NULL;

    if (llist->first == NULL) {
        llist->first = new_item;
    } else {
        item_ll_t* current_item = llist->first;
        while (current_item->next != NULL) {
            current_item = current_item->next;
        }
        current_item->next = new_item;
    }
    return true;
}

bool set_first_llist(linked_list_t* llist){
    if (llist->first == NULL) {
        return false;
    }
    llist->active = llist->first;
    return true;
}

bool next_llist(linked_list_t* llist){
    if(llist->active == NULL){
        return false;
    }
    llist->active = llist->active->next;
    return true;
}

bool free_llist(linked_list_t* llist){
    item_ll_t* current_item = llist->first;
    while (current_item != NULL) {
        item_ll_t* next_item = current_item->next;
        free(current_item->identifier);
        free(current_item);
        current_item = next_item;
    }
    llist->first = NULL;
    llist->active = NULL;
    return true;
}

bool get_value_llist(linked_list_t* llist, char** value){
    if (llist->active == NULL) {
        return false;
    }
    *value = llist->active->identifier;
    return true;
}

/**
* Debug function for printing node type
*/
void BT_print_node_type (TNode *node) {
    if (node == NULL) {
        printf("NULL node\n");
        return;
    }

    printf(WHITE_BOLD("NODE TYPE:")" %s\n", node_t_string[node->type]);
}
