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
    unsigned int interval = (height / edgeLines);
    unsigned int curr_interval = patchSize;
    for (int edgeNo = 0; edgeNo < edgeLines; edgeNo++, curr_interval += interval)
    {

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
        for (int index = 0; index <= edgePoints->leftEdgePoints[edgeNo].length; index++)
        {
            // printf("\n%d,%d,%d,%d",
            //        element_at(index, edgePoints->leftEdgePoints[edgeData]);
            //        edgePoints->leftEdgePoints[edgeNo].data->x - (patchSize) / 2,
            //        edgePoints->leftEdgePoints[edgeNo].data->y - (patchSize) / 2,
            //        edgePoints->leftEdgePoints[edgeNo].data->x + (patchSize) / 2,
            //        edgePoints->leftEdgePoints[edgeNo].data->y + (patchSize) / 2);
            createBorder(image,
                         edgePoints->leftEdgePoints[edgeNo].data[index].x - (patchSize) / 2,
                         edgePoints->leftEdgePoints[edgeNo].data[index].y - (patchSize) / 2,
                         edgePoints->leftEdgePoints[edgeNo].data[index].x + (patchSize) / 2,
                         edgePoints->leftEdgePoints[edgeNo].data[index].y + (patchSize) / 2);
        }
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

enum error_msg createBorder(BitMap *image, int x1, int y1, int x2, int y2)
{
    if ((x1 <= x2) &&
        (y1 <= y2))
    {
        if ((x1 >= 0) &&
            (x2 < image->header.biHeight) &&
            (y1 >= 0) &&
            (y2 < image->header.biWidth))
        { // (x1,y1) to (x1,y2) Top Line
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