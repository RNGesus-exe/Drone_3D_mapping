#include "vision.h"
const int PATCH_SIZE = 41;
const int EDGE_LINES = 10;

int main()
{
    BitMap left, right;
    EdgePoints edges;
    init(EDGE_LINES, &edges, "Images/imageA.bmp", &left, "Images/imageB.bmp", &right);
    if (applyEdgeDetection(&edges, &left, PATCH_SIZE, EDGE_LINES) != 0)
    {
        perror("There was an issue in edge detection...");
    }
    if (sobelTemplateMatching(&edges, &left, &right, PATCH_SIZE, EDGE_LINES) != 0)
    {
        perror("There was an issue in template matching...");
    }
    bmpWriteImage(&left, "Images/imageA_w.bmp");
    bmpWriteImage(&right, "Images/imageB_w.bmp");
    deallocateBuffers(&left, &right, &edges, EDGE_LINES);
    return 0;
}