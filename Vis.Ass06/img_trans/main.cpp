#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <string> 

using namespace std;
using namespace cv;

//Variables
Mat imageA, imageB;
int a_X, a_Y, b_X, b_Y; ; 
vector<cv::Point2i> imageA_ptAs;
vector<cv::Point2i> imageB_ptBs;
int pt_counterA = 0, pt_counterB = 0;
Mat *A = NULL;
Mat *B = NULL;


//Functions
void displayGraphics();
void onMouseA(int evt, int x, int y, int flags, void* param);
void onMouseB(int evt, int x, int y, int flags, void* param);
void printPointsInVector(const vector<cv::Point2i> point_vec);
void populateMatrixA(vector<cv::Point2i> vecPts, int rowSize, Mat* A);
void populateMatrixB(vector<cv::Point2i> vecPts, int rowSize, Mat* B);
void printPointsInMatrix(const Mat mat);
string type2str(int type);

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
	
	string ty =  type2str( imageA.type() );
	printf("Matrix: %s %dx%d \n", ty.c_str(), imageA.cols, imageA.rows );
	
	cv::setMouseCallback("ImageA", onMouseA, NULL);
	cv::setMouseCallback("ImageB", onMouseB, NULL);
  
	//cv::imshow("Output Window", frame);
	
	
	displayGraphics();
	
	waitKey(0);
	delete A;
	delete B;
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
        if(pt_counterA < 4)
        {
			imageA_ptAs.push_back(cv::Point2i(x,y));
			printPointsInVector(imageA_ptAs);
		}
        pt_counterA++;
    }
}


void onMouseB(int evt, int x, int y, int flags, void* param) 
{
    if(evt == EVENT_LBUTTONDOWN) 
    {
        b_X = x;
        b_Y = y;
        cout<<"Image B: X = " << b_X << ", Y = " << b_Y<<endl;
        if(pt_counterB < 4)
        {
			imageB_ptBs.push_back(cv::Point2i(x,y));
		}
        pt_counterB++;
    }
    if(evt == EVENT_RBUTTONDOWN)
    {
		if(pt_counterA >= 4 && pt_counterB >= 4)
		{
			A = new Mat(imageA_ptAs.size()*2, 4, CV_32S);
			populateMatrixA(imageA_ptAs, imageA_ptAs.size()*2, A);
			B = new Mat(imageB_ptBs.size()*2,1, CV_32S); 
			populateMatrixB(imageB_ptBs, imageB_ptBs.size()*2, B);
			printPointsInVector(imageA_ptAs);
			printPointsInMatrix(*A);
			printPointsInVector(imageB_ptBs);
			printPointsInMatrix(*B);
		}
	}
}

void printPointsInVector(const vector<cv::Point2i> point_vec)
{
	for(int i = 0; i < point_vec.size(); i++)
	{
		cout << "i: " << i << ": x = " << point_vec[i].x << ", y = " << point_vec[i].y << endl;
	}
}

void printPointsInMatrix(const Mat mat)
{
	cout << "matrix, rows: " << mat.rows << ", cols: " << mat.cols << endl;
	for(int i = 0; i < mat.rows; i++)
	{
		cout << "[\t";
		for(int j = 0; j < mat.cols; j++)
		{
			cout << +(mat.at<int>(i,j)) << "\t";
			if(j >= mat.cols-1)
			{
				cout << " ]" << endl;
			}
		}
	}
}



void populateMatrixA(vector<cv::Point2i> vecPts, int rowSize, Mat* A)
{
	int j = 0;
	for(int i = 0; i < rowSize; i++)
	{
		if(i%2 == 0)
		{
			A->at<int>(i, 0) = (int)vecPts[j].x;
			A->at<int>(i, 1) = (int)(-1*vecPts[j].y);
			A->at<int>(i, 2) = 1;
			A->at<int>(i, 3) = 0;
		}
		else
		{
			A->at<int>(i, 0) = (int)vecPts[j].y;
			A->at<int>(i, 1) = (int)vecPts[j].x;
			A->at<int>(i, 2) = 0;
			A->at<int>(i, 3) = 1;
			j++;
		}
	}
}

void populateMatrixB(vector<cv::Point2i> vecPts, int rowSize, Mat* B)
{
	int j = 0;
	for(int i = 0; i < rowSize; i++)
	{
		if(i%2 == 0)
		{
			B->at<int>(i, 0) = (int)vecPts[j].x;
		}
		else
		{
			B->at<int>(i, 0) = (int)vecPts[j].y;
			j++;
		}
	}
}

string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}
