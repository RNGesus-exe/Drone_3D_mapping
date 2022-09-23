/*
        FYP-1 TK 1, bmpreader.h
        Purpose: To perform operations on bmp images

        @author Khawaja Shaheryar
        @version 0.1 22/09/2022
*/

#ifndef BMPREADER_H
#define BMPREADER_H

/* Commenting your code is like cleaning your bathroom -
   you never want to do it, but it really does create a
   more pleasant experience for you and your guests.
   -Ryan Campbell
*/

#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <vector>

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
        This function will read the header of the bmp image and store the {Height x Width} in {this->img_dim}
        @param The file stream which is linked with the path of the bmp image
        @return A pair of {Height x Width} {pair.first = height && pair.second == width}
    */
    pair<uint16_t, uint16_t> getImageDimensions(fstream &);
    /*
        The bmp image is read and loaded into {this->img}
        @param The file stream which is linked with the path of the bmp image
    */
    void readBMPImage(fstream &);
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
        This function will read and load the header into {this->header} and the dimensions of image into {this->img_dim}
        This function will read the bmp image from the given {this->path + ".bmp"} and load it into {this->img}
    */
    void loadBmpImage();
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
    */
    void singleRowEdgeDetection(int, bool = false, int = 10);
    /*
        If edge points have been detected and stored in {this->edgePoints}
            We will take 8 neighbors of the edgePoints and compare the entire patch
            with the corresponding row (x-axis) of ImageB, We will use SSD(sum of squared differences)
            to find the most similar patch
        @param ImageB Pass the object of BmpReader containing the ImageB
        @param padding How much area you want to explore row-wise (y-axis)
    */
    void templateMatching(BmpHandler &, int = 10);
};

#endif