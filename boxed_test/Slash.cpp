#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;


Mat justforGrenn(Mat img)
{
	for (int i = 0; i < img.rows; i++)
	{
		uchar* p = img.ptr(i);//获取Mat某一行的首地址  

		for (int j = 0; j < img.cols; j++)
		{
			//顺序是BGR,不是RGB
			int B = *(p + j * 3);
			int G = *(p + j * 3 + 1);
			int R = *(p + j * 3 + 2);
			//float Gray = R*0.299 + G*0.587 + B*0.114;
			if ((G - R)>5 && (G - B)>5) // R > num && G>num && B>num  Gray >  num
			{
				*(p + j * 3) = 255;
				*(p + j * 3 + 1) = 255;
				*(p + j * 3 + 2) = 255;
			}
			else {
				//*(p + j * 3) = 255;
				//*(p + j * 3 + 1) = 255;
				//*(p + j * 3 + 2) = 255;
			}
		}
	}
	imwrite("JustGreen.jpg", img);
	return img;
}

Point Findcouter_Slash(Mat img, float * angle)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	Point center;
	bool inpicture = true;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));

		cvBoxPoints(rect, Corners);
		if (rect.size.width  * rect.size.height > 10000 &&
			rect.size.width > 100 &&
			rect.size.height > 100) {

			inpicture = true;

			*angle = rect.angle;
			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			center.x = (int)rect.center.x;
			center.y = (int)rect.center.y;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			//for (int n = 0; n < 4; n++) {
			//	line(img, Point((int)Corners[n].x, (int)Corners[n].y), Point((int)Corners[(n+1)%4].x, (int)Corners[(n + 1) % 4].y), Scalar(255, 0, 255), 2, 8, 0);
			//	}
		}
	}

	imwrite("cvLine.jpg", img);
	return center;
}

struct getotherLine {//每条线
	Point start;//起点
	Point end;//终点
	float  k;  // 线的长度
};

int  HoughLi_MM(Mat img, Point * linecenter)
{
	vector<Vec4i> lines;
	HoughLinesP(img, lines, 1, CV_PI / 360, 200, 100, 10);
	getotherLine L[1024];

	int numver = 0, numacr = 0;
	cout << "lines.size()" << lines.size() << endl;
	for (int i = 0; i < lines.size(); i++)
	{
		//line(img, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]),Scalar(255),2 ,8,0);
		//circle(img, p2, 3,Scalar(255,0,0),-1); //第五个参数我设为-1，表明这是个实点
		L[i].start.x = lines[i][0];
		L[i].start.y = lines[i][1];
		L[i].end.x = lines[i][2];
		L[i].end.y = lines[i][3];
		line(img, L[i].start, L[i].end, Scalar(255), 2, 8, 0);
		float k;
		//计算斜率 以及斜率不存在的情况 
		//由于是矩形不考虑畸变只分横和
		if ((L[i].start.x - L[i].end.x) != 0) //计算斜率 以及斜率不存在的情况  
		{
			L[i].k = ((float)L[i].start.y - L[i].end.y) / (L[i].start.x - L[i].end.x);
		}
		else {
			L[i].k = 1;
		}

		cout << " this is  " << i << "   point " << endl;
		cout << "start point    : " << " L[i].start.x =    " << L[i].start.x << "   L[i].start.y  =" << L[i].start.y << endl;
		cout << "  this k =   " << L[i].k << endl;

	}
	imwrite("getcvLINE.jpg", img);
	int JUSTone = -1;
	for (int i = 0; i < lines.size(); i++)
	{
		float p = L[i].k;
		int numsame = 0;

		for (int j = 0; j < lines.size(); j++)
		{
			if (i != j)
			{
				if (abs(p - L[j].k) <= 0.03)
				{
					numsame++;
				}
			}
		}

		if (numsame == 0)
		{
			JUSTone = i;
			break;
		}
	}

	if (JUSTone == -1)
	{
		return -1;
	}
	else {
		(*linecenter).x = (L[JUSTone].start.x + L[JUSTone].end.x) / 2;
		(*linecenter).y = (L[JUSTone].start.y + L[JUSTone].end.y) / 2;
	}
	return 0;
}

int main()
{
	system("color 17");
	Mat img, img2, img3, out;
	String beaforeclasspath = "E://photo_opencv/boxTest/what/what(";
	String afterclasspath = ").jpg";
	String classpath = beaforeclasspath + to_string(100) + afterclasspath;
	img = imread(classpath);
	Mat imme = img.clone();
	medianBlur(imme, imme, 7);
	imwrite("medianBlur.jpg", imme);
	justforGrenn(imme);
	cvtColor(imme, imme, CV_RGB2GRAY);
	imwrite("cvtColor.jpg", imme);
	//threshold(imme, imme, 220, 255, THRESH_BINARY);
	//imwrite("threshold.jpg", imme);
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	morphologyEx(imme, imme, MORPH_CLOSE, element);
	imwrite("CLOSE.jpg", imme);
	Canny(imme, imme, 50, 100, 3);
	imwrite("Canny.jpg", imme);
	float angle;
	Point center;
	center = Findcouter_Slash(imme, &angle);
	int flag = 0;
	Point linecenter;
	flag = HoughLi_MM(imme, &linecenter);
	if (flag == -1)
	{
		cout << "have not find the singale Line" << endl;
	}
	else {
		cout << "     linecenter.x =     " << linecenter.x << "       linecenter.y =    " << linecenter.y << endl;
		if (linecenter.x > center.x)
		{
			if (linecenter.y > center.y)
			{
				cout << " 右下角" << endl;
				angle = 180 + angle;
			}
			else {
				cout << " 右上角 " << endl;
				angle = 90 + angle;
			}
		}
		else {
			if (linecenter.y > center.y)
			{
				cout << " 左下角" << endl;
				angle = angle - 90;
			}
			else {
				cout << " 左上角 " << endl;
				angle = angle;
			}
		}
		cout << " final angle === " << angle;
		Mat dst(img.size(), img.type());
		Mat rotMat = getRotationMatrix2D(center, angle, 1);
		warpAffine(img, dst, rotMat, img.size());
		imwrite("xuangzhaung.jpg", dst);
	}
	////-------------------画圆---------------------------
	//vector<Vec3f> circles;
	////Hough圆检测  
	//HoughCircles(imme, circles, HOUGH_GRADIENT, 1, imme.rows / 8, 200, 100, 0, 0);
	////将得到的结果绘图  
	//cout << "   circles.size()  =  " << circles.size() << endl;
	//Mat sourg = img.clone();
	//for (size_t i = 0; i < circles.size(); i++)
	//{
	//	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
	//	int radius = cvRound(circles[i][2]);
	//	//检测圆中心  
	//	circle(sourg, center, 3, Scalar(0, 255, 0), -1, 8, 0);
	//	//检测圆轮廓  
	//	circle(sourg, center, radius, Scalar(120, 120, 120), 3, 8, 0);
	//}
	//imwrite("circle.jpg",sourg);
	getchar();
	return 0;
}