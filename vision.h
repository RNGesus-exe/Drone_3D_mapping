#ifndef VISION_H
#define VISION_H

#include <math.h>
#include "bitmap.h"
#include "list.h"

enum error_msg
{
    PATCH_SIZE_INCORRECT = -2,
    BORDER_ERROR,
    ALL_OK,
    NO_EDGES,
    INDEX_OUT_OF_RANGE,
};

typedef struct _edge_points
{
    List *leftEdgePoints;
    List *rightEdgePoints;
} EdgePoints;

void init(int, EdgePoints *, const char *, BitMap *, const char *, BitMap *);

enum error_msg createBorder(BitMap *, int, int, int, int);

void drawBordersFromVec(BitMap *right_img, List *edges, int patchSize);

enum error_msg applyEdgeDetection(EdgePoints *, BitMap *, int, int);

enum error_msg sobelTemplateMatchOnRow(EdgePoints *edges, BitMap *left_img, BitMap *right_img, int patchSize, int edgeLines);

void sobelTemplateMatch(EdgePoints *edges, BitMap *left_img, BitMap *right_img, int rowNo, int patchSize, int rowOffset);

/*
    Applies Sobel Operator on a patch to confirm if matched area is actually an edge.
    @param right_img - Reference to the struct of image from right camera
    @param patchSize - Size of patch
    @param x1 - X-Coordinate of top-left corner of the patch
    @param y1 - Y-Coordinate of top-left corner of the patch
    @param x2 - X-Coordinate of bottom-right corner of the patch
    @param y2 - Y-Coordinate of bottom-right corner of the patch
    @param horizontalFlag - True if you want to apply horizontal kernal
    @param verticalFlag - True if you want to apply vertical kernal
    @param cleanUp - True if you want to binarize the edges using Salt & Pepper algorithm

*/
bool applySobelEdgeDetectionOnPatch(BitMap *right_img, int patchSize, int x1, int y1, int x2, int y2, bool horizontalFlag, bool verticalFlag, bool cleanUp);

/*
    Calculates histograms of both Images and compares them.

    @param left_img - Reference to left image
    @param right_img - Reference to right image
    @param x1 - X-Coordinate of top-left corner of the patch from left image
    @param y1 - Y-Coordinate of top-left corner of the patch from left image
    @param x2 - X-Coordinate of bottom-right corner of the patch from left image
    @param y2 - Y-Coordinate of bottom-right corner of the patch from left image
    @param _x1 - X-Coordinate of top-left corner of the patch from right image
    @param _y1 - Y-Coordinate of top-left corner of the patch from right image
    @param _x2 - X-Coordinate of bottom-right corner of the patch from right image
    @param _y2 - Y-Coordinate of bottom-right corner of the patch from right image
    @returns True if histograms have atleast 60% match else False
*/
bool checkHistogram(BitMap *left_img, BitMap *right_img, int x1, int y1, int x2, int y2, int _x1, int _y1, int _x2, int _y2);

void deallocateBuffers(BitMap *, BitMap *, EdgePoints *, int);

#endif