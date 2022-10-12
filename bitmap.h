#ifndef BITMAP_H
#define BITMAP_H

#include <stdio.h>
#include <stdlib.h>

#define BMP_MAGIC 19778
#define COLOR_CHANNELS 3

// The structure to hold the bmp contents

enum bmp_error
{
    BMP_FILE_NOT_OPENED = -4,
    BMP_HEADER_NOT_INITIALIZED,
    BMP_INVALID_FILE,
    BMP_ERROR,
    BMP_OK = 0
};

typedef struct _bmp_header
{
    unsigned int bfSize;
    unsigned int bfReserved;
    unsigned int bfOffBits;
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BmpHeader;

typedef struct _bitmap_
{
    unsigned char **grayscale_img; // This will hold the grayscale image
    unsigned char **modified_img;  // This will hold the modified image, which will be displayed
    BmpHeader header;              // This will store the header of the bmp image
} BitMap;

enum bmp_error bmpReadHeader(BmpHeader *, FILE *);

void bmpAllocateBuffer(BitMap *);

enum bmp_error bmpReadImage(BitMap *, const char *);

enum bmp_error bmpWriteImage(const BitMap *, const char *);

enum bmp_error bmpWriteHeader(const BmpHeader *, FILE *);

void bmpDeallocateBuffer(BitMap *);

#endif