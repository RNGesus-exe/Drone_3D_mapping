#include "vision.h"

#define POINTS 16

void init(MultiList *mList,
          const char *leftImagePathIn, BitMap *leftImage,
          const char *rightImagePathIn, BitMap *rightImage)
{
    bmpReadImage(leftImage, leftImagePathIn);
    bmpReadImage(rightImage, rightImagePathIn);
    init_multi_list(mList);
}

void clean_up(MultiList *mList, BitMap *left_img, BitMap *right_img)
{
    dealloc_multi_list(mList);
    bmpDeallocateBuffer(left_img);
    bmpDeallocateBuffer(right_img);
}

void rotate_image(BitMap *img, float angle)
{
}

int check_if_possible_corner(int curr, int T, int i_1, int i_5, int i_9, int i_13)
{
    // 1 : darker , 2: brighter
    if (curr - T >= i_1 && curr - T >= i_5 && curr - T >= i_9)
    {
        return 1;
    }
    else if (curr + T <= i_1 && curr + T <= i_5 && curr + T <= i_9)
    {
        return 2;
    }
    else if (curr - T >= i_5 && curr - T >= i_9 && curr - T >= i_13)
    {
        return 1;
    }
    else if (curr + T <= i_5 && curr + T <= i_9 && curr + T <= i_13)
    {
        return 2;
    }
    else if (curr - T >= i_9 && curr - T >= i_13 && curr - T >= i_1)
    {
        return 1;
    }
    else if (curr + T <= i_9 && curr + T <= i_13 && curr + T <= i_1)
    {
        return 2;
    }
    else if (curr - T >= i_13 && curr - T >= i_1 && curr - T >= i_5)
    {
        return 1;
    }
    else if (curr + T <= i_13 && curr + T <= i_1 && curr + T <= i_5)
    {
        return 2;
    }
    return 0;
}

int check_if_corner(int curr, int T, int *arr, int size, int flag, int n)
{
    int score = curr;
    // check if dimmer
    if (flag == 1)
    {
        for (int i = 0; i < size; i++)
        {
            char flag = '0';

            for (int j = 0; j < n; j++)
            {
                if (curr - T < arr[(i + j) % size])
                {
                    flag = '1';
                    break;
                }
            }

            if (flag == '0')
            {
                for (int i = 0; i < size; i++)
                {
                    score = myAbs(score - arr[i]);
                }
                return score;
            }
        }
    }
    else // check if brighter
    {
        for (int i = 0; i < size; i++)
        {
            char flag = '0';

            for (int j = 0; j < n; j++)
            {
                if (curr + T > arr[(i + j) % size])
                {
                    flag = '1';
                    break;
                }
            }

            if (flag == '0')
            {
                for (int i = 0; i < size; i++)
                {
                    score = myAbs(score - arr[i]);
                }
                return score;
            }
        }
    }

    return 0;
}

void move_fast_to_image(int ***fast_map, BitMap *image, int patchSize)
{
    int height = image->header.biHeight;
    int width = image->header.biWidth;

    for (int row = (patchSize + PADDING); row < height - (patchSize + PADDING); row++)
    {
        for (int col = (patchSize + PADDING); col < width - (patchSize + PADDING); col++)
        {
            image->modified_img[row][col] =
                (*fast_map)[row - (patchSize + PADDING)][col - (patchSize + PADDING)];
        }
    }
}

void fast_feature_detection(int ***fast_map, BitMap *image, int threshold, int n, int patchSize)
{
    int height = image->header.biHeight;
    int width = image->header.biWidth;
    const int RAD = 3;
    const int T = threshold;

    int i_1, i_5, i_9, i_13;
    int curr_i;
    int is_corner;

    for (int row = (patchSize + PADDING); row < height - (patchSize + PADDING); row++)
    {
        for (int col = (patchSize + PADDING); col < width - (patchSize + PADDING); col++)
        {
            // check if any 3 pixels fill
            curr_i = (int)image->grayscale_img[row][col];
            i_1 = (int)image->grayscale_img[row - RAD][col];
            i_5 = (int)image->grayscale_img[row][col + RAD];
            i_9 = (int)image->grayscale_img[row + RAD][col];
            i_13 = (int)image->grayscale_img[row][col - RAD];

            // if any 3 of these are brighter or darker than curr pixel than we can perform further analysis
            is_corner = check_if_possible_corner(curr_i, T, i_1, i_5, i_9, i_13);

            if (is_corner != 0)
            {
                // Now check if any n cells are brighter or darker than the curr pixel
                int arr[POINTS];
                arr[0] = (int)image->grayscale_img[row - 3][col];
                arr[1] = (int)image->grayscale_img[row - 3][col + 1];
                arr[2] = (int)image->grayscale_img[row - 2][col + 2];
                arr[3] = (int)image->grayscale_img[row - 1][col + 3];
                arr[4] = (int)image->grayscale_img[row][col + 3];
                arr[5] = (int)image->grayscale_img[row + 1][col + 3];
                arr[6] = (int)image->grayscale_img[row + 2][col + 2];
                arr[7] = (int)image->grayscale_img[row + 3][col + 1];
                arr[8] = (int)image->grayscale_img[row + 3][col];
                arr[9] = (int)image->grayscale_img[row + 3][col - 1];
                arr[10] = (int)image->grayscale_img[row + 2][col - 2];
                arr[11] = (int)image->grayscale_img[row + 1][col - 3];
                arr[12] = (int)image->grayscale_img[row][col - 3];
                arr[13] = (int)image->grayscale_img[row - 1][col - 3];
                arr[14] = (int)image->grayscale_img[row - 2][col - 2];
                arr[15] = (int)image->grayscale_img[row - 3][col - 1];

                // printf(" i = %d, j= %d , flag = %d , curr = %d, threshold = %d \n", row, col, is_corner, curr_i, T);

                is_corner = check_if_corner(curr_i, T, arr, POINTS, is_corner, n);
            }

            if (is_corner != 0)
            {
                (*fast_map)[row - (patchSize + PADDING)][col - (patchSize + PADDING)] = is_corner;
            }
            else
            {
                (*fast_map)[row - (patchSize + PADDING)][col - (patchSize + PADDING)] = 0;
            }
        }
    }
}

void apply_non_maxima_suppression(int ***fast_map, int height, int width, int maskSize)
{
    // 1) Pad the image before applying non-maxima suppression
    const int PADDING_FACTOR = maskSize / 2;
    int **padded_img = (int **)malloc((height + (2 * PADDING_FACTOR)) * sizeof(int *));
    for (int i = 0; i < height + (2 * PADDING_FACTOR); ++i)
    {
        padded_img[i] = (int *)malloc((width + (2 * PADDING_FACTOR)) * sizeof(int));
    }

    // 2) copy the contents to the non-padded region
    for (unsigned int row = PADDING_FACTOR; row < (height + PADDING_FACTOR); ++row)
    {
        for (unsigned int col = PADDING_FACTOR; col < (width + PADDING_FACTOR); ++col)
        {
            padded_img[row][col] = (*fast_map)[row - PADDING_FACTOR][col - PADDING_FACTOR];
        }
    }

    // 3) Copy zeroes to padded region
    //{TOP}
    for (int row = 0; row < PADDING_FACTOR; row++)
    {
        for (unsigned int col = 0; col < width + 2 * PADDING_FACTOR; ++col)
        {
            padded_img[row][col] = 0;
        }
    }
    //{BOTTOM}
    for (int row = height + PADDING_FACTOR; row < height + 2 * PADDING_FACTOR; row++)
    {
        for (unsigned int col = 0; col < width + 2 * PADDING_FACTOR; ++col)
        {
            padded_img[row][col] = 0;
        }
    }
    //{LEFT}
    for (unsigned int row = PADDING_FACTOR; row < height + PADDING_FACTOR; ++row)
    {
        for (unsigned int col = 0; col < PADDING_FACTOR; ++col)
        {
            padded_img[row][col] = 0;
        }
    }
    //{RIGHT}
    for (unsigned int row = PADDING_FACTOR; row < height + PADDING_FACTOR; ++row)
    {
        for (unsigned int col = width + PADDING_FACTOR; col < width + 2 * PADDING_FACTOR; ++col)
        {
            padded_img[row][col] = 0;
        }
    }

    // 4) Create a 2d memory for non_maxima suppression
    // int **non_maxima = (int **)malloc(height * sizeof(int *));
    // for (int i = 0; i < height; i++)
    // {
    //     non_maxima[i] = (int *)malloc(width * sizeof(int));
    // }

    // 5) Now slide the window over the 2D template
    int middleVal = 0;
    int flag = 1;
    for (int row = PADDING_FACTOR; row < height + PADDING_FACTOR; row++)
    {
        for (int col = PADDING_FACTOR; col < width + PADDING_FACTOR; col++)
        {
            middleVal = padded_img[row][col];
            flag = 1;

            for (int i = 0; i < maskSize; i++)
            {
                for (int j = 0; j < maskSize; j++)
                {
                    if (padded_img[row - maskSize / 2 + i][col - maskSize / 2 + j] > middleVal)
                    {
                        (*fast_map)[row - PADDING_FACTOR][col - PADDING_FACTOR] = 0;
                        flag = 0;
                        break;
                    }
                }
                if (!flag)
                {
                    break;
                }
            }

            if (flag)
            {
                (*fast_map)[row - PADDING_FACTOR][col - PADDING_FACTOR] = 255;
            }
        }
    }

    if (padded_img)
    {
        for (int i = 0; i < height + 2 * PADDING_FACTOR; i++)
        {
            free(padded_img[i]);
        }
        free(padded_img);
        padded_img = NULL;
    }
}

enum error_msg applyEdgeDetection(MultiList *mList, BitMap *image, int patchSize)
{
    int height = image->header.biHeight;
    int width = image->header.biWidth;

    // 1) Define the sobel kernel
    const int sobelVerticalMask[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}};
    const int sobelHorizontalMask[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};

    // int inner_height = height - 2 * (patchSize + PADDING);
    // int inner_width = width - 2 * (patchSize + PADDING);
    // double **gradient_dir = (double **)malloc(inner_height * sizeof(double *));
    // unsigned char **non_maxima_supp = (unsigned char **)malloc(inner_height * sizeof(unsigned char *));

    // for (int i = 0; i < inner_height; i++)
    // {
    //     gradient_dir[i] = (double *)malloc(inner_width * sizeof(double));
    //     non_maxima_supp[i] = (unsigned char *)malloc(inner_width * sizeof(unsigned char));
    // }

    // 2) Apply Sobel edge detection and calculate gradient direction
    for (int edgeNo = (patchSize + PADDING); edgeNo < (height - (patchSize + PADDING)); edgeNo++)
    {
        int Gy = 0;
        int Gx = 0;

        for (int w = (patchSize + PADDING); w < (width - (patchSize + PADDING)); w++)
        {
            Gy = 0;
            Gx = 0;

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    Gy += ((int)image->grayscale_img[(edgeNo - 1 + i)][(w - 1 + j)] * sobelVerticalMask[i][j]);
                    Gx += ((int)image->grayscale_img[(edgeNo - 1 + i)][(w - 1 + j)] * sobelHorizontalMask[i][j]);
                }
            }

            // gradient_dir[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)] = (atan2(Gy, Gx) * 180.0 / PI) + 180;
            image->modified_img[edgeNo][w] = sqrt(Gy * Gy + Gx * Gx);
        }
    }

    // 2.5) Fix sobel, this doesn't work for canny

    for (int edgeNo = (patchSize + PADDING); edgeNo < (height - (patchSize + PADDING)); edgeNo++)
    {
        for (int w = (patchSize + PADDING); w < (width - (patchSize + PADDING)); w++)
        {

            if (image->modified_img[edgeNo][w] > 125)
            {
                image->modified_img[edgeNo][w] = 255;
            }
            else
            {
                image->modified_img[edgeNo][w] = 0;
            }
        }
    }

    // 3) Apply non maxima suppression

    // int q, r;
    // double gradient_val;
    // for (int edgeNo = (patchSize + PADDING); edgeNo < (height - (patchSize + PADDING)); edgeNo++)
    // {
    //     for (int w = (patchSize + PADDING); w < (width - (patchSize + PADDING)); w++)
    //     {
    //         q = 255;
    //         r = 255;
    //         gradient_val = gradient_dir[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)];

    //         // angles
    //         if ((0 <= gradient_val && gradient_val < 22.5) ||
    //             (157.5 <= gradient_val && gradient_val <= 180))
    //         {
    //             q = image->modified_img[edgeNo][w + 1];
    //             r = image->modified_img[edgeNo][w - 1];
    //         }
    //         else if (22.5 <= gradient_val && gradient_val < 67.5)
    //         {
    //             q = image->modified_img[edgeNo + 1][w - 1];
    //             r = image->modified_img[edgeNo - 1][w + 1];
    //         }
    //         else if (67.5 <= gradient_val && gradient_val < 112.5)
    //         {
    //             q = image->modified_img[edgeNo + 1][w];
    //             r = image->modified_img[edgeNo - 1][w];
    //         }
    //         else if (112.5 <= gradient_val && gradient_val < 157.5)
    //         {
    //             q = image->modified_img[edgeNo - 1][w - 1];
    //             r = image->modified_img[edgeNo + 1][w + 1];
    //         }

    //         if (image->modified_img[edgeNo][w] >= q && image->modified_img[edgeNo][w] >= r)
    //         {
    //             non_maxima_supp[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)] = image->modified_img[edgeNo][w];
    //         }
    //         else
    //         {
    //             non_maxima_supp[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)] = 0;
    //         }
    //     }
    // }

    // for (int edgeNo = (patchSize + PADDING); edgeNo < (height - (patchSize + PADDING)); edgeNo++)
    // {
    //     for (int w = (patchSize + PADDING); w < (width - (patchSize + PADDING)); w++)
    //     {
    //         image->modified_img[edgeNo][w] = non_maxima_supp[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)];
    //     }
    // }

    // // 4) DOUBLE THRESHOLD

    // double highThreshold = 0.15;
    // double lowThreshold = 0.09;

    // int max_val = INT_MIN;
    // for (int edgeNo = (patchSize + PADDING); edgeNo < (height - (patchSize + PADDING)); edgeNo++)
    // {
    //     for (int w = (patchSize + PADDING); w < (width - (patchSize + PADDING)); w++)
    //     {
    //         image->modified_img[edgeNo][w] = non_maxima_supp[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)];
    //         if (max_val < (int)image->modified_img[edgeNo][w])
    //         {
    //             max_val = (int)image->modified_img[edgeNo][w];
    //         }
    //     }
    // }

    // printf("Max Value = %d\n", max_val);

    // highThreshold *= max_val;
    // lowThreshold *= highThreshold;

    // highThreshold = 40;
    // lowThreshold = 25;

    // unsigned char **res = (unsigned char **)malloc(inner_height * sizeof(unsigned char *));
    // for (int i = 0; i < inner_height; i++)
    // {
    //     res[i] = (unsigned char *)malloc(inner_width * sizeof(unsigned char));
    // }

    // int strong = 255;
    // int weak = 50;
    // int zero = 0;

    // for (int edgeNo = (patchSize + PADDING); edgeNo < (height - (patchSize + PADDING)); edgeNo++)
    // {
    //     for (int w = (patchSize + PADDING); w < (width - (patchSize + PADDING)); w++)
    //     {
    //         if ((int)image->modified_img[edgeNo][w] >= highThreshold)
    //         {
    //             res[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)] = strong;
    //         }
    //         else if ((int)image->modified_img[edgeNo][w] < lowThreshold)
    //         {
    //             res[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)] = zero;
    //         }
    //         else
    //         {
    //             res[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)] = weak;
    //         }
    //     }
    // }

    // for (int edgeNo = (patchSize + PADDING); edgeNo < (height - (patchSize + PADDING)); edgeNo++)
    // {
    //     for (int w = (patchSize + PADDING); w < (width - (patchSize + PADDING)); w++)
    //     {
    //         image->modified_img[edgeNo][w] = res[edgeNo - (patchSize + PADDING)][w - (patchSize + PADDING)];
    //     }
    // }

    // // 5) Hysteresis Threshold

    // for (int edgeNo = (patchSize + PADDING) + 1; edgeNo < (height - (patchSize + PADDING)) - 1; edgeNo++)
    // {
    //     for (int w = (patchSize + PADDING) + 1; w < (width - (patchSize + PADDING)) - 1; w++)
    //     {
    //         if (image->modified_img[edgeNo][w] == weak)
    //         {
    //             if ((image->modified_img[edgeNo + 1][w - 1] == strong) ||
    //                 (image->modified_img[edgeNo + 1][w] == strong) ||
    //                 (image->modified_img[edgeNo + 1][w + 1] == strong) ||
    //                 (image->modified_img[edgeNo][w - 1] == strong) ||
    //                 (image->modified_img[edgeNo][w + 1] == strong) ||
    //                 (image->modified_img[edgeNo - 1][w - 1] == strong) ||
    //                 (image->modified_img[edgeNo - 1][w] == strong) ||
    //                 (image->modified_img[edgeNo - 1][w + 1] == strong))
    //             {
    //                 image->modified_img[edgeNo][w] = strong;
    //             }
    //             else
    //             {
    //                 image->modified_img[edgeNo][w] = zero;
    //             }
    //         }
    //     }
    // }

    // 6) Deallocation of buffers
    // if (gradient_dir != NULL)
    // {
    //     for (size_t h = 0; h < inner_height; h++)
    //     {
    //         free(gradient_dir[h]);
    //     }
    //     free(gradient_dir);
    //     gradient_dir = NULL;
    // }

    // if (res != NULL)
    // {
    //     for (size_t h = 0; h < inner_height; h++)
    //     {
    //         free(res[h]);
    //     }
    //     free(res);
    //     res = NULL;
    // }

    // if (non_maxima_supp != NULL)
    // {
    //     for (size_t h = 0; h < inner_height; h++)
    //     {
    //         free(non_maxima_supp[h]);
    //     }
    //     free(non_maxima_supp);
    //     non_maxima_supp = NULL;
    // }

    return ALL_OK;
}

void storePointsInMultiList(BitMap *image, MultiList *mList, int patchSize)
{
    // 4) Store the edge points in the list
    size_t height = image->header.biHeight;
    size_t width = image->header.biWidth;

    Data data;
    for (int edgeNo = (patchSize + PADDING); edgeNo < (height - (patchSize + PADDING)); edgeNo++)
    {
        for (int w = (patchSize + PADDING); w < (width - (patchSize + PADDING)); w++)
        {
            if (image->modified_img[edgeNo][w])
            {
                data.left_y = w;
                data.right_y = -1;
                data.distance = -1;
                insert_multi_list(edgeNo, data, mList);
            }
        }
    }
}

enum error_msg TemplateMatching(MultiList *mList, BitMap *left_img, BitMap *right_img, int patchSize)
{
    size_t height = right_img->header.biHeight;
    size_t width = right_img->header.biWidth;

    int count = 0;
    int curr_ssd = 0;
    double error = 0.02;
    int max_ssd = patchSize * patchSize * 255 * 255;
    int best_ssd = INT_MAX;
    int best_y = -1;
    int noOfEdges = 0;
    int iterations = 0;

    for (int edgeNo = 0; edgeNo < mList->length; edgeNo++)
    {
        if ((patchSize % 2 == 0) || patchSize < 3)
        {
            printf("Incorrect Patch Size\n");
            return PATCH_SIZE_INCORRECT;
        }

        for (int match_no = 0; match_no < mList->list[edgeNo].length; match_no++)
        {
            best_ssd = INT_MAX;

            for (int col = (patchSize + PADDING); col < (width - (patchSize + PADDING)); col++)
            {

                curr_ssd = 0;
                for (int r = 0; r < patchSize; r++)
                {
                    for (int w = 0; w < patchSize; w++)
                    {
                        curr_ssd += myAbs(left_img->grayscale_img[(mList->list[edgeNo].row - patchSize / 2) + r]
                                                                 [(mList->list[edgeNo].data[match_no].left_y - patchSize / 2) + w] -
                                          right_img->grayscale_img[(mList->list[edgeNo].row - patchSize / 2) + r]
                                                                  [(col - patchSize / 2) + w]);
                        // TURN THIS OFF WHEN RUNNING ON CLOUD
                        right_img->modified_img[(mList->list[edgeNo].row - patchSize / 2) + r][(col - patchSize / 2) + w] = 0;
                    }
                }

                // Compare current ssd with best ssd
                if (curr_ssd < best_ssd)
                {
                    best_ssd = curr_ssd;
                    best_y = col;
                }
            }
            // printf("left_y = %d, right_y = %d, row = %d , score = %d, viable_error = %d, width = %ld, distance = %d\n", mList->list[edgeNo].data[match_no].left_y, best_y, mList->list[edgeNo].row, best_ssd, (int)(max_ssd * error), (width / 2), myAbs(best_y - mList->list[edgeNo].data[match_no].left_y));
            // right_img->modified_img[mList->list[edgeNo].row][best_y] = 0;

            // DROP THE USELESS EDGE POINTS
            if ((myAbs(best_y - mList->list[edgeNo].data[match_no].left_y) < (width / 2)) &&
                (best_ssd < max_ssd * error))
            {
                noOfEdges++;
                mList->list[edgeNo].data[match_no].right_y = best_y;
            }
            else
            {
                count++;
                remove_at(match_no, &mList->list[edgeNo]);
                match_no--;
            }
            iterations++;
        }
    }
    // REMOVE
    markPointsFromLists(right_img, mList);
    printf("Total edges were = %d,\nAmount of edges which got matched = %d\n", iterations, noOfEdges);
    printf("Amount of edges removed = %d\n", count);
    return ALL_OK;
}

int myPow(int x)
{
    return x * x;
}

int myAbs(int x)
{
    return (x < 0) ? -1 * x : x;
}

void markPointsFromLists(BitMap *img, MultiList *mList)
{
    for (int i = 0; i < mList->length; i++)
    {
        for (int j = 0; j < mList->list[i].length; j++)
        {
            img->modified_img[mList->list[i].row][mList->list[i].data[j].right_y] = 255;
        }
    }
}

void merge_fast_edge_detection(int ***fast_map, BitMap *image, int patchSize)
{
    int height = image->header.biHeight;
    int width = image->header.biWidth;

    for (int row = (patchSize + PADDING); row < height - (patchSize + PADDING); row++)
    {
        for (int col = (patchSize + PADDING); col < width - (patchSize + PADDING); col++)
        {
            if (((*fast_map)[row - (patchSize + PADDING)][col - (patchSize + PADDING)] != 0) &&
                (image->modified_img[row][col] == 255))
            {
                image->modified_img[row][col] = (*fast_map)[row - (patchSize + PADDING)][col - (patchSize + PADDING)];
            }
            else
            {
                image->modified_img[row][col] = 0;
            }
        }
    }
}

void interpolateImage(BitMap *right_img, MultiList *mList, int max_val, int min_val)
{
    printf("min = %d, max = %d\n", min_val, max_val);
    const double val_fac = (double)(MAX_GRAY_VAL - MIN_GRAY_VAL) / (max_val - min_val);
    for (int i = 0; i < mList->length; i++)
    {
        if (mList->list[i].length >= 2)
        {
            for (int j = 0; j < mList->list[i].length - 1; j++)
            {
                // 255 - (curr_dis - min_dis) * ((255-min_gray_val)/(max_dis-min_dis))
                int start_val = MAX_GRAY_VAL - (mList->list[i].data[j].distance - min_val) * val_fac;
                int end_val = MAX_GRAY_VAL - (mList->list[i].data[j + 1].distance - min_val) * val_fac;
                int start_dis = mList->list[i].data[j].distance;
                int end_dis = mList->list[i].data[j + 1].distance;
                double pixel_fac = (double)(end_val - start_val) / myAbs(mList->list[i].data[j].right_y - mList->list[i].data[j + 1].right_y);
                for (int k = 0; k <= (mList->list[i].data[j + 1].right_y - mList->list[i].data[j].right_y); k++)
                {
                    right_img->modified_img[mList->list[i].row][k + mList->list[i].data[j].right_y] = start_val + (pixel_fac * k);
                }
            }
        }
    }
}

void calculateDistances(MultiList *mList)
{
    const double BASELINE = 21.1;
    const double CAM_CONSTANT = 2303.45;
    double dis;
    int count = 0;
    int drops = 0;
    for (int i = 0; i < mList->length; i++)
    {
        // printf("Row = %d, Length = %d\n", mList->list[i].row, mList->list[i].length);
        for (int j = 0; j < mList->list[i].length; j++)
        {
            dis = CAM_CONSTANT * BASELINE / myAbs(mList->list[i].data[j].left_y - mList->list[i].data[j].right_y);
            if (dis >= 300 && dis <= 600) // set a criteria for adding distances
            {
                count++;
                mList->list[i].data[j].distance = dis;
            }
            else
            {
                remove_at(j, &mList->list[i]);
                j--;
                drops++;
            }
        }
    }
    printf("Distances inserted = %d\n Distances Dropped = %d\n", count, drops);
}

void remove_outliers(MultiList *mList)
{
    double avg = 0;
    double sum_squared = 0;
    int count = 0;
    for (int i = 0; i < mList->length; i++)
    {
        for (int j = 0; j < mList->list[i].length; j++)
        {
            // printf("%d,", mList->list[i].data[j].distance);
            avg += mList->list[i].data[j].distance;
            sum_squared += myPow(mList->list[i].data[j].distance);
            count++;
        }
    }
    printf("avg = %f, sum_squared = %f, count = %d\n", avg, sum_squared, count);

    avg /= count; // Calculated the average of all the distances in the picture

    double std = sqrt((sum_squared - count * (avg * avg)) / (count - 1));

    printf("Standard deviation =  %f\n", std);
}

void find_min_max(MultiList *mList, int *max, int *min)
{
    int count = 0;
    for (int i = 0; i < mList->length; i++)
    {
        for (int j = 0; j < mList->list[i].length; j++)
        {
            if (*max < (int)mList->list[i].data[j].distance)
            {
                *max = (int)mList->list[i].data[j].distance;
            }
            if (*min > (int)mList->list[i].data[j].distance)
            {
                *min = (int)mList->list[i].data[j].distance;
            }

            // printf("Distance at Row = %d,  Lcol = %d, Rcol = %d is %fcm, min = %d, and max = %d\n",
            //        mList->list[i].row,
            //        mList->list[i].data[j].left_y,
            //        mList->list[i].data[j].right_y,
            //        mList->list[i].data[j].distance,
            //        *min,
            //        *max);
        }
    }
    printf("Amount of operations = %d\n", count);
}

void de_noise_image(BitMap *img, int mask_size, int patchSize)
{
    // assuming grayscale has already been applied + edge detection
    size_t height = img->header.biHeight;
    size_t width = img->header.biWidth;
    int flag;
    for (int i = patchSize + PADDING; i < (height - (patchSize + PADDING)); i++)
    {
        for (int j = patchSize + PADDING; j < (width - (patchSize + PADDING)); j++)
        {
            flag = 0;
            // upper region
            int k;
            for (k = 0; k < mask_size; k++)
            {
                if (img->modified_img[i][j + k])
                {
                    flag = 1;
                    break;
                }
            }
            if (flag)
            {
                continue;
            }

            // right region
            for (k = 0; k < mask_size; k++)
            {
                if (img->modified_img[i + k][j + mask_size - 1])
                {
                    flag = 1;
                    break;
                }
            }
            if (flag)
            {
                continue;
            }

            // left region
            for (k = 0; k < mask_size; k++)
            {
                if (img->modified_img[i + k][j])
                {
                    flag = 1;
                    break;
                }
            }
            if (flag)
            {
                continue;
            }

            // bottom region
            for (k = 0; k < mask_size; k++)
            {
                if (img->modified_img[i + mask_size - 1][j + k])
                {
                    flag = 1;
                    break;
                }
            }
            if (flag)
            {
                continue;
            }

            applyCleaning(img, i, j, mask_size);
        }
    }
}

void applyCleaning(BitMap *img, int x1, int y1, int mask_size)
{
    for (int i = 0; i < mask_size; i++)
    {
        for (int j = 0; j < mask_size; j++)
        {
            img->modified_img[i + x1][j + y1] = 0;
        }
    }
}