#include "bmpreader.h"

#define printVecPairs(vec) \
    for (auto v : vec)     \
    cout << v.first << " " << v.second << endl

int main()
{

    BmpHandler bmpA("Images/sampleA/left");
    bmpA.applyAutoContrast();
    bmpA.applyGrayscale();
    bmpA.writeBMPImage();
    return EXIT_SUCCESS;
}