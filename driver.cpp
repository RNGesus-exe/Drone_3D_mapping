#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{

    BmpHandler bmpA("imageA");
    bmpA.applyGrayscale();
    bmpA.singleRowEdgeDetection(bmpA.getImgHeight() / 2, true, 1, 2);
    bmpA.writeBMPImage();
    return EXIT_SUCCESS;
}