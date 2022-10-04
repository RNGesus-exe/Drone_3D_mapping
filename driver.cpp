#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{
    BmpHandler bmp("Images/imageA");
    BmpHandler bmp2("Images/imageB");

    bmp.applyAutoContrast();
    bmp.applyGrayscale();
    bmp.applySobelEdgeDetection(bmp.getImgHeight() / 2, false);
    bmp.sobelTemplateMatch(bmp2.getImgHeight() / 2, 9, bmp2);
    bmp.writeBMPImage();
    bmp2.writeBMPImage();

    // vector<pair<int, int>> vec = bmpA.getEdgePoint();
    // printVecPairs(vec);

    return EXIT_SUCCESS;
}