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


//int thresh = 50, N = 11;

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

double findMedian(vector<double> scores)
{
  double median;
  size_t size = scores.size();

  sort(scores.begin(), scores.end());

  if (size  % 2 == 0) {
      median = (scores[size / 2 - 1] + scores[size / 2]) / 2;
  } else {
      median = scores[size / 2];
  }
  return median;
}

double findStdDev(vector<double> scores, double average)
{
        vector<double> devs;
        for( size_t i = 0; i < scores.size(); i++ ) {
                double score = scores[i];
                double dev= fabs ( average - score);
                devs.push_back(dev);
        }
        return findMedian(devs);
}


double dist(Point a, Point b) {
	hypot (b.x - a.x, b.y - a.y);
}

double maxSide(vector<Point> sq) {

	vector<Point> csq = sq;
	csq.push_back(sq[0]);
	double m= 0.0;
	for (size_t i = 1; i < sq.size(); i++) {
		double d = dist(csq[i-1],csq[i]);
		m= MAX(m,d);
	}
	return m;
}

double minSide(vector<Point> sq) {

        vector<Point> csq = sq;
        csq.push_back(sq[0]);
        double m= 100.00;
        for (size_t i = 1; i < sq.size(); i++) {
                m= MIN (m, dist(csq[i-1],csq[i]));
        }
        return m;
}


vector<vector<Point> > filterSquares( vector<vector<Point> >& squares, vector<double> &areas, double average, double thresh )
{
	vector<vector<Point> > out;
        for( size_t i = 0; i < squares.size(); i++ ) {
		double area = areas[i];
		double error =  fabs( area - average );
		double sideDiff = minSide(squares[i]) / maxSide(squares[i]) ;
		if ((error < thresh) && (sideDiff >0.80)) {
			out.push_back( squares[i] );
                        cout << "area:" << area << endl;
		}
	}
	return out;
}

vector<Point> convertToPoints(const vector <Point2f> fp) {

    vector <Point> pts;
    for (int i = 0; i<fp.size(); i++) {
                Point2f p = fp[i];
                Point newp = Point(p.x,p.y);
                pts.push_back(newp) ;
    }
    return pts;
}

Mat preprocess(const Mat &image) {


    //Mat fix = createImage( Size(image.cols,image.rows), cv.IPL_DEPTH_8U, 1);
    Mat fix(image);

    cvtColor(image,fix,CV_BGR2GRAY);

    int delta = 5; //dont know what this should be
    adaptiveThreshold(fix, fix, 255.0, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,9,delta);

    // down-scale and upscale the image to filter out the noise
    Mat pyr, timg, smooth(image) ;
    pyrDown(fix, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, fix, image.size());

    medianBlur(fix, smooth,5 );
    adaptiveThreshold(smooth, smooth, 255.0, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,9,delta);

    
    cvtColor(smooth,fix,CV_GRAY2BGR);
    return fix;
}

void drawSquare(Mat &image, const vector<Point>& sq) {
     const Point* p = &sq[0];
     int n = (int)sq.size();
     polylines(image, &p, &n, 1, true, Scalar(0,255,0), 1, CV_AA);
 }


 // the function draws all the squares in the image
 void drawSquares( Mat& image, const vector<vector<Point> >& squares )
 {
         for( size_t i = 0; i < squares.size(); i++ )
         {
           drawSquare(image,squares[i]);
         }
 }


