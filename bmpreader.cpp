#include "bmpreader.h"

BmpHandler::BmpHandler(string _path_) : path(_path_)
{
    this->img = nullptr;
    this->isGrayScale = false;
    this->original_img = this->img = nullptr;
    this->grayscale_img = nullptr;
    if (this->path != "")
    {
        this->readImage();
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

int BmpHandler::getImgHeight() const
{
    return this->img_dim.first;
}

int BmpHandler::getImgWidth() const
{
    return this->img_dim.second;
}

int BmpHandler::readImage()
{
    string name = this->path + ".bmp";
    FILE *fileptr = fopen(name.c_str(), "rb");
    if (!fileptr)
    {
        throw runtime_error("Could not open file");
    }
    int bytes = fread(this->header, 1, HEADER_LEN, fileptr);
    if (bytes == 0 || bytes < HEADER_LEN)
    {
        throw runtime_error("Could not read header");
    }
    this->img_dim.second = *((int *)&this->header[18]);
    this->img_dim.first = *((int *)&this->header[22]);
    int imgsize = this->img_dim.first * this->img_dim.second * 3;
    int move = fseek(fileptr, 54, SEEK_SET);
    if (move != 0)
    {
        throw runtime_error("Could Not Move Pointer");
    }
    uint8_t *buf = new uint8_t[imgsize];
    long pixbytes = fread(buf, 1, imgsize, fileptr);
    if (pixbytes != imgsize)
    {
        throw runtime_error("Could Not Read Pixels");
    }
    fflush(fileptr);
    fclose(fileptr);
    this->convertTo3d(buf);
    delete[] buf;
    buf = nullptr;
    return 1;
}

void BmpHandler::convertTo3d(uint8_t *&buf)
{
    int height = this->img_dim.first;
    int width = this->img_dim.second;
    allocate3DBuffer(this->img);
    allocate3DBuffer(this->original_img);
    allocate2DBuffer(this->grayscale_img);

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

void BmpHandler::allocate3DBuffer(unsigned char ***&buf)
{
    if (this->path.empty())
    {
        throw runtime_error("There was no path defined, so 3D allocation failed");
    }
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

void BmpHandler::allocate2DBuffer(unsigned char **&buf)
{
    if (this->path.empty())
    {
        throw runtime_error("There was no path defined, so 2D allocation failed");
    }
    buf = new unsigned char *[this->img_dim.first];
    for (unsigned int row = 0; row < this->img_dim.first; ++row)
    {
        buf[row] = new unsigned char[this->img_dim.second];
    }
}

void BmpHandler::applyGrayscale(bool modify)
{
    if (!this->original_img)
    {
        throw runtime_error("There is no data in buffer {this->original_img}, Grayscale could not be applied");
    }
    if (!this->grayscale_img)
    {
        throw runtime_error("The buffer {this->grayscale_img} is not allocated, so Grayscale could not be applied");
    }

    for (unsigned int row = 0; row < this->img_dim.first; ++row)
    {
        for (unsigned int col = 0; col < this->img_dim.second; ++col)
        {
            if (modify)
            {
                this->img[0][row][col] =
                    this->img[1][row][col] =
                        this->img[2][row][col] =
                            (this->img[0][row][col] + this->img[1][row][col] + this->img[2][row][col]) / 3;
            }
            this->grayscale_img[row][col] = (this->img[0][row][col] + this->img[1][row][col] + this->img[2][row][col]) / 3;
        }
    }
    this->isGrayScale = true;
}

vector<vector<pair<int, int>>> BmpHandler::getRightEdgePoint() const
{
    return this->rightEdgePoints;
}

vector<vector<pair<int, int>>> BmpHandler::getLeftEdgePoint() const
{
    return this->leftEdgePoints;
}

void BmpHandler::applySobelEdgeDetection(int rowNo, int patchSize, int edgeLineNo, bool horizontalFlag, bool verticalFlag, bool cleanFlag)
{
    if (!this->isGrayScale)
    {
        throw runtime_error("The image needs to be in grayscale...");
    }

    if (((rowNo - patchSize) < 0) ||
        ((rowNo + patchSize) >= this->img_dim.first))
    {
        throw runtime_error("The maskSize or rowNo is out of bounds");
    }

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
                    Gx += (this->grayscale_img[this->abs((rowNo - 1) + i)][(w - 1) + j] * sobelHorizontalMask[i][j]);
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
                    Gy += (this->grayscale_img[this->abs((rowNo - 1) + i)][(w - 1) + j] * sobelVerticalMask[i][j]);
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
    vector<pair<int, int>> vec;
    for (int w = patchSize; w < (this->img_dim.second - patchSize); w++)
    {
        if ((int)this->img[0][rowNo][w] >= 100)
        {
            coord.first = rowNo;
            coord.second = w;
            vec.push_back(coord);
        }
    }
    for (int i = 0; i < vec.size(); i++)
    {
        this->createBorder(vec[i].first - (patchSize) / 2, vec[i].second - (patchSize) / 2,
                           vec[i].first + (patchSize) / 2, vec[i].second + (patchSize) / 2, 1);
    }
    this->leftEdgePoints.push_back(vec);
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

int BmpHandler::abs(int val)
{
    return (val >= 0) ? val : (-1 * val);
}

void BmpHandler::writeBMPImage(bool imgType)
{
    if (!this->original_img && !this->img)
    {
        throw runtime_error("There is nothing allocated in {this->original_img ^ this->img}");
    }

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
        for (int i = 0; i < this->img_dim.first; ++i)
        {
            delete[] this->grayscale_img[i];
        }
        delete[] this->grayscale_img;
        this->grayscale_img = nullptr;
    }

    leftEdgePoints.clear();
    rightEdgePoints.clear();
}

BmpHandler::~BmpHandler()
{
    cleanUp();
}

void BmpHandler::sobelTemplateMatch(int rowNo, int patchSize, int rowOffset, int edgeLineNo, BmpHandler &bmpB)
{
    vector<pair<int, int>> vec; // KEEP IT HERE, DON'T MOVE THIS VARIABLE BELOW

    // CHECK EDGE POINTS
    if (this->leftEdgePoints[edgeLineNo].size() <= 0)
    {
        // cerr << "\nNo Edges to match in row#" << edgeLineNo;
        this->rightEdgePoints.push_back(vec);
        return;
    }

    // CHECK PATCH SIZE
    if ((patchSize % 2 == 0) || patchSize < 3)
    {
        throw runtime_error("Invalid Patch Size. Please make patch > 3 and odd");
    }

    // CHECK IF IMAGES ARE GRAYSCALE
    if ((!this->isGrayScale) ||
        (!bmpB.isGrayScale))
    {
        throw runtime_error("\nOne of the images don't seem to be in grayscale...");
    }

    if (((rowNo - patchSize) < 0) &&
        ((rowNo + patchSize) >= this->img_dim.first))
    {
        throw runtime_error("\n Row index is out of range...");
    }

    int curr_ssd = 0;
    int best_ssd = INT32_MAX;
    pair<int, int> best_coord;
    for (int edgeNo = 0; edgeNo < this->leftEdgePoints[edgeLineNo].size(); edgeNo++)
    {
        best_ssd = INT32_MAX;
        for (int col = (patchSize - 1); col < (bmpB.getImgWidth() - (patchSize - 1)); col++)
        {
            curr_ssd = 0;
            for (int r = 0; r < patchSize; r++)
            {
                for (int w = 0; w < patchSize; w++)
                {
                    curr_ssd += this->pow(this->abs(this->grayscale_img[(this->leftEdgePoints[edgeLineNo][edgeNo].first - (patchSize - 1) / 2) + r]
                                                                       [(this->leftEdgePoints[edgeLineNo][edgeNo].second - (patchSize - 1) / 2) + w] -
                                                    bmpB.grayscale_img[(this->leftEdgePoints[edgeLineNo][edgeNo].first + rowOffset - (patchSize - 1) / 2) + r]
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
        if ((this->abs(best_coord.second - leftEdgePoints[edgeLineNo][edgeNo].second) < this->getImgWidth() / 2) && // Check Distance between points
            ((!this->rightEdgePoints[edgeLineNo].size()) || (this->isInRange(best_coord.second, this->rightEdgePoints[edgeLineNo], patchSize))) &&
            (bmpB.applySobelEdgeDetectionOnPatch(patchSize, best_coord.first - (patchSize) / 2, // Check Edges
                                                 best_coord.second - (patchSize) / 2, best_coord.first + (patchSize) / 2, best_coord.second + (patchSize) / 2, false)))
        {
            bmpB.createBorder(best_coord.first - (patchSize) / 2, best_coord.second - (patchSize) / 2,
                              best_coord.first + (patchSize) / 2, best_coord.second + (patchSize) / 2, 1);
            vec.push_back(best_coord);
        }
        else
        {
            this->leftEdgePoints[edgeLineNo].erase(this->leftEdgePoints[edgeLineNo].begin() + edgeNo);
            edgeNo--;
        }
    }
    this->rightEdgePoints.push_back(vec);
}

bool BmpHandler::isInRange(int val, vector<pair<int, int>> &vec, int range)
{
    // for (auto v : vec)
    // {
    //     cout << v.first << endl;
    //     if ((val > (v.second - range)) &&
    //         (val < (v.second + range)))
    //     {
    //         return false;
    //     }
    // }
    return true;
}

bool BmpHandler::applySobelEdgeDetectionOnPatch(int patchSize, int x1, int y1, int x2, int y2, bool horizontalFlag, bool verticalFlag, bool cleanUp)
{
    // 0) Apply a row check
    if (((x1 - patchSize) < 0) &&
        ((x1 + patchSize) >= this->img_dim.first) &&
        ((x2 - patchSize) < 0) &&
        ((x2 + patchSize) >= this->img_dim.first) &&
        ((y1 - patchSize) < 0) &&
        ((y1 + patchSize) >= this->img_dim.second) &&
        ((y2 - patchSize) < 0) &&
        ((y2 + patchSize) >= this->img_dim.second))
    {
        cerr << "\nThe patch is out of range...";
        return false;
    }

    bool hasEdge = false;
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
                            Gx += (this->grayscale_img[(row - 1) + i][(col - 1) + j] * sobelHorizontalMask[i][j]);
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
                            Gy += (this->grayscale_img[(row - 1) + i][(col - 1) + j] * sobelVerticalMask[i][j]);
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
            for (int r = 0; r <= (x2 - x1); r++)
            {
                for (int c = 0; c <= (y2 - y1); c++)
                {

                    this->img[0][x1 + r][y1 + c] =
                        this->img[1][x1 + r][y1 + c] =
                            this->img[2][x1 + r][y1 + c] = edgeData[r][c];
                }
            }

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

void BmpHandler::eraseRightEdgePoints()
{
    this->rightEdgePoints.clear();
}
