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

Mat  Findcouter_three(Mat img, Mat src, float * angle, Point * Center)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	bool inpicture = true;
	cout << "we have so many couters  =  " << contours.size() << endl;
	vector<Rect> boundRect(contours.size());
	Mat imme = img.clone();
	int get = 0;
	int numwid = 0;
	int numhei = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);
		boundRect[i] = boundingRect(Mat(contours[i]));
		if (rect.size.width > 100 && rect.size.height >100)
		{
			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			*angle = rect.angle;
			Point gett;
			gett = rect.center;
			(*Center).x = gett.x;
			(*Center).y = gett.y;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			for (int n = 0; n < 4; n++) {
				line(img, Point((int)Corners[n].x, (int)Corners[n].y), Point((int)Corners[(n + 1) % 4].x, (int)Corners[(n + 1) % 4].y), Scalar(255, 0, 255), 2, 8, 0);
			}
			cout << "boundRect[i].x =    " << boundRect[i].x << "        boundRect[i].x =    " << boundRect[i].y << endl;
			numwid = (boundRect[i].width * 0.2) / 2;
			numhei = (boundRect[i].height * 0.2) / 2;
			get = i;
			rectangle(imme, Point(boundRect[i].x + numwid, boundRect[i].y + numhei),
				Point(boundRect[i].x + boundRect[i].width*0.8 + numwid, boundRect[i].y + boundRect[i].height*0.8 + numhei), Scalar(255), 2, 8);
		}
	}

	Rect rect1(boundRect[get].x + numwid, boundRect[get].y + numhei, boundRect[get].width*0.9, boundRect[get].height*0.9);
	Mat dest = src(rect1);
	//imwrite("cvLine.jpg", img);
	imwrite("cvLine_2.jpg", imme);
	return dest;
}

int  Findcouter_TWO_inside(Mat img)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	bool inpicture = true;
	cout << "we have so many couters  =  " << contours.size() << endl;
	Point getTWO[2];
	int numpoint = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);

		if (rect.size.width > 100 && rect.size.height >100)
		{
			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			if (numpoint >= 2) {
				cout << " error numpoint is too big " << endl;
			}
			else {
				getTWO[numpoint].x = rect.center.x;
				getTWO[numpoint].y = rect.center.y;
				numpoint++;
			}
			for (int n = 0; n < 4; n++) {
				line(img, Point((int)Corners[n].x, (int)Corners[n].y), Point((int)Corners[(n + 1) % 4].x, (int)Corners[(n + 1) % 4].y), Scalar(255, 0, 255), 2, 8, 0);
			}
		}
	}

	imwrite("cvLine.jpg", img);
	float k;
	//计算斜率 以及斜率不存在的情况 
	//由于是矩形不考虑畸变只分横和
	if ((getTWO[1].x - getTWO[0].x) != 0) //计算斜率 以及斜率不存在的情况  
	{
		k = ((float)getTWO[1].y - getTWO[0].y) / (getTWO[1].x - getTWO[0].x);
		if (k >= 0)
		{
			return  1;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}

}

int main()
{
	system("color 17");
	Mat img, img2, img3, out;
	String beaforeclasspath = "E://photo_opencv/boxTest/what/what(";
	String afterclasspath = ").jpg";
	String classpath = beaforeclasspath + to_string(98) + afterclasspath;
	img = imread(classpath);
	Mat imme = img.clone();
	//medianBlur(imme, imme, 7);
	//imwrite("medianBlur.jpg",imme);
	justforGrenn(imme);
	cvtColor(imme, imme, CV_RGB2GRAY);
	imwrite("cvtColor.jpg", imme);
	Mat element = getStructuringElement(MORPH_RECT, Size(7, 7));
	morphologyEx(imme, imme, MORPH_OPEN, element);
	imwrite("OPEN.jpg", imme);
	Canny(imme, imme, 50, 100, 3);
	imwrite("Canny.jpg", imme);
	Mat cannyimg = imme.clone();
	Mat dest;
	float angle = 0.0;
	Point center;
	dest = Findcouter_three(imme, cannyimg, &angle, &center);
	cout << " this out     center.x  =  " << center.x << "      center.y =     " << center.y << endl;
	imwrite("dest.jpg", dest);
	int flag = 0;
	flag = Findcouter_TWO_inside(dest);
	if (flag == 0)
	{
		angle = 90 + angle;
	}
	cout << "angle =   " << angle << endl;
	Mat dst(img.size(), img.type());
	Mat rotMat = getRotationMatrix2D(center, angle, 1);
	warpAffine(img, dst, rotMat, img.size());
	imwrite("xuangzhaung.jpg", dst);
	getchar();
	return 0;
}
