gb : goboard.cpp
	g++ `pkg-config --cflags --libs opencv` -o gb goboard.cpp
