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

enum error_msg createBorder(BitMap *, int, int, int, int, bool);

enum error_msg markEdge(BitMap *image, Pair p);

void drawBordersFromVec(BitMap *right_img, List *edges, int patchSize, bool);

enum error_msg applyEdgeDetection(EdgePoints *, BitMap *, int, int);

/*
    Estimates depth from center of camera using triangulation
    @param a - Coordinate of object in left image
    @param b - Coordinate of object in right image
    @returns - Distance from center of cameras to object
*/
double triangulateDistance(Pair a, Pair b);

enum error_msg sobelTemplateMatching(EdgePoints *edges, BitMap *left_img, BitMap *right_img, int patchSize, int edgeLines);

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
bool applySobelEdgeDetectionOnPatch(BitMap *right_img, int patchSize, int x1, int y1, int x2, int y2);

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

/*
    Removes isolated white pixel of 6 neighbours
*/
void denoise(BitMap *image);

double maxDistanceInImage(EdgePoints *edges);

/*
    Linearly interpolates between a & b
    @param current_y - X-Coordinate of point to interpolate
*/
int linearInterpolate(Pair a, Pair b, int current_y);
/*
    Linearly extrapolate between a & b
    @param current_y - X-Coordinate of point to interpolate
*/
int linearExtrapolate(Pair a, Pair b, int current_y);

/*
    Linearly interpolates points between edges
    @param right_img - Image from right camera
    @param edges - Edge Points
*/
void interpolateImage(BitMap *right_img, EdgePoints *edges);

void deallocateBuffers(BitMap *, BitMap *, EdgePoints *, int);

int myPow(int);

int myAbs(int);

#endif