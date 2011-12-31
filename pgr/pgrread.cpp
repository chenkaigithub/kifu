#include "pgr_camera.h"
#include <unistd.h>
#include <iostream>

#include "precomp.hpp"


using namespace pgr_camera;
using namespace cv;



IplImage* lastimg;
int c = 0;


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
        cout << "diff: " << n << endl;
        c++;
}

int main( int argc, const char* argv[] ) {
        Camera *camera = new Camera();
        camera->setFrameCallback(getImage);
        camera->initpgr_camera();
        usleep(10000);
        camera->start();
        std::cout << "start capture" << std::endl;

        while (c<100000);
        camera->stop();
        std::cout << "stop capture" << std::endl;
}

