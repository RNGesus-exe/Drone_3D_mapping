#include "vision.h"

const int PADDING = 30;
const int PATCH_SIZE = 11;
const int MASK_SIZE = 5;
const int FAST_SENSITIVITY = 10;

/*
    Flow of Algorithm
    1) Load the left and right bmp images + Create Buffers
    2) Perform calibration of images (Rotate right camera image to best fit on left camera image)
    3) Apply Nearest Neighbour Filling to fix the issue of gaps in right image
    4) Apply Edge Detection on left image
    5) Apply Fast Feature Detection on left image
    6) Merge both Fast & Sobel output images for left camera
    7) Repeat steps 4 to 6 fo right image
    8) Delete the buffers used to merge Sobel & Fast Output images
    9) Row Based Template Matching (SSD)
    10) Create a Buffer to store the z-axis and find the distances using the Triangulation Formula
*/

/*
TO DO

1) Find Distances
2) Apply Interpolation
3) Apply to Real World Coordinate System

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
    const char *left_img_path_in = "./Images/left_1.bmp";
    const char *left_img_path_out = "./Images/left_1_w.bmp";
    const char *right_img_path_in = "./Images/right_1.bmp";
    const char *right_img_path_out = "./Images/right_1_w.bmp";

    init(&mList, left_img_path_in, &left_img, right_img_path_in, &right_img);
    printf("Images loaded successfully\n");

    // create a data struct to store the fast feature points in both images
    int left_inner_height, right_inner_height;
    left_inner_height = left_img.header.biHeight - 2 * PADDING;
    right_inner_height = right_img.header.biHeight - 2 * PADDING;
    int left_inner_width, right_inner_width;
    left_inner_width = left_img.header.biWidth - 2 * PADDING;
    right_inner_width = right_img.header.biWidth - 2 * PADDING;

    int **left_fast = (int **)malloc(left_inner_height * sizeof(int *));
    int **right_fast = (int **)malloc(right_inner_height * sizeof(int *));

    for (int i = 0; i < left_inner_height; i++)
    {
        left_fast[i] = (int *)malloc(left_inner_width * sizeof(int));
    }
    for (int i = 0; i < right_inner_height; i++)
    {
        right_fast[i] = (int *)malloc(right_inner_width * sizeof(int));
    }

    // 1) Calibrate the two images

    rotate_image(&right_img, 1); // To fix calibration issues
    apply_nearest_neighbour_filling(&right_img, PADDING, left_inner_height + PADDING,
                                    PADDING, left_inner_width + PADDING); // apply nearest neighbour filling

    printf("SUCCESS: Calibration complete\n");

    // 2) Apply FAST Corner + Sobel Operator on left image

    if (applyEdgeDetection(&left_img, PADDING) != ALL_OK)
    {
        fprintf(stderr, "Error detection\n");
        return -1;
    }
    fast_feature_detection(&left_fast, &left_img, FAST_SENSITIVITY, 12, PADDING);
    apply_non_maxima_suppression(&left_fast, left_inner_height, left_inner_width, MASK_SIZE);
    merge_fast_edge_detection(&left_fast, &left_img, PADDING);

    printf("SUCCESS: Left camera iamge edge detection complete\n");

    // 3) Apply FAST Corner + Sobel Operator on right image

    if (applyEdgeDetection(&right_img, PADDING) != ALL_OK)
    {
        fprintf(stderr, "Error detection\n");
        return -1;
    }
    fast_feature_detection(&right_fast, &right_img, FAST_SENSITIVITY, 12, PADDING);
    apply_non_maxima_suppression(&right_fast, right_inner_height, right_inner_width, MASK_SIZE);
    merge_fast_edge_detection(&right_fast, &right_img, PADDING);
    printf("SUCCESS: Right camera image edge detection complete\n");

    if (left_fast != NULL)
    {
        for (size_t h = 0; h < left_inner_height; h++)
        {
            free(left_fast[h]);
        }
        free(left_fast);
        left_fast = NULL;
    }

    if (right_fast != NULL)
    {
        for (size_t h = 0; h < right_inner_height; h++)
        {
            free(right_fast[h]);
        }
        free(right_fast);
        right_fast = NULL;
    }

    printf("SUCCESS: Clean up successful\n");

    double **distances = (double **)malloc(left_img.header.biHeight * sizeof(double *));

    for (int i = 0; i < left_img.header.biHeight; i++)
    {
        distances[i] = (double *)malloc(left_img.header.biWidth * sizeof(double));
        for (int j = 0; j < left_img.header.biWidth; j++)
        {
            distances[i][j] = 0;
        }
    }

    if (TemplateMatching(&left_img, &right_img, PADDING, PATCH_SIZE, &distances) != ALL_OK)
    {
        perror("Error finding Edges\n");
        return -1;
    }
    printf("SUCCESS: TEMPLATE MATCHING COMPLETE && DISTANCES CALCULATED\n");

    // TEST MODULE

    const int ROW_RANGE = 50;
    double sum = 0;
    double last_sum = 0;
    int count = 0;
    for (int c = PADDING; c < left_img.header.biWidth - PADDING; c++)
    {
        count = 0;
        sum = 0;
        for (int r = left_img.header.biHeight / 2 - ROW_RANGE; r < left_img.header.biHeight / 2 + ROW_RANGE; r++)
        {
            if (distances[r][c])
            {
                count++;
                sum += distances[r][c];
            }
        }

        if (count)
        {
            sum /= count;
            last_sum = sum;
        }
        else
        {
            // sum = last_sum;
        }
        printf("%f,", sum);
    }
    printf("\n");

    // remove_outliers(&mList);
    // printf("Standard Deviation has been calculated\n");

    // find_min_max(&mList, &max_val, &min_val);
    // printf("Min & Max has been found...\n");

    // interpolateImage(&right_img, &mList, max_val, min_val);
    // printf("Interpolation has been applied\n");

    bmpWriteImage(&left_img, left_img_path_out);
    bmpWriteImage(&right_img, right_img_path_out);

    if (distances != NULL)
    {
        for (int h = 0; h < left_img.header.biHeight; h++)
        {
            free(distances[h]);
        }
        free(distances);
        distances = NULL;
    }

    clean_up(&mList, &left_img, &right_img);

    t = clock() - t;
    double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds
    printf("Program took %.8f seconds to execute \n", time_taken);

    return 0;
}