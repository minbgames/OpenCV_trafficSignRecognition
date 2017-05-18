#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main(void)
{
	VideoCapture capture(0);

	if (!capture.isOpened()) {
		std::cerr << "Could not open camera" << std::endl;
		return 0;
	}

	//setting
	Mat frame;
	Mat hsv_frame;
	Mat binary_frame;
	Mat morphological_frame;

	// red value range
	Scalar lowerb_red(160, 50, 0);
	Scalar upperb_red(179, 255, 255);

	// blue value range
	Scalar lowerb_blue(75, 50, 0);
	Scalar upperb_blue(130, 255, 255);

	//set contours
	int mode = RETR_EXTERNAL;
	int method = CHAIN_APPROX_SIMPLE;

	float ratio;
	while (true)
	{
		vector<Point> g_points;
		try {
			capture >> frame; // get a new frame from webcam
		}
		catch (Exception& e) {
			std::cerr << "Exception occurred. Ignoring frame... " << e.err
				<< std::endl;
		}
		cvtColor(frame, hsv_frame, COLOR_BGR2HSV);
		inRange(hsv_frame, lowerb_red, upperb_red, binary_frame);

		//morphological opening 작은 점들을 제거
		erode(binary_frame, morphological_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );
		dilate( morphological_frame, morphological_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );

		//morphological closing 영역의 구멍 메우기
		dilate( morphological_frame, morphological_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );
		erode(morphological_frame, morphological_frame, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)) );

		/*
		medianBlur(dst_frame,blur_frame,blur_size);
		findContours(blur_frame,contours,noArray(),mode,method);
		for (int k = 0; k < contours.size(); k++)
		{
			Scalar color(255,0,0);
			drawContours(frame, contours, k, color, 4);

			for (int j = 0; j < contours[k].size(); j++)
			{
				Point pt = contours[k][j];
				g_points.push_back(Point(pt.x,pt.y));
			}
		}
		Rect rect=boundingRect(g_points);

		ptTopLeft = rect.tl();
		ptBottomRight = rect.br();

		cout<<ptTopLeft.x<<"-"<<ptTopLeft.y<<endl;
		cout<<ptBottomRight.x<<"-"<<ptBottomRight.y<<endl<<endl;
		*/

		//라벨링
		Mat img_labels,stats, centroids;
		int numOfLables = connectedComponentsWithStats(morphological_frame, img_labels,
																							 stats, centroids, 8,CV_32S);

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


		rectangle( frame, Point(left,top), Point(left+width,top+height),
								Scalar(0,0,255),1 );


		Rect rect(Point(left,top), Point(left+width,top+height));

		Mat wanted_frame = frame(rect);
		Mat resized_frame;
		resize( wanted_frame, resized_frame, Size( 640, 480 ), 0, 0, CV_INTER_CUBIC );

		imshow("binary_frame",binary_frame);
		imshow("morphological_frame",morphological_frame);
		imshow("original_frame", frame);
		imshow("wanted_frame",wanted_frame);
		imshow("resized_frame",resized_frame);

	/*when put key, program stop*/
	if (waitKey(10) >= 0) break;
	}
	return 0;
}
