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
void morphOps(Mat &thresh);
void createTrackbars();
void detectHand(Mat handDetectionImage, Mat& src);
float innerAngle(float px1, float py1, float px2, float py2, float cx1,
		float cy1);
//images
Mat src_image;
Mat returnImg;
Mat HSVImage;

int main(int argc, char *argv[])
{
	/// Load an image
	src_image = imread(argv[1]);

	if (argc != 2 || !src_image.data)
	{
		printf("No image data \n");
		return -1;
	}

	//create slider bars for HSV filtering
	createTrackbars();

	Mat handTresholded;

	cvtColor(src_image, HSVImage, COLOR_BGR2HSV);
	inRange(HSVImage, Scalar(0, 0, 0), Scalar(0, 0, 219), handTresholded);

	morphOps(handTresholded);
	detectHand(handTresholded, src_image);

	imshow("src_image", src_image);
	//imshow("hand_detection_image", handTresholded);

	waitKey(0);

	//no need to release memory
	return 0;
}
void morphOps(Mat &thresh)
{

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);

}
void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed
	Mat threshold;
	cvtColor(src_image, HSVImage, COLOR_BGR2HSV);
	inRange(HSVImage, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX),
			threshold);
	imshow("HSV_Image", HSVImage);
	imshow("Treshold_Image", threshold);

}

void detectHand(Mat handDetectionImage, Mat& src)
{
	Mat temp;
	handDetectionImage.copyTo(temp);
	//these two vectors needed for output of findContours
	vector<vector<Point> > contours;
	//Added to keep track the contour of detecting object
	vector<vector<Point> > objectContours;
	//find contours of filtered image using openCV findContours function
	vector<Vec4i> hierarchy;
	Rect bounding_rect;

	findContours(temp, contours, hierarchy, CV_RETR_CCOMP,
			CV_CHAIN_APPROX_SIMPLE);
	int largest_area = 0;
	int largest_contour_index = 0;
	for (int i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		double a = contourArea(contours[i], false); //  Find the area of contour
		if (a > largest_area)
		{
			largest_area = a;
			largest_contour_index = i; //Store the index of largest contour
		}

	}
	cout << "largest_area = " << largest_area << endl;

	Scalar color(255, 0, 255);
	// Draw the largest contour using previously stored index.
	drawContours(src, contours, largest_contour_index, color, 2, 8, hierarchy,
			0, Point());

	// Convex hull
	if (!contours.empty())
	{
		vector<vector<Point> > hull(1);
		convexHull(Mat(contours[largest_contour_index]), hull[0], false);
		drawContours(src, hull, 0, Scalar(0, 255, 0), 3);
		if (hull[0].size() > 2)
		{
			vector<int> hullIndexes;
			vector<Point> validPoints;
			Rect boundingBox = boundingRect(hull[0]);
			convexHull(Mat(contours[largest_contour_index]), hullIndexes, true);
			vector<Vec4i> convexityDefects;
			rectangle(src, boundingBox, Scalar(255, 0, 0));
			Point center = Point(boundingBox.x + boundingBox.width / 2,
					boundingBox.y + boundingBox.height / 2);
			cv::convexityDefects(cv::Mat(contours[largest_contour_index]),
					hullIndexes, convexityDefects);
			/*convexityDefects(Mat(contours[largest_contour_index]),
			 hullIndexes, convexityDefects);*/
			//TODO Debug this
			for (size_t i = 0; i < convexityDefects.size(); i++)
			{
				Point p1 =
						contours[largest_contour_index][convexityDefects[i][0]];
				Point p2 =
						contours[largest_contour_index][convexityDefects[i][1]];
				Point p3 =
						contours[largest_contour_index][convexityDefects[i][2]];
				//cv::line(src, p1, p3, cv::Scalar(0, 0, 255), 2);
				//cv::line(src, p3, p2, cv::Scalar(0, 0, 255), 2);

				double angle = atan2(center.y - p1.y, center.x - p1.x) * 180
						/ CV_PI;
				double inAngle = innerAngle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
				double length = sqrt(pow(p1.x - p3.x, 2) + (p1.y - p3.y, 2));
				if (angle > -30 && angle < 160 && abs(inAngle) > 20 && abs(
						inAngle) < 120 && length > 0.1 * boundingBox.height)
				{
					validPoints.push_back(p1);
				}
				for (size_t i = 0; i < validPoints.size(); i++)
				{
					cv::circle(src, validPoints[i], 9, cv::Scalar(0, 255, 0),
							2);
				}

			}
		}
	}

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

float innerAngle(float px1, float py1, float px2, float py2, float cx1,
		float cy1)
{

	float dist1 = std::sqrt(
			(px1 - cx1) * (px1 - cx1) + (py1 - cy1) * (py1 - cy1));
	float dist2 = std::sqrt(
			(px2 - cx1) * (px2 - cx1) + (py2 - cy1) * (py2 - cy1));

	float Ax, Ay;
	float Bx, By;
	float Cx, Cy;

	//find closest point to C
	//printf("dist = %lf %lf\n", dist1, dist2);

	Cx = cx1;
	Cy = cy1;
	if (dist1 < dist2)
	{
		Bx = px1;
		By = py1;
		Ax = px2;
		Ay = py2;

	}
	else
	{
		Bx = px2;
		By = py2;
		Ax = px1;
		Ay = py1;
	}

	float Q1 = Cx - Ax;
	float Q2 = Cy - Ay;
	float P1 = Bx - Ax;
	float P2 = By - Ay;

	float A = std::acos(
			(P1 * Q1 + P2 * Q2) / (std::sqrt(P1 * P1 + P2 * P2) * std::sqrt(
					Q1 * Q1 + Q2 * Q2)));

	A = A * 180 / CV_PI;

	return A;
}

