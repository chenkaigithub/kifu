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


double angle( Point pt1, Point pt2, Point pt0 );

double findMedian(vector<double> scores) ;

double findStdDev(vector<double> scores, double average) ;

double dist(Point a, Point b) ;

double maxSide(vector<Point> sq) ; 

double minSide(vector<Point> sq) ;

vector<vector<Point> > filterSquares( vector<vector<Point> >& squares, vector<double> &areas, double average, double thresh );


