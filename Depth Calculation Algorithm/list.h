#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>

#define INIT_SIZE 10

/*
    PAIR ARRAY ADT
    By default the array is made of 10 size both in x and y dimension
*/

typedef struct _data_
{
    int left_y;
    int right_y;
    double distance;
} Data;

typedef struct _list_
{
    Data *data;
    int row;
    int size;
    int length;
} List;

typedef struct _2d_list_
{
    List *list;
    int length;
    int size;
} MultiList;

//----- _list_ methods ------

void init_list(List *); // PASSED

void insertAtEnd_list(Data, List *); // PASSED

void remove_at(int, List *);

void print_list(List *); // PASSED

//----- _2d_list_ methods ------

void init_multi_list(MultiList *); // PASSED

void multi_list_remove(int, int, MultiList *); // PASSED

void print_multi_list(MultiList *); // PASSED

void dealloc_multi_list(MultiList *); // PASSED

int is_row_exist(MultiList *, int); // PASSED

void insert_multi_list(int, Data, MultiList *); // PASSED

#endif