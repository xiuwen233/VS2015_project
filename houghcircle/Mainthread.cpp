#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>

using namespace cv;
using namespace std;

struct Circlecenter
{
	Point center;
	int circleR;
};

Mat FindCirCen(Mat src_img, Mat color_img)
{
	Mat in_img = src_img.clone();
	vector<vector<Point> > contours;
	findContours(in_img, contours, RETR_LIST, CHAIN_APPROX_NONE);
	struct Circlecenter  cirpocenter[50];
	int len = 0;
	int have_circle = 0;
	if (contours.size() == 0)
	{
		return color_img;
	}

	for (size_t i = 0; i < contours.size(); i++)
	{
		size_t count = contours[i].size();
		if (count < 6)
			continue;

		Mat pointsf;
		Mat(contours[i]).convertTo(pointsf, CV_32F);
		RotatedRect box = fitEllipse(pointsf);

		if (MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height) * 1.02)
			continue;
		//if (MAX(box.size.width, box.size.height) < MIN(box.size.width, box.size.height) * 1.15)
		//	continue;
		if (box.size.width < 30 || box.size.height < 30 || box.size.width >  200 || box.size.height > 200)
			continue;

		//drawContours(cimage, contours, (int)i, Scalar::all(255), 1, 8);

		//---------------------------------增加一个教研功能，如果该点是黑色的就去掉-----------------------
		//先判断点有没有越界
		if (box.center.x < 5 || box.center.x >(in_img.cols - 5))
		{
			printf("point get out of the picuter");
			continue;
		}
		if (box.center.y < 5 || box.center.y >(in_img.rows - 5))
		{
			printf("point get out of the picuter");
			continue;
		}
		

		cirpocenter[len].center.x = (int)box.center.x;
		cirpocenter[len].center.y = (int)box.center.y;
		//cirpocenter[len].circleR = (640 - cirpocenter[len].center.x) * (640 - cirpocenter[len].center.x) + (480 - cirpocenter[len].center.y) * (480 - cirpocenter[len].center.y);
		cirpocenter[len].circleR = cirpocenter[len].center.x * cirpocenter[len].center.x + cirpocenter[len].center.y * cirpocenter[len].center.y;
		//circle(color_img, cirpocenter[len].center, 2, Scalar(255, 0, 0), 2);
		have_circle++;
		if ((++len) >= 50)
		{
			printf("worring: len is big than array's len \n");
			break;
		}
		ellipse(color_img, box, Scalar(0, 255, 255), 2, LINE_AA);
		//Point2f vtx[4];
		//box.points(vtx);
		//for (int j = 0; j < 4; j++)
		//	line(color_img, vtx[j], vtx[(j + 1) % 4], Scalar(0, 255, 0), 1, LINE_AA);
	}

	int sogolen = 0;
	int minddirect = cirpocenter[0].circleR;
	for (int mlen = 0; mlen < len; mlen++)
	{
		if (cirpocenter[mlen].circleR< minddirect)
		{
			sogolen = mlen;
		}
	}

	circle(color_img, cirpocenter[sogolen].center, 2, Scalar(255, 0, 0), 2);
	//x方向相反
	String showpoint = "(" + to_string(cirpocenter[sogolen].center.x - 323 + 128) + "," + to_string(cirpocenter[sogolen].center.y - 185 + 128) + ")";
	putText(color_img, showpoint, cirpocenter[sogolen].center, FONT_HERSHEY_PLAIN, 2, Scalar(128, 128, 128), 2);
	//串口发送数据，调整
	return color_img;
}

void draw_hist(Mat img2)
{
	MatND hist;       // 在cv中用CvHistogram *hist = cvCreateHist  
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };   //这里需要为const类型  
	int size = 256;
	int channels = 0;
	//计算图像的直方图  
	calcHist(&img2, 1, &channels, Mat(), hist, dims, &size, ranges);    //cv中是cvCalcHist  
	int hpt = saturate_cast<int>(0.9 * size);
	double minVal = 0;
	double maxVal = 0;
	int scale = 1;
	Mat imageShow(size * scale, size, CV_8U, Scalar(0));

	minMaxLoc(hist, &minVal, &maxVal, 0, 0);    //cv中用的是cvGetMinMaxHistValue  
	for (int i = 0; i < 256; i++)
	{
		float value = hist.at<float>(i);           // 注意hist中是float类型    cv中用cvQueryHistValue_1D  
		int realValue = saturate_cast<int>(value * hpt / maxVal);
		rectangle(imageShow, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255));
	}
	imshow("imageShow", imageShow);
}


static void ContrastAndBright(int, void *)
{
	Mat m = Mat(1, 640, 0);
	imshow("【效果图窗口】", m);
}

int main4__5()
{
	VideoCapture capture("E://photo_opencv/video.mp4");
	if (!capture.isOpened())
		cout << "fail to open!" << endl;

	int threadvalue = 100;
	int cannyLowThre = 50;
	int cannyHighThre = 100;
	int dp_cir = 1;
	int min_dist = 10;
	int param_cir = 200;
	int param_leijia = 100;

	namedWindow("【效果图窗口】", CV_WINDOW_NORMAL);

	//创建轨迹条  
	createTrackbar("阈值：", "【效果图窗口】", &threadvalue, 255, ContrastAndBright);
	createTrackbar("LowThre：", "【效果图窗口】", &cannyLowThre, 400, ContrastAndBright);
	createTrackbar("HighThre：", "【效果图窗口】", &cannyHighThre, 400, ContrastAndBright);
	createTrackbar("dp：", "【效果图窗口】", &dp_cir, 4, ContrastAndBright);
	createTrackbar("min_dist：", "【效果图窗口】", &min_dist, 50, ContrastAndBright);
	createTrackbar("param_cir：", "【效果图窗口】", &param_cir, 400, ContrastAndBright);
	createTrackbar("param_leijia：", "【效果图窗口】", &param_leijia, 400, ContrastAndBright);

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
		
		n++;
		Mat resizeimage = frame(Rect(250,0,250,480));
		Mat  image = resizeimage.clone();
		draw_hist(resizeimage);

		Mat src_image = image.clone();
		//---------------------------------------------------------------
	/*	Mat imageRGB[3];
		split(image, imageRGB);
		for (int i = 0; i < 3; i++)
		{
			equalizeHist(imageRGB[i], imageRGB[i]);
		}
		merge(imageRGB, 3, image);
		Mat aterimg = image.clone();*/
		//------------------------------------------------------------------------
		/*Mat imageEnhance = image.clone();
		Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 5, 0, 0, -1, 0);
		filter2D(image, imageEnhance, CV_8UC3, kernel);
		Mat aterimg = imageEnhance.clone();*/
		//----------------------------------------------------------------------

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
		//归一化到0~255    
		normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
		//转换成8bit图像显示    
		convertScaleAbs(imageLog, imageLog);
		Mat aterimg = imageLog.clone();

		//---------------------------------------------------------------------

		//Mat imageGamma(image.size(), CV_32FC3);
		//for (int i = 0; i < image.rows; i++)
		//{
		//	for (int j = 0; j < image.cols; j++)
		//	{
		//		imageGamma.at<Vec3f>(i, j)[0] = (image.at<Vec3b>(i, j)[0])*(image.at<Vec3b>(i, j)[0])*(image.at<Vec3b>(i, j)[0]);
		//		imageGamma.at<Vec3f>(i, j)[1] = (image.at<Vec3b>(i, j)[1])*(image.at<Vec3b>(i, j)[1])*(image.at<Vec3b>(i, j)[1]);
		//		imageGamma.at<Vec3f>(i, j)[2] = (image.at<Vec3b>(i, j)[2])*(image.at<Vec3b>(i, j)[2])*(image.at<Vec3b>(i, j)[2]);
		//	}
		//}
		////归一化到0~255    
		//normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
		////转换成8bit图像显示    
		//convertScaleAbs(imageGamma, imageGamma);
		//Mat aterimg = imageGamma.clone();

		//-------------------------------------------------------
		cvtColor(imageLog, imageLog, CV_BGR2GRAY);
		//cvtColor(src_image, src_image, CV_BGR2GRAY);

		Mat bimage;
		//Mat src_bimage;
		threshold(imageLog, bimage, threadvalue, 255, 1);
		//threshold(src_image, src_bimage, threadvalue, 255, 1);

		Mat imme;
		Canny(bimage, imme, cannyLowThre, cannyHighThre, 3);
		imshow("Canny", imme);

		Mat out;
		Mat element = getStructuringElement(MORPH_RECT, Size(9, 9));
		morphologyEx(imme, out, MORPH_CLOSE, element);
		imshow("MORPH_CLOSE", out);

		vector<Vec3f> circles;//保存矢量  
		HoughCircles(out, circles, CV_HOUGH_GRADIENT, dp_cir, min_dist, param_cir, param_leijia, 0, 0);
		for (size_t i = 0; i < circles.size(); i++)
		{
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			//绘制圆心  
			circle(resizeimage, center, 3, Scalar(0, 255, 0), -1, 8, 0);
			//绘制圆轮廓  
			circle(resizeimage, center, radius, Scalar(155, 50, 255), 3, 8, 0);
			//Scalar(55,100,195)参数中G、B、R颜色值的数值，得到想要的颜色  
		}

	//	imshow("quanquan",FindCirCen(out, aterimg));

		//imshow("SRC_二值化", src_bimage);
		imshow("二值化",bimage);
		//imshow("读取视频", frame);
		imshow("jiequ", resizeimage);
		imshow("getname", aterimg);
		waitKey(30);
	}
	cout << "over" << endl;
	getchar();
	return 0;
}