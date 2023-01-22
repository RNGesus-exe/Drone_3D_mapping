#ifndef VISION_H
#define VISION_H

#include <math.h>
#include <time.h>
#include "bitmap.h"
#include "list.h"

#define PADDING 0
#define INT_MAX 2147483647
#define INT_MIN -2147483648
#define MAX_GRAY_VAL 255
#define MIN_GRAY_VAL 80

enum error_msg
{
    PATCH_SIZE_INCORRECT = -2,
    BORDER_ERROR,
    ALL_OK,
    NO_EDGES,
    INDEX_OUT_OF_RANGE,

};

void init(MultiList *, const char *, BitMap *, const char *, BitMap *);

void clean_up(MultiList *, BitMap *, BitMap *);

enum error_msg applyEdgeDetection(MultiList *, BitMap *, int);

enum error_msg TemplateMatching(MultiList *, BitMap *, BitMap *, int);

void markPointsFromLists(BitMap *, MultiList *);

void calculateDistances(MultiList *);

void interpolateImage(BitMap *, MultiList *, int, int);

void de_noise_image(BitMap *, int, int);

void applyCleaning(BitMap *, int, int, int);

void storePointsInMultiList(BitMap *, MultiList *, int);

void remove_outliers(MultiList *);

void find_min_max(MultiList *, int *, int *);

void rotate_image(BitMap *, float);

void fast_feature_detection(int ***, BitMap *, int, int, int);

int check_if_possible_corner(int, int, int, int, int, int);

int check_if_corner(int, int, int *, int, int, int);

void move_fast_to_image(int ***, BitMap *, int);

void apply_non_maxima_suppression(int ***, int, int, int);

void merge_fast_edge_detection(int ***, BitMap *, int);

int myPow(int);

int myAbs(int);

#endif