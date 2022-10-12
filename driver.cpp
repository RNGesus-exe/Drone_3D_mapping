#include "bmpreader.h"

const int PATCH_SIZE = 31;

int main()
{
    BmpHandler bmp("Images/sampleC/left");
    bmp.applyGrayscale();
    bmp.applySobelEdgeDetection(bmp.getImgHeight() / 2, false, true, true, PATCH_SIZE);
    bmp.writeBMPImage();

    BmpHandler bmp2;
    for (int i = 1; i <= 10; i++)
    {
        bmp2.setPath("Images/sampleC/right" + to_string(i));
        bmp2.applyGrayscale();
        bmp.sobelTemplateMatch(bmp.getImgHeight() / 2, PATCH_SIZE, 0, bmp2);
        bmp2.writeBMPImage();
        bmp2.cleanUp();
        if (i != 10)
        {
            bmp.eraseRightEdgePoints();
        }
    }

    vector<pair<int, int>> left = bmp.getLeftEdgePoint();
    vector<pair<int, int>> right = bmp.getRightEdgePoint();

    cout << "\n SIZE = ("
         << left.size() << "," << right.size() << ')';
    for (int i = 0; i < left.size(); i++)
    {
        cout << "\n Index#" << i + 1
             << " = (" << left[i].first << "," << left[i].second << ") = ("
             << right[i].first << "," << right[i].second << ')';
    }

    return EXIT_SUCCESS;
}