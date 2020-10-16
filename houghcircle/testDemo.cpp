#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include "ExtendHoughCircle.h"

using namespace cv;
using namespace std;

static void ContrastAndBright(int, void *)
{
	Mat m = Mat(1, 640, 0);
	imshow("��Ч��ͼ���ڡ�", m);
}

int main()
{
	VideoCapture capture("E://photo_opencv/video.mp4");
	if (!capture.isOpened())
		cout << "fail to open!" << endl;

	int threadvalue = 100;
	int cannyLowThre = 50;
	int cannyHighThre = 100;
	int HoughCircles_dp = 1;

	int low_threshold = 183;
	int high_threshold = 332;
	int acc_threshold = 1;
	int i_minScore = 50;

	//int dp_cir = 1;
	//int min_dist = 10;
	//int param_cir = 200;
	//int param_leijia = 100;

	namedWindow("��Ч��ͼ���ڡ�", CV_WINDOW_NORMAL);

	//�����켣��  
	createTrackbar("��ֵ��", "��Ч��ͼ���ڡ�", &threadvalue, 255, ContrastAndBright);
	createTrackbar("LowThre��", "��Ч��ͼ���ڡ�", &cannyLowThre, 400, ContrastAndBright);
	createTrackbar("HighThre��", "��Ч��ͼ���ڡ�", &cannyHighThre, 400, ContrastAndBright);
	createTrackbar("dp��", "��Ч��ͼ���ڡ�", &HoughCircles_dp, 200, ContrastAndBright);
	createTrackbar("low_thre��", "��Ч��ͼ���ڡ�", &low_threshold, 500, ContrastAndBright);
	createTrackbar("high_thr��", "��Ч��ͼ���ڡ�", &high_threshold, 500, ContrastAndBright);
	createTrackbar("acc_thre��", "��Ч��ͼ���ڡ�", &acc_threshold, 50, ContrastAndBright);
	createTrackbar("i_minS��", "��Ч��ͼ���ڡ�", &i_minScore, 500, ContrastAndBright);
	//createTrackbar("dp��", "��Ч��ͼ���ڡ�", &dp_cir, 4, ContrastAndBright);
	//createTrackbar("min_dist��", "��Ч��ͼ���ڡ�", &min_dist, 50, ContrastAndBright);
	//createTrackbar("param_cir��", "��Ч��ͼ���ڡ�", &param_cir, 400, ContrastAndBright);
	//createTrackbar("param_leijia��", "��Ч��ͼ���ڡ�", &param_leijia, 400, ContrastAndBright);

	ContrastAndBright(threadvalue, 0);

	int frames = capture.get(CV_CAP_PROP_FRAME_COUNT);
	int n = 0;

	while (1)
	{
		Mat frame;
		capture >> frame;
		if (!frame.data)
		{
			destroyAllWindows();
			break;
		}

		if (n == frames - 1)
		{
			capture.set(CV_CAP_PROP_POS_FRAMES, 0);
			n = 0;
		}
		float minScore = i_minScore*0.01;
		HoughCircles_dp <= 0 ? HoughCircles_dp++ : HoughCircles_dp;
		low_threshold <= 0 ? low_threshold++ : low_threshold;
		low_threshold <= 0 ? low_threshold++ : low_threshold;
		high_threshold <= 0 ? high_threshold++ : high_threshold;
		acc_threshold <= 0 ? acc_threshold++ : acc_threshold;

		n++;
		Mat resizeimage = frame(Rect(250, 0, 250, 480));
		Mat  image = resizeimage.clone();
		//draw_hist(resizeimage);

		Mat src_image = image.clone();

		Mat out;
		GaussianBlur(image, image, Size(5, 5), 0, 0);

		Mat imageLog(image.size(), CV_32FC3);

		for (int i = 0; i < image.rows; i++)
		{
			for (int j = 0; j < image.cols; j++)
			{
				imageLog.at<Vec3f>(i, j)[0] = log(1 + image.at<Vec3b>(i, j)[0]);
				imageLog.at<Vec3f>(i, j)[1] = log(1 + image.at<Vec3b>(i, j)[1]);
				imageLog.at<Vec3f>(i, j)[2] = log(1 + image.at<Vec3b>(i, j)[2]);
			}
		}
		//��һ����0~255    
		normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
		//ת����8bitͼ����ʾ    
		convertScaleAbs(imageLog, imageLog);
		Mat aterimg = imageLog.clone();

		//-------------------------------------------------------
		cvtColor(imageLog, imageLog, CV_BGR2GRAY);
	
		//Mat bimage;
	//	threshold(imageLog, bimage, threadvalue, 255, 1);
	
		Mat imme;
		Canny(imageLog, imme, cannyLowThre, cannyHighThre, 3);
		imshow("tests", imme);
	
		vector<circle_found> found;
		//Mat immg = aterimg.clone();
		Mat color_img = aterimg.clone();
		ExtendCV::FindCircles(imme, found, HoughCircles_dp, 1, low_threshold, high_threshold, 10, 68, 30, minScore, imme);
		for (size_t i = 0; i < found.size(); i++)
		{
			Point center = cv::Point(found[i].circle[i], found[i].circle[1]);
			circle(aterimg, center, 3, Scalar(0, 255, 255), -1, 8, 0); //���ĵ�
			cv::circle(aterimg, center, found[i].circle[2], cv::Scalar(0, 0, 255), 2);
			String showpoint = "(" + to_string(center.x) + "," + to_string(center.y) + ")";
			center.x -= 20;
			putText(aterimg, showpoint, center, FONT_HERSHEY_PLAIN, 2, Scalar(0, 255, 255), 2);

			break;//��һ�����˳�������ֻ��Ҫһ��
		}
	//	cv::circle(aterimg, cv::Point(found[0].circle[0], found[0].circle[1]), found[0].circle[2], cv::Scalar(0, 0, 255), 2);
		imshow("out", aterimg);
		imshow("after", src_image);
		waitKey(30);
	}
	cout << "over" << endl;
	getchar();
	return 0;
}