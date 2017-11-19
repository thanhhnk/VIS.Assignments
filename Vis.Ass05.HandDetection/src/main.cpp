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
int dp_epsilon = 10;
int dis_max = 400;
int dis_min = 7;
int angle_max = 180;
int angle_min = 20;
int rh_thresh = 0.66;
int rpd_thresh = 100;

//main functions
void on_trackbar(int, void*);
void morphOps(Mat &thresh);
void createTrackbars();
void detectHand(Mat handDetectionImage, Mat& src);
vector<Vec4i> eleminateDefectsByDimentation(vector<Vec4i> defects,
		vector<Point> contours, vector<Point>& startPoints,
		vector<Point>& endPoints, vector<Point>&depthPoints);
float distanceP2P(Point a, Point b);
float getAngle(Point s, Point f, Point e);
vector<Point> removeRedundantEndPoints(vector<Vec4i> newDefects,
		vector<Point> contours);
string intToString(int number);

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
	inRange(HSVImage, Scalar(0, 0, 0), Scalar(0, 0, 220), handTresholded);

	morphOps(handTresholded);
	detectHand(handTresholded, src_image);

	//namedWindow("src_image", CV_WINDOW_NORMAL);
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
	int largest_contour_index = -1;
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

	//Scalar color(255, 0, 255);
	// Draw the largest contour using previously stored index.
	//drawContours(src, contours, largest_contour_index, color, 2, 8, hierarchy,
	//0, Point());
	//------------------------------------------------------
	// apply convex detection, convex defects detection and defects elimination to detect the points
	//------------------------------------------------------

	// check if there exist any detected largest contour
	if (largest_contour_index != -1)
	{
		// apply Ramer–Douglas–Peucker algorithm to smooth counour points
		//approxPolyDP(Mat(contours[largest_contour_index]),
		//contours[largest_contour_index], dp_epsilon, true);

		// draw the largest smooth contour points for display purpose
		drawContours(src, contours, largest_contour_index,
				Scalar(100, 100, 100), 0);

		// contains the convex hull for display purpose
		vector<vector<Point> > hull(1);
		// calculate the cinvex hull for display purpose
		convexHull(Mat(contours[largest_contour_index]), hull[0], false);
		// draw the convex hull for display purpose
		drawContours(src, hull, 0, Scalar(0, 0, 0), 2);

		// calculate the bounding rectangle of the largest contour
		Rect boundingRectect = boundingRect(
				Mat(contours[largest_contour_index]));
		// draw the bounding rectangle for display purpose
		rectangle(src, boundingRectect, Scalar(255, 255, 0), 2);

		// contains convex hull for calculation purpose
		vector<int> chull;

		// detect the convex hull of the largest contour for calculation
		convexHull(Mat(contours[largest_contour_index]), chull, false);

		vector<Point> startPoints;
		vector<Point> endPoints;
		vector<Point> depthPoints;
		// check if the largest contour has atleast 3 points to detect defects
		if (contours[largest_contour_index].size() > 3)
			// check if the convex hull has atleast 2 points to detect defects
			if (chull.size() > 2)
			{
				// vector that contains the defects in the convex hull
				vector<Vec4i> defects;
				// calculate convexity defects for the largest contour
				convexityDefects(contours[largest_contour_index], chull,
						defects);
				// eliminate the difects by their dimention constrant
				vector<Vec4i> defects_stage = eleminateDefectsByDimentation(
						defects, contours[largest_contour_index], startPoints,
						endPoints, depthPoints);

				cout << "startPoint_size: " << startPoints.size() << endl;
				for (int i = 0; i < startPoints.size(); i++)
				{

					// draw circle at the point for display purpose
					circle(src, startPoints[i], 10, Scalar(0, 0, 255), -1);
					putText(src, "S" + intToString(i),
							Point(startPoints[i].x - 3, startPoints[i].y - 4),
							1, 1, Scalar(0, 0, 0), 2);
				}
				cout << "end_size: " << endPoints.size() << endl;
				for (int i = 0; i < endPoints.size(); i++)
				{

					// draw circle at the point for display purpose
					circle(src, endPoints[i], 10, Scalar(0, 255, 0), -1);
					putText(src, "E" + intToString(i),
							Point(endPoints[i].x - 3, endPoints[i].y - 4), 1,
							1, Scalar(0, 0, 0), 2);

				}
				cout << "depth_size: " << depthPoints.size() << endl;
				for (int i = 0; i < depthPoints.size(); i++)
				{

					// draw circle at the point for display purpose
					circle(src, depthPoints[i], 10, Scalar(255, 0, 0), -1);
					putText(src, "D" + intToString(i),
							Point(depthPoints[i].x - 3, depthPoints[i].y - 4),
							1, 1, Scalar(0, 0, 0), 2);
				}

				// calculate the final points by eliminating the points that are too close to each other
				vector<Point> finalPoints;
				finalPoints = removeRedundantEndPoints(defects_stage,
						contours[largest_contour_index]);

				// calculate moment of the largest contour to get center of gravity
				Moments momentum = moments(contours[largest_contour_index],
						true);
				// calculate center of gravity of the largest contour from moment data
				Point centerofGravity = Point(momentum.m10 / momentum.m00,
						momentum.m01 / momentum.m00);
				// draw center of gravity
				circle(src, centerofGravity, 10, Scalar(0, 0, 255), -1);

				cout << "finalPoints size: " << finalPoints.size() << endl;
				// iterate through each points in the list of final points
				/*for (int i = 0; i < finalPoints.size(); i++)
				 {
				 // draw circle at the point for display purpose
				 circle(src, finalPoints[i], 10, Scalar(255, 0, 0), -1);
				 // draw a line from the center of gravity for display purpose
				 line(src, centerofGravity, finalPoints[i],
				 Scalar(0, 0, 255), 2);
				 }*/

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

vector<Vec4i> eleminateDefectsByDimentation(vector<Vec4i> defects,
		vector<Point> contours, vector<Point>& startPoints,
		vector<Point>& endPoints, vector<Point>&depthPoints)
{
	//------------------------------------------------------
	// eliminate defects by its Dimentation
	//------------------------------------------------------
	// contains the filtered defects
	vector<Vec4i> filteredDefects;
	// iterate through each defect points
	for (int i = 0; i < defects.size(); i++)
	{
		// get the starting point of the defect
		Point ptStart(contours[defects[i][0]]);
		// get the ending point of the defect
		Point ptEnd(contours[defects[i][1]]);
		// get the depth point of the defect
		Point ptFar(contours[defects[i][2]]);
		// calculate distance from starting point to depth point
		float distance1 = distanceP2P(ptStart, ptFar);
		// calculate destance from ending point to depth point
		float distance2 = distanceP2P(ptEnd, ptFar);
		// calculate the angle between line(start,far) and line(end,far)
		float angle = getAngle(ptStart, ptFar, ptEnd);

		cout << "distance1: " << distance1 << endl;
		cout << "distance2: " << distance2 << endl;
		cout << "angle: " << angle << endl;

		// check if the distance and angle are within the predefined constrant
		/*if ((distance1 >= dis_min) && (distance1 <= dis_max) && (distance2
		 >= dis_min) && (distance2 <= dis_max) && (angle >= angle_min)
		 && (angle <= angle_max))*/
		const Vec4i& v = defects[i];
		float depth = v[3] / 256;
		if (depth > 10) //  filter defects by depth

		{
			// add the defect to the list of filtered defects
			filteredDefects.push_back(defects[i]);
			startPoints.push_back(ptStart);
			endPoints.push_back(ptEnd);
			depthPoints.push_back(ptFar);
		}
	}
	cout << "startPoint_size(in function): " << startPoints.size() << endl;
	// return the set of filtered defects
	return filteredDefects;
}

float distanceP2P(Point a, Point b)
{
	// calculate the distance between points
	float distance = sqrt(fabs(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)));
	// return the distance
	return distance;
}

float getAngle(Point s, Point f, Point e)
{
	// get angle between three points representing two lines
	float l1 = distanceP2P(f, s);
	float l2 = distanceP2P(f, e);
	float dot = (s.x - f.x) * (e.x - f.x) + (s.y - f.y) * (e.y - f.y);
	float angle = acos(dot / (l1 * l2));
	angle = angle * 180 / M_PI;
	// return the angle
	return angle;
}
vector<Point> removeRedundantEndPoints(vector<Vec4i> newDefects,
		vector<Point> contours)
{
	//------------------------------------------------------
	// remove points that are too close and keep only one of them
	//------------------------------------------------------
	// set of filtered points
	vector<Point> pointsFinal;
	// set of all points
	vector<Point> pointsAll;
	// iterate through each defect
	for (int i = 0; i < newDefects.size(); i++)
	{
		// get the starting point of the defect
		Point ptStart(contours[newDefects[i][0]]);
		// get the ending point of the defect
		Point ptEnd(contours[newDefects[i][1]]);
		// add the starting point to list of all points
		pointsAll.push_back(ptStart);
		// add the ending point to list of all points
		pointsAll.push_back(ptEnd);
	}
	// boolean flag to indicate if current point has no close points in the list of filtered points
	bool flag = false;
	// iterate through all the points
	for (int i = 0; i < pointsAll.size(); i++)
	{
		// if list of filtered points have no previous points
		if (pointsFinal.size() == 0)
		{
			// add the point to list of filtered points
			pointsFinal.push_back(pointsAll[i]);
		}
		// if the list of filtered points already some points
		else
		{
			// set flag to indicate that there is no close points
			flag = false;
			// iterate through all filtered points
			for (int j = 0; j < pointsFinal.size(); j++)
			{
				// check if the distance is within threshold
				if (distanceP2P(pointsFinal[j], pointsAll[i]) < rpd_thresh)
				{
					// mark this point as close point
					flag = true;
					break;
				}
			}
			// if the point is not close point
			if (flag == false)
			{
				// add to list of filtered points
				pointsFinal.push_back(pointsAll[i]);
			}
		}
	}
	// return list of filtered points
	return pointsFinal;
}
string intToString(int number)
{

	std::stringstream ss;
	ss << number;
	return ss.str();
}

