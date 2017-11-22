#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

using namespace std;
using namespace cv;

Mat srcA, srcB;

int main(int argc, char *argv[])
{
	/// Load an image
	srcA = imread(argv[1]);
	srcB = imread(argv[2]);

	cout << "argv[1]" << argv[1] << endl;
	cout << "argv[2]" << argv[2] << endl;

	if (argc != 3 || !srcA.data)
	{
		printf("No image data \n");
		return -1;
	}

	
	namedWindow("srcA", CV_WINDOW_NORMAL);
	namedWindow("srcB", CV_WINDOW_NORMAL);

	imshow("srcA", srcA);
	imshow("srcB", srcB);

	waitKey(0);
	return 0;
}


