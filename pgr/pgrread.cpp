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
int frame = 1;
const int WINDOW_SIZE= 100;
const int START_FRAMES= 200;
vector<double> weights;

double thresh =0.0;
double avg;

const int ST_INIT = 0;
const int ST_WAIT_MVMT = 1;
const int ST_WAIT_STILL = 2;

const double THRESH= 1.75;

int state = ST_INIT;

void saveit(int frame) {
  cout << "saving image" << std::endl;
  ostringstream out;
  out << "test" << frame << ".jpg";
  cvSaveImage(out.str().c_str() ,lastimg);
}


void state_machine() {

        if (avg > thresh) {
                cout << "avg: " << avg << endl;
                cout << "c: " << c << endl;
                cout << "movement!" << endl;
                if (state == ST_WAIT_MVMT) {
                        state = ST_WAIT_STILL;
                }
        } else {
                if (state == ST_WAIT_STILL) {
                        //we were waiting for stillness, and we found it, so we can advance the frame counter
                        // and take a picture now.
                        state = ST_WAIT_MVMT;
                        frame++;
			saveit(frame);
                        cout << "stillness!" << endl;
                        cout << "frame:" <<frame << endl;
			cout<<(char)7<<endl;
                }
        }

}

void stats() {
        double sum = 0.0;
        int i = 0       ;
        for ( deque<double>::iterator it = moving.begin(); it != moving.end(); it++ ) {
                sum += (*it) * weights.at(i++);
        }

        avg = sum / (double) moving.size();
        c++;
        if ((thresh == 0.0) && (c >START_FRAMES)) {
                thresh = avg * THRESH;
                state = ST_WAIT_MVMT;
		cout<<(char)7<<endl;
	        saveit(frame);
        }

        state_machine();
}



void getImage(FlyCapture2::Image *img) 
{

        IplImage* cvimg;

        unsigned int col = img->GetCols();
        unsigned int row = img->GetRows();

        unsigned int data_size = img->GetDataSize();

        cvimg = cvCreateImage( cvSize( col, row),  IPL_DEPTH_8U,  1 );

        // Copy FlyCapture2 image into OpenCV struct  
        memcpy( cvimg->imageData,  
        img->GetData(),  
        data_size ); 
        
        double n = cvNorm(cvimg,lastimg, CV_RELATIVE_L2 );

	//release old cvimg before you do this - right now you're leaking memory!!!! JCS TODO
	cvReleaseImage(&lastimg);


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

	lastimg = cvCreateImage( cvSize( 640, 480),  IPL_DEPTH_8U,  1 );



        Camera *camera = new Camera();
        camera->setFrameCallback(getImage);
        camera->initpgr_camera();
        usleep(10000);
        camera->start();
        std::cout << "start capture" << std::endl;

        while (1) {
        usleep(1000);
        }

        camera->stop();
        std::cout << "stop capture" << std::endl;
}

