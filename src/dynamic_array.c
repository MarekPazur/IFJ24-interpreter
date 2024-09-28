/**
 * Název projektu: Implementace překladače imperativního jazyka IFJ24.
 *
 * @author xtomasp00, Patrik Tomaško
 *
 * @file dynamic_array.h
 */

#include "stdio.h"
#include "stdlib.h"
#include "dynamic_array.h"

void d_array_init(dynamic_array* array, int cap){
    array->array=(int*)malloc(cap * sizeof(char));
    if(array->array==NULL){
        fprintf(stderr, "Error: an error occured during the alocation of the dynamic array");
        return;
    }
    array->capacity = cap;
    array->length=0;
    return;
}

void d_array_append(dynamic_array* array, char element){
    if(array->capacity==array->length){
        array->capacity+=array->capacity;
        array->array=(int*)realloc(array->array, array->capacity*sizeof(int));
    }
    if(array->array==NULL){
        fprintf(stderr, "Error: an error occured during the realocation of the dynamic array");
        return;
    }
    array->array[array->length]=element;
    array->length++;
    return;
}

void d_array_set(dynamic_array* array, char element, int index){
    if(0<=index && index<array->length){
        array->array[index] = element;
    }else{
        fprintf(stderr, "Error: tried to set a value of an element an a dynamic array on the index %d but that index does not exist", index);
    }
    return;
}

char d_array_get(dynamic_array* array, int index){
    if(0<=index && index<array->length){
        return array->array[index];
    }else{
        fprintf(stderr, "Error: tried to get a value of an element an a dynamic array on the index %d but that index does not exist", index);
    }
    return '\0';
}

void d_array_remove(dynamic_array* array, int index){
    if(0<=index && index<array->length){
        for(int i = index; i < array->length; i++){
            array->array[index] = array->array[index+1];
        }
        array->length--;
    }else{
        fprintf(stderr, "Error: tried to remove an element of a dynamic array on the index %d but that index does not exist", index);
    }
    return;
}

void d_array_free(dynamic_array* array){
    array->length=0;
    array->capacity=0;
    free(array->array);
    array->array=NULL;
}