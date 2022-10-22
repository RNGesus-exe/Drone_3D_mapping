#include "truevision.h"

BitMap::BitMap(string left_path, string right_path)
{
    this->hasEdges = false;
    this->bmpHandle.getStreamHandle().readImages(left_path, right_path);
    this->bmpHandle.applyGrayscaleOnImages(true);
    this->bmpHandle.applyAutoContrastOnImages(true);
}

void BitMap::sobelTemplateMatch(int rowNo, int patchSize, int rowOffset, int edgeLineNo)
{
    vector<pair<int, int>> vec; // KEEP IT HERE, DON'T MOVE THIS VARIABLE BELOW

    if (!this->hasEdges)
    {
        throw runtime_error("You need to first find the edges before searching for the templates");
    }

    if (this->leftEdgePoints[edgeLineNo].size() <= 0)
    {
        // cerr << "\nNo Edges to match in row#" << edgeLineNo;
        this->rightEdgePoints.push_back(vec);
        return;
    }

    if ((patchSize % 2 == 0) || patchSize < 3)
    {
        throw runtime_error("Invalid Patch Size. Please make patch > 3 and odd");
    }

    if (((rowNo - patchSize) < 0) &&
        ((rowNo + patchSize) >= this->getHeight()))
    {
        throw runtime_error("\n Row index is out of range...");
    }

    int curr_ssd = 0;
    int best_ssd = INT32_MAX;
    pair<int, int> best_coord;
    for (int edgeNo = 0; edgeNo < this->leftEdgePoints[edgeLineNo].size(); edgeNo++)
    {
        best_ssd = INT32_MAX;
        for (int col = (patchSize - 1); col < (this->getWidth() - (patchSize - 1)); col++)
        {
            curr_ssd = 0;
            for (int r = 0; r < patchSize; r++)
            {
                for (int w = 0; w < patchSize; w++)
                {
                    curr_ssd += this->bmpHandle.pow(this->bmpHandle.abs(this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[(this->leftEdgePoints[edgeLineNo][edgeNo].first - (patchSize - 1) / 2) + r]
                                                                                                                                    [(this->leftEdgePoints[edgeLineNo][edgeNo].second - (patchSize - 1) / 2) + w] -
                                                                        this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[(this->leftEdgePoints[edgeLineNo][edgeNo].first + rowOffset - (patchSize - 1) / 2) + r]
                                                                                                                                     [(col - (patchSize - 1) / 2) + w]));
                }
            }
            // Compare curr_ssd with best_ssd
            if (curr_ssd < best_ssd)
            {
                best_ssd = curr_ssd;
                best_coord.first = this->leftEdgePoints[edgeLineNo][edgeNo].first + rowOffset;
                best_coord.second = col;
            }
        }

        // DROP THE USELESS EDGE POINTS
        if ((this->bmpHandle.abs(best_coord.second - leftEdgePoints[edgeLineNo][edgeNo].second) < this->getWidth() / 2) && // Check Distance between points
            ((!vec.size()) || (this->isInRange(best_coord.second, vec, patchSize))) &&
            (this->applySobelEdgeDetectionOnPatch(patchSize, best_coord.first - (patchSize) / 2, // Check Edges
                                                  best_coord.second - (patchSize) / 2, best_coord.first + (patchSize) / 2,
                                                  best_coord.second + (patchSize) / 2, false)) &&
            this->checkHistogram(leftEdgePoints[edgeLineNo][edgeNo].first - (patchSize) / 2, leftEdgePoints[edgeLineNo][edgeNo].second - (patchSize) / 2,
                                 leftEdgePoints[edgeLineNo][edgeNo].first + (patchSize) / 2, leftEdgePoints[edgeLineNo][edgeNo].second + (patchSize) / 2,
                                 best_coord.first - (patchSize) / 2, best_coord.second - (patchSize) / 2,
                                 best_coord.first + (patchSize) / 2, best_coord.second + (patchSize) / 2))
        {
            vec.push_back(best_coord);
        }
        else
        {
            this->leftEdgePoints[edgeLineNo].erase(this->leftEdgePoints[edgeLineNo].begin() + edgeNo);
            edgeNo--;
        }
    }
    this->rightEdgePoints.push_back(vec);
    this->bmpHandle.drawBordersFromVec(this->bmpHandle.getStreamHandle().getRightBmp(), vec, patchSize);
}

bool BitMap::isInRange(int val, vector<pair<int, int>> &vec, int range)
{
    for (auto v : vec)
    {
        if ((val > (v.second - range)) &&
            (val < (v.second + range)))
        {
            return false;
        }
    }
    return true;
}

bool BitMap::checkHistogram(int x1, int y1, int x2, int y2, int _x1, int _y1, int _x2, int _y2)
{
    const int FACTOR = 800;
    if ((x1 <= x2) &&
        (y1 <= y2))
    {
        if ((x1 >= 0) &&
            (x2 < this->getHeight()) &&
            (y1 >= 0) &&
            (y2 < this->getWidth()))
        {
            int left_hist[5] = {};
            int right_hist[5] = {};
            for (int r = x1; r <= x2; r++)
            {
                for (int c = y1; c <= y2; c++)
                {
                    if (((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] >= 0) &&
                        ((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] < 50))
                    {
                        left_hist[0]++;
                    }
                    else if (((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] >= 50) &&
                             ((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] < 100))
                    {
                        left_hist[1]++;
                    }
                    else if (((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] >= 100) &&
                             ((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] < 150))
                    {
                        left_hist[2]++;
                    }
                    else if (((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] >= 150) &&
                             ((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] < 200))
                    {
                        left_hist[3]++;
                    }
                    if (((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] >= 200) &&
                        ((int)this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[r][c] < 256))
                    {
                        left_hist[4]++;
                    }
                }
            }
            for (int r = _x1; r <= _x2; r++)
            {
                for (int c = _y1; c <= _y2; c++)
                {
                    if (((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] >= 0) &&
                        ((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] < 50))
                    {
                        right_hist[0]++;
                    }
                    else if (((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] >= 50) &&
                             ((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] < 100))
                    {
                        right_hist[1]++;
                    }
                    else if (((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] >= 100) &&
                             ((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] < 150))
                    {
                        right_hist[2]++;
                    }
                    else if (((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] >= 150) &&
                             ((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] < 200))
                    {
                        right_hist[3]++;
                    }
                    if (((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] >= 200) &&
                        ((int)this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[r][c] < 256))
                    {
                        right_hist[4]++;
                    }
                }
            }
            // cout << "LEFT = ";
            // for (int i = 0; i < 5; i++)
            // {
            //     cout << left_hist[i] << ",";
            // }
            // cout << "\nRIGHT = ";
            // for (int i = 0; i < 5; i++)
            // {
            //     cout << right_hist[i] << ",";
            // }
            // cout << endl;
            for (int i = 0; i < 5; i++)
            {
                if (this->bmpHandle.abs(left_hist[i] - right_hist[i]) >= FACTOR)
                {
                    return false;
                }
            }
            return true;
        }
        else
        {
            cerr << "\nThe coordinates are not within bounds of the image dimension...";
        }
    }
    else
    {
        cerr << "\nThe coordinates do not seem to be correctly placed...";
    }
    return false;
}

bool BitMap::applySobelEdgeDetectionOnPatch(int patchSize, int x1, int y1, int x2, int y2, bool horizontalFlag, bool verticalFlag, bool cleanUp)
{
    // 0) Apply a row check
    if (((x1 - patchSize) < 0) &&
        ((x1 + patchSize) >= this->getHeight()) &&
        ((x2 - patchSize) < 0) &&
        ((x2 + patchSize) >= this->getHeight()) &&
        ((y1 - patchSize) < 0) &&
        ((y1 + patchSize) >= this->getWidth()) &&
        ((y2 - patchSize) < 0) &&
        ((y2 + patchSize) >= this->getWidth()))
    {
        cerr << "\nThe patch is out of range...";
        return false;
    }

    bool hasEdge = false;
    // 1) Apply constraint checks
    if ((this->bmpHandle.getStreamHandle().getRightBmp().isGrayScale) &&
        (x1 <= x2) &&
        (y1 <= y2) &&
        (x1 > 0) &&
        (x2 < (this->getHeight() - 1)) &&
        (y1 > 0) &&
        (y2 < (this->getWidth() - 1)))
    {
        // 2) Allocate memory
        int **edgeData = new int *[(x2 - x1) + 1];
        for (int r = 0; r <= (x2 - x1); r++)
        {
            edgeData[r] = new int[(y2 - y1) + 1];
        }
        int Gx = 0;
        int Gy = 0;

        // 3) Define the vertical and horizontal filters
        const int sobelVerticalMask[3][3] = {
            {1, 0, -1},
            {2, 0, -2},
            {1, 0, -1}};
        const int sobelHorizontalMask[3][3] = {
            {1, 2, 1},
            {0, 0, 0},
            {-1, -2, -1}};

        // 4) Apply Sobel Operation on the patch : S = sqrt( Gx^2 + Gy^2 ) {Gx: Horizontal Sobel, Gy: Vertical Sobel}
        for (int row = x1; row < (x2 + 1); row++)
        {
            for (int col = y1; col < (y2 + 1); col++)
            {
                // Reset Gx and Gy
                Gx = 0;
                Gy = 0;

                // Apply horizontal sobel on 3x3 patch {Gx}
                if (horizontalFlag)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        for (int j = 0; j < 3; j++)
                        {
                            Gx += (this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[(row - 1) + i][(col - 1) + j] * sobelHorizontalMask[i][j]);
                        }
                    }
                }

                // Apply vertical sobel on 3x3 patch {Gy}
                if (verticalFlag)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        for (int j = 0; j < 3; j++)
                        {
                            Gy += (this->bmpHandle.getStreamHandle().getRightBmp().grayscale_img[(row - 1) + i][(col - 1) + j] * sobelVerticalMask[i][j]);
                        }
                    }
                }
                edgeData[row - x1][col - y1] = sqrt(this->bmpHandle.pow(Gx) + this->bmpHandle.pow(Gy));
            }
        }

        // 5) Clean the Edges
        if (cleanUp)
        {
            // a) First make the the edge into salt & pepper
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

        // 8) Deallocate the edgeData from Heap
        for (int r = 0; r <= (x2 - x1); r++)
        {
            delete[] edgeData[r];
        }
        delete[] edgeData;
        edgeData = nullptr;
    }
    else
    {
        cerr << "\n Something went wrong in {applySobelEdgeDetectionOnPatch}...";
    }
    return hasEdge;
}

void BitMap::displayImages(bool choice)
{
    this->bmpHandle.displayImages(choice);
}

int BitMap::getHeight()
{
    return this->bmpHandle.getStreamHandle().getLeftBmp().getImgHeight();
}

int BitMap::getWidth()
{
    return this->bmpHandle.getStreamHandle().getLeftBmp().getImgWidth();
}

void BitMap::executeTemplateMatching(int patchSize, int edgeLines)
{
    int height = this->bmpHandle.getStreamHandle().getLeftBmp().getImgHeight() - (patchSize * 2); // We will subtract to avoid the top and bot borders
    int interval = (height / edgeLines);
    int curr_interval = patchSize;
    for (int i = 0; i < edgeLines; i++, curr_interval += interval) // edges are detected in 10 rows
    {
        this->sobelTemplateMatch(curr_interval, patchSize, 0, i);
    }
    this->bmpHandle.rewriteLeftBMPEdges(patchSize, this->leftEdgePoints);
}

void BitMap::findEdgesInLeftImage(int patchSize, int edgeLines)
{
    int height = this->bmpHandle.getStreamHandle().getLeftBmp().getImgHeight() - (patchSize * 2); // We will subtract to avoid the top and bot borders
    int interval = (height / edgeLines);
    int curr_interval = patchSize;
    for (int i = 0; i < edgeLines; i++, curr_interval += interval) // edges are detected in 10 rows
    {
        this->applySobelEdgeDetection(curr_interval, patchSize, i, false);
    }
}

void BitMap::applySobelEdgeDetection(int rowNo, int patchSize, int edgeLines, bool horizontalFlag, bool verticalFlag, bool cleanFlag)
{

    if (!this->bmpHandle.getStreamHandle().getLeftBmp().isGrayScale)
    {
        throw runtime_error("The image needs to be in grayscale...");
    }

    if (((rowNo - patchSize) < 0) ||
        ((rowNo + patchSize) >= this->getHeight()))
    {
        throw runtime_error("The maskSize or rowNo is out of bounds");
    }

    // 1) Create a 1D array to hold the edgeData and the variable which will hold Gx and Gy
    int *edgeData = new int[this->getWidth() - (patchSize * 2)]; // Subtracted two to avoid the first and last column
    int Gx = 0;
    int Gy = 0;

    // 2) Define the vertical and horizontal filters
    const int sobelVerticalMask[3][3] = {
        {1, 0, -1},
        {2, 0, -2},
        {1, 0, -1}};
    const int sobelHorizontalMask[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}};

    // 3) Apply Sobel Operation on the row : S = sqrt( Gx^2 + Gy^2 ) {Gx: Horizontal Sobel, Gy: Vertical Sobel}
    for (int w = patchSize; w < (this->getWidth() - patchSize); w++)
    {
        // Reset Gx and Gy
        Gx = 0;
        Gy = 0;

        // Apply horizontal sobel on 3x3 patch {Gx}
        if (horizontalFlag)
        {
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    Gx += (this->bmpHandle.getStreamHandle().getLeftBmp().grayscale_img[((rowNo - 1) + i)][(w - 1) + j] * sobelHorizontalMask[i][j]);
                }
            }
        }

        // Apply vertical sobel on 3x3 patch {Gy}
        if (verticalFlag)
        {
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    Gy += (this->bmpHandle
                               .getStreamHandle()
                               .getLeftBmp()
                               .grayscale_img[this->bmpHandle.abs((rowNo - 1) + i)][(w - 1) + j] *
                           sobelVerticalMask[i][j]);
                }
            }
        }

        // S = sqrt(Gx^2 + Gy^2)
        edgeData[w - patchSize] = sqrt((Gx * Gx) + (Gy * Gy));
    }

    // 4) Clean the Edges
    if (cleanFlag)
    {
        int counter = 0;
        for (int w = 0; w < (this->getWidth() - patchSize * 2); w++)
        {
            if ((int)edgeData[w] >= 100)
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
    }

    // 5) Copy the edgeData into the Real Image
    for (int w = patchSize; w < (this->getWidth() - patchSize); w++)
    {
        this->bmpHandle.getStreamHandle().getLeftBmp().img[0][rowNo][w] =
            this->bmpHandle.getStreamHandle().getLeftBmp().img[1][rowNo][w] =
                this->bmpHandle.getStreamHandle().getLeftBmp().img[2][rowNo][w] = edgeData[w - patchSize];
    }

    // 6) Deallocate the edgeData from Heap
    delete[] edgeData;
    edgeData = nullptr;

    // 7) Store the edge points in {this->leftEdgePoints}
    pair<int, int> coord;
    vector<pair<int, int>> vec;
    for (int w = patchSize; w < (this->getWidth() - patchSize); w++)
    {
        if ((int)this->bmpHandle.getStreamHandle().getLeftBmp().img[0][rowNo][w] >= 100)
        {
            coord.first = rowNo;
            coord.second = w;
            vec.push_back(coord);
        }
    }
    for (int i = 0; i < vec.size(); i++)
    {
        this->bmpHandle.createBorder(this->bmpHandle.getStreamHandle().getLeftBmp(),
                                     vec[i].first - (patchSize) / 2, vec[i].second - (patchSize) / 2,
                                     vec[i].first + (patchSize) / 2, vec[i].second + (patchSize) / 2, 1);
    }
    this->leftEdgePoints.push_back(vec);
    this->hasEdges = true;
}