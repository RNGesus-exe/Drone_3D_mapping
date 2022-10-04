/*
        FYP-1 TK 1, bmpreader.h
        Purpose: To perform operations on bmp images
        @author Khawaja Shaheryar
        @version 0.3 27/09/2022
*/

#ifndef BMPREADER_H
#define BMPREADER_H

/*
   Commenting your code is like cleaning your bathroom -
   you never want to do it, but it really does create a
   more pleasant experience for you and your guests.
   -Ryan Campbell
*/

#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#define HEADER_LEN 54    // The size of the header is fixed for the bmp files
#define COLOR_CHANNELS 3 // BGR

using namespace std;

class BmpHandler
{
    pair<uint16_t, uint16_t> img_dim;                                 // This will store the {Height x Width}{first x second} of the bmp image
    unsigned char ***original_img;                                    // This will hold the original bitmap image
    unsigned char ***img;                                             // This will hold the BRG image
    string path;                                                      // This will store the path of the bmp image
    char header[HEADER_LEN];                                          // This will store the header of the bmp image
    int contrast[COLOR_CHANNELS][2] = {{0, 255}, {0, 255}, {0, 255}}; // Holds lowest and highest value of 3 channels
    bool isGrayScale;                                                 // A flag which indicates if the image is in grayscale or not
    vector<pair<int, int>> edgePoints;                                // This will hold the coordinates{x,y} of the edges detected in the bitmap image

    /*
        Allocates memory for {this->img} data member
        Precondition: Make sure you have set the path for the bmp image
        Postcondition: This will load the bmp image from the given path {this->path}
        @param: buffer Reference of the 3D buffer which has to be allocated
    */
    void allocateBuffer(unsigned char ***&);
    /*
        The bmp image is read and loaded into {this->img}.
        For some reason reading directly into the 3d array doesn't work properly on windows
        so it reads into intermediate 1-d Array and then converts to 3-d.
        Also populates the image dimensions
    */
    int readImage();
    /*
        Allocate memory for template matching patch.
        @param buf Pointer to the 3d array
        @param patchSize Size of the array
    */
    void allocatePatch(unsigned char ***&patch, int patchSize);
    /*
       De-Allocate memory for template matching patch.
       @param buf Pointer to the 3d array
       @param patchSize Size of the array
   */
    void deAllocatePatch(unsigned char ***&patch, int patchSize);
    /*
        This will find and update minimum and maximum value from the BRG layers of the bmp image
        @param The Blue, Red and Green pixel value
    */
    void findMinMaxContrast(int, int, int);
    /*
        There is an issue with the linux g++ compiler when using the abs() function
        so, I made my own abs function to fix that problem
        @param An integer
        @return The absolute of the integer
    */
    int abs(int);
    /*
        This functions will make a - colored box around the given patch
        @param x1 starting x coordinate
        @param y1 starting y coordinate
        @param x2 ending x coordinate
        @param y2 ending y coordinate
    */
    void createBorder(int, int, int, int);
    /*
        Converts 1-dimensional pixel data into 3-dimensional [RGB][ROW][COL]
        @param buf 1-d pixel array
    */
    void convertTo3d(uint8_t *buf);

    /*  This is a function to get square of a value
        @param value Enter the value to get square of
        @return square Returns the square of the value
    */
    int pow(int);

public:
    /*
        This is the parameterized constructor
        @param The path for the bmp image without the ".bmp" extension
        if(path is passed)
            The image will be loaded into {this->img}
    */

    BmpHandler(string = "");
    /*
        This is the destructor
    */
    ~BmpHandler();
    /*
        This is the mutator for {this->path}
        @param path: The path of the bmp image
    */
    void setPath(string);
    /*
        This is the accessor for {this->path}
        @return The path of the BmpImage
    */

    string getPath() const;

    /* Returns the pointer to pixel data */

    unsigned char ***getImg()
    {
        return this->img;
    }
    /*
            This is the accessor for {this->img_dim.first}
            @return The Height of the BmpImage
    */
    int getImgHeight() const;
    /*
        This will return the vector holding the edge points of the bitmap image in {this->img}
        @return A vector of pair with edge points of the bitmap image
    */
    vector<pair<int, int>> getEdgePoint() const;
    /*
       This is the accessor for {this->img_dim.second}
       @return The Width of the BmpImage
    */
    int getImgWidth() const;

    /*
        This function will write the bmp image to the given {this->path + "_w.bmp"}
        @param flag set true for original image, set false for modified image
    */
    void writeBMPImage(bool = false);
    /*
        This function will apply auto contrast on the original image
        Formula: Pixel = (Pixel - MinPixelValue) * (255/ (highest - lowest pixel))
    */
    void applyAutoContrast();
    /*
        The {this->img} will be converted into grayscale
        Formula: (B + G + R) / 3
        There is another formula available in the member function definition
    */
    void applyGrayscale();
    /*
        A basic edge detection algo will be used on a single row of the image
        The function will also flip the {this->isGrayScale} flag to (true)
        @param row: The row on which edge detection should be applied
        @param displayEdges: If set true, the edges will be visible when using {this->writeBMPImage}
        @param padding: How many columns to skip from left and right
        @param skip_amount: If edge is found, how many pixels to skip after it. DON'T USE LESS THEN 1
    */
    void singleRowEdgeDetection(int, bool = false, int = 1, int = 3);
    /*
        The sobel vertical and horizontal operator will be applied on {this->img}
            The points are then stored/appended in {this->edgePoints}
        @param rowNo Which row to apply edgeDetection on
        @param horizontalFlag true = Apply the horizontalFilter, false = vice versa
        @param verticalFlag true = Apply the verticalFilter, false = vice versa
        @param cleanFlag true = Clean the edges, false = Don't clean the edges
    */
    void applySobelEdgeDetection(int, bool = true, bool = true, bool = true);
    /*
        Match Sobel Edges
        @param rowNo Which row to start matching on the other image
        @param patchSize Size of the patch (Should be an odd number > 3)
        @param img Object for the image you want to match on
    */
    void sobelTemplateMatch(int rowNo, int patchSize, BmpHandler &img);

    void troubleShoot();
};

#endif