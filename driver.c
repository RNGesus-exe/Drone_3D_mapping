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

void printEdges(EdgePoints edges)
{
    for (int row = 0; row < EDGE_LINES; row++)
    {
        for (int col = 0; col < edges.rightEdgePoints[row].index; col++)
        {
            printf("Row - %d has %d edges\n", row, edges.rightEdgePoints[row].index);
        }
    }
}

int main()
{
    BitMap left, right;
    EdgePoints edges;
    init(EDGE_LINES, &edges, "Images/sampleD/left.bmp", &left, "Images/sampleD/right1.bmp", &right);

    if (applyEdgeDetection(&edges, &left, PATCH_SIZE, EDGE_LINES))
    {
        perror("Error detection\n");
        return -1;
    }

    if (sobelTemplateMatching(&edges, &left, &right, PATCH_SIZE, EDGE_LINES) != ALL_OK)
    {
        perror("Error finding Edges\n");
        return -1;
    }

    double **dist;
    dist = malloc(sizeof(double *) * EDGE_LINES);
    for (int i = 0; i < EDGE_LINES; i++)
    {
        dist[i] = malloc(sizeof(double) * edges.rightEdgePoints[i].index);
    }

    populateDistances(dist, EDGE_LINES, &edges);
    // interpolateImage(&right, dist, &edges, EDGE_LINES);

    bmpWriteImage(&left, "imageA_w.bmp");
    bmpWriteImage(&right, "imageB_w.bmp");
    deallocateBuffers(&left, &right, &edges, EDGE_LINES);
    _dealloc_dist(dist, EDGE_LINES);

    return 0;
}