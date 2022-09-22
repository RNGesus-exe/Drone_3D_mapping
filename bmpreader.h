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

#define HEADER_LEN 54    // The size of the header is fixed for the bmp files
#define COLOR_CHANNELS 3 // BGR

using namespace std;

class BmpHandler
{
    pair<uint16_t, uint16_t> img_dim;                                 // This will store the {Height x Width}{first x second} of the bmp image
    unsigned char ***img;                                             // This will hold the BRG image AKA original image
    string path;                                                      // This will store the path of the bmp image
    char header[HEADER_LEN];                                          // This will store the header of the bmp image
    int contrast[COLOR_CHANNELS][2] = {{0, 255}, {0, 255}, {0, 255}}; // Holds lowest and highest value of 3 channels
    bool isGrayScale;                                                 // A flag which indicates if the image is in grayscale or not

    //==========================================> HELPER FUNCTIONS <================================================

    /*
        Allocates memory for {this->img} data member
        Precondition: Make sure you have set the path for the bmp image
        Postcondition: This will load the bmp image from the given path {this->path}
    */
    void allocateOriginalImage();
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
    */
    void writeBMPImage();
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
    */
    void singleRowEdgeDetection(int);
};

#endif