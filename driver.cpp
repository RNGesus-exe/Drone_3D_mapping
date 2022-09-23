#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{
    srand(time(NULL)); // Seed

    BmpHandler bmpA("imageA");
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

    BmpHandler bmpB("imageB");
    bmpA.templateMatching(bmpB);

    bmpA.writeBMPImage(false);
    bmpB.writeBMPImage(false);

    // vector<pair<int, int>> vec = bmpA.getEdgePoint();
    // printVecPairs(vec);
    return EXIT_SUCCESS;
}