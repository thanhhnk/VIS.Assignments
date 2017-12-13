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
	if(argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " image" << std::endl;
		return -1;
	}	
	
	
	FileStorage fs(argv[2], FileStorage::READ);
	
	Mat camera_matrix, distortion_coefficients;
	
	fs["camera_matrix"] >> camera_matrix;
	fs["distortion_coefficients"] >> distortion_coefficients;
	
	printElementsInMatrix(camera_matrix);
	printElementsInMatrix(distortion_coefficients);
	
	string filename(argv[1]);
	
	Mat image = imread(filename);
	Mat undistortedImage;
	cout << "filename: " << filename << std::endl;
	undistort(image, undistortedImage, camera_matrix, distortion_coefficients);
	
	imwrite("undistorted_" + filename, undistortedImage);
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
