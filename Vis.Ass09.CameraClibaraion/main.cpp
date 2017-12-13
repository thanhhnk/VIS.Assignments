#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <string> 

using namespace std;
using namespace cv;

void printElementsInMatrix(const Mat mat);

int main(int argc, char *argv[])
{
	FileStorage fs("out_camera_data.yml", FileStorage::READ);
	
	Mat camera_matrix, distortion_coefficients;
	
	fs["camera_matrix"] >> camera_matrix;
	fs["distortion_coefficients"] >> distortion_coefficients;
	
	printElementsInMatrix(camera_matrix);
	printElementsInMatrix(distortion_coefficients);

	//the width of object (chessboard) in inches
	double W = 10;
	//focal length F of the camera
	double F = camera_matrix.at<double>(0,0);
	//distance from the object to the camera
	double D;
	//width of the object on the image (pixel)
	double P = fs["image_width"];

	cout << endl << "image width: " << P << endl;

	D = (W * F)/P;

	cout << endl << "distance from the camera to object: " << D << " inches" << endl;	

	return 0;
}

void printElementsInMatrix(const Mat mat)
{
	cout << "matrix, rows: " << mat.rows << ", cols: " << mat.cols << endl;
	for(int i = 0; i < mat.rows; i++)
	{
		cout << "[\t";
		for(int j = 0; j < mat.cols; j++)
		{
			cout << +(mat.at<float>(i,j)) << "\t";
			if(j >= mat.cols-1)
			{
				cout << " ]" << endl;
			}
		}
	}
}
