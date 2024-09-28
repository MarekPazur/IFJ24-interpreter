/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xtomasp00, Patrik Tomaško
 *
 * @file dynamic_array.h
 */

typedef struct{
    int* array;
    int length;
    int capacity;
} dynamic_array;

void d_array_init(dynamic_array* array, int cap);

void d_array_append(dynamic_array* array, char element);

void d_array_set(dynamic_array* array, char element, int index);

char d_array_get(dynamic_array* array, int index);

void d_array_remove(dynamic_array* array, int index);

void d_array_free(dynamic_array* array);