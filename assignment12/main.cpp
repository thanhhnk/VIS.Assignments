#include <opencv2/opencv.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

// Parameters for finding CCC targets
#define DPIXEL	3.0				// max distance between centroids

using namespace std;
using namespace cv;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
const string windowName = "Captured Image";


std::vector<cv::Point2d> findTargets(cv::Mat Image);
std::vector<cv::Point2d> orderTargets(std::vector<cv::Point2d> allTargets);
string intToString(int number);

int main(int argc, char *argv[])
{	
	Mat cameraFeed;

	//video capture object to acquire video file feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	//capture.open(0);
	if(argc > 1)
	{
		capture.open(argv[1]);	
	}
	else
	{
		capture.open("FontysLogo.mp4");
	}
	
	if (!capture.isOpened())
    {
        std::cout << "!!! Failed to open file: " << std::endl;
        return -1;
    }
    
    
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	
	while(capture.isOpened())
	{
		
		//store image to matrix
		if(!capture.read(cameraFeed))
			break;
		Mat temp;
		cv::cvtColor(cameraFeed, temp, CV_RGB2GRAY);
		std::vector<cv::Point2d> foundTargets = findTargets(temp);
		std::vector<cv::Point2d> orderedTargets = orderTargets(foundTargets);
		
		cout << "foundTargets.size= " << foundTargets.size() << endl;
		
		std::vector<cv::Point2d>::iterator it;
		int count = 0;
		
		for(it = orderedTargets.begin(); it != orderedTargets.end(); ++it)
		{
			cout << "x: " << (*it).x << ", y: " << (*it).y << endl;
			circle(cameraFeed, Point((*it).x, (*it).y), 4, Scalar(0, 255, 255), 3);
			putText(cameraFeed,"#"+intToString(count),
					(*it),2,1,Scalar(0,24,255),2);
			count++;
		}
		
		imshow(windowName,cameraFeed);
		//waitKey(30);
		char key = waitKey(30);
        if (key == 27) // ESC
            break;
	
	}
	capture.release();
	//waitKey(0);
	return 0;
}



std::vector<cv::Point2d> findTargets(cv::Mat Image)
{
	std::vector<cv::Point2d> targets;

	cv::Mat imageThresh;

	// Do Otsu global thresholding.
	cv::threshold(Image,
		imageThresh,		// output thresholded image
		200,					// threshold value // change from 0 to
		255,				// output value
		cv::THRESH_BINARY_INV);		// threshold_type - invert // get out of cv::THRESH_OTSU | 

	// Apply morphological operations to get rid of small (noise) regions
	cv::Mat structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3));
	cv::Mat imageOpen;
	morphologyEx(imageThresh, imageOpen, cv::MORPH_OPEN, structuringElmt);
	cv::Mat imageClose;
	morphologyEx(imageOpen, imageClose, cv::MORPH_CLOSE, structuringElmt);
	
	
	// Now find connected components
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	
	cv::findContours(
		imageClose,				// input image (is destroyed)
		contours,				// output vector of contours
		hierarchy,				// hierarchical representation
		cv::RETR_CCOMP,			// retrieve all contours 
		cv::CHAIN_APPROX_NONE);	// all pixels of each contours
	
	// Analyze components and find CCCs
	for(unsigned int i1 = 0; i1<(int)contours.size(); i1++)	{
		int i2 = hierarchy[i1][2];
		if (i2 < 0)	continue;			// See if it has a child inside

		// Compute centroids of inner and outer regions
		cv::Moments mu1 = cv::moments(contours[i1]);
		cv::Point2d x1(mu1.m10/mu1.m00, mu1.m01/mu1.m00);
		cv::Moments mu2 = cv::moments(contours[i2]);
		cv::Point2d x2(mu2.m10/mu2.m00, mu2.m01/mu2.m00);

	
		// Check if centroids coincide
		if (norm(x1-x2) > DPIXEL)	continue;

		// Check the "circularity" ratio of the outer region, which is 
		// the ratio of area to perimeter squared:  R = 4*pi*A/P^2.
		// R is 1 for a circle, and pi/4 for a square.
		double P1 = arcLength(contours[i1], true);
		double A1 = contourArea(contours[i1]);
		if (4*3.1415*A1/(P1*P1) < 3.1415/4)
			// Let's say that we want our region to be at least as round as a square.
			continue;

		// This must be a valid target; add it to the output list.
		targets.push_back(x1);
	}

	return targets;
}



// This function tries to find the 5-target pattern that looks like this
//  1  2  3
//  4     5
// It puts the targets in that order, and returns them.
std::vector<cv::Point2d> orderTargets(std::vector<cv::Point2d> allTargets)
{
	std::vector<cv::Point2d> orderedTargets;
	unsigned int i1,i2,i3,i4,i5;
	unsigned int nCCC = allTargets.size();

	// Find 3 CCCs that are in a line. ATTENTION !!! These values are not the corrected ones !!
	double dMin = 1e9;		// distance from a CCC to the midpt between points 1,3
	double d13 = 1;			// the distance between points 1,3
	for (unsigned int i = 0; i<nCCC; i++)	{
		for (unsigned int j = i + 1; j<nCCC; j++)	{
			// Get the mid point between i,j.
			cv::Point2d midPt = (allTargets[i] + allTargets[j]) * 0.5;

			// Find the CCC that is closest to this midpoint.
			for (unsigned int k = 0; k<nCCC; k++)	{
				if (k == i || k == j)	continue;
				double d = norm(allTargets[k] - midPt);	// distance from midpoint

				if (d < dMin)	{
					// This is the minimum found so far; save it.
					dMin = d;
					i1 = i;
					i2 = k;
					i3 = j;
					d13 = norm(allTargets[i] - allTargets[j]);
				}
			}
		}
	}
	// If the best distance from the midpoint is < 30% of the distance between
	// the two other points, then we probably have our colinear set.
	if (dMin/d13 > 0.3)	return orderedTargets;	// return an empty list

	// We have found 3 colinear targets:  p1 -- p2 -- p3.
	// Now find the one closest to p1; call it p4.
	dMin = 1e9; // 
	for (unsigned int i=0; i<nCCC; i++)	{
		if (i!=i1 && i!=i2 && i!=i3)	{
			double d = norm(allTargets[i]-allTargets[i1]);
			if (d < dMin)	{
				dMin = d;
				i4 = i;
			}
		}
	}
	if (dMin > 1e7)	return orderedTargets;	// return an empty list

	// Now find the one closest to p3; call it p5.
	dMin = 1e9;
	for (unsigned int i=0; i<nCCC; i++)	{
		if (i!=i1 && i!=i2 && i!=i3 && i!=i4)	{
			double d = norm(allTargets[i]-allTargets[i3]);
			if (d < dMin)	{
				dMin = d;
				i5 = i;
			}
		}
	}
	if (dMin > 1e7)	return orderedTargets;	// return an empty list

	// Now, check to see where p4 is with respect to p1,p2,p3.  If the
	// signed area of the triangle p1-p3-p4 is negative, then we have
	// the correct order; ie
	//		1   2   3
	//		4		5
	// Otherwise we need to switch the order; ie
	//		3	2	1
	//		5		4

	// Signed area is the determinant of the 2x2 matrix [ p4-p1, p3-p1 ]
	cv::Vec2d p41 = allTargets[i4] - allTargets[i1];
	cv::Vec2d p31 = allTargets[i3] - allTargets[i1];
	double m[2][2] = { {p41[0], p31[0]}, {p41[1], p31[1]} };
	double det = m[0][0]*m[1][1] - m[0][1]*m[1][0];

	// Put the targets into the output list.
	if (det < 0)	{
		orderedTargets.push_back(allTargets[i1]);
		orderedTargets.push_back(allTargets[i2]);
		orderedTargets.push_back(allTargets[i3]);
		orderedTargets.push_back(allTargets[i4]);
		orderedTargets.push_back(allTargets[i5]);
	} else	{
		orderedTargets.push_back(allTargets[i3]);
		orderedTargets.push_back(allTargets[i2]);
		orderedTargets.push_back(allTargets[i1]);
		orderedTargets.push_back(allTargets[i5]);
		orderedTargets.push_back(allTargets[i4]);
	}

	return orderedTargets;
}

string intToString(int number)
{
	std::stringstream ss;
	ss << number;
	return ss.str();
}
