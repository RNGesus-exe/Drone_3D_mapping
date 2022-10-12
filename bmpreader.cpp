#include "bmpreader.h"

BmpHandler::BmpHandler(string _path_) : path(_path_)
{
    this->img = nullptr;
    this->isGrayScale = false;
    if (this->path != "")
    {
        this->readImage();
    }
}

int BmpHandler::getImgHeight() const
{
    return this->img_dim.first;
}

int BmpHandler::getImgWidth() const
{
    return this->img_dim.second;
}

void BmpHandler::applyAutoContrast()
{
    if (this->img)
    {
        for (unsigned int row = 0; row < this->img_dim.first; row++)
        {
            for (unsigned int col = 0; col < this->img_dim.second; ++col)
            {
                this->img[0][row][col] = (this->img[0][row][col] - this->contrast[0][1]) * (255 / this->abs(this->contrast[0][0] - this->contrast[0][1]));
                this->img[1][row][col] = (this->img[1][row][col] - this->contrast[1][1]) * (255 / this->abs(this->contrast[1][0] - this->contrast[1][1]));
                this->img[2][row][col] = (this->img[2][row][col] - this->contrast[2][1]) * (255 / this->abs(this->contrast[2][0] - this->contrast[2][1]));
            }
        }
    }
    else
    {
        cerr << "\nThere is no data in buffer {this->img}, AutoContrast cannot be applied...";
    }
}

void BmpHandler::applyGrayscale()
{
    if (this->img)
    {
        for (unsigned int row = 0; row < this->img_dim.first; ++row)
        {
            for (unsigned int col = 0; col < this->img_dim.second; ++col)
            {
                // 0.299 ∙ Red + 0.587 ∙ Green + 0.114 ∙ Blue (Another way to grayscale)
                // go_board.image[0][row][width] = go_board.image[1][row][width] =  go_board.image[2][row][width] = 0.299 * go_board.tmpbuf[2] + 0.587 * go_board.tmpbuf[1] + 0.114 * go_board.tmpbuf[0]
                this->img[0][row][col] = this->img[1][row][col] = this->img[2][row][col] = (this->img[0][row][col] + this->img[1][row][col] + this->img[2][row][col]) / 3;
                this->grayscale_img[0][row][col] = this->grayscale_img[1][row][col] = this->grayscale_img[2][row][col] = (this->img[0][row][col] + this->img[1][row][col] + this->img[2][row][col]) / 3;
            }
        }
        isGrayScale = true;
    }
    else
    {
        cerr << "\nThere is no data in buffer {this->img}, Grayscale cannot be applied...";
    }
}

int BmpHandler::abs(int val)
{
    return (val >= 0) ? val : (-1 * val);
}

vector<pair<int, int>> BmpHandler::getLeftEdgePoint() const
{
    return this->leftEdgePoints;
}

void BmpHandler::singleRowEdgeDetection(int row, bool displayEdges, int padding, int skip_amount)
{
    if (this->img)
    {
        if (this->isGrayScale)
        {
            if ((row >= 0) &&
                (row < this->img_dim.first))
            {
                pair<int, int> coord;
                for (unsigned int col = padding; col < (this->img_dim.second - padding); ++col)
                {
                    if (this->abs((int)this->img[0][row][col] - (int)this->img[0][row][col + 1]) >= 30)
                    {
                        // Store the coords in a vector
                        coord.first = row;
                        coord.second = col;
                        leftEdgePoints.push_back(coord);

                        // Make the edges detected visible (white)
                        if (displayEdges)
                        {
                            this->createBorder(row - 1, col - 1, row + 1, col + 1, 1);
                        }

                        // To make sure pixels aren't clumped together, we will skip an extra step
                        col += skip_amount;
                    }
                }
            }
            else
            {
                cerr << "\n The entered row in {this->singleRowEdgeDetection} is out of bounds...";
            }
        }
        else
        {
            cerr << "\n The image needs to be in grayscale before you can apply singleRowEdgeDetection on it";
        }
    }
    else
    {
        cerr << "\n There was no data in buffer {this->img}, singleRowEdgeDetection cannot be applied...";
    }
}

void BmpHandler::setPath(string _path_)
{
    if (_path_ == "")
    {
        cerr << "\nYou can't pass an empty char array as a path...";
    }
    else
    {
        this->path = _path_;
        this->readImage();
    }
}

void BmpHandler::cleanUp()
{
    // Deallocation of original image
    if (this->img)
    {
        for (int i = 0; i < COLOR_CHANNELS; ++i)
        {
            for (int j = 0; j < this->img_dim.first; ++j)
            {
                delete[] this->img[i][j];
            }
            delete[] this->img[i];
        }
        delete[] this->img;
        this->img = nullptr;
    }

    if (this->original_img)
    {
        for (int i = 0; i < COLOR_CHANNELS; ++i)
        {
            for (int j = 0; j < this->img_dim.first; ++j)
            {
                delete[] this->original_img[i][j];
            }
            delete[] this->original_img[i];
        }
        delete[] this->original_img;
        this->original_img = nullptr;
    }

    if (this->grayscale_img)
    {
        for (int i = 0; i < COLOR_CHANNELS; ++i)
        {
            for (int j = 0; j < this->img_dim.first; ++j)
            {
                delete[] this->grayscale_img[i][j];
            }
            delete[] this->grayscale_img[i];
        }
        delete[] this->grayscale_img;
        this->grayscale_img = nullptr;
    }

    leftEdgePoints.clear();
    rightEdgePoints.clear();
}

void BmpHandler::eraseRightEdgePoints()
{
    this->rightEdgePoints.clear();
}

BmpHandler::~BmpHandler()
{
    // Deallocation of original image
    if (this->img)
    {
        for (int i = 0; i < COLOR_CHANNELS; ++i)
        {
            for (int j = 0; j < this->img_dim.first; ++j)
            {
                delete[] this->img[i][j];
            }
            delete[] this->img[i];
        }
        delete[] this->img;
        this->img = nullptr;
    }

    if (this->original_img)
    {
        for (int i = 0; i < COLOR_CHANNELS; ++i)
        {
            for (int j = 0; j < this->img_dim.first; ++j)
            {
                delete[] this->original_img[i][j];
            }
            delete[] this->original_img[i];
        }
        delete[] this->original_img;
        this->original_img = nullptr;
    }

    leftEdgePoints.clear();
    rightEdgePoints.clear();
}

void BmpHandler::createBorder(int x1, int y1, int x2, int y2, int color)
{
    if ((x1 <= x2) &&
        (y1 <= y2))
    {
        if ((x1 >= 0) &&
            (x2 < this->img_dim.first) &&
            (y1 >= 0) &&
            (y2 < this->img_dim.second))
        {
            switch (color)
            {
            case 1:
                // (x1,y1) to (x1,y2) Top Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    this->img[0][x1][col] = 255;
                    this->img[1][x1][col] = 0;
                    this->img[2][x1][col] = 0;
                }

                // (x2,y1) to (x2,y2) Bottom Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    this->img[0][x2][col] = 255;
                    this->img[1][x2][col] = 0;
                    this->img[2][x2][col] = 0;
                }

                // (x1,y1) to (x2,y1) Left Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    this->img[0][row][y1] = 255;
                    this->img[1][row][y1] = 0;
                    this->img[2][row][y1] = 0;
                }

                // (x1,y2) to (x2,y2) Right Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    this->img[0][row][y2] = 255;
                    this->img[1][row][y2] = 0;
                    this->img[2][row][y2] = 0;
                }
                break;
            case 2:
                // (x1,y1) to (x1,y2) Top Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    this->img[0][x1][col] = 0;
                    this->img[1][x1][col] = 255;
                    this->img[2][x1][col] = 0;
                }

                // (x2,y1) to (x2,y2) Bottom Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    this->img[0][x2][col] = 0;
                    this->img[1][x2][col] = 255;
                    this->img[2][x2][col] = 0;
                }

                // (x1,y1) to (x2,y1) Left Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    this->img[0][row][y1] = 0;
                    this->img[1][row][y1] = 255;
                    this->img[2][row][y1] = 0;
                }

                // (x1,y2) to (x2,y2) Right Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    this->img[0][row][y2] = 0;
                    this->img[1][row][y2] = 255;
                    this->img[2][row][y2] = 0;
                }
                break;
            case 3:
                // (x1,y1) to (x1,y2) Top Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    this->img[0][x1][col] = 0;
                    this->img[1][x1][col] = 0;
                    this->img[2][x1][col] = 255;
                }

                // (x2,y1) to (x2,y2) Bottom Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    this->img[0][x2][col] = 0;
                    this->img[1][x2][col] = 0;
                    this->img[2][x2][col] = 255;
                }

                // (x1,y1) to (x2,y1) Left Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    this->img[0][row][y1] = 0;
                    this->img[1][row][y1] = 0;
                    this->img[2][row][y1] = 255;
                }

                // (x1,y2) to (x2,y2) Right Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    this->img[0][row][y2] = 0;
                    this->img[1][row][y2] = 0;
                    this->img[2][row][y2] = 255;
                }
                break;
            }
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
}

int BmpHandler::pow(int val)
{
    return val * val;
}

void BmpHandler::applySobelEdgeDetection(int rowNo, bool horizontalFlag, bool verticalFlag, bool cleanFlag, int patchSize)
{
    if (this->isGrayScale)
    {
        if (((rowNo - patchSize) >= 0) &&
            ((rowNo + patchSize) < this->img_dim.first))
        {
            // 1) Create a 1D array to hold the edgeData and the variable which will hold Gx and Gy
            int *edgeData = new int[this->img_dim.second - (patchSize * 2)]; // Subtracted two to avoid the first and last column
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
            for (int w = patchSize; w < (this->img_dim.second - patchSize); w++)
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
                            Gx += (this->img[0][this->abs((rowNo - 1) + i)][(w - 1) + j] * sobelHorizontalMask[i][j]);
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
                            Gy += (this->img[0][this->abs((rowNo - 1) + i)][(w - 1) + j] * sobelVerticalMask[i][j]);
                        }
                    }
                }

                // S = sqrt(Gx^2 + Gy^2)
                edgeData[w - patchSize] = sqrt(this->pow(Gx) + this->pow(Gy));
            }

            // 4) Clean the Edges
            if (cleanFlag)
            {
                int counter = 0;
                for (int w = 0; w < (this->img_dim.second - patchSize * 2); w++)
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
            for (int w = patchSize; w < (this->img_dim.second - patchSize); w++)
            {
                this->img[0][rowNo][w] =
                    this->img[1][rowNo][w] =
                        this->img[2][rowNo][w] = edgeData[w - patchSize];
            }

            // 6) Deallocate the edgeData from Heap
            delete[] edgeData;
            edgeData = nullptr;

            // 7) Store the edge points in {this->leftEdgePoints}
            pair<int, int> coord;
            for (int w = patchSize; w < (this->img_dim.second - patchSize); w++)
            {
                if ((int)this->img[0][rowNo][w] >= 100)
                {
                    coord.first = rowNo;
                    coord.second = w;
                    this->leftEdgePoints.push_back(coord);
                    this->createBorder(coord.first - (patchSize) / 2, coord.second - (patchSize) / 2,
                                       coord.first + (patchSize) / 2, coord.second + (patchSize) / 2, 2);
                }
            }
        }
        else
        {
            cerr << "\nThe maskSize or rowNo is out of bounds...";
        }
    }
    else
    {
        cerr << "\n The image needs to be in grayscale...";
    }
}

bool BmpHandler::applySobelEdgeDetectionOnPatch(int x1, int y1, int x2, int y2, bool horizontalFlag, bool verticalFlag, bool cleanUp)
{
    // 1) Apply constraint checks
    if ((this->isGrayScale) &&
        (x1 <= x2) &&
        (y1 <= y2) &&
        (x1 > 0) &&
        (x2 < (this->img_dim.first - 1)) &&
        (y1 > 0) &&
        (y2 < (this->img_dim.second - 1)))
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
                            Gx += (this->img[0][(row - 1) + i][(col - 1) + j] * sobelHorizontalMask[i][j]);
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
                            Gy += (this->img[0][(row - 1) + i][(col - 1) + j] * sobelVerticalMask[i][j]);
                        }
                    }
                }
                edgeData[row - x1][col - y1] = sqrt(this->pow(Gx) + this->pow(Gy));
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

        // 6) Check if the patch has any edge in it
        for (int r = 0; r <= (x2 - x1); r++)
        {
            for (int w = 0; w <= (y2 - y1); w++)
            {
                if ((int)edgeData[r][w] >= 100)
                {
                    // 6.1) Copy the edgeData into the Real Image
                    for (int r = 0; r <= (x2 - x1); r++)
                    {
                        for (int c = 0; c <= (y2 - y1); c++)
                        {

                            this->img[0][x1 + r][y1 + c] =
                                this->img[1][x1 + r][y1 + c] =
                                    this->img[2][x1 + r][y1 + c] = edgeData[r][c];
                        }
                    }

                    // 6.2) Deallocate the edgeData from Heap
                    for (int r = 0; r <= (x2 - x1); r++)
                    {
                        delete[] edgeData[r];
                    }
                    delete[] edgeData;
                    edgeData = nullptr;

                    // 6.3) As we have found an edge and copied the data we can leave
                    return true;
                }
            }
        }

        // 7) Deallocate the edgeData from Heap
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
    return false;
}

void BmpHandler::sobelTemplateMatch(int rowNo, int patchSize, int rowOffset, BmpHandler &bmpB)
{
    // CHECK EDGE POINTS
    if (this->leftEdgePoints.size() <= 0)
    {
        cerr << "\nThere were no edge points in {this->leftEdgePoints}...";
        return;
    }

    // CHECK PATCH SIZE
    if ((patchSize % 2 == 0) || patchSize < 3)
    {
        cerr << "Invalid Patch Size. Please make patch > 3 and odd";
        return;
    }

    // CHECK IF IMAGES ARE GRAYSCALE
    if ((!this->isGrayScale) ||
        (!bmpB.isGrayScale))
    {
        cerr << "\nOne of the images don't seem to be in grayscale...";
        return;
    }

    int curr_ssd = 0;
    int best_ssd = INT32_MAX;
    pair<int, int> best_coord;
    for (int edgeNo = 0; edgeNo < this->leftEdgePoints.size(); edgeNo++)
    {
        best_ssd = INT32_MAX;
        for (int col = (patchSize - 1); col < (bmpB.getImgWidth() - (patchSize - 1)); col++)
        {
            curr_ssd = 0;
            for (int r = 0; r < patchSize; r++)
            {
                for (int w = 0; w < patchSize; w++)
                {
                    curr_ssd += this->pow(this->abs(this->grayscale_img[0]
                                                                       [(this->leftEdgePoints[edgeNo].first - (patchSize - 1) / 2) + r]
                                                                       [(this->leftEdgePoints[edgeNo].second - (patchSize - 1) / 2) + w] -
                                                    bmpB.grayscale_img[0]
                                                                      [(this->leftEdgePoints[edgeNo].first + rowOffset - (patchSize - 1) / 2) + r]
                                                                      [(col - (patchSize - 1) / 2) + w]));
                }
            }
            // Compare curr_ssd with best_ssd
            if (curr_ssd < best_ssd)
            {
                best_ssd = curr_ssd;
                best_coord.first = this->leftEdgePoints[edgeNo].first + rowOffset;
                best_coord.second = col;
            }
        }

        // Apply Edge Detection on the Patch
        if (bmpB.applySobelEdgeDetectionOnPatch(best_coord.first - (patchSize) / 2, best_coord.second - (patchSize) / 2,
                                                best_coord.first + (patchSize) / 2, best_coord.second + (patchSize) / 2, false))
        {
            // We will push the edges in this container
            bmpB.createBorder(best_coord.first - (patchSize) / 2, best_coord.second - (patchSize) / 2,
                              best_coord.first + (patchSize) / 2, best_coord.second + (patchSize) / 2, 1);
        }

        // DROP THE USELESS EDGE POINTS
        // if ((this->abs(best_coord.second - leftEdgePoints[edgeNo].second) < this->getImgWidth() / 2) &&
        //     (best_coord.second > leftEdgePoints[edgeNo].second))
        // {
        //     this->rightEdgePoints.push_back(best_coord);
        // }
        // else
        // {
        //     this->leftEdgePoints.erase(this->leftEdgePoints.begin() + edgeNo);
        //     edgeNo--;
        // }
    }

    // Now we check for vertical edges in the rightEdges vector'
}

void BmpHandler::grayScaleTemplateMatch(int rowNo, int patchSize, int rowOffset, BmpHandler &bmpB)
{
    // CHECK EDGE POINTS
    if (this->leftEdgePoints.size() <= 0)
    {
        cerr << "\nThere were no edge points in {this->leftEdgePoints}...";
        return;
    }

    // CHECK PATCH SIZE
    if ((patchSize % 2 == 0) || patchSize < 3)
    {
        cerr << "Invalid Patch Size. Please make patch > 3 and odd";
        return;
    }

    // CHECK IF IMAGES ARE GRAYSCALE
    if ((!this->isGrayScale) ||
        (!bmpB.isGrayScale))
    {
        cerr << "\nOne of the images don't seem to be in grayscale...";
        return;
    }

    // cout << this->leftEdgePoints[0].first << " " << this->leftEdgePoints[0].second << endl;

    int curr_ssd = 0;
    int best_ssd = INT32_MAX;
    pair<int, int> best_coord;
    for (int edgeNo = 0; edgeNo < this->leftEdgePoints.size(); edgeNo++)
    {
        best_ssd = INT32_MAX;
        for (int col = (patchSize - 1); col < (bmpB.getImgWidth() - (patchSize - 1)); col++)
        {
            curr_ssd = 0;
            for (int r = 0; r < patchSize; r++)
            {
                for (int w = 0; w < patchSize; w++)
                {
                    curr_ssd += this->pow(this->abs(this->img[0]
                                                             [(this->leftEdgePoints[edgeNo].first - (patchSize - 1) / 2) + r]
                                                             [(this->leftEdgePoints[edgeNo].second - (patchSize - 1) / 2) + w] -
                                                    bmpB.img[0]
                                                            [(this->leftEdgePoints[edgeNo].first + rowOffset - (patchSize - 1) / 2) + r]
                                                            [(col - (patchSize - 1) / 2) + w]));
                }
            }
            // Compare curr_ssd with best_ssd
            if (curr_ssd < best_ssd)
            {
                best_ssd = curr_ssd;
                best_coord.first = this->leftEdgePoints[edgeNo].first + rowOffset;
                best_coord.second = col;
            }
        }

        // DROP THE USELESS EDGE POINTS
        if ((this->abs(best_coord.second - leftEdgePoints[edgeNo].second) < this->getImgWidth() / 2) &&
            (best_coord.second > leftEdgePoints[edgeNo].second))
        {
            this->rightEdgePoints.push_back(best_coord);
        }
        else
        {
            this->leftEdgePoints.erase(this->leftEdgePoints.begin() + edgeNo);
            edgeNo--;
        }
    }
    cout << leftEdgePoints.size() << " " << rightEdgePoints.size() << endl;
    for (int i = 1; i < leftEdgePoints.size(); i++)
    {
        if (this->abs(this->abs(leftEdgePoints[i].second - leftEdgePoints[i - 1].second) -
                      this->abs(rightEdgePoints[i].second - rightEdgePoints[i - 1].second)) > 100)
        {
            rightEdgePoints.erase(rightEdgePoints.begin() + i);
            leftEdgePoints.erase(leftEdgePoints.begin() + i);
            i--;
        }
    }
    // cout << '\n';

    if (leftEdgePoints.size() != rightEdgePoints.size())
    {
        cerr << "\n The amount of left edges are not same as right edges...";
        return;
    }

    for (int edgeNo = 0; edgeNo < leftEdgePoints.size(); edgeNo++)
    {
        cout << "\n Left = ("
             << leftEdgePoints[edgeNo].first << "," << leftEdgePoints[edgeNo].second
             << "), Right = ("
             << rightEdgePoints[edgeNo].first << "," << rightEdgePoints[edgeNo].second
             << "), Border = ";
        bmpB.createBorder(rightEdgePoints[edgeNo].first - (patchSize - 1) / 2, rightEdgePoints[edgeNo].second - (patchSize - 1) / 2,
                          rightEdgePoints[edgeNo].first + (patchSize - 1) / 2, rightEdgePoints[edgeNo].second + (patchSize - 1) / 2, 1);
        this->createBorder(leftEdgePoints[edgeNo].first - (patchSize - 1) / 2, leftEdgePoints[edgeNo].second - (patchSize - 1) / 2,
                           leftEdgePoints[edgeNo].first + (patchSize - 1) / 2, leftEdgePoints[edgeNo].second + (patchSize - 1) / 2, 1);
    }
}

void BmpHandler::findMinMaxContrast(int blue, int red, int green)
{
    // Channel BLUE
    if (blue < this->contrast[0][1])
    {
        this->contrast[0][1] = blue;
    }
    if (blue > this->contrast[0][0])
    {
        this->contrast[0][0] = blue;
    }
    // Channel Green
    if (green < this->contrast[1][1])
    {
        this->contrast[1][1] = green;
    }
    if (green > this->contrast[1][0])
    {
        this->contrast[1][0] = green;
    }
    // Channel Red
    if (red < this->contrast[2][1])
    {
        this->contrast[2][1] = red;
    }
    if (red > this->contrast[2][0])
    {
        this->contrast[2][0] = red;
    }
}

int BmpHandler::readImage()
{
    string name = this->path + ".bmp";
    FILE *fileptr = fopen(name.c_str(), "rb");
    if (!fileptr)
    {
        printf("Could Not Open File.\n");
        return 0;
    }
    int bytes = fread(this->header, 1, HEADER_LEN, fileptr);
    if (bytes == 0 || bytes < HEADER_LEN)
    {
        printf("Could not read header \n");
        return 0;
    }
    this->img_dim.second = *((int *)&this->header[18]);
    this->img_dim.first = *((int *)&this->header[22]);
    int imgsize = this->img_dim.first * this->img_dim.second * 3;

    uint8_t *buf = new uint8_t[imgsize];

    int move = fseek(fileptr, 54, SEEK_SET);
    if (move != 0)
    {
        printf("Could Not Move Pointer\n");
        return 0;
    }
    long pixbytes = fread(buf, 1, imgsize, fileptr);
    if (pixbytes != imgsize)
    {
        printf("Could Not Read Pixels\n");
        return 0;
    }
    fflush(fileptr);
    fclose(fileptr);
    this->convertTo3d(buf);
    delete[] buf;
    buf = nullptr;
    return 1;
}

void BmpHandler::convertTo3d(uint8_t *buf)
{
    int height = this->img_dim.first;
    int width = this->img_dim.second;
    allocateBuffer(this->img);
    allocateBuffer(this->original_img);
    allocateBuffer(this->grayscale_img);

    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            for (int k = 0; k < 3; k++)
            {
                this->img[k][(height - 1) - r][c] = (unsigned char)buf[(r * width * 3) + (c * 3) + (k)];
                this->original_img[k][(height - 1) - r][c] = (unsigned char)buf[(r * width * 3) + (c * 3) + (k)];
            }
        }
    }
}

void BmpHandler::writeBMPImage(bool imgType)
{
    int height = this->img_dim.first;
    int width = this->img_dim.second;
    uint8_t *data = new uint8_t[(height * width * 3)];

    unsigned char ***buf = nullptr;

    if (imgType)
    {
        buf = this->original_img;
    }
    else
    {
        buf = this->img;
    }

    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            for (int k = 0; k < 3; k++)
            {
                data[(((height - 1) - r) * width * 3) + (c * 3) + (k)] = (uint8_t)buf[k][r][c];
            }
        }
    }
    FILE *file = fopen((this->path + "_w.bmp").c_str(), "wb");
    if (file)
    {
        fwrite(this->header, 1, HEADER_LEN, file);
        fseek(file, HEADER_LEN, SEEK_SET);
        fwrite((char *)data, 1, (height * width * 3), file);
        fflush(file);
        fclose(file);
    }
    else
    {
        printf("Could not open outfile\n");
    }
    delete[] data;
}

void BmpHandler::allocateBuffer(unsigned char ***&buf)
{
    buf = new unsigned char **[COLOR_CHANNELS];
    for (unsigned int channel = 0; channel < COLOR_CHANNELS; ++channel)
    {
        buf[channel] = new unsigned char *[this->img_dim.first];
        for (unsigned int row = 0; row < this->img_dim.first; ++row)
        {
            buf[channel][row] = new unsigned char[this->img_dim.second];
        }
    }
}

void BmpHandler::allocatePatch(unsigned char ***&patch, int patchSize)
{
    patch = new unsigned char **[COLOR_CHANNELS];
    for (int color = 0; color < COLOR_CHANNELS; ++color)
    {
        patch[color] = new unsigned char *[patchSize];
        for (int c = 0; c < patchSize; c++)
        {
            patch[color][c] = new unsigned char[patchSize];
        }
    }
}

void BmpHandler::deAllocatePatch(unsigned char ***&patch, int patchSize)
{
    if (patch)
    {
        for (int i = 0; i < COLOR_CHANNELS; ++i)
        {
            for (int j = 0; j < patchSize; ++j)
            {
                delete[] patch[i][j];
            }
            delete[] patch[i];
        }
        delete[] patch;
        patch = nullptr;
    }
}