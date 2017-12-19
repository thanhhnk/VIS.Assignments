#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

Stitcher::Mode mode = Stitcher::PANORAMA;
vector<Mat> imgs;
bool try_use_gpu = false; //Try for now
string result_name = "result.jpg";
int parseCmdArgs(int argc, char **argv);

int main(int argc, char *argv[])
{
	int retval = parseCmdArgs(argc, argv);
	if (retval)
		return -1;
	Mat pano;
	Ptr<Stitcher> stitcher = Stitcher::create(mode, try_use_gpu);
	Stitcher::Status status = stitcher->stitch(imgs, pano);

	if (status != Stitcher::OK)
	{
		cout << "Can't stitch images, error code = " << int(status) << endl;
		return -1;
	}

	imshow("stitching completed imaged", pano);
	imwrite(result_name, pano);
	cout << "stitching completed successfully\n"
		 << result_name << " saved!";

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