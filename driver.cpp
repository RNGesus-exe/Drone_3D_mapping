#include "bmpreader.h"

/*
    This is the main function
*/
int main()
{
    BmpHandler bmpA("imageA");
    bmpA.applyAutoContrast();
    bmpA.applyGrayscale();
    bmpA.singleRowEdgeDetection(bmpA.getImgHeight() / 2);
    bmpA.writeBMPImage();
    return EXIT_SUCCESS;
}