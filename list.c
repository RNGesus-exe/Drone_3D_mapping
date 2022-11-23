#include "list.h"

/*
    Initializes list with initial size of 10
*/
void init_list(List *list)
{
    list->data = (Pair *)malloc((sizeof(Pair) * 10));
    list->length = 10;
    list->index = 0;
}

void init_pair(int _x, int _y, Pair *pair)
{
    pair->x = _x;
    pair->y = _y;
}

/*
    Inserts element at last index
    @param value Value to be inserted
    @param list List to insert in
*/
void push(Pair value, List *list)
{
    if (list->index < list->length)
    {
        list->data[list->index] = value;
        list->index++;
    }
    else
    {
        Pair *temp = (Pair *)malloc(sizeof(Pair) * list->length);
        for (int i = 0; i < list->length; i++)
        {
            temp[i] = list->data[i];
        }
        list->data = (Pair *)malloc(((list->length + 10) * sizeof(Pair)));
        list->length += 10;
        for (int i = 0; i < list->length - 10; i++)
        {
            list->data[i] = temp[i];
        }
        free(temp);
        temp = NULL;

        list->data[list->index] = value;
        list->index++;
    }
}

Pair element_at(int index, List *list)
{
    if (index <= list->length)
    {
        return list->data[index];
    }
}

/*
    Shrinks size of list if more than 10 cells are unused
*/
void __shrink_list(List *list)
{
    if (list->length - list->index >= 10)
    {
        Pair *temp = (Pair *)malloc(sizeof(Pair) * list->index);
        for (int i = 0; i < list->index; i++)
        {
            temp[i] = list->data[i];
        }
        free(list->data);
        list->data = (Pair *)malloc(sizeof(Pair) * list->index);
        for (int i = 0; i < list->index; i++)
        {
            list->data[i] = temp[i];
        }
        free(temp);
        list->length -= 10;
    }
}
void remove_at(int index, List *list)
{
    if (index < 0 || index > list->index)
    {
        fprintf(stderr, "Index out of range");
        return;
    }
    for (int i = index; i < list->length - 1; i++)
    {
        Pair temp = list->data[i];
        list->data[i] = list->data[i + 1];
        list->data[i + 1] = temp;
    }
    list->index--;
    // shrink the array if index - length > 10
    __shrink_list(list);
}

void print_list(List *list)
{
    for (int i = 0; i < list->index; i++)
    {
        print_pair(element_at(i, list));
    }
}

void dealloc_list(List *list)
{
    free(list->data);
}

void list_clear(List *list)
{
    dealloc_list(list);
    init_list(list);
}

void print_pair(Pair pair)
{
    printf("(%d,%d)\n", pair.x, pair.y);
}

bool isInRange(int val, List *edges, int range)
{
    for (int i = 0; i < edges->index; i++)
    {
        if ((val > (edges->data[i].y - range)) &&
            (val < (edges->data[i].y + range)))
        {
            return false;
        }
    }
    return true;
}

/////////////////////////// ARRAY ADT  /////////////////////////////////////////////

void _dealloc_dist(double **arr, int edgelines)
{
    if (arr == NULL)
        return;
    for (int i = 0; i < edgelines; i++)
    {
        free(arr[i]);
    }
    free(arr);
}