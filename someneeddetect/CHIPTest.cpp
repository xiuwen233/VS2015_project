#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>

using namespace cv;
using namespace std;

int rect_img_x = 0;
int rect_img_y = 0;
int rect_img_w = 0;
int rect_img_h = 0;

int box_size_auto = 5;
int i_match_type = 3;
static void ContrastAndBright(int, void *)
{
	Mat m = Mat(840, 680, 0);
	imshow("【控制面板】", m);
}

//用来画一个模板进行匹配
Mat mask1;
void REctPIcTURE()
{
	Mat mask = Mat(Size(340 + 2, 305 + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, cvPoint(1, 1), cvPoint(340, 305), cvScalar(255, 255, 255),1, 1, 0);
	mask1 = mask.clone();
	imwrite("mask.jpg",mask1);
}


//一个函数用来进行模板匹配
int MatchTemp()
{

}


Point MatchCircleTemplate(Mat bitwise_out, Mat templ)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//进行匹配
	matchTemplate(bitwise_out, templ, g_resultImage, i_match_type);
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;


	//定位最匹配的位置
	minMaxLoc(g_resultImage, &minValue, &maxValue, &minLocation, &maxLocation);

	if (i_match_type == TM_SQDIFF || i_match_type == CV_TM_SQDIFF_NORMED)
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}

	int x = matchLocation.x ;
	int y = matchLocation.y ;

	Point p_re;
	p_re.x = matchLocation.x ;
	p_re.y = matchLocation.y;

	return p_re;

}


int main23()
{ 
	//rect_img_x, rect_img_y, rect_img_w, rect_img_h
	Mat  imgsrc;
	imgsrc = imread("E://photo_opencv/chipSet/t3536.jpg");
	Mat nng;
	nng = imgsrc.clone();
	//resize(imgsrc, nng,Size(imgsrc.cols/8,imgsrc.rows/8));
	//
	//rect_img_w = nng.cols;
	//rect_img_h = nng.rows;
	//namedWindow("【控制面板】", 0);
	//createTrackbar("rect_img_x：", "【控制面板】", &rect_img_x,500, ContrastAndBright);
	//createTrackbar("rect_img_y：", "【控制面板】", &rect_img_y,500, ContrastAndBright);
	//createTrackbar("rect_img_w：", "【控制面板】", &rect_img_w,500, ContrastAndBright); //  imgsrc.cols
	//createTrackbar("rect_img_h：", "【控制面板】", &rect_img_h, 500, ContrastAndBright); // imgsrc.rows
	//ContrastAndBright(rect_img_x, 0);

	//nng = nng(Rect(rect_img_x, rect_img_y, rect_img_w, rect_img_h));
	imshow("HAppy", nng);
	Mat colvor;
	Mat canny;

	cvtColor(nng, colvor, CV_BGR2GRAY);
	imshow("MYtesy",colvor);

	Canny(colvor, canny,200,200,3);
	imshow("YYTESY", canny);
	
	waitKey();
	return 0;
}


//用摄像头来看
int main456()
{
	Mat src;
	VideoCapture videocapture(1);
    namedWindow("【控制面板】", 0);
	createTrackbar("box_size_auto：", "【控制面板】", &box_size_auto, 100, ContrastAndBright);
	ContrastAndBright(rect_img_x, 0);
	REctPIcTURE();

	int box_size_autosize;
	while (1)
	{
		videocapture >> src;
		if (!src.data)
		{
			printf("error have no video\n");
			break;
		}

		imshow("SRC", src);

		Mat color;
		Mat canny;
		cvtColor(src,color, CV_BGR2GRAY);

		Mat mmf = color.clone();
		Canny(color,canny,200,200,3);
		imshow("canny",canny);
		Mat result;
		box_size_autosize = box_size_auto;
		if ( (box_size_autosize%2) == 0)
		{
			box_size_autosize++;
		}
		adaptiveThreshold(mmf, result,255,ADAPTIVE_THRESH_GAUSSIAN_C,	THRESH_BINARY, box_size_autosize,0);
	    imshow("adaptive", result);

		 Mat grad_x, grad_y;
		 Mat abs_grad_x, abs_grad_y, dst;
		 //求x方向梯度
		 Sobel(color, grad_x, CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT);
		 convertScaleAbs(grad_x, abs_grad_x);
		 imshow("x方向soble", abs_grad_x);

		 //求y方向梯度
		 Sobel(color, grad_y, CV_16S, 0, 1, 3, 1, 1, BORDER_DEFAULT);
		 convertScaleAbs(grad_y, abs_grad_y);
		 imshow("y向soble", abs_grad_y);

		 //合并梯度
		 addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);
		 imshow("整体方向soble", dst);

		if (waitKey(10) == 's')
		{
			break;
		}
	
	}
	getchar();
	return 0;
}

//只是用来显示图片的循环   漫水填充（边界要求比较明显）  霍夫直线（通过直线找到4个比较好的线 先去噪）  
// findcouter 找到内圆   模板匹配直接用圆去检测  去噪

//这个是模板匹配
int mainMatch()
{
	Mat src;
	//VideoCapture videocapture(1);
	src = imread("E://photo_opencv/chipSet/NewCamera.jpg");
	namedWindow("【控制面板】", 0);
	createTrackbar("box_size_auto：", "【控制面板】", &box_size_auto, 100, ContrastAndBright);
	createTrackbar("i_match_type：", "【控制面板】", &i_match_type, 5, ContrastAndBright);
	ContrastAndBright(rect_img_x, 0);
	REctPIcTURE();
	int box_size_autosize;
	int itimes = 1;
	while (1)
	{
		
		if (!src.data)
		{
			printf("error have no video\n");
			//break;
		}

		imshow("SRC", src);

		Mat color;
		Mat canny;
		if (itimes == 1)
		{
			cout << src.channels() << endl;
		}
		
		cvtColor(src, color, CV_BGR2GRAY);

		Mat mmf = color.clone();
		Canny(color, canny, 200, 200, 3);
		imshow("canny", canny);
		Mat result;
		box_size_autosize = box_size_auto;
		if ((box_size_autosize % 2) == 0)
		{
			box_size_autosize++;
		}
		adaptiveThreshold(mmf, result, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, box_size_autosize, 0);
		//imshow("adaptive", result);

		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y, dst;
		//求x方向梯度
		Sobel(color, grad_x, CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT);
		convertScaleAbs(grad_x, abs_grad_x);
		//imshow("x方向soble", abs_grad_x);

		//求y方向梯度
		Sobel(color, grad_y, CV_16S, 0, 1, 3, 1, 1, BORDER_DEFAULT);
		convertScaleAbs(grad_y, abs_grad_y);
		//imshow("y向soble", abs_grad_y);

		//合并梯度
		addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst);
		imshow("整体方向soble", dst);
		
		Mat rectimg;
		rectimg = dst(Rect(360,140,450,460));
		Point sssy;
		sssy = MatchCircleTemplate(rectimg, mask1);
		cout << "dfgh  " << sssy.x << " " << sssy.y << endl;
		imshow("rect", rectimg);
		
	    
	   Mat foolimg = rectimg.clone();

		Mat three_channel = Mat::zeros(foolimg.rows, foolimg.cols, CV_8UC3);
		vector<Mat> channels;
		for (int i = 0; i<3; i++)
		{
			channels.push_back(foolimg);
		}
		merge(channels, three_channel);

		rectangle(three_channel, sssy, Point(sssy.x+340, sssy.y+305), cvScalar(255, 0, 255), 1, 1, 0);
		imshow("three_channel = ", three_channel);
		/*int  area = 0;
		Rect ccomp;
		area = floodFill(three_channel, Point(201, 205), Scalar(255, 0, 255), &ccomp, Scalar(20, 20, 20), Scalar(20, 20, 20));
		imshow("Floodfill", three_channel);*/
		/*Mat rectimg;
		rectimg = dst(Rect(230,140,140,150));
		imshow("rect", rectimg);
		Point sssy;
		sssy = MatchCircleTemplate(rectimg, mask1);
		cout << "Point  == " << sssy.x << " " << sssy.y << endl;*/
	//	cout << rectimg.channels() << endl;;
		/*if (itimes == 1)
		{
			cout << rectimg.channels() << endl;
		}
		
		Mat foolimg = rectimg.clone();

		Mat three_channel = Mat::zeros(foolimg.rows, foolimg.cols, CV_8UC3);
		vector<Mat> channels;
		for (int i = 0; i<3; i++)
		{
			channels.push_back(foolimg);
		}
		merge(channels, three_channel);

		if (itimes == 1)
		{
			cout << "three_channel.channels() =  " << three_channel.channels() << endl;
		}
		
		int  area = 0;*/
		/*area = floodFill(dst, seed, newVal, &ccomp, Scalar(LowDifference, LowDifference, LowDifference),
			Scalar(UpDifference, UpDifference, UpDifference), flags);*/

		//Rect ccomp;
		//area = floodFill(three_channel, Point(70, 75), Scalar(255, 0, 255), &ccomp, Scalar(20, 20, 20), Scalar(20, 20, 20));
		//imshow("Floodfill", three_channel);
		
		itimes = 2;
		if (waitKey(10) == 's')
		{
			break;
		}

	}
	getchar();
	return 0;
}

int dp = 1;
int param1 = 100;
int param2 = 100;
int said = 4;

//用来查看圆心是否在
int  main78()
{
	Mat img;
	img = imread("E://photo_opencv/chipSet/TERD23.jpg"); // TERD.jpg   circle.jpg

	namedWindow("【控制面板】", 0);
	createTrackbar("dp：", "【控制面板】", &dp, 10, ContrastAndBright);
	createTrackbar("param1：", "【控制面板】", &param1, 500, ContrastAndBright);
	createTrackbar("param2：", "【控制面板】", &param2, 500, ContrastAndBright);
	createTrackbar("said：", "【控制面板】", &said, 10, ContrastAndBright);

	Mat src_clon = img.clone();
	while (1)
	{
		Mat gray;
		cvtColor(img, gray, COLOR_BGR2GRAY);
		vector<Vec3f> circles;
		HoughCircles(gray, circles, CV_HOUGH_GRADIENT, dp, img.rows / said, param1, param2);

		//在原图中画出圆心和圆  
		src_clon = img.clone();
		for (size_t i = 0; i < circles.size(); i++)
		{
			//提取出圆心坐标  
			Point center(round(circles[i][0]), round(circles[i][1]));
			//提取出圆半径  
			int radius = round(circles[i][2]);
			//圆心  
			circle(src_clon, center, 3, Scalar(0, 255, 0), -1, 4, 0);
			//圆  
			circle(src_clon, center, radius, Scalar(255, 0, 255), 1, 4, 0);
		}
		//namedWindow("img");
		cout << circles.size() << endl;
		imshow("src_clon", src_clon);
		if (waitKey(20) == 's')
		{
			break;
		}
	}

	getchar();
	return 0;
}

//用来检测圆的函数
int DetectCircle(Mat  img ,Mat src)
{
	vector<Vec3f> circles;
	HoughCircles(img, circles, CV_HOUGH_GRADIENT, 1, img.rows / 5, 150, 70, 0, 0);

	//在原图中画出圆心和圆  
	for (size_t i = 0; i < circles.size(); i++)
	{
		//提取出圆心坐标  
		Point center(round(circles[i][0]), round(circles[i][1]));
		//提取出圆半径  
		int radius = round(circles[i][2]);
		//圆心  
		circle(src, center, 3, Scalar(0, 255, 0), -1, 4, 0);
		//圆  
		circle(src, center, radius, Scalar(0, 0, 255), 3, 4, 0);
	}

	return 0;

}
