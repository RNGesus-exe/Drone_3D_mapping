#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{
    BmpHandler bmp("Images/imageA");
    BmpHandler bmp2("Images/imageB");
    bmp.applyGrayscale();
    bmp.singleRowEdgeDetection(240, true, 1, 1);
    bmp.writeBMPImage();
    return EXIT_SUCCESS;
}