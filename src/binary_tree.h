typedef struct _BST BST;

BST* BST_init();

void BST_free(BST* bst);

bool BST_add_node(BST* bst, int key, void* data);
