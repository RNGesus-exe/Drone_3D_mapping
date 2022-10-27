#ifndef VISION_H
#define VISION_H

#include <math.h>
#include "bitmap.h"
#include "list.h"

enum error_msg
{
    PATCH_SIZE_INCORRECT = -2,
    BORDER_ERROR,
    ALL_OK
};

typedef struct _edge_points
{
    List *leftEdgePoints;
    List *rightEdgePoints;
} EdgePoints;

void init(int, EdgePoints *, const char *, BitMap *, const char *, BitMap *);

enum error_msg createBorder(BitMap *, int, int, int, int);

enum error_msg applyEdgeDetection(EdgePoints *, BitMap *, int, int);

void deallocateBuffers(BitMap *, BitMap *, EdgePoints *, int);

#endif