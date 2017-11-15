#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

//these will be changed using trackbars
const string trackbarWindowName = "Trackbars";
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

//main functions
void displayGraphics(Mat &src_image, string name);
void on_trackbar(int, void*);
void createTrackbars();
//images
Mat src_image;
Mat returnImg;

int main(int argc, char *argv[])
{
	/// Load an image
	src_image = imread(argv[1]);

	if (argc != 2 || !src_image.data)
	{
		printf("No image data \n");
		return -1;
	}

	Mat threshold;
	Mat HSVImage;

	cvtColor(src_image, HSVImage, COLOR_BGR2HSV);

	//create slider bars for HSV filtering
	createTrackbars();

	inRange(HSVImage, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX),
			threshold);

	imshow("Src", src_image);
	imshow("Treshold_Image", threshold);

	waitKey(0);

	//no need to release memory
	return 0;
}

void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed


}
void createTrackbars()
{
	//create window for trackbars
	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", H_MIN);
	sprintf(TrackbarName, "H_MAX", H_MAX);
	sprintf(TrackbarName, "S_MIN", S_MIN);
	sprintf(TrackbarName, "S_MAX", S_MAX);
	sprintf(TrackbarName, "V_MIN", V_MIN);
	sprintf(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH),
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);

}
