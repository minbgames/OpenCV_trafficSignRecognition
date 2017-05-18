#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main(void)
{
	// VideoCapture capture("source/test_1280.mp4");
	VideoCapture capture(0);
	if (!capture.isOpened()) {
		std::cerr << "Could not open camera" << std::endl;
		return 0;
	}

	int k = 2;
	int max_goodmatch;
	int final_goodmatch;
	int roi_ok;
	//setting
	Mat frame;
	Mat frame_gray;
  Mat resized_frame_gray;
	Mat hsv_frame;
	Mat binary_frame_red;
	Mat binary_frame_red1;
	Mat binary_frame_red2;
	Mat binary_frame_blue;
	Mat binary_frame_merge;
	Mat morphological_frame;
	Mat resized_frame;
	Mat wanted_frame;
	Mat draw_frame;

	// red value range
	Scalar lowerb_red1(0, 140, 0);
	Scalar upperb_red1(15, 255, 255);
	Scalar lowerb_red2(175, 140, 0);
	Scalar upperb_red2(179, 255, 255);
	// blue value range
	Scalar lowerb_blue(100, 100, 0);
	Scalar upperb_blue(130, 255, 255);

	//0.횡단보도 1.협로구간 2.동적장애물 3.정적장애물 4.곡선코스 5.U턴 6.자동주차
	Mat img[7];

	img[0] = imread("source/img_0.JPG", IMREAD_GRAYSCALE);
	img[1] = imread("source/img_1.JPG", IMREAD_GRAYSCALE);
	img[2] = imread("source/img_2.JPG", IMREAD_GRAYSCALE);
	img[3] = imread("source/img_3.JPG", IMREAD_GRAYSCALE);
	img[4] = imread("source/img_4.JPG", IMREAD_GRAYSCALE);
	img[5] = imread("source/img_5.JPG", IMREAD_GRAYSCALE);
	img[6] = imread("source/img_6.JPG", IMREAD_GRAYSCALE);
	int empty_test=0;

	for (int n = 0; n < 7; n++) { empty_test = empty_test || img[n].empty(); }
	if (empty_test) return -1;

	while (true)
	{
		try {
			capture >> frame; // get a new frame from webcam
			resize( frame, frame, Size( 640, 480 ), 0, 0, CV_INTER_CUBIC );
		}
		catch (Exception& e) {
			std::cerr << "Exception occurred. Ignoring frame... " << e.err
				<< std::endl;
		}

		cvtColor(frame, hsv_frame, COLOR_BGR2HSV);
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

		inRange(hsv_frame, lowerb_red1, upperb_red1, binary_frame_red1);
		inRange(hsv_frame, lowerb_red2, upperb_red2, binary_frame_red2);
		inRange(hsv_frame, lowerb_blue, upperb_blue, binary_frame_blue);

		binary_frame_red = binary_frame_red1 | binary_frame_red2;
		binary_frame_merge = binary_frame_red | binary_frame_blue;

		//morphological opening 작은 점들을 제거
		erode(binary_frame_merge, morphological_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );
		dilate( morphological_frame, morphological_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );

		//morphological closing 영역의 구멍 메우기
		dilate( morphological_frame, morphological_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );
		erode(morphological_frame, morphological_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );

		//라벨링
		Mat img_labels,stats, centroids;
		int numOfLables = connectedComponentsWithStats(morphological_frame, img_labels, stats, centroids, 8,CV_32S);

		//영역박스 그리기
		int max = -1, idx=0;
		for (int j = 1; j < numOfLables; j++) {
				int area = stats.at<int>(j, CC_STAT_AREA);
				if ( max < area )
				{
						max = area;
						idx = j;
				}
		}

		int left = stats.at<int>(idx, CC_STAT_LEFT);
		int top  = stats.at<int>(idx, CC_STAT_TOP);
		int width = stats.at<int>(idx, CC_STAT_WIDTH);
		int height  = stats.at<int>(idx, CC_STAT_HEIGHT);

		//rectangle( frame, Point(left,top), Point(left+width,top+height), Scalar(0,0,255),1 );

		max_goodmatch = 0;
		final_goodmatch = -1;
		roi_ok = -1;

		if(width>50 && height>50){
			roi_ok=1;
			Rect rect(Point(left,top), Point(left+width,top+height));

			wanted_frame = frame(rect);

			resize( wanted_frame, resized_frame, Size( 190, 190 ), 0, 0, CV_INTER_CUBIC );
			cvtColor(resized_frame, resized_frame_gray, CV_BGR2GRAY);

			/**************************matching*****************************/
			vector<KeyPoint> key_frame, key_img[7];
			Mat des_frame, des_img[7];

			Ptr<ORB> orbF = ORB::create(1000);
			orbF->detectAndCompute(resized_frame_gray, noArray(), key_frame, des_frame);
			orbF->detectAndCompute(img[0], noArray(), key_img[0], des_img[0]);
			orbF->detectAndCompute(img[1], noArray(), key_img[1], des_img[1]);
			orbF->detectAndCompute(img[2], noArray(), key_img[2], des_img[2]);
			orbF->detectAndCompute(img[3], noArray(), key_img[3], des_img[3]);
			orbF->detectAndCompute(img[4], noArray(), key_img[4], des_img[4]);
			orbF->detectAndCompute(img[5], noArray(), key_img[5], des_img[5]);
			orbF->detectAndCompute(img[6], noArray(), key_img[6], des_img[6]);

			if(des_frame.elemSize()>0){

				vector< DMatch > goodMatches[7];
				float nndrRatio = 0.6f;

				Mat indices;
				Mat dists;

				for (int index = 0; index < 7; index++) {
					try{
						flann::Index flannIndex(des_frame, flann::LshIndexParams(12, 20, 2), cvflann::FLANN_DIST_HAMMING);
						flannIndex.knnSearch(des_img[index], indices, dists, k, flann::SearchParams());
					}
					catch(Exception& e){
						continue;
					}

					for (int i = 0; i < des_img[index].rows; i++)
					{
						float d1, d2;
						d1 = (float)dists.at<int>(i, 0);
						d2 = (float)dists.at<int>(i, 1);

						if (indices.at<int>(i, 0) >= 0 && indices.at<int>(i, 1) >= 0 &&
							d1 <= nndrRatio*d2)
						{
							DMatch match(i, indices.at<int>(i, 0), d1);
							goodMatches[index].push_back(match);
						}
					}
				}

				for (int index = 0; index < 7; index++) {
					if(goodMatches[index].size() > max_goodmatch)
					{
						max_goodmatch = goodMatches[index].size();
						final_goodmatch = index;
					}
					cout << "goodmatches["<< index <<"].size()=" << goodMatches[index].size() << endl;
				}
				cout << "max_goodmatch=" << max_goodmatch << endl;

				if (max_goodmatch > 10)
				{
					// draw good_matches
					Mat imgMatches;
					drawMatches(img[final_goodmatch], key_img[final_goodmatch], resized_frame_gray, key_frame,
					goodMatches[final_goodmatch], imgMatches, Scalar::all(-1), Scalar::all(-1),
					vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS); //DEFAULT
					imshow("Good Matches", imgMatches);
				}
				else{
					final_goodmatch = -1;
				}
			}
			imshow("output_frame",resized_frame);
		}

		imshow("morphological_frame",morphological_frame);
		// final_goodmatch //roi_ok 출력

		cout << "width-height: " << width << " , "<< height << endl;
		cout << "roi: " << roi_ok << "   final: " << final_goodmatch << endl;

		if (waitKey(10) >= 0) break;
		}
		return 0;
}
