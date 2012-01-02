#include "pgr_camera.h"
#include <unistd.h>
#include <iostream>

#include "precomp.hpp"
#include <deque>
#include <numeric>


using namespace pgr_camera;
using namespace cv;
using namespace std;

deque<double> moving;

IplImage* lastimg;
int c = 0;
const int WINDOW_SIZE= 100;
const int START_FRAMES= 200;
vector<double> weights;

double thresh =0.0;


void stats() {
        double sum = 0.0;
        int i = 0       ;
        for ( deque<double>::iterator it = moving.begin(); it != moving.end(); it++ ) {
                sum += (*it) * weights.at(i++);
        }

        double avg = sum / (double) moving.size();
        c++;
        if ((thresh == 0.0) && (c >START_FRAMES)) {
                thresh = avg * 1.5;
        }

        cout << "avg: " << avg << endl;
        cout << "c: " << c << endl;
        if (avg > thresh) {
        cout << "movement!" << endl;
        }

}


void getImage(FlyCapture2::Image *img) 
{
        std::cout << "entered image callback" << std::endl;
        FlyCapture2::Error error;
        //error = img->Save("test.pgm" );


        IplImage* cvimg;

        unsigned int col = img->GetCols();
        unsigned int row = img->GetRows();

        unsigned int data_size = img->GetDataSize();

        cvimg = cvCreateImage( cvSize( col, row),  
             IPL_DEPTH_8U,  
             1 );

        // Copy FlyCapture2 image into OpenCV struct  
        memcpy( cvimg->imageData,  
        img->GetData(),  
        data_size ); 
        
        double n = cvNorm(cvimg,lastimg, CV_RELATIVE_L2 );
        lastimg = cvimg;
        moving.push_front(n);
        if (moving.size() > WINDOW_SIZE ) {
                moving.pop_back();
        }       
        stats();
}


vector<double> decay( int count) {

        vector<double> weights;

        for (double w=WINDOW_SIZE; w>=1; w--) {
                double d = w / WINDOW_SIZE ;
                cout << "d:" << d << endl;
                weights.push_back(d);
        }
        return weights;

}

int main( int argc, const char* argv[] ) {

        weights = decay(WINDOW_SIZE);

        Camera *camera = new Camera();
        camera->setFrameCallback(getImage);
        camera->initpgr_camera();
        usleep(10000);
        camera->start();
        std::cout << "start capture" << std::endl;

        while (1) {
        }

        camera->stop();
        std::cout << "stop capture" << std::endl;
}

