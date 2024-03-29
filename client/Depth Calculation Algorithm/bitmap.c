#include "bitmap.h"

enum bmp_error bmpReadHeader(BmpHeader *header, FILE *img_file)
{
    if (img_file == NULL)
    {
        return BMP_FILE_NOT_OPENED;
    }

    // Since an address must be passed to fread, create a variable!
    unsigned short magic;

    // Check if its an bmp file by comparing the magic nbr:
    if (fread(&magic, sizeof(magic), 1, img_file) != 1 ||
        magic != BMP_MAGIC)
    {
        return BMP_INVALID_FILE;
    }

    if (fread(header, sizeof(BmpHeader), 1, img_file) != 1)
    {
        return BMP_ERROR;
    }

    return BMP_OK;
}

void bmpAllocateBuffer(BitMap *bitmap)
{
    const int height = bitmap->header.biHeight;
    const int width = bitmap->header.biWidth;

    // Since we have moved the fptr ahead of the header in the bmp file, we can now just read the image data

    // bitmap->original_img = (unsigned char ***)malloc(COLOR_CHANNELS * sizeof(unsigned char **)); // 3D
    // bitmap->original_img[0] = (unsigned char **)malloc(sizeof(unsigned char *) * height);
    // bitmap->original_img[1] = (unsigned char **)malloc(sizeof(unsigned char *) * height);
    // bitmap->original_img[2] = (unsigned char **)malloc(sizeof(unsigned char *) * height);

    bitmap->modified_img = (unsigned char **)malloc(height * sizeof(unsigned char *));  // 2D
    bitmap->grayscale_img = (unsigned char **)malloc(height * sizeof(unsigned char *)); // 2D

    for (int r = 0; r < height; r++)
    {
        // bitmap->original_img[0][r] = (unsigned char *)malloc(width * sizeof(unsigned char));
        // bitmap->original_img[1][r] = (unsigned char *)malloc(width * sizeof(unsigned char));
        // bitmap->original_img[2][r] = (unsigned char *)malloc(width * sizeof(unsigned char));

        bitmap->modified_img[r] = (unsigned char *)malloc(width * sizeof(unsigned char));
        bitmap->grayscale_img[r] = (unsigned char *)malloc(width * sizeof(unsigned char));
    }
}

enum bmp_error bmpReadImage(BitMap *bitmap, const char *path)
{
    // Load left image first
    FILE *fptr = NULL;
    fptr = fopen(path, "rb");

    // 1) Allocate Header
    if (bmpReadHeader(&bitmap->header, fptr) != BMP_OK)
    {
        fclose(fptr);
        printf("\nThere was an issue reading the header of %s\n", path);
        return BMP_ERROR;
    }

    // 2) Allocate Buffers {this->modified_img ^ this->grayscale_img}
    bmpAllocateBuffer(bitmap);

    const int height = bitmap->header.biHeight;
    const int width = bitmap->header.biWidth;
    int row_pad = 0;
    if ((width * 3) % 4)
    {
        row_pad = 4 - ((width * 3) % 4);
    }
    unsigned char tmpBuff[3];

    for (int r = 0; r < height; r++)
    {
        for (int w = 0; w < width; w++)
        {
            if (fread(tmpBuff, sizeof(tmpBuff), 1, fptr) != 1)
            {
                fclose(fptr);
                printf("\nThere was an error while reading the image...");
                return BMP_ERROR;
            }

            bitmap->grayscale_img[height - r - 1][w] =
                bitmap->modified_img[height - r - 1][w] =
                    (double)(tmpBuff[0] + tmpBuff[1] + tmpBuff[2]) / COLOR_CHANNELS;
        }
        fseek(fptr, row_pad, SEEK_CUR);
    }

    // 3) Apply CDF on image

    unsigned int hist[256] = {0};

    // 3.1) Calculate Histogram of Image

    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            hist[(int)bitmap->grayscale_img[i][j]]++;
        }
    }
    // 3.2) Calculate CDF image

    unsigned int sum_of_hist[256];
    unsigned int sum = 0;
    for (short i = 1; i < 256; i++)
    {
        sum += hist[i];
        sum_of_hist[i] = sum;
    }

    // 3.3) Apply CDF on Image

    size_t area = height * width;
    size_t dm = 256;
    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            bitmap->modified_img[i][j] =
                bitmap->grayscale_img[i][j] =
                    sum_of_hist[(int)bitmap->grayscale_img[i][j]] * ((double)dm / area);
            // bitmap->modified_img[i][j] = bitmap->grayscale_img[i][j];
        }
    }

    // 4) Apply Gaussian Filter for sigma = 1
    // const double sigma = 1;
    // const double normal = (double)1 / (2.0 * PI * sigma * sigma);
    // const int k = 2;

    // double gaussian_kernel[5][5];
    // for (int i = 1; i < 6; i++)
    // {
    //     for (int j = 1; j < 6; j++)
    //     {
    //         gaussian_kernel[i - 1][j - 1] = exp(-1 * (((i - (k + 1)) * (i - (k + 1))) + ((j - (k + 1)) * (j - (k + 1)))) / (2.0 * sigma * sigma)) * normal;
    //     }
    // }

    // for (int r = 2; r < height - 2; r++)
    // {
    //     for (int w = 2; w < width - 2; w++)
    //     {
    //         int sum = 0;
    //         for (int i = 0; i < 5; i++)
    //         {
    //             for (int j = 0; j < 5; j++)
    //             {
    //                 sum += (double)bitmap->grayscale_img[(r - 2 + i)][(w - 2 + j)] * gaussian_kernel[i][j];
    //             }
    //         }
    //         bitmap->modified_img[r][w] = bitmap->grayscale_img[r][w] = sum;
    //     }
    // }

    // 4) Close the file
    fflush(fptr);
    fclose(fptr);
    return BMP_OK;
}

enum bmp_error bmpWriteHeader(const BmpHeader *header, FILE *fptr)
{
    if (header == NULL)
    {
        return BMP_HEADER_NOT_INITIALIZED;
    }

    // Since an adress must be passed to fwrite, create a variable!
    const unsigned short magic = BMP_MAGIC;
    fwrite(&magic, sizeof(magic), 1, fptr);

    // Use the type instead of the variable because its a pointer!
    fwrite(header, sizeof(BmpHeader), 1, fptr);
    return BMP_OK;
}

enum bmp_error bmpWriteImage(const BitMap *bitmap, const char *path)
{
    FILE *fptr = fopen(path, "wb");
    if (fptr == NULL)
    {
        return BMP_FILE_NOT_OPENED;
    }

    if (bmpWriteHeader(&bitmap->header, fptr) != BMP_OK)
    {
        fclose(fptr);
        return BMP_ERROR;
    }

    const size_t height = bitmap->header.biHeight;
    const size_t width = bitmap->header.biWidth;
    unsigned char tmpBuf[3];

    int row_pad = 0;
    if ((width * 3) % 4)
    {
        row_pad = 4 - ((width * 3) % 4);
    }

    char pad[3] = {0, 0, 0};
    for (size_t r = 0; r < height; r++)
    {
        for (size_t w = 0; w < width; w++)
        {
            tmpBuf[0] = tmpBuf[1] = tmpBuf[2] = bitmap->modified_img[height - r - 1][w];
            if (fwrite((char *)tmpBuf, 1, COLOR_CHANNELS, fptr) != COLOR_CHANNELS)
            {
                fclose(fptr);
                printf("\nThere was an issue writing the file...");
                return BMP_ERROR;
            }
        }
        fwrite(pad, 1, row_pad, fptr);
    }

    fflush(fptr);
    fclose(fptr);
    return BMP_OK;
}

void bmpDeallocateBuffer(BitMap *bitmap)
{
    const size_t height = bitmap->header.biHeight;

    if (bitmap->modified_img != NULL)
    {
        for (size_t h = 0; h < height; h++)
        {
            free(bitmap->modified_img[h]);
        }
        free(bitmap->modified_img);
        bitmap->modified_img = NULL;
    }
    if (bitmap->grayscale_img != NULL)
    {
        for (size_t h = 0; h < height; h++)
        {
            free(bitmap->grayscale_img[h]);
        }
        free(bitmap->grayscale_img);
        bitmap->grayscale_img = NULL;
    }
}