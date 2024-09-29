/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xtomasp00, Patrik Tomaško
 *@author xpazurm00, Marek Pazúr
 * 
 * @file dynamic_array.h
 */

#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

typedef struct{
    char* array;
    int length;
    int capacity;
} dynamic_array;

void d_array_init(dynamic_array* array, int cap);

void d_array_append(dynamic_array* array, char element);

void d_array_set(dynamic_array* array, char element, int index);

char d_array_get(dynamic_array* array, int index);

void d_array_remove(dynamic_array* array, int index);

void d_array_free(dynamic_array* array);

void d_array_print(dynamic_array* lexeme);

#endif