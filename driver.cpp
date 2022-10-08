#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{
    BmpHandler bmp("Images/sampleB/left");
    bmp.applyGrayscale();

    BmpHandler bmp2("Images/sampleB/right1");
    bmp2.applyGrayscale();

    bmp.applySobelEdgeDetection(bmp.getImgHeight() / 2, false);
    bmp.grayScaleTemplateMatch(bmp.getImgHeight() / 2, 21, 0, bmp2);
    bmp.writeBMPImage();
    bmp2.writeBMPImage();

    // vector<pair<int, int>> vec = bmpA.getEdgePoint();
    // printVecPairs(vec);

    return EXIT_SUCCESS;
}