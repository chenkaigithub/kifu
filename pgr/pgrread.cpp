#include "pgr_camera.h"
#include <unistd.h>
#include <iostream>

#include "precomp.hpp"
#include <numeric>

#include <opencv2/video/background_segm.hpp>
#include <opencv2/highgui/highgui.hpp>

#include  <opencv/cvaux.h>

#include <opencv2/legacy/blobtrack.hpp>

using namespace pgr_camera;
using namespace cv;
using namespace std;


IplImage* lastimg;

CvBGStatModel* bg_model = NULL;

bool update_bg_model = true;
int frame =0;

CvBlobDetector* blobDetect = cvCreateBlobDetectorCC(); //or cvCreateBlobDetectorSimple();
//CvBlobDetector* blobDetect = cvCreateBlobDetectorSimple();
CvBlobSeq *newBlobList = new CvBlobSeq;
CvBlobSeq *blobList = new CvBlobSeq;
int nextBlobID=1;

void saveit(int frame,IplImage * img) {
  cout << "saving image" << std::endl;
  ostringstream out;
  out << "test" << frame << ".jpg";
  cvSaveImage(out.str().c_str() ,img);
}

IplImage *copyImage(FlyCapture2::Image *img) 
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
        return cvimg;
}



void detectBlobs( IplImage *current_frame) {
    //Then once the BG is trained use FG to detect new blob.
    if(frame > 50 ) {
        cout << " calling DetectNewBlob " <<endl;
        blobDetect->DetectNewBlob(current_frame, bg_model->foreground, newBlobList, blobList);

        int blobCount = newBlobList->GetBlobNum();
        if (blobCount >0) {
            cout << "blob count: " << blobCount << std::endl;
              for(int i=0; i<newBlobList->GetBlobNum(); ++i) {
                newBlobList->DelBlob(i);
              }
            //newBlobList->Clear();
        }
    }
}
          
void getImage(FlyCapture2::Image *img) { 
    frame++;
    IplImage* cvimg  = copyImage(img);

     //create BG model
     if (bg_model == NULL ) {
           cout <<"creating model" << endl;
                // Select parameters for Gaussian model.
                CvGaussBGStatModelParams* params = new CvGaussBGStatModelParams;                     
                params->win_size=2;  
                params->n_gauss=5;
                params->bg_threshold=0.7; 
                params->std_threshold=3.5;
                params->minArea=15;
                params->weight_init=0.05;
                params->variance_init=30; 
            bg_model = cvCreateGaussianBGModel( cvimg, params );
     }

     if (0 == ( frame % 20))  {
         int r = cvUpdateBGStatModel( cvimg, bg_model );
     }

     detectBlobs(cvimg);
     cvShowImage("FG", bg_model->foreground);
     cvShowImage("BG", bg_model->background);
     cvWaitKey(50); //we need to do this, otherwise the window doesn't get updated
}

int main( int argc, const char* argv[] ) {

        lastimg = cvCreateImage( cvSize( 640, 480),  IPL_DEPTH_8U,  1 );


        cvNamedWindow("FG");
        cvNamedWindow("BG");

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
         cvReleaseBGStatModel( &bg_model );

}

