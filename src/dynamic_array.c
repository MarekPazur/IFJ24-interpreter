/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xtomasp00, Patrik Tomaško
 * @author xpazurm00, Marek Pazúr
 * 
 * @file dynamic_array.c
 */

#include "stdio.h"
#include "stdlib.h"
#include "dynamic_array.h"
#include "compiler_error.h"

void d_array_init(dynamic_array* array, int cap){
    if(array == NULL) {
        error = ERR_COMPILER_INTERNAL;
        fprintf(stderr, "Error: an empty pointer was given to d_array_init function");
    }else {
        array->array = (char *) calloc(cap + 1, sizeof(char));
        if (array->array == NULL) {
            fprintf(stderr, "Error: an error occured during the alocation of the dynamic array");
            error = ERR_COMPILER_INTERNAL;
        }else {
            array->capacity = cap;
            array->length = 0;
            error = SUCCESS;
        }
    }
}

void d_array_append(dynamic_array* array, char element){
    if(array == NULL){
        error = ERR_COMPILER_INTERNAL;
        fprintf(stderr, "Error: an empty pointer was given to d_array_append function");
    }else {
        if (array->capacity == array->length) {
            array->capacity += array->capacity;
            array->array = (char *) realloc(array->array, (array->capacity + 1) * sizeof(char));
        }
        if (array->array == NULL) {
            fprintf(stderr, "Error: an error occured during the realocation of the dynamic array");
            error = ERR_COMPILER_INTERNAL;
        }else {
            array->array[array->length] = element;
            array->length++;
            array->array[array->length] = '\0';

            error = SUCCESS;
        }
    }
}

void d_array_set(dynamic_array* array, char element, int index){
    if(array == NULL){
        error = ERR_COMPILER_INTERNAL;
        fprintf(stderr, "Error: an empty pointer was given to d_array_set function");
    }else {
        if (0 <= index && index < array->length) {
            array->array[index] = element;
        } else {
            fprintf(stderr,
                    "Error: tried to set a value of an element an a dynamic array on the index %d but that index does not exist",
                    index);
            error = ERR_COMPILER_INTERNAL;
        }
    }
}

char d_array_get(dynamic_array* array, int index){
    if(array == NULL){
        error = ERR_COMPILER_INTERNAL;
        fprintf(stderr, "Error: an empty pointer was given to d_array_get function");
        return '\0';
    }
    if(0 <= index && index < array->length){
        return array->array[index];
    }else{
        fprintf(stderr, "Error: tried to get a value of an element an a dynamic array on the index %d but that index does not exist", index);
        error = ERR_COMPILER_INTERNAL;
    }
    return '\0';
}

void d_array_remove(dynamic_array* array, int index){
    if(array == NULL){
        error = ERR_COMPILER_INTERNAL;
        fprintf(stderr, "Error: an empty pointer was given to d_array_remove function");
    }else {
        if (0 <= index && index < array->length) {
            for (int i = index; i < array->length; i++) {
                array->array[index] = array->array[index + 1];
            }
            array->length--;
        } else {
            fprintf(stderr,
                    "Error: tried to remove an element of a dynamic array on the index %d but that index does not exist",
                    index);
            error = ERR_COMPILER_INTERNAL;
        }
    }
}

void d_array_free(dynamic_array* array) {
    if(array == NULL){
        error = ERR_COMPILER_INTERNAL;
        fprintf(stderr, "Error: an empty pointer was given to d_array_free function");
    }else {
        array->length = 0;
        array->capacity = 0;
        free(array->array);
        array->array = NULL;
    }
}

void d_array_print(dynamic_array* lexeme) {
    if (lexeme != NULL && lexeme->array != NULL) {
        for (int i = 0; i < lexeme->length; ++i)
        {
            char c = lexeme->array[i];
            putchar(c);
        }
    }
}
