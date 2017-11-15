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

//main functions
void displayGraphics();


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

	displayGraphics();

	waitKey(0);

	//no need to release memory
	return 0;
}

void displayGraphics()
{
	//display both images
	imshow("Image", src_image);
}

Mat ConvertToHSV(Mat src)
{

}
