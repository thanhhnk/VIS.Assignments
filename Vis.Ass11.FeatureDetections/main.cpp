#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"
#include <iostream>
#include <string>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"

//#include "opencv2/nonfree/features2d.hpp"
// #include <opencv2/nonfree/nonfree.hpp>

#include <vector>
using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

Stitcher::Mode mode = Stitcher::PANORAMA;
vector<Mat> imgs;
bool try_use_gpu = false; //Try for now
string result_name = "result.jpg";
int parseCmdArgs(int argc, char **argv);
int stitchingTwoImages(Mat imag1, Mat imag2);

int main(int argc, char *argv[])
{
	//initModule_nonfree();//THIS LINE IS IMPORTANT 
	int retval = parseCmdArgs(argc, argv);
	if (retval)
		return -1;
	Mat pano;
	/*Using openCV stitcher*/
	Ptr<Stitcher> stitcher = Stitcher::create(mode, try_use_gpu);
	Stitcher::Status status = stitcher->stitch(imgs, pano);

	if (status != Stitcher::OK)
	{
		cout << "Can't stitch images, error code = " << int(status) << endl;
		return -1;
	}
	//NOTE! It is should start from image 3
	//The hight and with of the result image needed to be adjust following the distance results

	imshow("stitching completed imaged", pano);
	imwrite(result_name, pano);
	cout << "stitching completed successfully\n"
		 << result_name << " saved!";

	return 0;
}

int stitchingTwoImages(Mat imag1, Mat imag2)
{
	//initModule_nonfree();

	Size size(1024, 780);
	resize(imag1, imag1, size);
	resize(imag2, imag2, size);
	Mat gray_image1;
	Mat gray_image2;
	//Covert to Grayscale
	cvtColor(imag1, gray_image1, CV_RGB2GRAY);
	cvtColor(imag2, gray_image2, CV_RGB2GRAY);
	//imshow( "First Image", image2 );
	//imshow( "Second Image", image1 );
	if (!gray_image1.data || !gray_image2.data)
	{
		std::cout << " --(!) Error reading images " << std::endl;
		return -1;
	}
	//--Step 1 : Detect the keypoints using SURF Detector
	int minHessian = 400;

	Ptr<SURF> detector = SURF::create(minHessian);
	std::vector<KeyPoint> keypoints_1, keypoints_2;
	detector->detect(gray_image1, keypoints_1);
	detector->detect(gray_image2, keypoints_2);


	//--Step 2 : Calculate Descriptors (feature vectors)
	//SurfDescriptorExtractor extractor;
	Ptr<SURF> extractor = SURF::create();
	Mat descriptors_img1, descriptors_img2;
	extractor->compute(gray_image1, keypoints_1, descriptors_img1);
	extractor->compute(gray_image2, keypoints_2, descriptors_img2);


	//--Step 3 : Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;
	std::vector<DMatch> matches;
	matcher.match(descriptors_img1, descriptors_img2, matches);
	double max_dist = 0;
	double min_dist = 100;
	//--Quick calculation of min-max distances between keypoints
	for (int i = 0; i < descriptors_img1.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dist)
			min_dist = dist;
		if (dist > max_dist)
			max_dist = dist;
	}
	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);
	//--Use only "good" matches (i.e. whose distance is less than 3 X min_dist )
	std::vector<DMatch> good_matches;
	for (int i = 0; i < descriptors_img1.rows; i++)
	{
		if (matches[i].distance < 3 * min_dist)
		{
			good_matches.push_back(matches[i]);
		}
	}
	std::vector<Point2f> obj;
	std::vector<Point2f> scene;
	for (int i = 0; i < good_matches.size(); i++)
	{
		//--Get the keypoints from the good matches
		obj.push_back(keypoints_1[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints_2[good_matches[i].trainIdx].pt);
	}
	//Find the Homography Matrix
	Mat H = findHomography(obj, scene, CV_RANSAC);
	// Use the homography Matrix to warp the images
	cv::Mat result;
	warpPerspective(imag1, result, H, cv::Size(imag1.cols + imag2.cols, imag1.rows));
	cv::Mat half(result, cv::Rect(0, 0, imag2.cols, imag2.rows));
	imag2.copyTo(half);
	/* To remove the black portion after stitching, and confine in a rectangular region*/
	// vector with all non-black point positions
	std::vector<cv::Point> nonBlackList;
	nonBlackList.reserve(result.rows * result.cols);
	// add all non-black points to the vector
	// there are more efficient ways to iterate through the image
	for (int j = 0; j < result.rows; ++j)
		for (int i = 0; i < result.cols; ++i)
		{
			// if not black: add to the list
			if (result.at<cv::Vec3b>(j, i) != cv::Vec3b(0, 0, 0))
			{
				nonBlackList.push_back(cv::Point(i, j));
			}
		}
	// create bounding rect around those points
	cv::Rect bb = cv::boundingRect(nonBlackList);
	// display result and save it
	cv::imshow("Result", result(bb));
}
int parseCmdArgs(int argc, char **argv)
{
	if (argc == 1)
	{
		cout << "No images" << endl;
		return -1;
	}
	else
	{
		for (int i = 1; i < argc; ++i)
		{
			Mat img = imread(argv[i]);
			if (img.empty())
			{
				cout << "Can't read image" << argv[i] << endl;
				return -1;
			}
			else
			{
				imgs.push_back(img);
				cout << "Added imgs[" << i << "]: " << argv[i] << endl;
			}
		}
	}
	return 0;
}