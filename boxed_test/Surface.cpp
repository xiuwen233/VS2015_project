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


Point Findcouter_Positive(Mat img, float * angle)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	Point  Twocengter[2];

	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));

		cvBoxPoints(rect, Corners);
		if (rect.size.width  * rect.size.height > 2500 &&
			rect.size.width > 50 &&
			rect.size.height > 50) {

			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			*angle = rect.angle;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			for (int n = 0; n < 4; n++) {
				line(img, Point((int)Corners[n].x, (int)Corners[n].y), Point((int)Corners[(n + 1) % 4].x, (int)Corners[(n + 1) % 4].y), Scalar(255, 0, 255), 2, 8, 0);
			}
			if (rect.size.width > 150 && rect.size.height > 150)
			{
				Twocengter[0].x = rect.center.x;
				Twocengter[0].y = rect.center.y;
			}
			else {
				Twocengter[1].x = rect.center.x;
				Twocengter[1].y = rect.center.y;
			}
		}

	}

	if (Twocengter[1].x >Twocengter[0].x)
	{
		if (Twocengter[1].y >Twocengter[0].y)
		{
			*angle = *angle;
			cout << "左上角" << endl;
		}
		else {
			*angle = *angle - 90;
			cout << "左下角" << endl;
		}
	}
	else {
		if (Twocengter[1].y >Twocengter[0].y)
		{
			*angle = 90 + *angle;
			cout << "右上角" << endl;
		}
		else {
			*angle = 180 + *angle;
			cout << "右下角" << endl;
		}
	}


	imwrite("cvLine.jpg", img);
	return Twocengter[0];

}


int main()
{
	system("color 16");
	Mat img, img2, img3, out;
	String beaforeclasspath = "E://photo_opencv/boxTest/what/what(";
	String afterclasspath = ").jpg";
	String classpath = beaforeclasspath + to_string(18) + afterclasspath;
	img = imread(classpath);
	Mat imme = img.clone();
	justforGrenn(imme);
	medianBlur(imme, imme, 7);
	imwrite("mediaBlur.jpg", imme);
	cvtColor(imme, imme, CV_RGB2GRAY);
	imwrite("cvtColor.jpg", imme);
	Canny(imme, imme, 50, 100, 3);
	imwrite("cvCanny.jpg", imme);
	Point center;
	float angle = 0.0;
	center = Findcouter_Positive(imme, &angle);
	cout << "result angle =  " << angle << endl;

	Mat dst(img.size(), img.type());
	Mat rotMat = getRotationMatrix2D(center, angle, 1);
	warpAffine(img, dst, rotMat, img.size());
	imwrite("xuangzhaung.jpg", dst);
	getchar();
	return 0;
}