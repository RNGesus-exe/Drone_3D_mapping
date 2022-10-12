#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{
    BmpHandler bmp("Images/imageA");
    bmp.applyGrayscale();
    bmp.applySobelEdgeDetection(bmp.getImgHeight() / 2, false, true, true, 21);
    bmp.writeBMPImage();

    BmpHandler bmp2;
    // for (int i = 1; i <= 10; i++)
    // {
    //     bmp2.setPath("Images/sampleC/right" + to_string(i));
    bmp2.setPath("Images/imageB");
    bmp2.applyGrayscale();
    // bmp.grayScaleTemplateMatch(bmp.getImgHeight() / 2, 21, 0, bmp2);
    bmp.sobelTemplateMatch(bmp.getImgHeight() / 2, 21, 0, bmp2);
    bmp2.writeBMPImage();
    bmp2.cleanUp();
    bmp.eraseRightEdgePoints();
    // }

    // vector<pair<int, int>> vec = bmpA.getEdgePoint();
    // printVecPairs(vec);

    return EXIT_SUCCESS;
}