typedef struct symtable TSymtable;

typedef int TKey;

typedef struct data TData;

struct data{
    int temp;
};

// SYMTABLE OPERATIONS

/**
 * Initializes symtable
 * \return initialized symtable or NULL in case of an memory allocation error
 */
TSymtable* symtable_init(void);

/**
 * Insert item into the symtable / Rewrite data of an existing item if the keys match
 * \param symtable
 * \param key
 * \param data
 * \return
 */
bool symtable_insert(TSymtable* symtable, TKey key, TData data);

/**
 * Get data from the symtable
 * \param symtable
 * \param key
 * \param data_out
 * \return
 */
bool symtable_get_data(TSymtable* symtable, TKey key, TData* data_out);

/**
 * Check if item with a certain key is in the symtable
 * \param symtable
 * \param key
 * \return
 */
bool symtable_search(TSymtable* symtable, TKey key);

/**
 * Delete item from the symtable if it exists
 * \param symtable
 * \param key
 * \return
 */
bool symtable_delete(TSymtable* symtable, TKey key);

/**
 * Frees symtable memory
 * \param symtable
 */
void symtable_free(TSymtable* symtable);

// DEBUG FUNCTIONS

void debug_print_keys(TSymtable* symtable);
