#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
    PAIR ARRAY ADT
*/
typedef struct
{
    int x;
    int y;
} Pair;

typedef struct
{
    Pair *data;
    int length;
    int index;
} List;

void init_list(List *);

void init_pair(int _x, int _y, Pair *pair);

void push(Pair, List *);

void print_pair(Pair);

Pair element_at(int, List *);

void __shrink_list(List *);

void remove_at(int, List *);

void print_list(List *);

void dealloc_list(List *);

void list_clear(List *);

bool isInRange(int val, List *edges, int range);

/*
    1D-INT ARRAY ADT
*/

typedef struct
{
    int *data;
    int size;
    int index;
} ArrayADT;

void init_array(ArrayADT *array);

void _dealloc_dist(double **arr, int edgelines);

#endif