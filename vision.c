#include "vision.h"

void init(int edgeLines, EdgePoints *edgePoints,
          const char *leftImagePathIn, BitMap *leftImage,
          const char *rightImagePathIn, BitMap *rightImage)
{
    bmpReadImage(leftImage, leftImagePathIn);
    bmpReadImage(rightImage, rightImagePathIn);
    edgePoints->leftEdgePoints = (List *)malloc(sizeof(List) * edgeLines);
    edgePoints->rightEdgePoints = (List *)malloc(sizeof(List) * edgeLines);
    for (int i = 0; i < edgeLines; i++)
    {
        init_list(&edgePoints->leftEdgePoints[i]);
        init_list(&edgePoints->rightEdgePoints[i]);
    }
}

enum error_msg applyEdgeDetection(EdgePoints *edgePoints, BitMap *image, int patchSize, int edgeLines)
{
    size_t height = image->header.biHeight;
    size_t width = image->header.biWidth;
    unsigned int areaToProcess = height - (patchSize * 2);
    unsigned int interval = (areaToProcess / edgeLines);
    unsigned int curr_interval = patchSize;
    for (int edgeNo = 0; edgeNo < edgeLines; edgeNo++, curr_interval += interval)
    {
        // printf("\n%d,%d", curr_interval, patchSize);
        if (((curr_interval - patchSize) < 0) ||
            ((curr_interval + patchSize >= height)))
        {
            printf("\nThe patchSize or rowNo is not valid for sobelEdgeDetection...");
            return PATCH_SIZE_INCORRECT;
        }

        // 1) Create a 1D array to hold the edgeData
        int *edgeData = (int *)malloc(width * (patchSize * 2) * sizeof(int));
        int Gy = 0;

        // 2) Define the sobel kernel
        const int sobelVerticalMask[3][3] = {
            {1, 0, -1},
            {2, 0, -2},
            {1, 0, -1}};

        // 3) Apply Sobel Operator on the row
        for (int w = patchSize; w < (width - patchSize); w++)
        {
            Gy = 0;

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    Gy += (image->grayscale_img[(curr_interval - 1 + i)][(w - 1 + j)] * sobelVerticalMask[i][j]);
                }
            }

            edgeData[w - patchSize] = sqrt(Gy * Gy);
        }

        // 4) Clean edges
        int counter = 0;
        for (int w = 0; w < (width - patchSize * 2); w++)
        {
            if (edgeData[w] >= 100)
            {
                // 4.1) First make the the edge into salt & pepper
                edgeData[w] = 255;

                // 4.2) Remove clusters of salts
                counter = 1;
                while (counter < patchSize)
                {
                    edgeData[w + counter] = 0;
                    counter++;
                }
                w += (counter - 1);
            }
            else
            {
                edgeData[w] = 0;
            }
        }

        // 5) Copy the edgeData into the modified image buffer
        for (int w = patchSize; w < (width - patchSize); w++)
        {
            image->modified_img[curr_interval][w] = edgeData[w - patchSize];
        }

        // 6) Deallocate the edgeData from heap
        free(edgeData);
        edgeData = NULL;

        // 7) Store the edge points in the list
        Pair pair;
        for (int w = patchSize; w < (width - patchSize); w++)
        {
            if ((int)image->modified_img[curr_interval][w] >= 100)
            {
                pair.x = curr_interval;
                pair.y = w;
                push(pair, &edgePoints->leftEdgePoints[edgeNo]);
            }
        }

        // 8) Draw borders
        // for (int i = 0; i < edgePoints->leftEdgePoints[edgeNo].index; i++)
        // {
        //     createBorder(image,
        //                  edgePoints->leftEdgePoints[edgeNo].data[i].x - (patchSize) / 2,
        //                  edgePoints->leftEdgePoints[edgeNo].data[i].y - (patchSize) / 2,
        //                  edgePoints->leftEdgePoints[edgeNo].data[i].x + (patchSize) / 2,
        //                  edgePoints->leftEdgePoints[edgeNo].data[i].y + (patchSize) / 2,
        //                  1);
        // }
    }
    return ALL_OK;
}

void deallocateBuffers(BitMap *leftImage, BitMap *rightImage, EdgePoints *edgePoints, int edgeLines)
{
    bmpDeallocateBuffer(leftImage);
    bmpDeallocateBuffer(rightImage);
    for (int i = 0; i < edgeLines; i++)
    {
        dealloc_list(&edgePoints->leftEdgePoints[i]);
        dealloc_list(&edgePoints->rightEdgePoints[i]);
    }
    free(edgePoints->leftEdgePoints);
    free(edgePoints->rightEdgePoints);
    edgePoints->leftEdgePoints =
        edgePoints->rightEdgePoints = NULL;
}

enum error_msg createBorder(BitMap *image, int x1, int y1, int x2, int y2, bool color)
{
    // printf("\n%d,%d,%d,%d", x1, x2, y1, y2);
    if ((x1 <= x2) &&
        (y1 <= y2))
    {
        if ((x1 >= 0) &&
            (x2 < image->header.biHeight) &&
            (y1 >= 0) &&
            (y2 < image->header.biWidth))
        { // (x1,y1) to (x1,y2) Top Line
            if (color)
            {
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    image->modified_img[x1][col] = 255;
                }

                // (x2,y1) to (x2,y2) Bottom Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    image->modified_img[x2][col] = 255;
                }

                // (x1,y1) to (x2,y1) Left Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    image->modified_img[row][y1] = 255;
                }

                // (x1,y2) to (x2,y2) Right Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    image->modified_img[row][y2] = 255;
                }
            }
            else
            {
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    image->modified_img[x1][col] = 0;
                }

                // (x2,y1) to (x2,y2) Bottom Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    image->modified_img[x2][col] = 0;
                }

                // (x1,y1) to (x2,y1) Left Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    image->modified_img[row][y1] = 0;
                }

                // (x1,y2) to (x2,y2) Right Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    image->modified_img[row][y2] = 0;
                }
            }
        }
        else
        {
            printf("\nThe coordinates are not within bounds of the image dimension...");
            return BORDER_ERROR;
        }
    }
    else
    {
        printf("\nThe coordinates do not seem to be correctly placed...");
        return BORDER_ERROR;
    }
}

void drawBordersFromVec(BitMap *right_img, List *edges, int patchSize, bool color)
{
    for (int i = 0; i < edges->index; i++)
    {
        createBorder(right_img, edges->data[i].x - (patchSize / 2), edges->data[i].y - (patchSize / 2),
                     edges->data[i].x + (patchSize / 2), edges->data[i].y + (patchSize / 2), color);
    }
}

int myPow(int x)
{
    return x * x;
}

int myAbs(int x)
{
    return (x < 0) ? -1 * x : x;
}

enum error_msg sobelTemplateMatching(EdgePoints *edges, BitMap *left_img, BitMap *right_img, int patchSize, int edgeLines)
{
    size_t height = right_img->header.biHeight;
    size_t width = right_img->header.biWidth;
    unsigned int areaToProcess = height - (patchSize * 2);
    unsigned int interval = (areaToProcess / edgeLines);
    unsigned int curr_interval = patchSize;

    int curr_ssd = 0;
    int best_ssd = INT_MAX;
    Pair best_coord;

    for (int edgeNo = 0; edgeNo < edgeLines; edgeNo++, curr_interval += interval)
    {

        if ((patchSize % 2 == 0) || patchSize < 3)
        {
            printf("Incorrect Patch Size\n");
            return PATCH_SIZE_INCORRECT;
        }
        if (((curr_interval - patchSize) < 0) &&
            ((curr_interval + patchSize) >= right_img->header.biHeight))
        {
            printf("Row Index is out of range...\n");
            return INDEX_OUT_OF_RANGE;
        }

        for (int match_no = 0; match_no < edges->leftEdgePoints[edgeNo].index; match_no++)
        {
            best_ssd = INT_MAX;

            for (int col = (patchSize - 1); col < (right_img->header.biWidth - (patchSize - 1)); col++)
            {

                curr_ssd = 0;
                for (int r = 0; r < patchSize; r++)
                {
                    for (int w = 0; w < patchSize; w++)
                    {
                        curr_ssd += myPow(myAbs(left_img->grayscale_img[(edges->leftEdgePoints[edgeNo].data[match_no].x - patchSize / 2) + r]
                                                                       [(edges->leftEdgePoints[edgeNo].data[match_no].y - patchSize / 2) + w] -
                                                right_img->grayscale_img[(edges->leftEdgePoints[edgeNo].data[match_no].x - patchSize / 2) + r]
                                                                        [(col - patchSize / 2) + w]));
                    }
                }

                // Compare current ssd with best ssd
                if (curr_ssd < best_ssd)
                {
                    best_ssd = curr_ssd;
                    best_coord.x = edges->leftEdgePoints[edgeNo].data[match_no].x;
                    best_coord.y = col;
                }
            }

            bool patchContainsEdge = applySobelEdgeDetectionOnPatch(right_img, patchSize, best_coord.x - (patchSize) / 2,
                                                                    best_coord.y - (patchSize) / 2, best_coord.x + (patchSize) / 2,
                                                                    best_coord.y + (patchSize) / 2);

            bool histogramMatches = checkHistogram(left_img, right_img, edges->leftEdgePoints[edgeNo].data[match_no].x - (patchSize / 2), edges->leftEdgePoints[edgeNo].data[match_no].y - (patchSize / 2), edges->leftEdgePoints[edgeNo].data[match_no].x + (patchSize / 2),
                                                   edges->leftEdgePoints[edgeNo].data[match_no].y + (patchSize / 2), best_coord.x - (patchSize / 2), best_coord.y - (patchSize / 2), best_coord.x + (patchSize / 2), best_coord.y + (patchSize / 2));

            histogramMatches = true; // Remove later on

            // DROP THE USELESS EDGE POINTS
            if ((abs(best_coord.y - edges->leftEdgePoints[edgeNo].data[match_no].y) < right_img->header.biWidth / 2) && // Check Distance between points
                (!edges->rightEdgePoints[edgeNo].index || (isInRange(best_coord.y, &edges->rightEdgePoints[edgeNo], patchSize))) &&
                patchContainsEdge &&
                histogramMatches)
            {
                push(best_coord, &edges->rightEdgePoints[edgeNo]);
            }
            else
            {
                remove_at(match_no, &edges->leftEdgePoints[edgeNo]);
                match_no--;
            }
        }
        drawBordersFromVec(right_img, &edges->rightEdgePoints[edgeNo], patchSize, 1);
        drawBordersFromVec(left_img, &edges->leftEdgePoints[edgeNo], patchSize, 1);
    }
    return ALL_OK;
}

bool applySobelEdgeDetectionOnPatch(BitMap *right_img, int patchSize, int x1, int y1, int x2, int y2)
{
    if (((x1 - patchSize) < 0) &&
        ((x1 + patchSize) >= right_img->header.biHeight) &&
        ((x2 - patchSize) < 0) &&
        ((x2 + patchSize) >= right_img->header.biHeight) &&
        ((y1 - patchSize) < 0) &&
        ((y1 + patchSize) >= right_img->header.biHeight) &&
        ((y2 - patchSize) < 0) &&
        ((y2 + patchSize) >= right_img->header.biHeight))
    {
        fprintf(stderr, "Patch is out of range.\n");
        return false;
    }
    bool hasEdge = false;
    // 1) Apply Constraints check
    if ((x1 <= x2) &&
        (y1 <= y2) &&
        (x1 > 0) &&
        (x2 < (right_img->header.biHeight - 1)) &&
        (y1 > 0) &&
        (y2 < (right_img->header.biWidth - 1)))
    {
        // 2) Allocating memory
        int **edgeData = (int **)malloc(((x2 - x1) + 1) * sizeof(int *));
        for (int r = 0; r <= (x2 - x1); r++)
        {
            edgeData[r] = (int *)malloc(((y2 - y1) + 1) * sizeof(int));
        }
        int Gy = 0;

        // 3) Define vertical and horizontal kernals
        const int sobelVerticalMask[3][3] = {
            {1, 0, -1},
            {2, 0, -2},
            {1, 0, -1}};

        // 4) Apply Sobel Operation on the patch : S = sqrt( Gx^2 + Gy^2 ) {Gx: Horizontal Sobel, Gy: Vertical Sobel}
        for (int row = x1; row < (x2 + 1); row++)
        {
            for (int col = y1; col < (y2 + 1); col++)
            {
                Gy = 0;

                // Apply vertical sobel on 3x3 patch {Gy}
                for (int i = 0; i < 3; i++)
                {
                    for (int j = 0; j < 3; j++)
                    {
                        Gy += (right_img->grayscale_img[(row - 1) + i][(col - 1) + j] * sobelVerticalMask[i][j]);
                    }
                }
                edgeData[row - x1][col - y1] = (int)sqrt((int)pow(Gy, 2));
            }
        }

        // 5) Clean the Edges into salt and pepper
        for (int r = 0; r <= (x2 - x1); r++)
        {
            for (int w = 0; w <= (y2 - y1); w++)
            {
                if ((int)edgeData[r][w] >= 100)
                {
                    edgeData[r][w] = 255;
                }
                else
                {
                    edgeData[r][w] = 0;
                }
            }
        }

        // 6) Check if the patch has atleast patchSize amount of edges in it
        int amountOfEdges = 0;
        for (int r = 0; r <= (x2 - x1); r++)
        {
            for (int w = 0; w <= (y2 - y1); w++)
            {
                if ((int)edgeData[r][w] >= 100)
                {
                    amountOfEdges++;
                }
            }
        }

        if (amountOfEdges >= patchSize)
        {
            // 7.1) Copy the edgeData into the Real Image
            // for (int r = 0; r <= (x2 - x1); r++)
            // {
            //     for (int c = 0; c <= (y2 - y1); c++)
            //     {

            //         this->img[0][x1 + r][y1 + c] =
            //             this->img[1][x1 + r][y1 + c] =
            //                 this->img[2][x1 + r][y1 + c] = edgeData[r][c];
            //     }
            // }

            // 7.2) As we have found an edge and copied the data we can leave
            hasEdge = true;
        }

        // 8) Free patch memory
        for (int r = 0; r <= (x2 - x1); r++)
        {
            free(edgeData[r]);
        }
        free(edgeData);
        edgeData = NULL;
    }
    else
    {
        fprintf(stderr, "Something wrong in {applySobelEdgeDetectionOnPatch}\n");
    }
    return hasEdge;
}

bool checkHistogram(BitMap *left_img, BitMap *right_img, int x1, int y1, int x2, int y2, int _x1, int _y1, int _x2, int _y2)
{
    const int THRESHOLD = 25; // amount of pixels per index in arr
    const double FACTOR = 0.4;
    if ((x1 <= x2) && (y1 <= y2))
    {
        if ((x1 >= 0) && (x2 < left_img->header.biHeight) && (y1 >= 0) && (y2 < left_img->header.biWidth))
        {
            int left_hist[10] = {0};
            int right_hist[10] = {0};
            for (int r = x1; r <= x2; r++)
            {
                for (int c = y1; c <= y2; c++)
                {
                    if (((int)left_img->grayscale_img[r][c] >= 0) &&
                        ((int)left_img->grayscale_img[r][c] < 25))
                    {
                        left_hist[0]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 25) &&
                             ((int)left_img->grayscale_img[r][c] < 50))
                    {
                        left_hist[1]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 50) &&
                             ((int)left_img->grayscale_img[r][c] < 75))
                    {
                        left_hist[2]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 75) &&
                             ((int)left_img->grayscale_img[r][c] < 100))
                    {
                        left_hist[3]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 100) &&
                             ((int)left_img->grayscale_img[r][c] < 125))
                    {
                        left_hist[4]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 125) &&
                             ((int)left_img->grayscale_img[r][c] < 150))
                    {
                        left_hist[5]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 150) &&
                             ((int)left_img->grayscale_img[r][c] < 175))
                    {
                        left_hist[6]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 175) &&
                             ((int)left_img->grayscale_img[r][c] < 200))
                    {
                        left_hist[7]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 200) &&
                             ((int)left_img->grayscale_img[r][c] < 225))
                    {
                        left_hist[8]++;
                    }
                    else if (((int)left_img->grayscale_img[r][c] >= 225) &&
                             ((int)left_img->grayscale_img[r][c] <= 255))
                    {
                        left_hist[9]++;
                    }
                }

                for (int r = _x1; r <= _x2; r++)
                {
                    for (int c = _y1; c <= _y2; c++)
                    {
                        if (((int)right_img->grayscale_img[r][c] >= 0) &&
                            ((int)right_img->grayscale_img[r][c] < 25))
                        {
                            right_hist[0]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 25) &&
                                 ((int)right_img->grayscale_img[r][c] < 50))
                        {
                            right_hist[1]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 50) &&
                                 ((int)right_img->grayscale_img[r][c] < 75))
                        {
                            right_hist[2]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 75) &&
                                 ((int)right_img->grayscale_img[r][c] < 100))
                        {
                            right_hist[3]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 100) &&
                                 ((int)right_img->grayscale_img[r][c] < 125))
                        {
                            right_hist[4]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 125) &&
                                 ((int)right_img->grayscale_img[r][c] < 150))
                        {
                            right_hist[5]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 150) &&
                                 ((int)right_img->grayscale_img[r][c] < 175))
                        {
                            right_hist[6]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 175) &&
                                 ((int)right_img->grayscale_img[r][c] < 200))
                        {
                            right_hist[7]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 200) &&
                                 ((int)right_img->grayscale_img[r][c] < 225))
                        {
                            right_hist[8]++;
                        }
                        else if (((int)right_img->grayscale_img[r][c] >= 225) &&
                                 ((int)right_img->grayscale_img[r][c] <= 255))
                        {
                            right_hist[9]++;
                        }
                    }
                }
                // printf("LEFT = ");
                // for (int i = 0; i < 10; i++)
                // {
                //     printf("%d,", left_hist[i]);
                // }
                // printf("\tRIGHT = ");
                // for (int i = 0; i < 10; i++)
                // {
                //     printf("%d,", right_hist[i]);
                // }
                // printf("\n");
                for (int i = 0; i < 10; i++)
                {
                    if (abs(left_hist[i] - right_hist[i]) >= (THRESHOLD * FACTOR))
                    {
                        return false;
                    }
                }
                return true;
            }
        }
        else
        {
            fprintf(stderr, "The coordinates are not within the bounds of image\n");
        }
    }
    else
    {
        fprintf(stderr, "The coordinates do not seem to be correct\n");
    }
    return false;
}