#include "bmpreader.h"

const int PATCH_SIZE = 51;
const int EDGE_LINES = 10;
const int TOTAL_IMAGES = 1;

int main()
{
    BmpHandler bmp("Images/sampleC/left");
    bmp.applyGrayscale(true);
    bmp.applyAutoContrast(true);
    int height = bmp.getImgHeight() - (PATCH_SIZE * 2); // We will subtract to avoid the top and bot borders
    int interval = (height / EDGE_LINES);
    int curr_interval = PATCH_SIZE;
    for (int i = 0; i < EDGE_LINES; i++, curr_interval += interval) // edges are detected in 10 rows
    {
        bmp.applySobelEdgeDetection(curr_interval, PATCH_SIZE, i, false);
    }
    bmp.writeBMPImage();

    BmpHandler bmp2;
    for (int i = 1; i <= TOTAL_IMAGES; i++)
    {
        bmp2.setPath("Images/sampleC/right" + to_string(i));
        // bmp2.setPath("Images/imageB");
        bmp2.applyGrayscale(true);
        bmp2.applyAutoContrast(true);

        curr_interval = PATCH_SIZE;
        for (int i = 0; i < EDGE_LINES; i++, curr_interval += interval)
        {
            bmp.sobelTemplateMatch(curr_interval, PATCH_SIZE, 0, i, bmp2);
        }
        bmp2.writeBMPImage();
        bmp2.cleanUp();
        if (i != TOTAL_IMAGES)
        {
            bmp.eraseRightEdgePoints();
        }
    }

    bmp.writeBMPEdges(PATCH_SIZE, true, true);

    vector<vector<pair<int, int>>> left = bmp.getLeftEdgePoint();
    vector<vector<pair<int, int>>> right = bmp.getRightEdgePoint();

    cout << "\n SIZE = ("
         << left.size() << "," << right.size() << ')';
    for (int i = 0; i < left.size(); i++)
    {
        cout << "\n ROW#" << i;
        for (int j = 0; j < left[i].size(); j++)
        {
            cout << "\n Index#" << i + 1
                 << " = (" << left[i][j].first << "," << left[i][j].second << ") = ("
                 << right[i][j].first << "," << right[i][j].second << ')';
        }
    }

    return EXIT_SUCCESS;
}