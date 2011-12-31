/**
 * @file compareHist_Demo.cpp
 * @brief Sample code to use the function compareHist
 * @author OpenCV team
 */

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>


using namespace std;
using namespace cv;


MatND getHistogram(Mat img) {
   Mat hsv_base;

   cvtColor( img, hsv_base, CV_BGR2HSV );

  /// Using 30 bins for hue and 32 for saturation
    int h_bins = 50; int s_bins = 60;
    int histSize[] = { h_bins, s_bins };

    // hue varies from 0 to 256, saturation from 0 to 180
    float h_ranges[] = { 0, 256 };
    float s_ranges[] = { 0, 180 };

    const float* ranges[] = { h_ranges, s_ranges };

    // Use the o-th and 1-st channels
    int channels[] = { 0, 1 };

 /// Calculate the histograms for the HSV images
    MatND hist;
    calcHist( &hsv_base, 1, channels, Mat(), hist, 2, histSize, ranges, true, false );
    normalize( hist, hist, 0, 1, NORM_MINMAX, -1, Mat() );
    return hist;
}

double compare(Mat img1, Mat img2) {

        MatND hist1, hist2;

        hist1 = getHistogram(img1);
        hist2 = getHistogram(img2);
        //int method = CV_COMP_CHISQR;
        //int method = CV_COMP_CORREL;
        int method = CV_COMP_INTERSECT;
        //int method = CV_COMP_BHATTACHARYYA;

        double hcomp= compareHist( hist1, hist2, method );
        return hcomp;
}


/**
 * @function main
 */
int main_test( int argc, char** argv )
{
   Mat first = imread( argv[1], 1 );
   Mat snd = imread( argv[2], 1 );
   double cmp = compare(first,snd);
   std::cout << "comparison score: " << cmp << std::endl;
}

