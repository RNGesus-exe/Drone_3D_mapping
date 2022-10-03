#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

// imageB is 5 pixel above imageA
int main()
{
    BmpHandler bmp("Images/imageA");
    BmpHandler bmp2("Images/imageB");

    bmp.applyAutoContrast();
    bmp.applyGrayscale();
    bmp.applySobelEdgeDetection(bmp.getImgHeight() / 2);
    bmp.sobelTemplateMatch(bmp2.getImgHeight() / 2, 9, bmp2);
    bmp.writeBMPImage();
    bmp2.writeBMPImage();

    return EXIT_SUCCESS;
}
