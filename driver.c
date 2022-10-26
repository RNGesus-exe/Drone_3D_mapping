#include "bitmap.h"
const int PATCH_SIZE = 41;
const int EDGE_LINES = 10;

int main()
{
    BitMap left;
    bmpReadImage(&left, "Images/imageA.bmp");
    bmpWriteImage(&left, "Images/imageA_w.bmp");
    bmpDeallocateBuffer(&left);
    return 0;
}