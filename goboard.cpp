#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <algorithm>
#include <numeric>
#include <iostream>
#include <math.h>
#include <cmath>
#include <string.h>

using namespace cv;
using namespace std;


int thresh = 50, N = 11;

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

  if (size  % 2 == 0)
  {
      median = (scores[size / 2 - 1] + scores[size / 2]) / 2;
  }
  else 
  {
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
		       circle(img, b, 20, Scalar( 255,0,0), CV_FILLED, CV_AA);
			devs.push_back(b);
		//cout << "added: " << cosine << endl;
		}
		//cout << "cosine is:" << cosine << endl;
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


vector<vector<Point> > filterSquares( vector<vector<Point> >& squares, vector<double> &areas, double average, double top_thresh, double bottom_thresh )
{
	vector<vector<Point> > out;
        for( size_t i = 0; i < squares.size(); i++ ) {
		double area = areas[i];
		double error =  area  - average;
		if ((error < top_thresh ) && (error > bottom_thresh))  {
			out.push_back( squares[i] );
		}
	}
	return out;
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
                    area > 1000 &&
                    isContourConvex(Mat(approx)) )
                {
                    double maxCosine = 0;

                    for( int j = 2; j < 5; j++ )
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }

                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence
                    if( maxCosine < 0.3 ) {
		
                        squares.push_back(approx);
			areas.push_back(area);
		
		}
                }
            }
        }
    }
}


void logSquares( const vector<vector<Point> >& squares ) 
{
	for( size_t i = 0; i < squares.size(); i++ )
	{
	cout << "square:";
		for( size_t j = 0; j < squares[i].size(); j++ ) {
			const Point* p = &squares[i][j];
			cout << *p << endl;
		}
	cout << endl;
	}
	
}

// the function draws all the squares in the image
void drawSquares( Mat& image, const vector<vector<Point> >& squares )
{
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,255,0), 3, CV_AA);
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
		cout << "ideal: " << p2 << " image: " << p1 << endl;
	}

	Mat trans = getPerspectiveTransform(idealCorners, imageCorners);
	return trans;
}


int main(int argc, char** argv)
{
 const char* filename = argc >= 2 ? argv[1] : "pic1.png";

   vector<vector<Point> > squares;
    vector<double> area;
    
        Mat image = imread(filename, 1);
        if( image.empty() )
        {
            cout << "Couldn't load " << filename << endl;
        } else {
        
		findSquares(image, squares, area);

		double medianArea = findMedian(area);
		double stddev = findStdDev(area,medianArea);

		//filter out the squares that are more than 3 std deviations bigger or 2 deviations smaller than the median
		vector<vector<Point> > goodsquares = filterSquares(squares,area,medianArea,stddev*3, stddev * -2.0);

		//now find the convex hull of the points in the remaining squares.
		vector<Point2f> hull = cHull(goodsquares);
		vector<Point2f> culled = cullSegments(hull,image);


		cout << "number of culled points in hull:" << culled.size() << endl;
		cout << "culled points in hull:" << culled << endl;

		Mat homography = getHomography( culled );

		double bsize = 19.00;
		double finc = 20/19;
		vector<Point2f> verts;
		for (double x=0.0; x<bsize; x+=finc)
			for (double y=0.0; y<bsize; y+=finc) {
				Point2f p = Point2f(x,y);
				verts.push_back( p);
				cout << "src point:" << x  << endl;
			}

		Mat src = Mat(verts);
		Mat dst;

		perspectiveTransform( src, dst, homography  );

		//cout << "number of output points:" << dst.size().height << endl;
		//cout << dst << endl;
		for( size_t i = 0; i < dst.size().height; i++ ) {
			float x = dst.at<float>(i,0);
			float y = dst.at<float>(i,1);
			Point2f p = Point2f(x,y);
			//cout << "point:" <<p << endl;	
			circle(image, Point(x,y), 20, Scalar( 255,255,0), CV_FILLED, CV_AA);
		}


		const Point2f *p = &culled[0];
		int n = (int)culled.size();
		//cout << "number of points in the convex hull:" << n << endl;
		//polylines(image, &p, &n, 1, true, Scalar(255,255,0), 3, CV_AA);
		imshow("goban", image);
		imwrite("goodsqares.jpg", image );

		int c = waitKey();
	}

    return 0;
}
