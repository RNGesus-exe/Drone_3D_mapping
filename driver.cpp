#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{

    BmpHandler bmpA("Images/sampleA/left");
    bmpA.applyAutoContrast();
    bmpA.applyGrayscale();
    int row = bmpA.getImgHeight() / 2;
    // int row = 0;
    // while ((row <= 10) ||
    //        (row >= (bmpA.getImgHeight() - 10)))
    // {
    //     row = rand() % bmpA.getImgHeight();
    // }
    bmpA.singleRowEdgeDetection(row, true);

    BmpHandler bmpB("Images/sampleA/right2");
    bmpA.singleRowTemplateMatching(bmpB, -10);

    bmpA.writeBMPImage(false);
    bmpB.writeBMPImage(false);
    return EXIT_SUCCESS;
}