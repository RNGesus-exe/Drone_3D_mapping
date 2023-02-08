#include "list.h"

void init_list(List *list)
{
    list->data = (Data *)malloc((sizeof(Data) * INIT_SIZE));
    list->length = 0;
    list->size = INIT_SIZE;
    list->row = 0;
}

void init_multi_list(MultiList *container)
{
    container->list = (List *)malloc((sizeof(List) * INIT_SIZE));
    container->length = 0;
    container->size = INIT_SIZE;
    for (int i = 0; i < container->size; i++)
    {
        init_list(&container->list[i]);
    }
}

void print_multi_list(MultiList *container)
{
    for (int i = 0; i < container->length; i++)
    {
        if (container->list[i].length > 0)
        {
            printf("List# %d Row# %d Length# %d Size# %d\n",
                   i, container->list[i].row, container->list[i].length, container->list[i].size);
            print_list(&container->list[i]);
            printf("\n");
        }
    }
}

void insert_multi_list(int row, Data value, MultiList *container)
{
    int index = is_row_exist(container, row);
    if (index == -1)
    {
        if (container->length >= container->size)
        {
            // extend the size by INIT_SIZE
            List *temp = (List *)malloc(sizeof(List) * (container->size + INIT_SIZE));
            for (int i = 0; i < container->length; i++)
            {
                temp[i] = container->list[i];
            }
            container->size += INIT_SIZE;
            for (int i = container->length; i < container->size; i++)
            {
                init_list(temp + i);
            }
            free(container->list);
            container->list = temp;
            temp = NULL;
        }
        container->list[container->length].row = row;
        index = container->length++;
    }
    insertAtEnd_list(value, &container->list[index]);
}

void insertAtEnd_list(Data value, List *list)
{
    if (list->length >= list->size)
    {
        Data *temp = (Data *)malloc(sizeof(Data) * (list->size + INIT_SIZE));
        for (int i = 0; i < list->length; i++)
        {
            temp[i] = list->data[i];
        }
        list->size += INIT_SIZE;
        free(list->data);
        list->data = temp;
        temp = NULL;
    }
    list->data[list->length++] = value;
}

int is_row_exist(MultiList *container, int row)
{
    for (int i = 0; i < container->length; i++)
    {
        if (container->list[i].row == row)
        {
            return i;
        }
    }
    return -1;
}

void multi_list_remove(int row, int index, MultiList *container)
{
    remove_at(index, &container->list[row]);
}

void remove_at(int index, List *list)
{
    if (index < 0 || index > (int)list->length)
    {
        fprintf(stderr, "Trying to delete data that is out of range...");
        return;
    }

    for (int i = index; i < list->length - 1; i++)
    {
        list->data[i] = list->data[i + 1];
    }
    list->length--;
}

void print_list(List *list)
{
    for (int i = 0; i < list->length; i++)
    {
        printf("Index = %d left_y = %d right_y = %d distance= %f\n",
               i, list->data[i].left_y, list->data[i].right_y, list->data[i].distance);
    }
}

void dealloc_multi_list(MultiList *container)
{
    if (container != NULL)
    {
        for (int i = 0; i < container->size; i++)
        {
            free(container->list[i].data);
            container->list[i].data = NULL;
        }
        free(container->list);
        container->list = NULL;
    }
    else
    {
        fprintf(stderr, "Illegal free() called...");
    }
}