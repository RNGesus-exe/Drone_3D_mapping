/*
        FYP-1, bmpreader.h
        Purpose: To perform operations on bmp images
        @author Khawaja Shaheryar
        @version 1.0 21/10/2022
*/

#ifndef TRUEVISION_H
#define TRUEVISION_H

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

struct Bmp
{
    pair<uint16_t, uint16_t> img_dim; // This will store the {Height x Width}{first x second} of the bmp image
    unsigned char ***original_img;    // This will hold the original bitmap image
    unsigned char ***img;             // This will hold the modified image, which will be displayed
    unsigned char **grayscale_img;    // This will hold the grayscale image
    string path;                      // This will store the path of the bmp image
    char header[HEADER_LEN];          // This will store the header of the bmp image
    bool isGrayScale;                 // A flag which indicates if the image is in grayscale or not
    bool hasImage;                    // This flag will indicate whether {this->original_img} is allocated

    Bmp();
    ~Bmp();
    Bmp(const Bmp &);
    /*
       Allocates memory for a 3D array
       @param: buffer Reference of the 3D buffer which has to be allocated
    */
    void allocate3DBuffer(unsigned char ***&);
    /*
        Deep Copy contents from source to destination
        @param destination The buffer in which data will be copied
        @param source The buffer from which data will be copied
    */
    void copy3DBuffer(unsigned char ***&, unsigned char ***&);
    /*
        Allocates memory for a 2D array
        @param: buffer Reference of the 2D buffer which has to be allocated
    */
    void allocate2DBuffer(unsigned char **&);
    /*
     */
    void deallocate3DBuffer(unsigned char ***&);
    /*
     */
    void deallocate2DBuffer(unsigned char **&);
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
    /**/
    void operator=(const Bmp &);
};

class StreamHandler
{

    Bmp leftBmp;
    Bmp rightBmp;

    /*
       The bmp image is read and loaded into {this->img ^ this->original_img}.
       For some reason reading directly into the 3d array doesn't work properly on windows
       so it reads into intermediate 1D Array and then converts to 3D.
       Also populates the image dimensions into {this->img_dim}
       @param Bmp An object of class Bmp
       @return bool A flag indicating whether reading was successful or not
    */
    bool readImage(Bmp &);
    /*
        Transfers 1D data into 3D [CHANNEL][ROW][COL]
        @param buf 1D pixel array
    */
    void convertTo3d(Bmp &, uint8_t *&);
    /*
        This function will write the bmp image to the given {this->path + "_w.bmp"}
        @param flag set true for original image, set false for modified image
    */
    void writeBMPImage(Bmp &, bool = false);

public:
    /*
        This is the parameterized constructor
        @param The path for the left_bmp image without the ".bmp" extension
        @param The path for the right_bmp image without the ".bmp" extension
    */
    StreamHandler(string = "", string = "");
    /*
     */
    void writeBMPImages(bool = false);
    /*
     */
    void readImages(string = "", string = "");
    /*
     */
    bool hasReadImage() const;
    /*
     */
    Bmp &getLeftBmp();
    /*
     */
    Bmp &getRightBmp();
    /**/
    StreamHandler(const StreamHandler &);
    /**/
    void operator=(const StreamHandler &);
};

class BmpHandler
{
    StreamHandler streamHandle;

public:
    /*
        There is an issue with the linux g++ compiler when using the abs() function
        so, I made my own abs function to fix that problem
        @param An integer
        @return The absolute of the integer
    */
    int abs(int);
    /*
     */
    int pow(int);
    /*
     */
    BmpHandler(string = "", string = "");
    /*
     */
    void displayImages(bool = false);
    /*
        The {this->grayscale_img} will be loaded with a grayscale image from {this->original_img}
        Formula: (B + G + R) / 3
        There is another formula available in the member function definition
        @param modify Should the grayscale be stored for future use
    */
    void applyGrayscaleOnImages(bool);
    /*
        This function will apply auto contrast on {this->img}
        Formula: Pixel = (Pixel - MinPixelValue) * (255/ (highest - lowest pixel))
        @param modify should the auto contrast be applied on true:{this->grayscale_img} || false:{this->img}
    */
    void applyAutoContrastOnImages(bool);
    /*
        This functions will make a - colored box around the given patch
        @param x1 starting x coordinate
        @param y1 starting y coordinate
        @param x2 ending x coordinate
        @param y2 ending y coordinate
        @param color 1:blue 2:green 3:red
    */
    void createBorder(Bmp &, int, int, int, int, int);
    /*
     */
    void applyCDFOnImages(bool);
    /*
        This will clean {this->img} and then display all the edges from {this->leftEdgePoints}
        @param patchSize The size of the borders to be displayed
        @param grayscale should the image be in grayscale
        @param autoContrast should autoContrast be applied on the image
    */
    void rewriteLeftBMPEdges(int, vector<vector<pair<int, int>>> &);
    /**/
    void drawBordersFromVec(Bmp &, vector<pair<int, int>> &, int);
    /**/
    StreamHandler &getStreamHandle();
    /**/
    BmpHandler(const BmpHandler &);
    /**/
    void operator=(const BmpHandler &);
};

class BitMap
{
    BmpHandler bmpHandle;
    vector<vector<pair<int, int>>> leftEdgePoints;  // This will hold the coordinates{x,y} of the edges detected in the left bitmap image
    vector<vector<pair<int, int>>> rightEdgePoints; // This will hold the coordinates{x,y} of the edges detected in the right bitmap image
    bool hasEdges;

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
    void
    applySobelEdgeDetection(int, int, int, bool = true, bool = true, bool = true);
    /*
           Match Sobel Edges from {this->leftEdges}
           @param rowNo Which row to start matching on the other image
           @param patchSize Size of the patch (Should be an odd number > 3)
           @param rowOffset How many rows to go up(negative) or down(positive)
           @param EdgeLinesNo Input the current EdgeLine
       */
    void sobelTemplateMatch(int, int, int, int);
    /*
    Helper function for template matching
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
    /**/
    bool checkHistogram(int, int, int, int, int, int, int, int);
    /**/
    bool isInRange(int val, vector<pair<int, int>> &vec, int range);

public:
    /*
     */
    int getHeight();
    /*
     */
    int getWidth();
    /*
     */
    BitMap(string = "", string = "");
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
     */
    void findEdgesInLeftImage(int, int);
    /*
     */
    void executeTemplateMatching(int, int);
    /*
     */
    void displayImages(bool = false);
};

#endif