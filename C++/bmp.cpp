#include "truevision.h"

Bmp::Bmp()
{
    this->original_img = this->img = nullptr;
    this->grayscale_img = nullptr;
    this->isGrayScale = false;
    this->hasImage = false;
}

void Bmp::allocate3DBuffer(unsigned char ***&buf)
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

void Bmp::allocate2DBuffer(unsigned char **&buf)
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

void Bmp::copy3DBuffer(unsigned char ***&destination, unsigned char ***&source)
{

    if (!this->hasImage)
    {
        throw runtime_error("There was no data in source to copy in destination");
    }

    for (int c = 0; c < COLOR_CHANNELS; c++)
    {
        for (int r = 0; r < this->img_dim.first; r++)
        {
            for (int w = 0; w < this->img_dim.second; w++)
            {
                destination[c][r][w] = source[c][r][w];
            }
        }
    }
}

void Bmp::operator=(const Bmp &bmp)
{
    if (bmp.hasImage)
    {
        this->img_dim.first = bmp.img_dim.first;
        this->img_dim.second = bmp.img_dim.second;
        this->isGrayScale = bmp.isGrayScale;
        this->path = bmp.path;

        this->allocate3DBuffer(this->original_img);
        for (int c = 0; c < COLOR_CHANNELS; c++)
        {
            for (int r = 0; r < this->img_dim.first; r++)
            {
                for (int w = 0; w < this->img_dim.second; w++)
                {
                    this->original_img[c][r][w] = bmp.original_img[c][r][w];
                }
            }
        }

        this->allocate3DBuffer(this->img);
        for (int c = 0; c < COLOR_CHANNELS; c++)
        {
            for (int r = 0; r < this->img_dim.first; r++)
            {
                for (int w = 0; w < this->img_dim.second; w++)
                {
                    this->img[c][r][w] = bmp.img[c][r][w];
                }
            }
        }

        if (this->isGrayScale)
        {
            this->allocate2DBuffer(this->grayscale_img);
            for (int r = 0; r < this->img_dim.first; r++)
            {
                for (int w = 0; w < this->img_dim.second; w++)
                {
                    this->grayscale_img[r][w] = bmp.grayscale_img[r][w];
                }
            }
        }
        else
        {
            this->grayscale_img = nullptr;
        }
    }
    else
    {
        this->original_img = nullptr;
        this->img = nullptr;
        this->grayscale_img = nullptr;
    }
}

Bmp::Bmp(const Bmp &bmp)
{
    if (bmp.hasImage)
    {
        this->img_dim.first = bmp.img_dim.first;
        this->img_dim.second = bmp.img_dim.second;
        this->isGrayScale = bmp.isGrayScale;
        this->path = bmp.path;

        this->allocate3DBuffer(this->original_img);
        for (int c = 0; c < COLOR_CHANNELS; c++)
        {
            for (int r = 0; r < this->img_dim.first; r++)
            {
                for (int w = 0; w < this->img_dim.second; w++)
                {
                    this->original_img[c][r][w] = bmp.original_img[c][r][w];
                }
            }
        }

        this->allocate3DBuffer(this->img);
        for (int c = 0; c < COLOR_CHANNELS; c++)
        {
            for (int r = 0; r < this->img_dim.first; r++)
            {
                for (int w = 0; w < this->img_dim.second; w++)
                {
                    this->img[c][r][w] = bmp.img[c][r][w];
                }
            }
        }

        if (this->isGrayScale)
        {
            this->allocate2DBuffer(this->grayscale_img);
            for (int r = 0; r < this->img_dim.first; r++)
            {
                for (int w = 0; w < this->img_dim.second; w++)
                {
                    this->grayscale_img[r][w] = bmp.grayscale_img[r][w];
                }
            }
        }
        else
        {
            this->grayscale_img = nullptr;
        }
    }
    else
    {
        this->original_img = nullptr;
        this->img = nullptr;
        this->grayscale_img = nullptr;
    }
}

void Bmp::deallocate3DBuffer(unsigned char ***&buf)
{
    if (buf)
    {
        for (int i = 0; i < COLOR_CHANNELS; ++i)
        {
            for (int j = 0; j < this->img_dim.first; ++j)
            {
                delete[] buf[i][j];
            }
            delete[] buf[i];
        }
        delete[] buf;
        buf = nullptr;
    }
}

void Bmp::deallocate2DBuffer(unsigned char **&buf)
{
    if (buf)
    {
        for (int i = 0; i < this->img_dim.first; ++i)
        {
            delete[] buf[i];
        }
        delete[] buf;
        buf = nullptr;
    }
}

int Bmp::getImgHeight() const
{
    return this->img_dim.first;
}

int Bmp::getImgWidth() const
{
    return this->img_dim.second;
}

Bmp::~Bmp()
{
    this->deallocate3DBuffer(this->original_img);
    this->deallocate3DBuffer(this->img);
    this->deallocate2DBuffer(this->grayscale_img);

    this->path = "";
    this->isGrayScale = false;
    this->hasImage = false;
}