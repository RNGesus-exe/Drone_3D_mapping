#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{

    BmpHandler bmpA("imageA");
    bmpA.applyGrayscale();
    bmpA.writeBMPImage();
    return EXIT_SUCCESS;
}