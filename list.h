#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

void push(Pair, List *);

void print_pair(Pair);

Pair element_at(int, List *);

void __shrink_list(List *);

void remove_at(int, List *);

void print_list(List *);

void dealloc_list(List *);

void list_clear(List *);

bool is_inrange(int val, List *edges, int range);

#endif
