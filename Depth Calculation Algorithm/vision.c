#include "vision.h"

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

void rotate_image(BitMap *img, double theta)
{
    int height = img->header.biHeight;
    int width = img->header.biWidth;
    //=========================CREATE NEW IMAGE BUFFER
    int **rotated_image = (int **)malloc(height * sizeof(int *));
    for (int i = 0; i < height; ++i)
    {
        rotated_image[i] = (int *)malloc(width * sizeof(int));
    }
    //====================STORE ROTATION IN NEW IMAGE BUFFER
    theta = 1.0 * theta * (PI / 180); // convert from degree to radian
    double new_col;
    double new_row;
    int row_mid = height / 2;
    int col_mid = width / 2;
    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            // Rotation
            new_row = (row * cos(theta)) - (col * sin(theta)) + (row_mid * (1 - cos(theta)) + (col_mid * sin(theta)));
            new_col = (row * sin(theta)) + (col * cos(theta)) + (col_mid * (1 - cos(theta))) - (row_mid * sin(theta));
            // printf("row = %d, col = %d, new_row = %d, new_col = %d\n", row, col, (int)new_row, (int)new_col);
            if ((int)round(new_row) >= 0 && (int)round(new_row) < height &&
                (int)round(new_col) >= 0 && (int)round(new_col) < width)
            {
                rotated_image[(int)round(new_row)][(int)(new_col)] = (int)img->grayscale_img[row][col];
            }
        }
    }

    //========================================EMPTY THE ORIGINAL IMAGE

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            img->modified_img[row][col] = img->grayscale_img[row][col] = 0;
        }
    }

    //===========================================COPY ROTATED IMAGE FROM NEW BUFFER TO OLD BUFFER

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            img->modified_img[row][col] =
                img->grayscale_img[row][col] = rotated_image[row][col];
        }
    }
    //============================================DEALLOCATE NEW BUFFER

    if (rotated_image)
    {
        for (int i = 0; i < height; ++i)
        {
            free(rotated_image[i]);
        }
        free(rotated_image);
        rotated_image = NULL;
    }
}

void apply_nearest_neighbour_filling(BitMap *image, int x1, int x2, int y1, int y2)
{
    int height = image->header.biHeight;
    int width = image->header.biWidth;
    int avg = 0;
    if (x1 < 0 || y1 < 0 || x2 >= height || y2 >= width)
    {
        perror("NEAREST NEIGHBOUR BOUNDARY ISSUE...");
        return;
    }
    for (int i = x1; i < x2; i++)
    {
        for (int j = y1; j < y2; j++)
        {
            if ((int)image->grayscale_img[i][j] == 0)
            {
                // calculate average of eight neighbors
                avg = (int)image->grayscale_img[i - 1][j - 1] +
                      (int)image->grayscale_img[i - 1][j] +
                      (int)image->grayscale_img[i - 1][j + 1] +
                      (int)image->grayscale_img[i][j - 1] +
                      (int)image->grayscale_img[i][j + 1] +
                      (int)image->grayscale_img[i + 1][j - 1] +
                      (int)image->grayscale_img[i + 1][j] +
                      (int)image->grayscale_img[i + 1][j + 1];
                avg /= 8;

                image->modified_img[i][j] = image->grayscale_img[i][j] = avg;
            }
        }
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (((i < x1 || i >= x2) || (j < y1 || j >= y2)) && ((int)image->grayscale_img[i][j] == 0))
            {
                image->modified_img[i][j] = image->grayscale_img[i][j] = 170;
            }
        }
    }
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

void move_fast_to_image(int ***fast_map, BitMap *image, int padding)
{
    int height = image->header.biHeight;
    int width = image->header.biWidth;

    for (int row = padding; row < height - padding; row++)
    {
        for (int col = padding; col < width - padding; col++)
        {
            image->modified_img[row][col] =
                (*fast_map)[row - padding][col - padding];
        }
    }
}

void fast_feature_detection(int ***fast_map, BitMap *image, int threshold, int n, int padding)
{
    int height = image->header.biHeight;
    int width = image->header.biWidth;
    const int RAD = 3;
    const int POINTS = 16;
    const int T = threshold;

    int i_1, i_5, i_9, i_13;
    int curr_i;
    int is_corner;

    for (int row = padding; row < height - padding; row++)
    {
        for (int col = padding; col < width - padding; col++)
        {
            // check if any 3 pixels fill
            curr_i = (int)image->grayscale_img[row][col];
            i_1 = (int)image->grayscale_img[row - RAD][col];
            i_5 = (int)image->grayscale_img[row][col + RAD];
            i_9 = (int)image->grayscale_img[row + RAD][col];
            i_13 = (int)image->grayscale_img[row][col - RAD];

            // if any 3 of these are brighter or darker than curr pixel than we can perform further analysis
            is_corner = check_if_possible_corner(curr_i, T, i_1, i_5, i_9, i_13);

            if (is_corner)
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

                (*fast_map)[row - padding][col - padding] = is_corner;
            }
            else
            {
                (*fast_map)[row - padding][col - padding] = 0;
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
            if (padded_img[row][col] == 0)
            {
                continue;
            }
            middleVal = padded_img[row][col];
            flag = 1;

            for (int i = 0; i < maskSize; i++)
            {
                for (int j = 0; j < maskSize; j++)
                {
                    if (i != j && padded_img[(row - maskSize / 2) + i][(col - maskSize / 2) + j] > middleVal)
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

enum error_msg applyEdgeDetection(BitMap *image, int padding)
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

    // 2) Apply Sobel edge detection and calculate gradient direction
    for (int edgeNo = padding; edgeNo < height - padding; edgeNo++)
    {
        int Gy = 0;
        int Gx = 0;

        for (int w = padding; w < width - padding; w++)
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

            image->modified_img[edgeNo][w] = sqrt(Gy * Gy + Gx * Gx);
        }
    }

    // 2.5) Fix sobel, this doesn't work for canny

    for (int edgeNo = padding; edgeNo < height - padding; edgeNo++)
    {
        for (int w = padding; w < width - padding; w++)
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
    return ALL_OK;
}

enum error_msg TemplateMatching(BitMap *left_img, BitMap *right_img, int padding, int patchSize, double ***distances)
{
    // Row Based template Matching

    if ((patchSize % 2 == 0) || patchSize < 3)
    {
        printf("Incorrect Patch Size\n");
        return PATCH_SIZE_INCORRECT;
    }

    size_t height = right_img->header.biHeight;
    size_t width = right_img->header.biWidth;

    const double BASELINE = 44;     // 4.4 cm
    const double FOCAL_LENGTH = 4;  // 4m,
    const double PIXEL_SIZE = 6.35; // 0.25 inch

    const int ROW_RANGE = 3;
    int best_ssd = INT_MAX;
    int curr_ssd = 0;
    int best_x = 0, best_y = 0;
    const double ERROR = 0.02;
    const int MAX_SSD = patchSize * patchSize * 255 * 255;

    int matches_found = 0;
    int total_matches = 0;

    // Take a template from left image and compare with right image
    for (int r1 = padding; r1 < height - padding; r1++)
    {
        for (int c1 = padding; c1 < width - padding; c1++)
        {
            // printf("r = %d, c= %d\n", r1, c1);
            best_ssd = INT_MAX;
            if (left_img->modified_img[r1][c1])
            {
                total_matches++;
                // As we are searching within the same row, we won't iterate over the entire image
                for (int r2 = r1 - ROW_RANGE; r2 < r1 + ROW_RANGE; r2++)
                {
                    for (int c2 = padding; c2 < width - padding; c2++)
                    {
                        // Calculate SAD
                        if (right_img->modified_img[r2][c2])
                        {
                            curr_ssd = 0;
                            for (int x = 0; x < patchSize; x++)
                            {
                                for (int y = 0; y < patchSize; y++)
                                {
                                    curr_ssd += myAbs(left_img->grayscale_img[r1][c1] - right_img->grayscale_img[r2][c2]);
                                }
                            }

                            if (curr_ssd < best_ssd)
                            {
                                // We need to replace the old right_image[r2][c2] with NULL
                                // right_img->modified_img[best_x][best_y] = 0;

                                best_ssd = curr_ssd;
                                best_x = r2;
                                best_y = c2;
                            }
                        }
                    }
                }

                if (!((myAbs(c1 - best_y) < (width / 2)) && (best_ssd < MAX_SSD * ERROR)))
                {
                    // If this body executes than the current point is not a good match
                    left_img->modified_img[r1][c1] = 0;
                }
                else
                {
                    matches_found++;

                    double dist;
                    int count = 0;
                    int drops = 0;
                    dist = FOCAL_LENGTH / PIXEL_SIZE * BASELINE / (double)myAbs(c1 - best_y);
                    // if (dist >= 300 && dist <= 600) // set a criteria for adding distances
                    // {
                    //     count++;
                    //     mList->list[i].data[j].distance = dis;
                    // }
                    // else
                    // {
                    //     remove_at(j, &mList->list[i]);
                    //     j--;
                    //     drops++;
                    // }
                    // printf("Distances inserted = %d\n Distances Dropped = %d\n", count, drops);

                    (*distances)[r1][c1] = dist;
                    // printf("Distance = %f\n", dist);
                }
            }
        }
    }

    printf("TOTAL MATCHES FOUND %d out of %d\n", matches_found, total_matches);
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

void merge_fast_edge_detection(int ***fast_map, BitMap *image, int padding)
{
    int height = image->header.biHeight;
    int width = image->header.biWidth;

    for (int row = padding; row < height - padding; row++)
    {
        for (int col = padding; col < width - padding; col++)
        {
            if (((*fast_map)[row - padding][col - padding] != 0) &&
                (image->modified_img[row][col] == 255))
            {
                image->modified_img[row][col] = (*fast_map)[row - padding][col - padding];
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

// void calculateDistances(int ***distances, BitMap *left)
// {
//     const double BASELINE = 44;     // 4.4 cm
//     const double FOCAL_LENGTH = 4;  // 4m,
//     const double PIXEL_SIZE = 6.35; // 0.25 inch
//     double dist;
//     int count = 0;
//     int drops = 0;
//     for (int i = 0; i < mList->length; i++)
//     {
//         // printf("Row = %d, Length = %d\n", mList->list[i].row, mList->list[i].length);
//         for (int j = 0; j < mList->list[i].length; j++)
//         {
//             dist = CAM_CONSTANT * BASELINE / myAbs(mList->list[i].data[j].left_y - mList->list[i].data[j].right_y);
//             if (dis >= 300 && dis <= 600) // set a criteria for adding distances
//             {
//                 count++;
//                 mList->list[i].data[j].distance = dis;
//             }
//             else
//             {
//                 remove_at(j, &mList->list[i]);
//                 j--;
//                 drops++;
//             }
//         }
//     }
//     printf("Distances inserted = %d\n Distances Dropped = %d\n", count, drops);
// }

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

void de_noise_image(BitMap *img, int mask_size, int padding)
{
    // assuming grayscale has already been applied + edge detection
    size_t height = img->header.biHeight;
    size_t width = img->header.biWidth;
    int flag;
    for (int i = padding; i < height - padding; i++)
    {
        for (int j = padding; j < width - padding; j++)
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