#include "bmpreader.h"

BmpHandler::BmpHandler(string _path_) : path(_path_)
{
    this->img = nullptr;
    this->isGrayScale = false;
    if (this->path != "")
    {
        this->loadBmpImage();
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
                this->img[0][row][col] = (this->img[0][row][col] - this->contrast[0][1]) * (255 / (this->contrast[0][0] - this->contrast[0][1]));
                this->img[1][row][col] = (this->img[1][row][col] - this->contrast[1][1]) * (255 / (this->contrast[1][0] - this->contrast[1][1]));
                this->img[2][row][col] = (this->img[2][row][col] - this->contrast[2][1]) * (255 / (this->contrast[2][0] - this->contrast[2][1]));
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

void BmpHandler::singleRowEdgeDetection(int row)
{
    if (this->img)
    {
        if (this->isGrayScale)
        {
            if ((row >= 0) &&
                (row < this->img_dim.first))
            {
                for (unsigned int col = 0; col < (this->img_dim.second - 1); ++col)
                {
                    if (this->abs((int)this->img[0][row][col] - (int)this->img[0][row][col + 1]) > 30)
                    {
                        this->img[0][row][col] =
                            this->img[1][row][col] =
                                this->img[2][row][col] = 255;
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

void BmpHandler::loadBmpImage()
{
    if (this->path != "") // Check if path is set
    {
        fstream bmp;
        bmp.open(path + ".bmp", ios::in);
        if (bmp)
        {
            this->img_dim = this->getImageDimensions(bmp); // This will load the Height x Width of the bmp
            this->allocateOriginalImage();                 // This will allocate the memory for the 3D array holding the original image
            readBMPImage(bmp);                             // This will read the bmp and load it into the data structure
        }
        bmp.close();
    }
    else
    {
        cerr << "\nThere was no <path> given from which bmp file could be loaded...";
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
}

void BmpHandler::writeBMPImage()
{
    if (this->img)
    {
        fstream bmp;
        int avg = 0;
        bmp.open(this->path + "_w.bmp", ios::out);
        if (bmp)
        {
            bmp.write(this->header, HEADER_LEN);
            for (unsigned int row = 0; row < this->img_dim.first; ++row)
            {
                for (unsigned int col = 0; col < this->img_dim.second; ++col)
                {
                    bmp << (char)this->img[0][row][col] << (char)this->img[1][row][col] << (char)this->img[2][row][col];
                }
            }
        }
        else
        {
            cerr << "\nThere was an error opening/creating your bmp image <" << path << "_w.bmp>";
        }
        bmp.close();
    }
    else
    {
        cerr << "\nThere was no data in buffer {this->img}, Nothing could be written in {this->path + \"_w.bmp\"}...";
    }
}

pair<uint16_t, uint16_t> BmpHandler::getImageDimensions(fstream &bmp)
{
    bmp.read(this->header, HEADER_LEN);

    uint16_t width = (uint8_t)this->header[19];
    width <<= 8;
    width += (uint8_t)this->header[18];

    uint16_t height = (uint8_t)this->header[23];
    height <<= 8;
    height += (uint8_t)this->header[22];

    pair<uint16_t, uint16_t> img_dim;
    img_dim.first = height;
    img_dim.second = width;

    return img_dim;
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

void BmpHandler::readBMPImage(fstream &bmp)
{
    int row = 0;
    int width = 0;
    char tmp[3];
    while (row < this->img_dim.first)
    {
        width = 0;
        while (width < this->img_dim.second)
        {
            bmp.read(tmp, 3);
            for (unsigned int color = 0; color < COLOR_CHANNELS; ++color)
            {
                this->img[color][row][width] = tmp[color];
            }
            findMinMaxContrast(this->img[0][row][width], this->img[1][row][width], this->img[2][row][width]);
            width++;
        }
        row++;
    }
}

void BmpHandler::allocateOriginalImage()
{
    this->img = new unsigned char **[COLOR_CHANNELS];
    for (unsigned int channel = 0; channel < COLOR_CHANNELS; ++channel)
    {
        this->img[channel] = new unsigned char *[this->img_dim.first];
        for (unsigned int row = 0; row < this->img_dim.first; ++row)
        {
            this->img[channel][row] = new unsigned char[this->img_dim.second];
        }
    }
}