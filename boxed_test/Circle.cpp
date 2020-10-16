#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<Windows.h> 
#include"Histogram.h"
#include"DifferThread.h"

using namespace cv;
using namespace std;

Mat justforBLACK(Mat img,int n)
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
			if ( G>160 && R>160 && G>160) // R > num && G>num && B>num  Gray >  num
			{
				*(p + j * 3) = 255;
				*(p + j * 3 + 1) = 255;
				*(p + j * 3 + 2) = 255;
			}
			else {
				*(p + j * 3) = 0;
				*(p + j * 3 + 1) = 0;
				*(p + j * 3 + 2) = 0;
			}
		}
	}
	imwrite("JustBlaack_"+to_string(n)+".jpg", img);
	return img;
}

int  voifmain()
{
	Mat src, mid1, mid2, dest;
	String beaforeclasspath = "E://photo_opencv/circle(";
	String afterclasspath = ").jpg";
	//src = imread("E://photo_opencv/circle(5).jpg",1);
	for (int i = 1; i < 9; i++)
	{
		String filepath = beaforeclasspath + to_string(i) + afterclasspath;
		src = imread(filepath);
		//medianBlur(src, mid1, 7);
		//adaptiveThreshold(mid1,mid2,255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,7,0);
		//String writename = "result_" + to_string(i) + ".jpg";
		//imwrite(writename,mid2);
		//justforBLACK(src,i);
		HistGet(src,i);
	}
	cout << "enter KEY to stop" << endl;
	getchar();
	return 0;
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

int main9()
{
	VideoCapture captue(1);
	Mat imme;
	while (1)
	{
		captue >> imme;

		//faceCascade.detectMultiScale(ime, faces, 1.2, 6, 0, Size(0, 0));
		//if (faces.size()>0)
		//{   
		//	cout << "检测到人脸" << endl;
		//	for (int i = 0; i<faces.size(); i++)
		//	{
		//	rectangle(ime, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height),
		//		Scalar(0, 255, 0), 1, 8);    //框出人脸位置  
		//	}
		//	}
		Mat gray;
		Mat mbr;
		Mat canny;
		cvtColor(imme, gray, CV_RGB2GRAY);
		draw_hist(gray);
		//medianBlur(gray, mbr, 9);
		//GaussianBlur(gray, mbr, Size(9, 9), 2, 2);
		Canny(gray, canny, 200, 200, 3);
		//GaussianBlur(canny, canny, Size(3, 3), 2, 2);
		
		//threshold(canny, canny,0,255,0);
		vector<Vec3f> circles;

		//dp 累加器图像的分辨率。这个参数允许创建一个比输入图像分辨率低的累加器。（这样做是因为有理由认为图像中存在的圆会自然降低到与图像宽高相同数量的范畴）。如果dp设置为1，则分辨率是相同的；如果设置为更大的值（比如2），累加器的分辨率受此影响会变小（此情况下为一半）。dp的值不能比1小，但也不要太大，越大越易误判，最好为2。
		//	min_dist 该参数是让算法能明显区分的两个不同圆之间的最小距离。
		//	param1 用于Canny的边缘阀值上限，下限被置为上限的一半。根据图像总体灰度情况确定。
		//	param2 累加器的阀值。较大，只检测较大的圆；较小，小圆也检测。
		//	min_radius 最小圆半径。
		//	max_radius 最大圆半径。
		HoughCircles(canny, circles, CV_HOUGH_GRADIENT, 2, 50, 100, 80, 20, 80);

		for (size_t i = 0; i < circles.size(); i++)
		{
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			printf("found circle.\n");
			circle(canny, center, 3, Scalar(128, 128, 128), -1, 8, 0);
			// circle outline
			circle(canny, center, radius, Scalar(128, 128, 128), 1, 8, 0);
		}

		imshow("gray", gray);
		//imshow("mbr", mbr);
		imshow("canny", canny);

		//imshow("computer",ime);
		waitKey(30);

	}
}



struct Circlecenter
{
	Point center;
	int circleR;
};

void HIST_get(Mat img2)
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
	imshow("HIST", imageShow);

	int realValue[256];
	for (int i = 0; i < 256; i++)
	{
		realValue[i] = saturate_cast<int>(hist.at<float>(i) * hpt / maxVal);
		//rectangle(dstImage, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue[0]), Scalar(255), 1, 8, 0);
	}
	int num = Get1DMaxEntropyThreshold(realValue);
	cout  << "  get the threadshold  number is  =   " << num << endl;

	//int realValue[256];
	//int realValue_tmp1;
	//int realValue_tmp2;
	//int realValue_tmp3;

	//for (int i = 0; i < 256; i++)
	//{
	//	realValue[i] = saturate_cast<int>(hist.at<float>(i) * hpt / maxVal);
	//	//rectangle(dstImage, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue[0]), Scalar(255), 1, 8, 0);
	//}

	//for (int k = 0; k < 5; k++)
	//{
	//	realValue[0] = (realValue[0] + realValue[0] + realValue[1]) / 3;

	//	for (int i = 1; i < 255; i++)
	//	{
	//		realValue_tmp1 = realValue[i - 1];
	//		realValue_tmp2 = realValue[i];
	//		realValue_tmp3 = realValue[i + 1];
	//		realValue[i] = (realValue_tmp1 + realValue_tmp2 + realValue_tmp3) / 3;
	//	}
	//	realValue[255] = (realValue[254] + realValue[255] + realValue[255]) / 3;
	//}

	//Mat afterImage(size * scale, size, CV_8U, Scalar(0));
	//for (int i = 0; i < 256; i++)
	//{
	//	rectangle(afterImage, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue[i]), Scalar(255));
	//}
	//imshow("after", afterImage);
}

void main()
{
	VideoCapture captue(1);
	Mat image;
	while (1)
	{
		DWORD start_time = GetTickCount();
		captue >> image;

		if (!image.data)
		{
			printf("not image.data, please check does the camera open ? \n");
			printf(" restart try again \n");
			break;
		}
		//imshow("image", image);
		vector<vector<Point> > contours;
	//	Mat bimage = image >= 60;

		Mat in_img = image.clone();
		HIST_get(image);
		cvtColor(image, image,CV_BGR2GRAY);

		Mat bimage;
		threshold(image, bimage,110,255,1);
		Mat out;
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

		//------------------------------------------
		Mat med = in_img.clone();
		Mat mde_out;
		medianBlur(med, mde_out, 33);
		cvtColor(mde_out, mde_out, CV_BGR2GRAY);
		Mat med_binimg;
		threshold(mde_out, med_binimg, 110, 255, 1);
		imshow("med_binimg", med_binimg);
		//--------------------------------------------
		//morphologyEx(bimage, out, MORPH_OPEN, element);
		//imshow("MORPH_OPEN", out);
		Mat out2;
		morphologyEx(bimage, out2, MORPH_CLOSE, element);
		imshow("MORPH_CLOSE", out2);
		//Mat canny;
		//Canny(bimage, canny, 200, 200, 3);
		//imshow("canny", canny);
		imshow("threshol", bimage);

		findContours(bimage, contours, RETR_LIST, CHAIN_APPROX_NONE);

		//Mat cimage = Mat::zeros(bimage.size(), CV_8UC3);

		//Point  cirpocenter[50];
		struct Circlecenter  cirpocenter[50];
		int len = 0;
		int have_circle = 0;
		if (contours.size()==0)
		{
			continue;
		}
		for (size_t i = 0; i < contours.size(); i++)
		{
			size_t count = contours[i].size();
			if (count < 6)
				continue;

			Mat pointsf;
			Mat(contours[i]).convertTo(pointsf, CV_32F);
			RotatedRect box = fitEllipse(pointsf);

			if (MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height) * 1.2)
				continue;

			if(box.size.width < 50 ||  box.size.height < 50 || box.size.width >  500 || box.size.height > 500  )
				continue;

			//drawContours(cimage, contours, (int)i, Scalar::all(255), 1, 8);

			//---------------------------------增加一个教研功能，如果该点是黑色的就去掉-----------------------
			  //先判断点有没有越界
			if (box.center.x < 5 || box.center.x >(in_img.cols -5) )
			{
				printf("point get out of the picuter");
				continue;
			}
			if (box.center.y < 5 || box.center.y >(in_img.rows - 5))
			{
				printf("point get out of the picuter");
				continue;
			}
		   //再判断点周围的颜色是否是黑色  需要修改一下
			bool flag = true;
			for (int m = -5; m < 5;m++) {

				uchar* p = in_img.ptr((int)box.center.y +m);
				int B = *(p + ((int)box.center.x +m)* 3);
				int G = *(p + ((int)box.center.x + m) * 3 + 1);
				int R = *(p + ((int)box.center.x + m) * 3 + 2);
				if (B < 105 && G < 105 && R < 105)
				{
					cout << "it is broken point " << endl;
					cout << "B=  " << B << "  G=  " << G << "  R =  " << R << endl;
					circle(in_img, box.center, 2, Scalar(0, 255, 255), 2);
					flag = false;
					break;
				}
				int B1 = *(p + ((int)box.center.x - m) * 3);
				int G1 = *(p + ((int)box.center.x - m) * 3 + 1);
				int R1 = *(p + ((int)box.center.x - m) * 3 + 2);
				if (B1 < 105 && G1 < 105 && R1 < 105)
				{
					cout << "it is broken point " << endl;
					cout << "B1=  " << B1 << "  G1=  " << G1 << "  R1 =  " << R1 << endl;
					circle(in_img, box.center, 2, Scalar(0, 255, 255), 2);
					flag = false;
					break;
				}
				//中心点
			}
			if (flag == false)
			{
				continue;
			}

			cirpocenter[len].center.x = (int)box.center.x;
			cirpocenter[len].center.y = (int)box.center.y;
			//cirpocenter[len].circleR = (640 - cirpocenter[len].center.x) * (640 - cirpocenter[len].center.x) + (480 - cirpocenter[len].center.y) * (480 - cirpocenter[len].center.y);
			cirpocenter[len].circleR = cirpocenter[len].center.x * cirpocenter[len].center.x + cirpocenter[len].center.y * cirpocenter[len].center.y;
			have_circle++;
			if ((++len)>=50)
			{
				printf("worring: len is big than array's len \n");
				break;
			}
		
		//circle(in_img, box.center, 2, Scalar(123,60, 78), 2);
			//String showpoint = "(" + to_string((int)box.center.x) + "," + to_string((int)box.center.y) + ")";
		//putText(in_img,showpoint,box.center ,FONT_HERSHEY_PLAIN,1,Scalar(0,255,255) );

			//外接圆
		//	ellipse(in_img, box, Scalar(0, 255, 255), 2, LINE_AA);
			//ellipse(cimage, box.center, box.size*0.5f, box.angle, 0, 360, Scalar(0, 255, 255), 1, LINE_AA);

		/*	Point2f vtx[4];
			box.points(vtx);
			for (int j = 0; j < 4; j++)
				line(cimage, vtx[j], vtx[(j + 1) % 4], Scalar(0, 255, 0), 1, LINE_AA);*/
		}

		//用于排序获得最右下角的值
		/*if (have_circle == 0)
		{
			printf("have no circle");
			continue;
		}*/
		int sogolen = 0;
		int minddirect = cirpocenter[0].circleR;
		for (int mlen = 0; mlen < len; mlen++)
		{
			if (cirpocenter[mlen].circleR< minddirect)
			{
				sogolen = mlen;
			}
		}

		circle(in_img, cirpocenter[sogolen].center, 2, Scalar(255,0,0), 2);
		//cout << "point:" << cirpocenter[sogolen].center << endl;
		String showpoint = "(" + to_string(cirpocenter[sogolen].center.x) + "," + to_string(cirpocenter[sogolen].center.y) + ")";
		putText(in_img, showpoint, cirpocenter[sogolen].center, FONT_HERSHEY_PLAIN, 2, Scalar(0, 255, 255),2);

		imshow("result", in_img);
		DWORD end_time = GetTickCount();
		//cout << "The run time is:" << (end_time - start_time) << "ms!" << endl;
		waitKey(1);
	}
}

void ffmain()
{
	VideoCapture captue(1);
	Mat image;
	while (1)
	{
		captue >> image;
		Mat imme = image.clone();
		for (int i = 0; i < imme.rows; i++)
		{
			uchar* p = imme.ptr(i);//获取Mat某一行的首地址  

			for (int j = 0; j < imme.cols; j++)
			{
				//顺序是BGR,不是RGB
				int B = *(p + j * 3);
				int G = *(p + j * 3 + 1);
				int R = *(p + j * 3 + 2);
				//float Gray = R*0.299 + G*0.587 + B*0.114;
				if ((G+B+R)>250 &&(B-R)<90) // R > num && G>num && B>num  Gray >  num
				{
					*(p + j * 3) = 255;
					*(p + j * 3 + 1) = 255;
					*(p + j * 3 + 2) = 255;
				}
				else {
					*(p + j * 3) = 0;
					*(p + j * 3 + 1) = 0;
					*(p + j * 3 + 2) = 0;
				}
			}
		}
		imshow("imme", imme);
		imshow("image", image);
		waitKey(10);
	}
}

void intmain()
{
	Mat src, mid1, mid2, dest;
	String beaforeclasspath = "E://photo_opencv/GetBox/circle(";
	String afterclasspath = ").jpg";
	//src = imread("E://photo_opencv/circle(5).jpg",1);
	for (int i = 1; i < 20; i++)
	{
		String filepath = beaforeclasspath + to_string(i) + afterclasspath;
		src = imread(filepath);
		cvtColor(src, mid1, CV_BGR2GRAY);
		//medianBlur(src, mid1, 7);
		//adaptiveThreshold(mid1,mid2,255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,7,0);
		//String writename = "result_" + to_string(i) + ".jpg";
		//imwrite(writename,mid2);
		//justforBLACK(src,i);
		String aftername = "E://photo_opencv/GetBox/CvtClolor_" + to_string(i) + ".jpg";
		imwrite(aftername, mid1);

		int num = HistGet(mid1, i);
		if (num == 0) {
			continue;
		}
		threshold(mid1, mid2, num, 255, THRESH_BINARY);
		String aftername23 = "E://photo_opencv/GetBox/THreshold_" + to_string(i) + ".jpg";
		imwrite(aftername23, mid2);

		vector<vector<Point> > contours;
		findContours(mid2, contours, RETR_LIST, CHAIN_APPROX_NONE);

		//Mat cimage = Mat::zeros(bimage.size(), CV_8UC3);

		for (size_t i = 0; i < contours.size(); i++)
		{
			size_t count = contours[i].size();
			if (count < 6)
				continue;

			Mat pointsf;
			Mat(contours[i]).convertTo(pointsf, CV_32F);
			RotatedRect box = fitEllipse(pointsf);

			if (MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height) * 1.2)
				continue;

			if (box.size.width < 30 || box.size.height < 30 || box.size.width >  500 || box.size.height > 500)
				continue;

			//drawContours(cimage, contours, (int)i, Scalar::all(255), 1, 8);

			//中心点
			dest = src.clone();
			circle(dest, box.center, 2, Scalar(0, 255, 255), 2);
			//	String showpoint = "(" + to_string((int)box.center.x) + "," + to_string((int)box.center.y) + ")";
			//	putText(in_img,showpoint,box.center ,FONT_HERSHEY_PLAIN,2,Scalar(0,255,255) );

			//外接圆
			//	ellipse(in_img, box, Scalar(0, 255, 255), 2, LINE_AA);
			//ellipse(cimage, box.center, box.size*0.5f, box.angle, 0, 360, Scalar(0, 255, 255), 1, LINE_AA);

			/*	Point2f vtx[4];
			box.points(vtx);
			for (int j = 0; j < 4; j++)
			line(cimage, vtx[j], vtx[(j + 1) % 4], Scalar(0, 255, 0), 1, LINE_AA);*/
		}
		String aftername_1= "E://photo_opencv/GetBox/midcenter_" + to_string(i) + ".jpg";
		imwrite(aftername_1, dest);
	}
	cout << "enter KEY to stop" << endl;
	getchar();
}
