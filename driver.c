#include "vision.h"
const int PATCH_SIZE = 41;
const int EDGE_LINES = 10;

int main()
{
    BitMap left, right;
    EdgePoints edges;
    init(EDGE_LINES, &edges, "Images/imageA.bmp", &left, "Images/imageB.bmp", &right);
    applyEdgeDetection(&edges, &left, PATCH_SIZE, EDGE_LINES);
    for (int i = 0; i < EDGE_LINES; i++)
    {
        for (int j = 0; j < edges.leftEdgePoints[i].index; j++)
        {
            printf("%d - EDGE DATA : (%d,%d)\n", i, edges.leftEdgePoints[i].data[j].x, edges.leftEdgePoints[i].data[j].y);
        }
    }

    printf("TEMPLATE ERROR : %d\n", sobelTemplateMatchOnRow(&edges, &left, &right, PATCH_SIZE, EDGE_LINES));
    bmpWriteImage(&left, "Images/imageA_w.bmp");
    bmpWriteImage(&right, "Images/imageB_w.bmp");
    deallocateBuffers(&left, &right, &edges, EDGE_LINES);
    return 0;
}