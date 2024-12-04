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

/**
 * @brief This function initializes the dynamic array
 *
 * @param array pointer to where the dynamic array should be initialized
 * @param cap the capacity witch which the array should be initialized
 */
void d_array_init(dynamic_array* array, int cap);

/**
 * @brief This function appends an element to a dynamic array
 *
 * @param array pointer to the array to which the function shall append the new element
 * @param element the element (character) to be appended
 */
void d_array_append(dynamic_array* array, char element);

/**
 * @brief This function sets an element on a specified index to a certain character
 *
 * @param array the dynamic array where the element will be set
 * @param element the value that is to be set on the element on the specified index
 * @param index the index of the element to be set
 */
void d_array_set(dynamic_array* array, char element, int index);

/**
 * @brief This function calculates the barymetric values of a point
 *
 * @param array array containing the desired element
 * @param index index of the element to be returned
 *
 * @return returns the element(character) of the array on the index specified in the input
 */
char d_array_get(dynamic_array* array, int index);

/**
 * @brief This function removes a cetain element of a dynamic array on a certain index and shifts the rest of the array
 *
 * @param array the array in which we want to remove the element
 * @param index index of the element to be removed
 */
void d_array_remove(dynamic_array* array, int index);

/**
 * @brief Frees the memory assigned to a dynamic array
 *
 * @param array array to be freed
 */
void d_array_free(dynamic_array* array);

/**
 * @brief This function prints out the contents of a dynamic array
 *
 * @param array the dynamic array to be printed out
 */
void d_array_print(dynamic_array* lexeme);

#endif
