#include "truevision.h"

StreamHandler::StreamHandler(string left_path, string right_path)
{
    this->readImages(left_path, right_path);
}

void StreamHandler::readImages(string left_path, string right_path)
{
    if (left_path != "" && right_path != "")
    {
        this->leftBmp.path = left_path;
        this->readImage(leftBmp);
        this->rightBmp.path = right_path;
        this->readImage(rightBmp);
    }
}

bool StreamHandler::hasReadImage() const
{
    return this->leftBmp.hasImage && this->rightBmp.hasImage;
}

Bmp &StreamHandler::getRightBmp()
{
    return this->rightBmp;
}

Bmp &StreamHandler::getLeftBmp()
{
    return this->leftBmp;
}

StreamHandler::StreamHandler(const StreamHandler &handle)
{
    this->leftBmp = handle.leftBmp;
    this->rightBmp = handle.rightBmp;
}

void StreamHandler::operator=(const StreamHandler &handle)
{
    this->leftBmp = handle.leftBmp;
    this->rightBmp = handle.rightBmp;
}

void StreamHandler::convertTo3d(Bmp &bmp, uint8_t *&buf)
{
    bmp.allocate3DBuffer(bmp.original_img);
    bmp.allocate3DBuffer(bmp.img);

    for (int r = 0; r < bmp.img_dim.first; r++)
    {
        for (int c = 0; c < bmp.img_dim.second; c++)
        {
            for (int k = 0; k < 3; k++)
            {
                bmp.img[k][(bmp.img_dim.first - 1) - r][c] = (unsigned char)buf[(r * bmp.img_dim.second * 3) + (c * 3) + (k)];
                bmp.original_img[k][(bmp.img_dim.first - 1) - r][c] = (unsigned char)buf[(r * bmp.img_dim.second * 3) + (c * 3) + (k)];
            }
        }
    }

    bmp.hasImage = true;
}

void StreamHandler::writeBMPImage(Bmp &bmp, bool imgType)
{
    if (!bmp.hasImage)
    {
        throw runtime_error("There is nothing allocated in {this->original_img ^ this->img}");
    }

    uint8_t *data = new uint8_t[(bmp.getImgHeight() * bmp.getImgWidth() * 3)];

    unsigned char ***buf = nullptr;

    if (imgType)
    {
        buf = bmp.original_img;
    }
    else
    {
        buf = bmp.img;
    }

    for (int r = 0; r < bmp.getImgHeight(); r++)
    {
        for (int c = 0; c < bmp.getImgWidth(); c++)
        {
            for (int k = 0; k < 3; k++)
            {
                data[(((bmp.getImgHeight() - 1) - r) * bmp.getImgWidth() * 3) + (c * 3) + (k)] = (uint8_t)buf[k][r][c];
            }
        }
    }
    FILE *file = fopen((bmp.path + "_w.bmp").c_str(), "wb");
    if (file)
    {
        fwrite(bmp.header, 1, HEADER_LEN, file);
        fseek(file, HEADER_LEN, SEEK_SET);
        fwrite((char *)data, 1, (bmp.getImgHeight() * bmp.getImgWidth() * 3), file);
        fflush(file);
        fclose(file);
    }
    else
    {
        printf("Could not open outfile\n");
    }
    delete[] data;
}

void StreamHandler::writeBMPImages(bool imgType)
{
    this->writeBMPImage(this->leftBmp, imgType);
    this->writeBMPImage(this->rightBmp, imgType);
}

bool StreamHandler::readImage(Bmp &bmp)
{
    if (bmp.path == "")
    {
        throw runtime_error("There was no path given, so data could not be read");
    }

    FILE *filePtr = fopen((bmp.path + ".bmp").c_str(), "rb");
    if (!filePtr)
    {
        throw runtime_error("Could not open file");
    }

    int bytes = fread(bmp.header, 1, HEADER_LEN, filePtr);
    if (bytes == 0 || bytes < HEADER_LEN)
    {
        throw runtime_error("Could not read header");
    }

    bmp.img_dim.second = *((int *)&bmp.header[18]);
    bmp.img_dim.first = *((int *)&bmp.header[22]);
    int imgSize = bmp.img_dim.first * bmp.img_dim.second * 3;

    int move = fseek(filePtr, 54, SEEK_SET);
    if (move != 0)
    {
        throw runtime_error("Could Not Move Pointer");
    }

    uint8_t *buf = new uint8_t[imgSize];
    long pixBytes = fread(buf, 1, imgSize, filePtr);
    if (pixBytes != imgSize)
    {
        throw runtime_error("Could Not Read Pixels");
    }

    fflush(filePtr);
    fclose(filePtr);
    filePtr = nullptr;
    this->convertTo3d(bmp, buf);
    delete[] buf;
    buf = nullptr;

    return true;
}