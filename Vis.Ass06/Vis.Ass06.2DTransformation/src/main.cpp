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

//Variables
Mat imageA, imageB;
float a_X, a_Y, b_X, b_Y;
vector<cv::Point2f> imageA_ptAs;
vector<cv::Point2f> imageB_ptBs;
int pt_counterA = 0, pt_counterB = 0;
Mat *A = NULL;
Mat *B = NULL;
Mat *invA = NULL;
Mat *X = NULL;

//Functions
/* Simple function for displaying the two images A and B in the namedWindows provided*/
void displayGraphics();
/*Event handler function that is passed to function setMouseCallback for
 a provided namedWindows A and B. The x and y parameters store the coordinate for clicks
 and evt parameter corresponds to event types enum such as EVENT_LBUTTONDOWN etc.*/
void onMouseA(int evt, int x, int y, int flags, void* param);
void onMouseB(int evt, int x, int y, int flags, void* param);
/*Prints the points in a vector of Points*/
void printPointsInVector(const vector<cv::Point2f> point_vec);
/*Puts the points collected in vector of Points imageA_ptAs in matrix A according to the outline provided in the slide.
 The result is put in matrix A and only the row-size of matrix A varies the number columns is always 4.*/
void populateMatrixA(vector<cv::Point2f> vecPts, int rowSize, Mat* A);
/*Puts the points collected in vector of Points imageB_ptBs in matrix B according to the outline provided in the slide.
 The result is put in matrix B and only the row-size of matrix B varies the number columns is always 4.*/
void populateMatrixB(vector<cv::Point2f> vecPts, int rowSize, Mat* B);
/*Simple function for printing the elements of the matrix provided in the parameter*/
void printPointsInMatrix(const Mat mat, string name);
/*Takes the opencv data type provided in the argument and returns it as a well understood string*/
string type2str(int type);
//Finding the transformation matrix M
Mat findTransformMatrix(vector<cv::Point2f> imageA_ptAs,
		vector<cv::Point2f> imageB_ptBs);

int main(int argc, char *argv[])
{
	//create a window
	namedWindow("ImageA", CV_WINDOW_NORMAL);
	namedWindow("ImageB", CV_WINDOW_NORMAL);

	//load the image
	if (argc > 2)
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
	if (imageA.empty() || imageB.empty())
		exit(1);

	string ty = type2str(imageA.type());
	printf("Matrix: %s %dx%d \n", ty.c_str(), imageA.cols, imageA.rows);

	cv::setMouseCallback("ImageA", onMouseA, NULL);
	cv::setMouseCallback("ImageB", onMouseB, NULL);

	//cv::imshow("Output Window", frame);

	displayGraphics();

	waitKey(0);
	delete A;
	delete B;
	delete invA;
	delete X;

	return 0;
}

void displayGraphics()
{
	//display both images
	imshow("ImageA", imageA);
	imshow("ImageB", imageB);
}

/*For the mouse clicks on image A we store the clicked coordinates in vector of Points imageA_ptAs, and the
 pt_counterA keeps track of the number of clicks on image A. We only need 4 click points after which no further points
 will be added to imageA_ptAs*/
void onMouseA(int evt, int x, int y, int flags, void* param)
{
	if (evt == EVENT_LBUTTONDOWN)
	{
		a_X = x;
		a_Y = y;
		if (pt_counterA < 4)
		{
			cout << "Image A: X = " << a_X << ", Y = " << a_Y << endl;
			imageA_ptAs.push_back(cv::Point2f(a_X, a_Y));
			//printPointsInVector(imageA_ptAs);
		}
		pt_counterA++;
	}
}

/*For the mouse clicks on image B we store the clicked coordinates in vector of Points imageB_ptBs, and the
 pt_counterB keeps track of the number of clicks on image B. We only need 4 click points after which no further points
 will be added to imageB_ptBs. Furthermore we assume at this point you have 4 points in imageA_ptAs and imageB_ptBs, clicking
 on the right click button on image B will do the rest of the processing for finding matrix X.*/
void onMouseB(int evt, int x, int y, int flags, void* param)
{
	if (evt == EVENT_LBUTTONDOWN)
	{
		b_X = x;
		b_Y = y;
		//cout << "Image B: X = " << b_X << ", Y = " << b_Y << endl;
		if (pt_counterB < 4)
		{
			cout << "Image B: X = " << b_X << ", Y = " << b_Y << endl;
			imageB_ptBs.push_back(cv::Point2f(b_X, b_Y));
		}
		pt_counterB++;
	}
	if (evt == EVENT_RBUTTONDOWN)
	{
		Mat M_X;
		if (pt_counterA >= 4 && pt_counterB >= 4)
		{
			M_X = findTransformMatrix(imageA_ptAs, imageB_ptBs);
			cout << "After get M_X" << endl;
		}

		cv::Mat TransformedImangeWithM_X;

		//Square least tramsforamtion
		cv::warpPerspective(imageA, TransformedImangeWithM_X, M_X,
				TransformedImangeWithM_X.size());
		namedWindow("warpPerspectiveWith_M_X", CV_WINDOW_NORMAL);
		imshow("warpPerspectiveWith_M_X", TransformedImangeWithM_X);

		//OpenCV transformation
		Mat M_getPers = getPerspectiveTransform(imageA_ptAs, imageB_ptBs);
		printPointsInMatrix(M_getPers, "M_getPerspectiveTransform");
		cv::warpPerspective(imageA, TransformedImangeWithM_X, M_getPers,
				TransformedImangeWithM_X.size());
		namedWindow("warpPerspectiveWith_M_getPers", CV_WINDOW_NORMAL);
		imshow("warpPerspectiveWith_M_getPers", TransformedImangeWithM_X);

	}
}

Mat findTransformMatrix(vector<cv::Point2f> imageA_ptAs,
		vector<cv::Point2f> imageB_ptBs)
{
	A = new Mat(imageA_ptAs.size() * 2, 4, CV_32F);
	populateMatrixA(imageA_ptAs, imageA_ptAs.size() * 2, A);
	B = new Mat(imageB_ptBs.size() * 2, 1, CV_32F);
	populateMatrixB(imageB_ptBs, imageB_ptBs.size() * 2, B);
	printPointsInVector(imageA_ptAs);
	printPointsInMatrix(*A, "A");
	printPointsInVector(imageB_ptBs);
	printPointsInMatrix(*B, "B");

	invA = new Mat(imageA_ptAs.size() * 2, 4, CV_32F);

	invert(*A, *invA, DECOMP_SVD);
	printPointsInMatrix(*invA, "Inverse A");
	/*At this point we have matrices A, b and inverse of A. We proceed to compute matrix X by multiplying
	 matrix inverse of A with matrix b*/
	X = new Mat(imageB_ptBs.size(), 1, CV_32F);
	gemm(*invA, *B, 1.0, NULL, 0, *X, 0);/*Matrix multiplication between inverse A and B*/
	printPointsInMatrix(*X, "X");
	/*If our points are accurate the theta we get from arccos and arcsin should be the same*/
	float theta1 = acos(X->at<float> (0, 0));
	float theta2 = asin(X->at<float> (1, 0));
	cout << "theta 1: " << theta1 * 180 / M_PI << ",\ttheta2: " << theta2 * 180
			/ M_PI << endl;

	Mat* M_fromX = new Mat(3, 3, CV_32F);
	//(0,0) is cos, (1,0) is sin
	M_fromX->at<float> (0, 0) = X->at<float> (0, 0); //costheta
	M_fromX->at<float> (0, 1) = -(X->at<float> (1, 0)); //-sinTheta
	M_fromX->at<float> (0, 2) = X->at<float> (2, 0);//tx
	M_fromX->at<float> (1, 0) = X->at<float> (1, 0); //sintheta
	M_fromX->at<float> (1, 1) = X->at<float> (0, 0); //cosTheta
	M_fromX->at<float> (1, 2) = X->at<float> (3, 0);
	M_fromX->at<float> (2, 0) = 0;
	M_fromX->at<float> (2, 1) = 0;
	M_fromX->at<float> (2, 2) = 1;

	Mat M;
	(*M_fromX).copyTo(M);
	cout << "M size" << M.size() << endl;
	printPointsInMatrix(M, "m");

	delete M_fromX;
	return M;
}
void printPointsInVector(const vector<cv::Point2f> point_vec)
{
	for (int i = 0; i < point_vec.size(); i++)
	{
		cout << "i: " << i << ": x = " << point_vec[i].x << ", y = "
				<< point_vec[i].y << endl;
	}
}

void printPointsInMatrix(const Mat mat, string name)
{
	cout << "matrix " << name << " , rows: " << mat.rows << ", cols: "
			<< mat.cols << endl;
	for (int i = 0; i < mat.rows; i++)
	{
		cout << "[\t";
		for (int j = 0; j < mat.cols; j++)
		{
			cout << +(mat.at<float> (i, j)) << "\t";
			if (j >= mat.cols - 1)
			{
				cout << " ]" << endl;
			}
		}
	}
}

void populateMatrixA(vector<cv::Point2f> vecPts, int rowSize, Mat* A)
{
	int j = 0;
	for (int i = 0; i < rowSize; i++)
	{
		if (i % 2 == 0)
		{
			A->at<float> (i, 0) = (float) vecPts[j].x;
			A->at<float> (i, 1) = (float) (-1 * vecPts[j].y);
			A->at<float> (i, 2) = 1;
			A->at<float> (i, 3) = 0;
		}
		else
		{
			A->at<float> (i, 0) = (float) vecPts[j].y;
			A->at<float> (i, 1) = (float) vecPts[j].x;
			A->at<float> (i, 2) = 0;
			A->at<float> (i, 3) = 1;
			j++;
		}
	}
}

void populateMatrixB(vector<cv::Point2f> vecPts, int rowSize, Mat* B)
{
	int j = 0;
	for (int i = 0; i < rowSize; i++)
	{
		if (i % 2 == 0)
		{
			B->at<float> (i, 0) = (float) vecPts[j].x;
		}
		else
		{
			B->at<float> (i, 0) = (float) vecPts[j].y;
			j++;
		}
	}
}

string type2str(int type)
{
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth)
	{
	case CV_8U:
		r = "8U";
		break;
	case CV_8S:
		r = "8S";
		break;
	case CV_16U:
		r = "16U";
		break;
	case CV_16S:
		r = "16S";
		break;
	case CV_32S:
		r = "32S";
		break;
	case CV_32F:
		r = "32F";
		break;
	case CV_64F:
		r = "64F";
		break;
	default:
		r = "User";
		break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}
