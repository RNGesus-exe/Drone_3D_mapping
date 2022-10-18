/*
        FYP-1, bmpreader.h
        Purpose: To perform operations on bmp images
        @author Khawaja Shaheryar
        @version 0.5 7/10/2022
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

/*Below are some macros defined for debugging purposes*/

#define println(x) std::cout << x << std::endl

using namespace std;

class BmpHandler
{
    pair<uint16_t, uint16_t> img_dim;                                 // This will store the {Height x Width}{first x second} of the bmp image
    unsigned char ***original_img;                                    // This will hold the original bitmap image
    unsigned char ***img;                                             // This will hold the BRG image
    unsigned char **grayscale_img;                                    // This will hold the grayscale image
    string path;                                                      // This will store the path of the bmp image
    char header[HEADER_LEN];                                          // This will store the header of the bmp image
    int contrast[COLOR_CHANNELS][2] = {{0, 255}, {0, 255}, {0, 255}}; // Holds lowest and highest value of 3 channels
    bool isGrayScale;                                                 // A flag which indicates if the image is in grayscale or not
    vector<vector<pair<int, int>>> leftEdgePoints;                    // This will hold the coordinates{x,y} of the edges detected in the left bitmap image
    vector<vector<pair<int, int>>> rightEdgePoints;                   // This will hold the coordinates{x,y} of the edges detected in the right bitmap image

    /*
        Allocates memory for a 3D array
        @param: buffer Reference of the 3D buffer which has to be allocated
    */
    void allocate3DBuffer(unsigned char ***&);
    /*
        Allocates memory for a 2D array
        @param: buffer Reference of the 2D buffer which has to be allocated
    */
    void allocate2DBuffer(unsigned char **&);
    /*
        Transfers 1D data into 3D [CHANNEL][ROW][COL]
        @param buf 1D pixel array
    */
    void convertTo3d(uint8_t *&);
    /*
        The bmp image is read and loaded into {this->img ^ this->original_img}.
        For some reason reading directly into the 3d array doesn't work properly on windows
        so it reads into intermediate 1D Array and then converts to 3D.
        Also populates the image dimensions into {this->img_dim}
    */
    int readImage();
    /*  This is a function to get square of a value
        @param value Enter the value to get square of
        @return square Returns the square of the value
    */
    int pow(int);
    /*
        There is an issue with the linux g++ compiler when using the abs() function
        so, I made my own abs function to fix that problem
        @param An integer
        @return The absolute of the integer
    */
    int abs(int);
    /*
        This is a helper function for {this->sobelTemplateMatch} to check if a value(x) is in range(w,z)
            so, {w <= x <= z}
        @param val The value to check if it is in range
        @param vec vector of pairs to check for range
        @param patchSize The size of the patch to check if it is in range
        @return true:not in range, false:in range
    */
    bool isInRange(int, vector<pair<int, int>> &, int range = 0);
    /*
        This function will
        @param patchSize The size of the patch to check for sobel edge detection
        @param x1 The starting x coordinate
        @param y1 The starting y coordinate
        @param x2 The ending x coordinate
        @param y2 The ending y coordinate
        @param horizontalFlag The flag for checking if we need to apply horizontal Sobel Operator or not
        @param verticalFlag The flag for checking if we need to apply vertical sobel operator or not
        @param cleanUp To make the image more smoother
    */
    bool applySobelEdgeDetectionOnPatch(int, int, int, int, int, bool = true, bool = true, bool = true);

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
            This is the accessor for {this->img_dim.first}
            @return The Height of the BmpImage
    */
    int getImgHeight() const;
    /*
       This is the accessor for {this->img_dim.second}
       @return The Width of the BmpImage
    */
    int getImgWidth() const;
    /*
        The {this->grayscale_img} will be loaded with a grayscale image from {this->original_img}
        Formula: (B + G + R) / 3
        There is another formula available in the member function definition
        @param modify Should the grayscale be stored for future use
    */
    void applyGrayscale(bool);
    /*
        This will return the 2D vector holding the edge points of the bitmap LEFT IMAGE
        @return A 2D vector of pair with edge points of the bitmap LEFT IMAGE
    */
    vector<vector<pair<int, int>>> getLeftEdgePoint() const;
    /*
      This will return the vector holding the edge points of the bitmap RIGHT IMAGE
      @return A 2D vector of pair with edge points of the bitmap RIGHT IMAGE
    */
    vector<vector<pair<int, int>>> getRightEdgePoint() const;
    /*
        The sobel vertical and horizontal operator will be applied on {this->img}
            The points are then stored/appended in {this->edgePoints}
        @param rowNo Which row to apply edgeDetection on
        @param patchSize How many columns to skip from start & end
        @param EdgeLinesNo Input the current EdgeLine
        @param horizontalFlag true = Apply the horizontalFilter, false = vice versa
        @param verticalFlag true = Apply the verticalFilter, false = vice versa
        @param cleanFlag true = Clean the edges, false = Don't clean the edges
    */
    void applySobelEdgeDetection(int, int, int, bool = true, bool = true, bool = true);
    /*
        This functions will make a - colored box around the given patch
        @param x1 starting x coordinate
        @param y1 starting y coordinate
        @param x2 ending x coordinate
        @param y2 ending y coordinate
        @param color 1:blue 2:green 3:red
    */
    void createBorder(int, int, int, int, int);
    /*
        This function will write the bmp image to the given {this->path + "_w.bmp"}
        @param flag set true for original image, set false for modified image
    */
    void writeBMPImage(bool = false);
    /*
        This is the mutator for {this->path}
        @param path: The path of the bmp image
    */
    void setPath(string);
    /*
        This is a garbage collector
    */
    void cleanUp();
    /*
       Match Sobel Edges from {this->leftEdges}
       @param rowNo Which row to start matching on the other image
       @param patchSize Size of the patch (Should be an odd number > 3)
       @param rowOffset How many rows to go up(negative) or down(positive)
       @param EdgeLinesNo Input the current EdgeLine
       @param img Object for the image you want to match on
   */
    void sobelTemplateMatch(int, int, int, int, BmpHandler &img);
    /*
        This function will just erase the data stored in {this->rightEdgePoints}
    */
    void eraseRightEdgePoints();
    /*
        This function will apply auto contrast on {this->img}
        Formula: Pixel = (Pixel - MinPixelValue) * (255/ (highest - lowest pixel))
        @param modify should the auto contrast be applied on true:{this->grayscale_img} || false:{this->img}
    */
    void applyAutoContrast(bool);
    /*
        This will clean {this->img} and then display all the edges from {this->leftEdgePoints}
        @param patchSize The size of the borders to be displayed
        @param grayscale should the image be in grayscale
        @param autoContrast should autoContrast be applied on the image
    */
    void writeBMPEdges(int, bool, bool);
    /*
     */
    void refineTemplateEdges(vector<pair<int, int>> &, int);
    /**/
    void drawBordersFromVec(vector<pair<int, int>> &, int);
};

#endif