typedef struct _BST BST;

/**
 * Allocates memory for BST and initializes it
 * \return Newly allocated BST or NULL
 */
BST* BST_init();

/**
 * \brief Frees all nodes (now including data) and the bst
 * \param bst
 */
void BST_free(BST* bst);

/**
 * \brief Adds node to the tree if there is not a node with the same key
 * \param bst
 * \param key
 * \param data
 * \return True: succes, False: node with the same key exists, memory error
 */
bool BST_add_node(BST* bst, int key, void* data);

/**
 * \brief Prints key data (considered int*) into stdout from the lowest key to the highest
 * @param bst
 */
void BST_print_inorder_int(BST* bst);
