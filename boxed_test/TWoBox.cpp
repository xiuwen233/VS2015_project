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

Point Findcouter_foroutside(Mat img, float &angle)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];

	CvPoint2D32f center;
	Point gtecross;
	gtecross.x = 0;
	gtecross.y = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);

		if (rect.size.height *rect.size.width > 10000 && rect.size.height > 100 && rect.size.width > 100)
		{
			cout << "rect.angle  =   " << rect.angle << endl;
			angle = rect.angle;
			cout << "rect.center.x =  " << rect.center.x << "      rect.center.y = " << rect.center.y << endl;

			gtecross = rect.center;
			for (int j = 0; j <= 3; j++)
			{
				line(img, Corners[j], Corners[(j + 1) % 4], Scalar(255), 2);
			}
		}
	}
	imwrite("foroutside.jpg", img);
	return gtecross;

}

Mat justforWhite(Mat img)
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
				*(p + j * 3) = 200;
				*(p + j * 3 + 1) = 200;
				*(p + j * 3 + 2) = 200;
			}
			else {
				//*(p + j * 3) = 255;
				//*(p + j * 3 + 1) = 255;
				//*(p + j * 3 + 2) = 255;
			}
		}
	}
	imwrite("JustWhite.jpg", img);
	return img;
}


float  Findcouter_center(Mat img, Point * point)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//rect = cvMinAreaRect2(contours, 0);
	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
	float angle = 0.0;
	int pointnum = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));

		cvBoxPoints(rect, Corners);
		if (rect.size.width  * rect.size.height > 10000 &&
			rect.size.width > 100 &&
			rect.size.height > 100) {


			cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
			angle = rect.angle;
			cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
			point[pointnum].x = rect.center.x;
			point[pointnum].y = rect.center.y;
			pointnum++;
			//for (int n = 0; n < 4; n++) {
			//	cout << "n =   " << n << "    x = " << Corners[n].x << "  y = " << Corners[n].y << endl;
			//}

			line(img, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
			line(img, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
			line(img, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
			line(img, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);

		}
	}

	imwrite("cvLine.jpg", img);
	return angle;

}

int main()
{
	Mat img, img2, img3, out;
	String beaforeclasspath = "E://photo_opencv/boxTest/what/what(";
	String afterclasspath = ").jpg";
	String classpath = beaforeclasspath + to_string(96) + afterclasspath;
	img = imread(classpath);
	Mat outside = img.clone();
	//-------------------------------------得到外面那个框的中心坐标--------------------------------------------------------
	Mat immg = justforGrenn(outside);
	medianBlur(immg, out, 7);
	cvtColor(out, out, CV_RGB2GRAY);
	imwrite("cvtcolor.jpg", out);
	threshold(out, out, 200, 255, THRESH_BINARY);
	imwrite("threshold.jpg", out);
	Mat getelement = out.clone();
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	morphologyEx(getelement, img3, MORPH_CLOSE, element);
	imwrite("CLOSE.jpg", img3);
	Mat cannyimg;
	Canny(img3, cannyimg, 50, 100, 3);
	imwrite("canny.jpg", cannyimg);

	float sideangle = 0.0;
	Point Bigside;
	Bigside = Findcouter_foroutside(cannyimg, sideangle);
	cout << "the out side max sideangle =  " << sideangle << endl;
	cout << "the out  Bigside x  =  " << Bigside.x << "       the out Bigside  y =  " << Bigside.y << endl;



	//---------------------------------------------得到中心两个白标签的坐标---------------------------------------------------
	Mat inside = img.clone();
	Mat immg2 = justforWhite(inside);
	medianBlur(immg2, out, 7);
	cvtColor(out, out, CV_RGB2GRAY);
	imwrite("cvtcolor2.jpg", out);
	threshold(out, out, 200, 255, THRESH_BINARY);
	imwrite("threshold2.jpg", out);
	Mat getelement2 = out.clone();
	// Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	//morphologyEx(getelement, img3, MORPH_OPEN, element);
	//	//imwrite("OPen.jpg", img3);
	morphologyEx(getelement2, img3, MORPH_CLOSE, element);
	imwrite("CLOSE2.jpg", img3);
	//	morphologyEx(img3, img3, MORPH_ERODE, element);
	//	imwrite("ERODE_2.jpg", img3);
	////	morphologyEx(img3, img3, MORPH_DILATE, element);
	//	//imwrite("DILATE.jpg", img3);
	Mat cannyimg2;
	Canny(img3, cannyimg2, 50, 100, 3);
	imwrite("canny.jpg", cannyimg2);
	//得到中间两个白色的中心点
	float angle;
	Point  gtepoint[5];
	angle = Findcouter_center(cannyimg2, gtepoint);
	cout << "out side angle   =   " << angle << endl;
	for (int i = 0; i < 2; i++)
	{
		cout << "gtepoint[i].x" << gtepoint[i].x << "gtepoint[i].y" << gtepoint[i].y << endl;
	}
	//Findcouter(cannyimg);

	//------------------------------------判断两个点谁是中心点----------------------------------------
	int mx, my;
	mx = Bigside.x - gtepoint[0].x;
	my = Bigside.y - gtepoint[0].y;
	mx = mx*mx;
	my = my*my;
	cout << "mx + my =    " << (mx + my) << endl;
	int flag = 0;
	if ((mx + my) > 1000) {
		flag = 0;
	}
	else {
		flag = 1;
	}
	//--------------------------------------------------------
	if (gtepoint[flag].x > Bigside.x)
	{
		if (gtepoint[flag].y > Bigside.y) {
			angle = 180 + angle;
			cout << "右下角 " << endl;
		}
		else {
			angle = 90 + angle;
			cout << "右上角 " << endl;
		}
	}
	else {
		if (gtepoint[flag].y > Bigside.y) {
			angle = angle - 90;
			cout << "左下角 " << endl;
		}
		else {
			angle = angle;
			cout << "左上角 " << endl;
		}
	}
	cout << " final angle === " << angle;
	Mat dst(img.size(), img.type());
	Mat rotMat = getRotationMatrix2D(Bigside, angle, 1);
	warpAffine(img, dst, rotMat, img.size());
	imwrite("xuangzhaung.jpg", dst);
	getchar();
}