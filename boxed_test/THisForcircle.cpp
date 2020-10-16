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
				
			}
		}
	}
	imwrite("JustGreen.jpg", img);
	return img;
}


Mat whatTHis(Mat img, Mat src, Point * center1)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat imme = img.clone();
	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
	CvBox2D rect;
	CvPoint2D32f Corners[4];

	CvPoint2D32f center;
	Point gtecross;
	gtecross.x = 0;
	gtecross.y = 0;

	vector<Rect> boundRect(contours.size());
	//	vector<Rect> getlast;
	//int getnum = 0;
	//	boundRect[i] = boundingRect(Mat(contours[i]));
	int num[5][5];
	int numlen = 0;
	int numxy = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);
		boundRect[i] = boundingRect(Mat(contours[i]));
		if (rect.size.height *rect.size.width > 15000 && rect.size.height > 100 && rect.size.width > 100)
		{
			
			gtecross = rect.center;
			for (int j = 0; j <= 3; j++)
			{
				line(img, Corners[j], Corners[(j + 1) % 4], Scalar(255), 2);
			}
			//画出外面正常框出的矩形
			rectangle(imme, Point(boundRect[i].x, boundRect[i].y), Point(boundRect[i].x + boundRect[i].width, boundRect[i].y + boundRect[i].height), Scalar(255), 2, 8);
			num[numlen][numxy] = i;
			num[numlen][numxy + 1] = boundRect[i].x;
			numlen++;
			numxy++;

		}
	}
	imwrite("getout.jpg", imme);
	int d = 0;
	if (num[0][1] > num[1][1])
	{
		d = num[0][0];
	}
	else {
		d = num[1][0];
	}
	Rect rect1(boundRect[d].x, boundRect[d].y, boundRect[d].width, boundRect[d].height);
	*center1 = gtecross;
	Mat dest = src(rect1);
	return dest;
	//rectangle(dstImg, Point(boundRect[i].x, boundRect[i].y), Point(boundRect[i].x + boundRect[i].width, boundRect[i].y + boundRect[i].height), Scalar(0, 255, 0), 2, 8);
}


float Findcouter_inside_small(Mat img)
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
	cout << "img.rows  =   " << img.rows << "      img.cols  =  " << img.cols << endl;
	int n = 0;
	float angle = 0.0;
	for (int i = 0; i < contours.size(); i++)
	{
		rect = minAreaRect((contours[i]));
		cvBoxPoints(rect, Corners);

		if (rect.size.height *rect.size.width > 2500 && rect.size.height > 50 && rect.size.width > 50
			&& rect.size.height < 150 && rect.size.width < 150)
		{
			if (rect.center.x >= 0 && rect.center.x <= img.cols &&rect.center.y >= 0 && rect.center.y <= img.rows)
			{
				cout << "rect.angle  =   " << rect.angle << endl;
				angle = rect.angle;
				cout << "rect.center.x =  " << rect.center.x << "      rect.center.y = " << rect.center.y << endl;
		
				gtecross = rect.center;
				for (int j = 0; j <= 3; j++)
				{
					line(img, Corners[j], Corners[(j + 1) % 4], Scalar(255), 2);
				}
				n++;
			}
		}
	}

	if (n > 1) {
		return 0;
	}
	if (gtecross.x > img.cols / 2)
	{
		if (gtecross.y > img.rows / 2)
		{
			angle = 90 + angle;
			cout << "右上角" << endl;
		}
		else {
			angle = angle;
			cout << "左上角 " << endl;
		}
	}
	else {
		if (gtecross.y > img.rows / 2) {
			angle = 180 + angle;
			cout << "右下角 " << endl;
		}
		else {
			angle = angle - 90;
			cout << "左下角 " << endl;
		}

	}
	imwrite("wherehththttTTTTT.jpg", img);
	return angle;

}


int    main()
{
	Mat img, img2, img3, out;
	String beaforeclasspath = "E://photo_opencv/boxTest/what/what(";
	String afterclasspath = ").jpg";
	String classpath = beaforeclasspath + to_string(27) + afterclasspath;
	img = imread(classpath);
	Mat finalimg = img.clone();
	img2 = img.clone();
	Mat immg = justforGrenn(img);
	medianBlur(img, img, 7);
	imwrite("mediaBlur.jpg", img);
	cvtColor(img, img, CV_RGB2GRAY);
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	morphologyEx(img, out, MORPH_OPEN, element);
	//cvtColor(img, img, CV_RGB2GRAY);
	Canny(out, out, 50, 100, 3);
	Mat dest;
	Point imgcenter;
	dest = whatTHis(out, img2, &imgcenter);
	cout << "imgcenter .x   =   " << imgcenter.x << "     imgcenter.y =  " << imgcenter.y << endl;
	imwrite("Rect.jpg", dest);
	cvtColor(dest, dest, CV_RGB2GRAY);
	imwrite("dest.jpg", dest);
	threshold(dest, dest, 222, 255, THRESH_BINARY);
	imwrite("66666fu.jpg", dest);
	Mat element1 = getStructuringElement(MORPH_RECT, Size(5, 5));
	morphologyEx(dest, dest, MORPH_OPEN, element1);
	imwrite("nameOPen.jpg", dest);
	morphologyEx(dest, dest, MORPH_DILATE, element1);
	imwrite("nameDILATE.jpg", dest);
	//morphologyEx(dest, dest, MORPH_DILATE, element1);
	//imwrite("nameDILATE2.jpg", dest);
	Canny(dest, dest, 50, 100, 3);
	imwrite("twocanny.jpg", dest);
	float angle = 0.0;
	angle = Findcouter_inside_small(dest);
	cout << " angle =   " << angle << endl;
	Mat dst(img.size(), img.type());
	Mat rotMat = getRotationMatrix2D(imgcenter, angle, 1);
	warpAffine(finalimg, dst, rotMat, img.size());
	imwrite("result.jpg", dst);

	getchar();
	return 0;
}