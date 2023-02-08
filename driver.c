#include "vision.h"

const int PATCH_SIZE = 50;
const int MASK_SIZE = 5;
const int FAST_SENSITIVITY = 10;

/*
TO DO

1) Improve Template Matching
2) Vertical Interpolation
3) Extrapolation
4) Load user defined options from params.txt
*/

void test(MultiList *mList)
{
    int sum = 0;
    for (int i = 0; i < mList->length; i++)
    {
        sum += mList->list[i].length;
    }
    printf("The amount of data in mList = %d\n", sum);
}

int main(void)
{
    clock_t t;
    t = clock();

    // DEFINE THE DATA STRUCTURES

    MultiList mList;
    BitMap left_img;
    BitMap right_img;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    const char *left_img_path_in = "./Images/imageA.bmp";
    const char *left_img_path_out = "./Images/imageA_w.bmp";
    const char *right_img_path_in = "./Images/imageB.bmp";
    const char *right_img_path_out = "./Images/imageB_w.bmp";

    init(&mList, left_img_path_in, &left_img, right_img_path_in, &right_img);
    printf("Images loaded successfully\n");

    // create a data struct to store the fast feature points in both images
    int inner_height = left_img.header.biHeight - 2 * (PATCH_SIZE + PADDING);
    int inner_width = left_img.header.biWidth - 2 * (PATCH_SIZE + PADDING);
    int **left_fast = (int **)malloc(inner_height * sizeof(int *));
    int **right_fast = (int **)malloc(inner_height * sizeof(int *));

    for (int i = 0; i < inner_height; i++)
    {
        left_fast[i] = (int *)malloc(inner_width * sizeof(int));
        right_fast[i] = (int *)malloc(inner_width * sizeof(int));
    }

    rotate_image(&right_img, 1); // To fix calibration issues
    apply_nearest_neighbour_filling(&right_img, PATCH_SIZE, inner_height + PATCH_SIZE,
                                    PATCH_SIZE, inner_width + PATCH_SIZE); // apply nearest neighbour filling

    // Apply FAST Corner + Sobel Operator on left image

    if (applyEdgeDetection(&mList, &left_img, PATCH_SIZE) != ALL_OK)
    {
        fprintf(stderr, "Error detection\n");
        return -1;
    }
    printf("EDGE DETECTION COMPLETE\n");
    fast_feature_detection(&left_fast, &left_img, FAST_SENSITIVITY, 12, PATCH_SIZE);
    apply_non_maxima_suppression(&left_fast, inner_height, inner_width, MASK_SIZE);
    merge_fast_edge_detection(&left_fast, &left_img, PATCH_SIZE);

    // Apply FAST Corner + Sobel Operator on right image

    if (applyEdgeDetection(&mList, &right_img, PATCH_SIZE) != ALL_OK)
    {
        fprintf(stderr, "Error detection\n");
        return -1;
    }
    printf("EDGE DETECTION COMPLETE\n");
    fast_feature_detection(&right_fast, &right_img, FAST_SENSITIVITY, 12, PATCH_SIZE);
    apply_non_maxima_suppression(&right_fast, inner_height, inner_width, MASK_SIZE);
    merge_fast_edge_detection(&right_fast, &right_img, PATCH_SIZE);

    // if (TemplateMatching(&mList, &left_img, &right_img, PATCH_SIZE) != ALL_OK)
    // {
    //     perror("Error finding Edges\n");
    //     return -1;
    // }
    // printf("TEMPLATE MATCHING COMPLETE\n");

    // improve template matching

    // calculateDistances(&mList);
    // printf("Distances have been calculated\n");

    // remove_outliers(&mList);
    // printf("Standard Deviation has been calculated\n");

    // find_min_max(&mList, &max_val, &min_val);
    // printf("Min & Max has been found...\n");

    // interpolateImage(&right_img, &mList, max_val, min_val);
    // printf("Interpolation has been applied\n");

    bmpWriteImage(&left_img, left_img_path_out);
    bmpWriteImage(&right_img, right_img_path_out);

    clean_up(&mList, &left_img, &right_img);

    if (left_fast != NULL)
    {
        for (size_t h = 0; h < inner_height; h++)
        {
            free(left_fast[h]);
        }
        free(left_fast);
        left_fast = NULL;
    }

    if (right_fast != NULL)
    {
        for (size_t h = 0; h < inner_height; h++)
        {
            free(right_fast[h]);
        }
        free(right_fast);
        right_fast = NULL;
    }

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds
    printf("Program took %f seconds to execute \n", time_taken);

    return 0;
}