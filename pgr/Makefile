
LIBS=-L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lflycapture 

INC = 

pgrread : pgrread.o pgr_camera.o compare.o
	g++ -o pgrread pgrread.o compare.o pgr_camera.o ${LIBS} 

pgrread.o : pgrread.cpp pgr_camera.cpp pgr_camera.h
	g++ -c pgrread.cpp 

pgr_camera.o : pgr_camera.cpp pgr_camera.h
	g++ -c pgr_camera.cpp 

compare.o : compare.cpp compare.h
	g++ -c compare.cpp

clean : 
	rm *.o pgrread

