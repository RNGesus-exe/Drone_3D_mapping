#include "bmpreader.h"

const int PATCH_SIZE = 31;
const int EDGE_LINES = 10;

int main()
{
    BmpHandler bmp("Images/sampleA/left");
    bmp.applyGrayscale(true);
    int height = bmp.getImgHeight() - (PATCH_SIZE * 2); // We will subtract to avoid the top and bot borders
    int interval = (height / EDGE_LINES);
    int curr_interval = PATCH_SIZE;
    for (int i = 0; i < EDGE_LINES; i++, curr_interval += interval) // edges are detected in 10 rows
    {
        bmp.applySobelEdgeDetection(curr_interval, PATCH_SIZE, i, false);
    }
    bmp.writeBMPImage();

    BmpHandler bmp2;
    for (int i = 1; i <= 10; i++)
    {
        bmp2.setPath("Images/sampleA/right" + to_string(i));
        bmp2.applyGrayscale(true);

        curr_interval = PATCH_SIZE;
        for (int i = 0; i < EDGE_LINES; i++, curr_interval += interval)
        {
            bmp.sobelTemplateMatch(curr_interval, PATCH_SIZE, 0, i, bmp2);
        }

        bmp2.writeBMPImage();
        bmp2.cleanUp();
        if (i != 10)
        {
            bmp.eraseRightEdgePoints();
        }
    }

    // vector<pair<int, int>> left = bmp.getLeftEdgePoint();
    // vector<pair<int, int>> right = bmp.getRightEdgePoint();

    // cout << "\n SIZE = ("
    //      << left.size() << "," << right.size() << ')';
    // for (int i = 0; i < left.size(); i++)
    // {
    //     cout << "\n Index#" << i + 1
    //          << " = (" << left[i].first << "," << left[i].second << ") = ("
    //          << right[i].first << "," << right[i].second << ')';
    // }

    return EXIT_SUCCESS;
}