#include "opencv2/opencv.hpp"
#include <time.h>

using namespace cv;
using namespace std;


int main(void)
{
	VideoCapture capture(0);

	if (!capture.isOpened()) {
		std::cerr << "Could not open camera" << std::endl;
		return 0;
	}

	bool isthisgoodmatch = true;
	Mat frame;
	Mat frame_gray;
	namedWindow("window");

	Mat src_speed_30 = imread("img/s_30.JPG", IMREAD_GRAYSCALE);
	if (src_speed_30.empty())
		return -1;
	resize( src_speed_30, src_speed_30, cv::Size(src_speed_30.cols*2, src_speed_30.rows*2),0,0,CV_INTER_NN);

	while (true) {
		clock_t start = clock();
		capture >> frame;

		if(frame.empty()) return 0;

		cvtColor(frame, frame_gray, CV_BGR2GRAY);

		vector<KeyPoint> keypoints1, keypoints2;
		Mat descriptors1, descriptors2;

		//OpenCV3.1.0
		Ptr<ORB> orbF = ORB::create(1000);
		orbF->detectAndCompute(src_speed_30, noArray(), keypoints1, descriptors1);
		orbF->detectAndCompute(frame_gray, noArray(), keypoints2, descriptors2);
		cout << "keypoints1.size()=" << keypoints1.size() << endl;

		//  Step 3: Matching descriptor vectors
		vector< vector< DMatch > > matches;
		BFMatcher matcher(NORM_HAMMING);

		int k = 2;
	  matcher.knnMatch(descriptors1, descriptors2, matches, k);

		cout << "matches.size()=" << matches.size() << endl;

		vector< DMatch > goodMatches;
		float nndrRatio = 0.6f;
		for (int i = 0; i < matches.size(); i++)
		{
			//		cout << "matches[i].size()=" << matches[i].size() << endl;
			if (matches.at(i).size() == 2 &&
				matches.at(i).at(0).distance <= nndrRatio * matches.at(i).at(1).distance)
			{
				goodMatches.push_back(matches[i][0]);
			}
		}
		cout << "goodMatches.size()=" << goodMatches.size() << endl;

		if (goodMatches.size() < 10)
		{
			isthisgoodmatch = false;
		}
		else
		{
			isthisgoodmatch = true;
			cout << "speed is 30"<< endl;
		}

		if (isthisgoodmatch) {

			// draw good_matches
			Mat imgMatches;
			drawMatches(src_speed_30, keypoints1, frame_gray, keypoints2,
				goodMatches, imgMatches, Scalar::all(-1), Scalar::all(-1),
				vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS); //DEFAULT

			imshow("window", imgMatches);
		}
		else
		{
			imshow("window", frame_gray);
		}

		clock_t finish = clock();
		cout << (double)(finish - start)/ CLOCKS_PER_SEC << endl;
		if (waitKey(1) >= 0) break;
	}
	// VideoCapture automatically deallocate camera object
	return 0;
}
