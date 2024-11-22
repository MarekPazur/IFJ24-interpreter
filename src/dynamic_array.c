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

/**
 * @brief This function initializes the dynamic array
 *
 * @param array pointer to where the dynamic array should be initialized
 * @param cap the capacity witch which the array should be initialized
 */
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

/**
 * @brief This function appends an element to a dynamic array
 *
 * @param array pointer to the array to which the function shall append the new element
 * @param element the element (character) to be appended
 */
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

/**
 * @brief This function sets an element on a specified index to a certain character
 *
 * @param array the dynamic array where the element will be set
 * @param element the value that is to be set on the element on the specified index
 * @param index the index of the element to be set
 */
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

/**
 * @brief This function calculates the barymetric values of a point
 *
 * @param array array containing the desired element
 * @param index index of the element to be returned
 *
 * @return returns the element(character) of the array on the index specified in the input
 */
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

/**
 * @brief This function removes a cetain element of a dynamic array on a certain index and shifts the rest of the array
 *
 * @param array the array in which we want to remove the element
 * @param index index of the element to be removed
 */
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

/**
 * @brief Frees the memory assigned to a dynamic array
 *
 * @param array array to be freed
 */
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

/**
 * @brief This function prints out the contents of a dynamic array
 *
 * @param array the dynamic array to be printed out
 */
void d_array_print(dynamic_array* lexeme) {
    if (lexeme != NULL && lexeme->array != NULL) {
        for (int i = 0; i < lexeme->length; ++i)
        {
            char c = lexeme->array[i];
            putchar(c);
        }
    }
}