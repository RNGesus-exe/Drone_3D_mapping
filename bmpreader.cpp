#include "bmpreader.h"

#define println(x) std::cout << x << std::endl

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

vector<pair<int, int>> BmpHandler::getEdgePoint() const
{
    return this->edgePoints;
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
                        edgePoints.push_back(coord);

                        // Make the edges detected visible (white)
                        if (displayEdges)
                        {
                            this->createBorder(row - 1, col - 1, row + 1, col + 1);
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
    }
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
}

void BmpHandler::createBorder(int x1, int y1, int x2, int y2)
{
    if ((x1 <= x2) &&
        (y1 <= y2))
    {
        if ((x1 >= 0) &&
            (x2 < this->img_dim.first) &&
            (y1 >= 0) &&
            (y2 < this->img_dim.second))
        {
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

void BmpHandler::applySobelEdgeDetection(int rowNo, bool horizontalFlag, bool verticalFlag, bool cleanFlag)
{
    if (this->isGrayScale)
    {
        if (((rowNo - 1) >= 0) &&
            ((rowNo + 1) < this->img_dim.first))
        {
            // 1) Create a 1D array to hold the edgeData and the variable which will hold Gx and Gy
            int *edgeData = new int[this->img_dim.second - 2]; // Subtracted two to avoid the first and last column
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

            for (int w = 1; w < (this->img_dim.second - 1); w++)
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
                edgeData[w - 1] = sqrt(this->pow(Gx) + this->pow(Gy));
            }

            // 4) Copy the edgeData into the Real Image
            for (int w = 1; w < (this->img_dim.second - 1); w++)
            {
                this->img[0][rowNo][w] = edgeData[w - 1];
                this->img[1][rowNo][w] = edgeData[w - 1];
                this->img[2][rowNo][w] = edgeData[w - 1];
            }

            // 5) Deallocate the edgeData from Heap
            delete[] edgeData;

            // 6) Clean the Edges
            if (cleanFlag)
            {
                // a) First make the the edge into salt & pepper
                for (int w = 1; w < (this->img_dim.second - 1); w++)
                {
                    for (int c = 0; c < COLOR_CHANNELS; c++)
                    {
                        if ((int)this->img[c][rowNo][w] >= 100)
                        {
                            this->img[c][rowNo][w] = 255;
                        }
                        else
                        {
                            this->img[c][rowNo][w] = 0;
                        }
                    }
                }

                // b) Remove clusters of salts
                for (int w = 1; w < (this->img_dim.second - 2); w++)
                {
                    for (int c = 0; c < COLOR_CHANNELS; c++)
                    {
                        if ((int)this->img[c][rowNo][w] == (int)this->img[c][rowNo][w + 1])
                        {
                            this->img[c][rowNo][w] = 0;
                        }
                    }
                }
            }

            // 7) Store the edge points in {this->edgePoints}
            pair<int, int> coord;
            for (int w = 1; w < (this->img_dim.second - 1); w++)
            {
                if ((int)this->img[0][rowNo][w] >= 100)
                {
                    coord.first = rowNo;
                    coord.second = w;
                    this->edgePoints.push_back(coord);
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

void BmpHandler::grayScaleTemplateMatch(int rowNo, int patchSize, int rowOffset, BmpHandler &bmpB)
{
    // CHECK EDGE POINTS
    if (this->edgePoints.size() <= 0)
    {
        cerr << "\nThere were no edge points in {this->edgePoints}...";
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

    // cout << this->edgePoints[0].first << " " << this->edgePoints[0].second << endl;

    int curr_ssd = 0;
    int best_ssd = INT32_MAX;
    pair<int, int> best_coord;
    for (int edgeNo = 0; edgeNo < this->edgePoints.size(); edgeNo++) // Only 1 edge Point will be match for now
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
                                                             [(this->edgePoints[edgeNo].first - (patchSize - 1) / 2) + r]
                                                             [(this->edgePoints[edgeNo].second - (patchSize - 1) / 2) + w] -
                                                    bmpB.img[0]
                                                            [(this->edgePoints[edgeNo].first + rowOffset - (patchSize - 1) / 2) + r]
                                                            [(col - (patchSize - 1) / 2) + w]));
                }
            }
            // Compare curr_ssd with best_ssd
            if (curr_ssd < best_ssd)
            {
                best_ssd = curr_ssd;
                best_coord.first = this->edgePoints[edgeNo].first + rowOffset;
                best_coord.second = col;
            }
        }
        if ((this->abs(best_coord.second - edgePoints[edgeNo].second) < this->getImgWidth() / 2) &&
            (best_coord.second > edgePoints[edgeNo].second))
        {
            bmpB.createBorder(best_coord.first - (patchSize - 1) / 2, best_coord.second - (patchSize - 1) / 2,
                              best_coord.first + (patchSize - 1) / 2, best_coord.second + (patchSize - 1) / 2);
            cout << "\n Left = "
                 << edgePoints[edgeNo].first << "," << edgePoints[edgeNo].second
                 << ", Right = "
                 << best_coord.first << "," << best_coord.second
                 << " , Error = " << best_ssd;
        }
    }
}

void BmpHandler::sobelTemplateMatch(int rowNo, int patchSize, BmpHandler &bmpB)
{
    if ((patchSize % 2 == 0) || patchSize < 3)
    {
        cerr << "Invalid Patch Size. Please make patch > 3 and odd";
        return;
    }

    pair<int, int> edge = this->edgePoints[0];
    cout << this->edgePoints[0].first << " " << this->edgePoints[0].second << endl;
    unsigned char ***imageB = bmpB.getImg();
    unsigned char ***patch;
    allocatePatch(patch, patchSize); // allocation on Heap
    println("Patch Allocated");

    // populate patch
    for (int i = 0; i < 3; i++)
        for (int r = 0; r < patchSize; r++)
            for (int w = 0; w < patchSize; w++)
                patch[i][r][w] = this->original_img[i][edge.first + (r - ((patchSize - 1) / 2))][edge.second + (w - ((patchSize - 1) / 2))];

    int ssd = 0;
    int min_ssd = INT32_MAX;
    pair<int, int> match_coord;
    for (int w = 10; w < bmpB.getImgWidth() - 10; w++)
    {
        ssd = 0;
        for (int r = 0; r < patchSize; r++)
        {
            for (int c = 0; c < patchSize; c++)
            {
                for (int k = 0; k < 3; k++)
                {
                    ssd += this->pow(this->abs((int)patch[k][r][c] -
                                               (int)bmpB.original_img[k][rowNo + (r - ((patchSize - 1) / 2))][w - ((patchSize - 1) / 2)]));
                }
            }
        }
        // std::cout << "SSD : " << ssd << std::endl;
        if (ssd < min_ssd)
        {
            min_ssd = ssd;
            match_coord.first = rowNo;
            match_coord.second = w;
            cout << rowNo << " " << w << " = " << ssd << endl;
        }
    }
    cout << match_coord.first << " " << match_coord.second << endl;
    // threshold for a match -> 150000
    std::cout << "Minimum Distance : " << min_ssd << std::endl;
    bmpB.createBorder(match_coord.first - (patchSize - 1) / 2, match_coord.second - (patchSize - 1) / 2,
                      match_coord.first + (patchSize - 1) / 2, match_coord.second + (patchSize - 1) / 2);
    deAllocatePatch(patch, patchSize); // deallocation on Heap
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