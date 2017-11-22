#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;

//Variables
Mat imageA, imageB;
int a_X, a_Y, b_X, b_Y; ; 
vector<cv::Point2i> imageA_ptAs(4);
vector<cv::Point2i> imageB_ptBs(4);

//Functions
void displayGraphics();
void onMouseA(int evt, int x, int y, int flags, void* param);
void onMouseB(int evt, int x, int y, int flags, void* param);

int main(int argc, char *argv[])
{
	//create a window
	namedWindow("ImageA", CV_WINDOW_NORMAL);
	namedWindow("ImageB", CV_WINDOW_NORMAL);
	
	//load the image
	if(argc > 2)
	{	
		imageA = imread(argv[1]);
		imageB = imread(argv[2]);
	}	
	else
	{
		imageA = imread("ImageA.jpg");
		imageB = imread("ImageB.jpg");
	}
	//check if image is empty
	if(imageA.empty() || imageB.empty())
		exit(1);
	
	cv::setMouseCallback("ImageA", onMouseA, NULL);
	cv::setMouseCallback("ImageB", onMouseB, NULL);
  
	displayGraphics();
	//cv::imshow("Output Window", frame);
	
	waitKey(0);
	return 0;
}


void displayGraphics()
{
	//display both images
	imshow("ImageA", imageA);
	imshow("ImageB", imageB);
}


void onMouseA(int evt, int x, int y, int flags, void* param) 
{
    if(evt == EVENT_LBUTTONDOWN) 
    {
        a_X = x;
        a_Y = y;
        cout<<"Image A: X = " << a_X << ", Y = " << a_Y<<endl;
    }
}


void onMouseB(int evt, int x, int y, int flags, void* param) 
{
    if(evt == EVENT_LBUTTONDOWN) 
    {
        b_X = x;
        b_Y = y;
        cout<<"Image B: X = " << b_X << ", Y = " << b_Y<<endl;
    }
}
