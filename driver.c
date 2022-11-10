#include "vision.h"

const int PATCH_SIZE = 41;
const int EDGE_LINES = 1080 - 82;

void printDistances(EdgePoints edges)
{
    int totalEdgesFound = 0;
    for (int j = 0; j < EDGE_LINES; j++)
    {
        printf("EDGE LINE - %d\n", j);
        for (int i = 0; i < edges.rightEdgePoints[j].index; i++)
        {
            Pair b = edges.leftEdgePoints[j].data[i];
            Pair a = edges.rightEdgePoints[j].data[i];
            printf("Distance : %fcm at (%d,%d)\n", triangulateDistance(a, b), a.x, b.y);
            totalEdgesFound++;
        }
    }
    printf("TOTAL EDGES FOUND : %d\n", totalEdgesFound);
}

int main()
{
    BitMap left, right;
    EdgePoints edges;
    init(EDGE_LINES, &edges, "Images/SampleJ/left_3.bmp", &left, "Images/SampleJ/right_3.bmp", &right);
    if (applyEdgeDetection(&edges, &left, PATCH_SIZE, EDGE_LINES) != 0)
    {
        perror("There was an issue in edge detection...");
    }
    if (sobelTemplateMatching(&edges, &left, &right, PATCH_SIZE, EDGE_LINES) != 0)
    {
        perror("There was an issue in template matching...");
    }

    printDistances(edges);

    bmpWriteImage(&left, "Images/imageA_w_ii.bmp");
    bmpWriteImage(&right, "Images/imageB_w_ii.bmp");
    deallocateBuffers(&left, &right, &edges, EDGE_LINES);

    return 0;
}