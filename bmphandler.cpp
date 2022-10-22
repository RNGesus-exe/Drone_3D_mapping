#include "truevision.h"

BmpHandler::BmpHandler(string left_path, string right_path)
{
    this->streamHandle.readImages(left_path, right_path);
}

void BmpHandler::displayImages(bool choice)
{
    this->streamHandle.writeBMPImages(choice);
}

int BmpHandler::abs(int val)
{
    return (val >= 0) ? val : (-1 * val);
}

void BmpHandler::applyAutoContrastOnImages(bool modify)
{
    if (modify)
    {
        if (!this->streamHandle.getLeftBmp().isGrayScale || !this->streamHandle.getRightBmp().isGrayScale)
        {
            throw runtime_error("The image needs to be in grayscale, before applying auto contrast!!!");
        }

        // LEFT IMAGE

        int min_val = INT32_MAX;
        int max_val = INT32_MIN;

        for (int i = 0; i < this->streamHandle.getLeftBmp().getImgHeight(); i++)
        {
            for (int j = 0; j < this->streamHandle.getLeftBmp().getImgWidth(); j++)
            {
                if (this->streamHandle.getLeftBmp().grayscale_img[i][j] > max_val)
                {
                    max_val = this->streamHandle.getLeftBmp().grayscale_img[i][j];
                }
                if (this->streamHandle.getLeftBmp().grayscale_img[i][j] < min_val)
                {
                    min_val = this->streamHandle.getLeftBmp().grayscale_img[i][j];
                }
            }
        }

        for (int i = 0; i < this->streamHandle.getLeftBmp().getImgHeight(); i++)
        {
            for (int j = 0; j < this->streamHandle.getLeftBmp().getImgWidth(); j++)
            {
                this->streamHandle.getLeftBmp().grayscale_img[i][j] = (this->streamHandle.getLeftBmp().grayscale_img[i][j] - min_val) * (255 / this->abs(max_val - min_val));
            }
        }

        // RIGHT IMAGE

        min_val = INT32_MAX;
        max_val = INT32_MIN;

        for (int i = 0; i < this->streamHandle.getRightBmp().getImgHeight(); i++)
        {
            for (int j = 0; j < this->streamHandle.getRightBmp().getImgWidth(); j++)
            {
                if (this->streamHandle.getRightBmp().grayscale_img[i][j] > max_val)
                {
                    max_val = this->streamHandle.getRightBmp().grayscale_img[i][j];
                }
                if (this->streamHandle.getRightBmp().grayscale_img[i][j] < min_val)
                {
                    min_val = this->streamHandle.getRightBmp().grayscale_img[i][j];
                }
            }
        }

        for (int i = 0; i < this->streamHandle.getRightBmp().getImgHeight(); i++)
        {
            for (int j = 0; j < this->streamHandle.getRightBmp().getImgWidth(); j++)
            {
                this->streamHandle.getRightBmp().grayscale_img[i][j] = (this->streamHandle.getRightBmp().grayscale_img[i][j] - min_val) * (255 / this->abs(max_val - min_val));
            }
        }
    }
    else
    {
        int min_val[COLOR_CHANNELS] = {INT32_MAX, INT32_MAX, INT32_MAX};
        int max_val[COLOR_CHANNELS] = {INT32_MIN, INT32_MIN, INT32_MIN};

        for (int c = 0; c < COLOR_CHANNELS; c++)
        {
            for (int i = 0; i < this->streamHandle.getRightBmp().getImgHeight(); i++)
            {
                for (int j = 0; j < this->streamHandle.getRightBmp().getImgWidth(); j++)
                {
                    if (this->streamHandle.getRightBmp().img[c][i][j] > max_val[c])
                    {
                        max_val[c] = this->streamHandle.getRightBmp().img[c][i][j];
                    }
                    if (this->streamHandle.getRightBmp().img[c][i][j] < min_val[c])
                    {
                        min_val[c] = this->streamHandle.getRightBmp().img[c][i][j];
                    }
                }
            }
        }

        min_val[0] = min_val[1] = min_val[2] = INT32_MAX;
        max_val[0] = max_val[1] = max_val[2] = INT32_MIN;

        for (int c = 0; c < COLOR_CHANNELS; c++)
        {
            for (int i = 0; i < this->streamHandle.getRightBmp().getImgHeight(); i++)
            {
                for (int j = 0; j < this->streamHandle.getRightBmp().getImgWidth(); j++)
                {
                    if (this->streamHandle.getRightBmp().img[c][i][j] > max_val[c])
                    {
                        max_val[c] = this->streamHandle.getRightBmp().img[c][i][j];
                    }
                    if (this->streamHandle.getRightBmp().img[c][i][j] < min_val[c])
                    {
                        min_val[c] = this->streamHandle.getRightBmp().img[c][i][j];
                    }
                }
            }
        }
    }
}

void BmpHandler::rewriteLeftBMPEdges(int patchSize, vector<vector<pair<int, int>>> &edgePoints)
{
    for (int i = 0; i < edgePoints.size(); i++)
    {
        for (int j = 0; j < edgePoints[i].size(); j++)
        {
            this->createBorder(this->streamHandle.getLeftBmp(), edgePoints[i][j].first - (patchSize) / 2, edgePoints[i][j].second - (patchSize) / 2,
                               edgePoints[i][j].first + (patchSize) / 2, edgePoints[i][j].second + (patchSize) / 2, 2);
        }
    }
}

void BmpHandler::createBorder(Bmp &bmp, int x1, int y1, int x2, int y2, int color)
{
    if ((x1 <= x2) &&
        (y1 <= y2))
    {
        if ((x1 >= 0) &&
            (x2 < bmp.img_dim.first) &&
            (y1 >= 0) &&
            (y2 < bmp.img_dim.second))
        {
            switch (color)
            {
            case 1:
                // (x1,y1) to (x1,y2) Top Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    bmp.img[0][x1][col] = 255;
                    bmp.img[1][x1][col] = 0;
                    bmp.img[2][x1][col] = 0;
                }

                // (x2,y1) to (x2,y2) Bottom Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    bmp.img[0][x2][col] = 255;
                    bmp.img[1][x2][col] = 0;
                    bmp.img[2][x2][col] = 0;
                }

                // (x1,y1) to (x2,y1) Left Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    bmp.img[0][row][y1] = 255;
                    bmp.img[1][row][y1] = 0;
                    bmp.img[2][row][y1] = 0;
                }

                // (x1,y2) to (x2,y2) Right Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    bmp.img[0][row][y2] = 255;
                    bmp.img[1][row][y2] = 0;
                    bmp.img[2][row][y2] = 0;
                }
                break;
            case 2:
                // (x1,y1) to (x1,y2) Top Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    bmp.img[0][x1][col] = 0;
                    bmp.img[1][x1][col] = 255;
                    bmp.img[2][x1][col] = 0;
                }

                // (x2,y1) to (x2,y2) Bottom Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    bmp.img[0][x2][col] = 0;
                    bmp.img[1][x2][col] = 255;
                    bmp.img[2][x2][col] = 0;
                }

                // (x1,y1) to (x2,y1) Left Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    bmp.img[0][row][y1] = 0;
                    bmp.img[1][row][y1] = 255;
                    bmp.img[2][row][y1] = 0;
                }

                // (x1,y2) to (x2,y2) Right Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    bmp.img[0][row][y2] = 0;
                    bmp.img[1][row][y2] = 255;
                    bmp.img[2][row][y2] = 0;
                }
                break;
            case 3:
                // (x1,y1) to (x1,y2) Top Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    bmp.img[0][x1][col] = 0;
                    bmp.img[1][x1][col] = 0;
                    bmp.img[2][x1][col] = 255;
                }

                // (x2,y1) to (x2,y2) Bottom Line
                for (unsigned int col = y1; col <= y2; ++col)
                {
                    bmp.img[0][x2][col] = 0;
                    bmp.img[1][x2][col] = 0;
                    bmp.img[2][x2][col] = 255;
                }

                // (x1,y1) to (x2,y1) Left Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    bmp.img[0][row][y1] = 0;
                    bmp.img[1][row][y1] = 0;
                    bmp.img[2][row][y1] = 255;
                }

                // (x1,y2) to (x2,y2) Right Line
                for (unsigned int row = x1; row <= x2; ++row)
                {
                    bmp.img[0][row][y2] = 0;
                    bmp.img[1][row][y2] = 0;
                    bmp.img[2][row][y2] = 255;
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

int BmpHandler::pow(int x)
{
    return x * x;
}

StreamHandler &BmpHandler::getStreamHandle()
{
    return this->streamHandle;
}

BmpHandler::BmpHandler(const BmpHandler &bmpHandle)
{
    this->streamHandle = bmpHandle.streamHandle;
}

void BmpHandler::operator=(const BmpHandler &bmpHandle)
{
    this->streamHandle = bmpHandle.streamHandle;
}

void BmpHandler::drawBordersFromVec(Bmp &bmp, vector<pair<int, int>> &vec, int patchSize)
{
    for (auto v : vec)
    {
        this->createBorder(bmp, v.first - (patchSize) / 2, v.second - (patchSize) / 2,
                           v.first + (patchSize) / 2, v.second + (patchSize) / 2, 2);
    }
}

void BmpHandler::applyGrayscaleOnImages(bool modify)
{
    if (!streamHandle.hasReadImage())
    {
        throw runtime_error("There is no data in buffer {this->original_img}, Grayscale could not be applied");
    }

    this->streamHandle.getLeftBmp().allocate2DBuffer(this->streamHandle.getLeftBmp().grayscale_img);
    this->streamHandle.getRightBmp().allocate2DBuffer(this->streamHandle.getRightBmp().grayscale_img);

    for (unsigned int row = 0; row < this->streamHandle.getLeftBmp().img_dim.first; ++row)
    {
        for (unsigned int col = 0; col < this->streamHandle.getLeftBmp().img_dim.second; ++col)
        {
            if (modify)
            {
                this->streamHandle.getLeftBmp().img[0][row][col] =
                    this->streamHandle.getLeftBmp().img[1][row][col] =
                        this->streamHandle.getLeftBmp().img[2][row][col] =
                            (this->streamHandle.getLeftBmp().img[0][row][col] +
                             this->streamHandle.getLeftBmp().img[1][row][col] +
                             this->streamHandle.getLeftBmp().img[2][row][col]) /
                            COLOR_CHANNELS;
            }
            this->streamHandle.getLeftBmp().grayscale_img[row][col] = (this->streamHandle.getLeftBmp().img[0][row][col] +
                                                                       this->streamHandle.getLeftBmp().img[1][row][col] +
                                                                       this->streamHandle.getLeftBmp().img[2][row][col]) /
                                                                      COLOR_CHANNELS;
        }
    }

    for (unsigned int row = 0; row < this->streamHandle.getRightBmp().img_dim.first; ++row)
    {
        for (unsigned int col = 0; col < this->streamHandle.getRightBmp().img_dim.second; ++col)
        {
            if (modify)
            {
                this->streamHandle.getRightBmp().img[0][row][col] =
                    this->streamHandle.getRightBmp().img[1][row][col] =
                        this->streamHandle.getRightBmp().img[2][row][col] =
                            (this->streamHandle.getRightBmp().img[0][row][col] +
                             this->streamHandle.getRightBmp().img[1][row][col] +
                             this->streamHandle.getRightBmp().img[2][row][col]) /
                            COLOR_CHANNELS;
            }
            this->streamHandle.getRightBmp().grayscale_img[row][col] = (this->streamHandle.getRightBmp().img[0][row][col] +
                                                                        this->streamHandle.getRightBmp().img[1][row][col] +
                                                                        this->streamHandle.getRightBmp().img[2][row][col]) /
                                                                       COLOR_CHANNELS;
        }
    }
    this->streamHandle.getLeftBmp().isGrayScale = true;
    this->streamHandle.getRightBmp().isGrayScale = true;
}