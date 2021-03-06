#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <algorithm>
#include <numeric>
#include <iostream>
#include <math.h>
#include <cmath>
#include <string.h>

#include "imgutil.h"

using namespace cv;
using namespace std;


int thresh = 50, N = 11;

vector<Point2f> cullSegments( vector<Point2f> &hullPoints, Mat &img)
{
	hullPoints.push_back( hullPoints[0]);
        vector<Point2f> devs;
	for( size_t i = 1; i < hullPoints.size(); i++ ) {
		Point a = hullPoints[i-1];
		Point b = hullPoints[i];
		Point c = hullPoints[i+1];

		double cosine = fabs(angle(b,a,c));
		if (cosine < 0.999) {
			devs.push_back(b);
		}
	}
	return devs;
}

vector<Point2f> cHull( vector<vector<Point> > &squares )
{
	vector <Point> points;

	for( size_t i = 0; i < squares.size(); i++ )
        {
                for( size_t j = 0; j < squares[i].size(); j++ ) {
                        const Point* p = &squares[i][j];
			points.push_back(*p);
                }
        }

	vector<int> hull;
        convexHull(Mat(points), hull, CV_CLOCKWISE);

	vector<Point2f>  hullPoints;

	for (int i=0; i<hull.size(); i++ ) 
	{
		Point2f p = Point2f( (float) points[hull[i]].x , (float) points[hull[i]].y  );
		hullPoints.push_back(p);
	}
	return hullPoints;
}


// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
void findSquares( const Mat& image, vector<vector<Point> >& squares, vector<double> &areas )
{
    squares.clear();
    
    Mat pyr, timg, gray0(image.size(), CV_8U), gray;
    
    // down-scale and upscale the image to filter out the noise
    pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, timg, image.size());
    vector<vector<Point> > contours;
    
    // find squares in every color plane of the image
    for( int c = 0; c < 3; c++ )
    {
        int ch[] = {c, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);
        
        // try several threshold levels
        for( int l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
            }

            // find contours and store them all as a list
            findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

            vector<Point> approx;
            
            // test each contour
            for( size_t i = 0; i < contours.size(); i++ )
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);
                
                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
		double area = fabs(contourArea(Mat(approx)));

                if( approx.size() == 4 &&
                    area > 150 &&
                    isContourConvex(Mat(approx)) )
                {
                    double maxCosine = 0;
                    double minCosine = 2;

                    for( int j = 2; j < 5; j++ )
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                        maxCosine = MAX(maxCosine, cosine);
                        minCosine = MIN(minCosine, cosine);
                    }

                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence
                    if( maxCosine < 3.0 ) {
                        squares.push_back(approx);
			areas.push_back(area);
		    }
                }
            }
        }
    }
}


Mat getHomography(vector<Point2f> corners ) {

	vector <Point2f> iCorners;
	iCorners.push_back( Point2f(0.0,0.0) );
	iCorners.push_back( Point2f(0.0,18.0) );
	iCorners.push_back( Point2f(18.0,18.0) );
	iCorners.push_back( Point2f(18.0,0.0) );

	const Point2f *imageCorners = &corners[0];
	const Point2f *idealCorners = &iCorners[0];


	for (int i=0; i<4; i++ ) {
		Point2f p1 = imageCorners[i];
		Point2f p2 = idealCorners[i];
	}

	Mat trans = getPerspectiveTransform(idealCorners, imageCorners);
	return trans;
}

vector<Point> findIntersections(Mat &image)
{
	vector<vector<Point> > squares;
	vector<double> area;
    
	findSquares(image, squares, area);

	double medianArea = findMedian(area);
	double stddev = findStdDev(area,medianArea);

	//filter out the squares that are more than 2 std deviations out
	vector<vector<Point> > goodsquares = filterSquares(squares,area,medianArea,stddev *2.0 );
        drawSquares(image,goodsquares);
       // drawSquares(image,squares);

	//now find the convex hull of the points in the remaining squares.
	vector<Point2f> hull = cHull(goodsquares);
	vector<Point2f> culled = cullSegments(hull,image);

        drawSquare(image,convertToPoints(culled));
	Mat homography = getHomography( culled );

	double bsize = 19.00;
	vector<Point2f> verts;
	for (double x=0.0; x<bsize; x++)
		for (double y=0.0; y<bsize; y++) {
			Point2f p = Point2f(x,y);
			verts.push_back( p);
		}

	Mat src = Mat(verts);
	Mat dst;

	perspectiveTransform( src, dst, homography  );

	vector<Point> newVerts;
	for( size_t i = 0; i < dst.size().height; i++ ) {
		int x = dst.at<float>(i,0);
		int y = dst.at<float>(i,1);
		Point p = Point(x,y);
		//circle(image, p, 5, Scalar( 255,0,0), CV_FILLED, CV_AA);
		newVerts.push_back(p);
	}

	return newVerts;
}


int main(int argc, char** argv)
{
	const char* filename = argc >= 2 ? argv[1] : "gb.jpg";

	Mat image = imread(filename, 1);
    Mat fixed = preprocess(image);

	if( image.empty() )
	{
	    cout << "Couldn't load " << filename << endl;
	} else {
		vector<Point> points = findIntersections(fixed);
		imshow("goban",fixed);
		waitKey();
	}
}
