/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xukropj00, Jan Ukropec
 *
 * @file symtable.h
 */

#ifndef SYMTABLE_H
#define SYMTABLE_H
#include "dynamic_array.h"
typedef struct symtable TSymtable;

typedef char* TKey;

typedef union data TData;

typedef enum type {
    UNKNOWN_T,    // type unknown when defined, must be updated later else error
    VOID_T,       // void
    INTEGER_T,    // i32
    FLOAT_T,      // f64
    U8_SLICE_T,   // u8[]
    BOOL_T,       // boolean, can only be in conditions
    NIL_T,        // NULLABLE
    STR_T         // string literal
} Type;

union data {
    struct {
        bool is_null_type;            // Return type can be nullable '?type'
        
        dynamic_array argument_types; // Formal parameter types
        Type return_type;             // Return type
        TSymtable* function_scope;     // Local scope of function instance
    } function;

    struct {
        bool is_null_type;            // Variable type can be nullable '?type'
        bool is_constant;             // Variable is constant
        bool is_used;                 // Variable has to be used in its life
        bool is_mutated;              // Variable was changed in its life
        bool comp_runtime;            // Variable or value that is known and evaluated during compilation
        
        struct node* value_pointer;

        Type type;                    // Variable data type
    } variable;
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
 * \return True: success False: Memory allocation error | NULL pointer error
 */
bool symtable_insert(TSymtable* symtable, TKey key, TData data);

/**
 * Get data from the symtable
 * \param symtable
 * \param key
 * \param data_out
 * \return True: succes False: data is not in table | NULL pointer error
 */
bool symtable_get_data(TSymtable* symtable, TKey key, TData* data_out);

/**
 * Check if item with a certain key is in the symtable
 * \param symtable
 * \param key
 * \return True: item is in the symtable False: data is not in the symtable | NULL pointer error
 */
bool symtable_search(TSymtable* symtable, TKey key);

/**
 * Delete item from the symtable if it exists
 * \param symtable
 * \param key
 * \return True: item was deleted | False: item was not deleted | NULL pointer | memory allocation error
 */
bool symtable_delete(TSymtable* symtable, TKey key);

/**
 * Frees symtable memory
 * \param symtable
 */
void symtable_free(TSymtable* symtable);

// DEBUG FUNCTIONS

void debug_print_keys(TSymtable* symtable);

bool check_is_used(TSymtable* symtable);

#endif
