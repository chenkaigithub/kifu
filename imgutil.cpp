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


