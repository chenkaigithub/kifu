INCLUDE=-I/usr/local/include/opencv -I/usr/local/include  
LIBS=-L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann 

gb : goboard.cpp imgutil.o
	g++ ${INCLUDE} -o gb goboard.cpp ${LIBS} imgutil.o

imgutil.o : imgutil.h imgutil.cpp
	g++ ${INCLUDE} -c imgutil.cpp ${LIBS} 

clean :
	rm gb imgutil.o

