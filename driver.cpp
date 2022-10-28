#include "truevision.h"

const int PATCH_SIZE = 41;
const int EDGE_LINES = 10;

int main()
{
    println("Start");
    BitMap bmp("Images/sampleH/left", "Images/sampleH/right1");
    println("AFter constructr");
    bmp.findEdgesInLeftImage(PATCH_SIZE, EDGE_LINES);
    println("Edges Found");
    bmp.executeTemplateMatching(PATCH_SIZE, EDGE_LINES);
    bmp.displayImages();
    println("END");
    return EXIT_SUCCESS;
}