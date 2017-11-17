#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int H_MIN = 110;
int H_MAX = 125;
int S_MIN = 100;
int S_MAX = 255;
int V_MIN = 100;
int V_MAX = 255;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
//some boolean variables for different functionality within this

bool useMorphOps = true;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string trackbarWindowName = "Trackbars";
Mat src;
//matrix storage for HSV image
Mat HSV;
//matrix storage for binary threshold image
Mat thresh_hold;

//Functions
void on_trackbar(int, void*);
string intToString(int number);
void createTrackbars();
void drawObject(int x, int y,Mat &frame);
void morphOps(Mat &thresh);
void processThreshold(Mat &thresh);
void findBiggestContour();

void processThreshold(Mat &HSV, Mat &thresh, Scalar H_min, Scalar H_max, bool use_morphs)
{
	//filter HSV image between values and store filtered image to
	//threshold matrix
	inRange(HSV, H_min, H_max, thresh);
	//perform morphological operations on thresholded image to eliminate noise
	//and emphasize the filtered object(s)
		
	if(use_morphs)
	{
		morphOps(thresh);
	}
}

int main(int argc, char *argv[])
{
	if(argc > 1)
	{
		src = imread(argv[1]);
	}
	else
	{
		src = imread("hands.jpg");
	}
	
	//createTrackbars();
	
	cvtColor(src,HSV,COLOR_BGR2HSV);
		
	processThreshold(HSV, thresh_hold, Scalar(0,0,0), Scalar(0,0,219), true);
	
	findBiggestContour();
	
	namedWindow(windowName, CV_WINDOW_NORMAL);
	imshow(windowName,src);
	
	namedWindow(windowName2, CV_WINDOW_NORMAL);
	namedWindow(windowName1, CV_WINDOW_NORMAL);
	
	imshow(windowName2,thresh_hold);
	imshow(windowName1,HSV);
	
	waitKey(0);
	return 0;
}

void on_trackbar(int, void*)
{
	//convert frame from BGR to HSV colorspace
	cvtColor(src,HSV,COLOR_BGR2HSV);
		
	processThreshold(HSV, thresh_hold, Scalar(H_MIN,S_MIN,V_MIN), Scalar(H_MAX,S_MAX,V_MAX), true);
	
	namedWindow(windowName2, CV_WINDOW_NORMAL);
	namedWindow(windowName1, CV_WINDOW_NORMAL);
	
	imshow(windowName2,thresh_hold);
	imshow(windowName1,HSV);
}

string intToString(int number)
{
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars()
{
	//create window for trackbars
    namedWindow(trackbarWindowName,0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf( TrackbarName, "H_MIN", H_MIN);
	sprintf( TrackbarName, "H_MAX", H_MAX);
	sprintf( TrackbarName, "S_MIN", S_MIN);
	sprintf( TrackbarName, "S_MAX", S_MAX);
	sprintf( TrackbarName, "V_MIN", V_MIN);
	sprintf( TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );
}

void morphOps(Mat &thresh)
{
	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);

	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);
}

void findBiggestContour()
{
	int largest_area=0;
	int largest_contour_index=0;
	Rect bounding_rect;
 
	vector<vector<Point>> contours; // Vector for storing contour
    vector<Vec4i> hierarchy;
    
	findContours(thresh_hold, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE ); // Find the contours in the image

	for( int i = 0; i< contours.size(); i++ ) // iterate through each contour. 
	{
		double a = contourArea( contours[i],false);  //  Find the area of contour
		if(a > largest_area)
		{
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
			bounding_rect = boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}
	}
	
	Scalar color1(0,255,0);
	drawContours( src, contours, largest_contour_index, color1, 4, 8, hierarchy, 0, Point() );
	
	//Finding and drawing the convexity hull
	
	/// Find the convex hull object for each contour and convexitydefects
	vector<vector<Point>> hull(contours.size());
	vector<int> hullI;
	//vector<vector<Vec4i>> defects(contours.size());
	vector<Vec4i> defects;
	
	//cout << "largest_contour_index: " << largest_contour_index << ", hull.size: " << hull.size() 
		//<< ", contours.size: " << contours.size() << endl;  
	convexHull( Mat(contours[largest_contour_index]), hull[largest_contour_index], false );
	convexHull( Mat(contours[largest_contour_index]), hullI, false );
	      
	if(hullI.size() > 3 )
	{
		convexityDefects(contours[largest_contour_index], hullI, defects);
	}
	Scalar color2(255,0,128);
	drawContours( src, hull, largest_contour_index, color2, 4, 8, vector<Vec4i>(), 0, Point() );
	cout << "convexity defects.size: " << defects.size() << endl; 
	
	
	for(int j=0; j < defects.size(); ++j)
	{
		const Vec4i& v = defects[j];
		float depth = v[3] / 256;
		if (depth > 10) //  filter defects by depth
		{
			int startidx = v[0]; Point ptStart(contours[largest_contour_index][startidx]);
			int endidx = v[1]; Point ptEnd(contours[largest_contour_index][endidx]);
			int faridx = v[2]; Point ptFar(contours[largest_contour_index][faridx]);

			line(src, ptStart, ptEnd, Scalar(255, 255, 0), 2);
			circle(src, ptEnd, 4, Scalar(128, 0, 128), 3);
			line(src, ptStart, ptFar, Scalar(255, 255, 0), 2);
			line(src, ptEnd, ptFar, Scalar(255, 255, 0), 2);
			circle(src, ptFar, 4, Scalar(0, 255, 255), 3);
		}
	}
            
}


