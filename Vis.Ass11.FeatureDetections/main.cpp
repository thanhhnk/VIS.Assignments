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
int findHomographyMatrix(Mat imag1, Mat imag2, Mat& H, Mat& invert_H);
int stitchingTwoImages(Mat imag1, Mat imag2, Mat& result);

int main(int argc, char *argv[])
{
	int retval = parseCmdArgs(argc, argv);
	if (retval)
		return -1;
	Mat panoOpenCV;
	/*Using openCV stitcher*/
	Ptr<Stitcher> stitcher = Stitcher::create(mode, try_use_gpu);
	Stitcher::Status status = stitcher->stitch(imgs, panoOpenCV);
	if (status != Stitcher::OK)
	{
		cout << "Can't stitch images, error code = " << int(status) << endl;
		return -1;
	}
	// else
	// 	imshow("stitchedImageOpenCV", panoOpenCV);

	
	//NOTE! It is should start from image 3
	//The hight and with of the result image needed to be adjust following the distance results

	Mat pano;
	stitchingTwoImages(imgs[0], imgs[1], pano);
	imshow("Image.00", imgs[0]);
	imshow("Image.01", imgs[1]);
	imshow("stitching completed imaged", pano);
	// imwrite(result_name, pano);
	// cout << "stitching completed successfully\n"
	// 	 << result_name << " saved!";

	waitKey(0);
	return 0;
}

int stitchingTwoImages(Mat imag1, Mat imag2, Mat &result)
{

	// Use the homography Matrix to warp the images
	Mat H, H_Invert;
	findHomographyMatrix(imag1, imag2, H, H_Invert);

   
	Mat warpImage2;
	Mat warpImage1;
	warpPerspective(imag2, warpImage2, H, Size(imag2.cols*2, imag2.rows*2), INTER_CUBIC);
	warpPerspective(imag1, warpImage1, H, Size(imag2.cols*2, imag2.rows*2), INTER_CUBIC);
	imshow("wapImage2", warpImage2);
    imshow("warpImage1", warpImage1);

	// Mat warpImage_invert2;
	// Mat warpImage_invert1;
	// warpPerspective(imag2, warpImage_invert2, H_invert, Size(imag2.cols*2, imag2.rows*2), INTER_CUBIC);
	// warpPerspective(imag1, warpImage_invert1, H_invert, Size(imag2.cols*2, imag2.rows*2), INTER_CUBIC);
	// imshow("wapImage2_invert", warpImage_invert2);
    // imshow("warpImage1_invert", warpImage_invert1);

	Mat final(Size(imag2.cols*2 + imag1.cols, imag2.rows*2),CV_8UC3);

	//Devide final into two parts (ROI - Region of interest)
	//TODO, bug is here
	Mat roi1(final, Rect(0, 0,  imag1.cols, imag1.rows));
	Mat roi2(final, Rect(imag1.cols, 0, warpImage2.cols, warpImage2.rows));
	warpImage2.copyTo(roi2);
	//imag1.copyTo(roi1);
	imag1.copyTo(roi1);

	result = final;

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
	//assign it back to the result
	result = result(bb);	
	// display result and save it
	
	cv::imshow("Reult", result(bb));

	return 0;
}

int findHomographyMatrix(Mat imag1, Mat imag2, Mat& H, Mat& invert_H)
{
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
	std::vector<Point2f> features_image1;
	std::vector<Point2f> features_image2;
	for (int i = 0; i < good_matches.size(); i++)
	{
		//--Get the keypoints from the good matches
		features_image1.push_back(keypoints_1[good_matches[i].queryIdx].pt);
		features_image2.push_back(keypoints_2[good_matches[i].trainIdx].pt);
	}
	//Find the Homography Matrix
	//The homography matrix will use these matching points, to estimate a relative orientation transform within the two images
	H = findHomography(features_image1, features_image2, CV_LMEDS);
	 //0 - a regular method using all the points
    //CV_RANSAC - RANSAC-based robust method
    //CV_LMEDS - Least-Median robust method

	//Find invert H
	invert_H = findHomography(features_image2, features_image1, CV_LMEDS);

	return 0;
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