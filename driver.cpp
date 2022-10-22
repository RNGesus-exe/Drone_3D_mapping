#include "truevision.h"

const int PATCH_SIZE = 41;
const int EDGE_LINES = 10;

int main()
{
    BitMap bmp("Images/sampleA/left", "Images/sampleA/right1");
    bmp.findEdgesInLeftImage(PATCH_SIZE, EDGE_LINES);
    bmp.executeTemplateMatching(PATCH_SIZE, EDGE_LINES);
    bmp.displayImages();
    return EXIT_SUCCESS;
}