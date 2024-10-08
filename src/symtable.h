typedef struct symtable TSymtable;

typedef int TKey;

typedef struct data TData;

struct data{
    int temp;
};

// SYMTABLE OPERATIONS

TSymtable* symtable_init(void);

bool symtable_insert(TSymtable* symtable, TKey key, TData data);

bool symtable_get_data(TSymtable* symtable, TKey key, TData* data_out);

bool symtable_search(TSymtable* symtable, TKey key);

void symtable_delete(TSymtable* symtable, TKey key);

void symtable_free(TSymtable* symtable);

// DEBUG FUNCTIONS

void debug_print_keys(TSymtable* symtable);


