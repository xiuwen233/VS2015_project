#include <opencv2\core\core.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<opencv2\highgui\highgui.hpp>

#include <iostream>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
//Windows.h和WinBase.h

#include <Windows.h>
#include<WinBase.h>
using namespace cv;
using namespace std;


int threadshold = 210;
int element_type = 2;

static void ContrastAndBright(int, void *)
{
	Mat m = Mat(800, 700, 0);
	imshow("【控制面板】", m);
}

void InitControlBoard()
{
	namedWindow("【控制面板】", 0);
	createTrackbar("腐蚀膨胀", "【控制面板】", &element_type, 10, ContrastAndBright);
	createTrackbar("二值化阈值：", "【控制面板】", &threadshold, 256, ContrastAndBright);

}


//void Findcouter(Mat img)
//{
//	vector<vector<Point>> contours;
//	vector<Vec4i> hierarchy;
//	findContours(img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
//	CvBox2D rect;
//	CvPoint2D32f Corners[4];
//	//rect = cvMinAreaRect2(contours, 0);
//	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
//	bool inpicture = true;
//	for (int i = 0; i < contours.size(); i++)
//	{
//		rect = minAreaRect((contours[i]));
//
//		cvBoxPoints(rect, Corners);
//		if (rect.size.width  * rect.size.height > 10000 &&
//			rect.size.width > 100 &&
//			rect.size.height > 100) {
//
//
//			inpicture = true;
//			// for (int n = 0; n < 4; n++) {
//			// if (Corners[n].x<0 || Corners[n].x>img.cols || Corners[n].y<0 || Corners[n].y>img.rows) {
//			//	 inpicture = false;
//			//	 break;
//			//	 }
//
//			//	}
//
//			if (inpicture == true) {
//				cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
//				cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
//				for (int n = 0; n < 4; n++) {
//					cout << "n =   " << n << "    x = " << Corners[n].x << "  y = " << Corners[n].y << endl;
//				}
//
//				line(img, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(img, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(img, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(img, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
//
//			}
//		}
//	}
//
//	//imwrite("cvLine.jpg", img);
//	//Mat Contours = Mat::zeros(img.size(), CV_8UC1);
//	//	for (int i = 0; i < contours.size(); i++)
//	//{
//	//contours[i]代表的是第i个轮廓，contours[i].size()代表的是第i个轮廓上所有的像素点数  
//	//	for (int j = 0; j < contours[i].size(); j++)
//	//	{
//	//绘制出contours向量内所有的像素点  
//	//	Point P = Point(contours[i][j].x, contours[i][j].y);
//	//	Contours.at<uchar>(P) = 255;
//	//	}
//	//for (int i = 0; i < contours.size();i++)
//	//{
//	//	Point2f rect_points[4];
//
//	//}
//
//	//	}
//	//imwrite("Findcouter.jpg", Contours);
//
//	//CvBox2D rect;
//	//CvPoint2D32f Corners[4];
//	//rect = cvMinAreaRect2(pContour, 0);
//	//cvBoxPoints(rect, Corners);  //转换为矩形的四个顶点
//}
//
//
////针对中国移动的照片
//int hehemain()
//{
//	Mat image = imread("right.jpg");
//	Mat binarying, nameget, canyou;
//	cvtColor(image, binarying, CV_RGB2GRAY);
//	namedWindow("nihaoa");
//	InitControlBoard();
//	int num;
//	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
//
//	vector<vector<Point>> contours;
//	vector<Vec4i> hierarchy;
//	CvBox2D rect;
//	CvPoint2D32f Corners[4];
//
//	Mat what_fuck = image.clone();
//	while (1) {
//		what_fuck = image.clone();
//		threshold(binarying, nameget, threadshold, 255, CV_THRESH_BINARY);
//		imshow("nihaoa", nameget);
//		if (element_type > 7 ){
//
//		}
//		else {
//			morphologyEx(nameget, canyou, element_type, element);
//			imshow("niubi", canyou);
//		}
//		
//		findContours(canyou, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
//		for (int i = 0; i < contours.size(); i++)
//		{
//			rect = minAreaRect((contours[i]));
//
//			cvBoxPoints(rect, Corners);
//			if (rect.size.width  * rect.size.height > 10000 && rect.size.width > 90 && rect.size.height > 90 && 
//				rect.size.width<200 && rect.size.height < 200) {
//				 
//
//				// for (int n = 0; n < 4; n++) {
//				// if (Corners[n].x<0 || Corners[n].x>img.cols || Corners[n].y<0 || Corners[n].y>img.rows) {
//				//	 inpicture = false;
//				//	 break;
//				//	 }
//
//				//	}
//
//				/*if (inpicture == true) {
//					cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
//					cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
//					for (int n = 0; n < 4; n++) {
//						cout << "n =   " << n << "    x = " << Corners[n].x << "  y = " << Corners[n].y << endl;
//					}*/
//
//					line(what_fuck, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
//					line(what_fuck, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
//					line(what_fuck, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
//					line(what_fuck, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
//
//				//}
//			}
//		}
//		imshow("couter finder test", what_fuck);
//		num = waitKey(5);
//		if (num == 'q' || num == 'Q')
//		{
//			destroyAllWindows();
//			break;
//		}
//
//		if (num == 's')
//		{
//			imwrite("save.jpg", canyou);
//		}
//	}
//	getchar();
//	return 0;
//}
//
//
//void Gammaimg(Mat img)
//{
//	Mat image = img.clone();
//	Mat imageGamma(image.size(), CV_32FC3);
//	for (int i = 0; i < image.rows; i++)
//	{
//		for (int j = 0; j < image.cols; j++)
//		{
//			imageGamma.at<Vec3f>(i, j)[0] = (image.at<Vec3b>(i, j)[0])*(image.at<Vec3b>(i, j)[0])*(image.at<Vec3b>(i, j)[0]);
//			imageGamma.at<Vec3f>(i, j)[1] = (image.at<Vec3b>(i, j)[1])*(image.at<Vec3b>(i, j)[1])*(image.at<Vec3b>(i, j)[1]);
//			imageGamma.at<Vec3f>(i, j)[2] = (image.at<Vec3b>(i, j)[2])*(image.at<Vec3b>(i, j)[2])*(image.at<Vec3b>(i, j)[2]);
//		}
//	}
//	//归一化到0~255    
//	normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
//	//转换成8bit图像显示    
//	convertScaleAbs(imageGamma, imageGamma);
//	imwrite("Gammaimg.jpg", imageGamma);
//}
//
//
//int fuckmain()
//{
//	Mat srcimage = imread("hjjk.bmp");
//	Mat smallsrc;
//	resize(srcimage, smallsrc, Size(520, 520));
//	Mat binary, theraimg, elemimg;
//	Mat getbackimg;
//	int num;
//
//	Mat imageGamma(smallsrc.size(), CV_32FC3);
//	
//
//	for (int i = 0; i < smallsrc.rows; i++)
//	{
//		for (int j = 0; j < smallsrc.cols; j++)
//		{
//			imageGamma.at<Vec3f>(i, j)[0] = (smallsrc.at<Vec3b>(i, j)[0])*(smallsrc.at<Vec3b>(i, j)[0])*(smallsrc.at<Vec3b>(i, j)[0]);
//			imageGamma.at<Vec3f>(i, j)[1] = (smallsrc.at<Vec3b>(i, j)[1])*(smallsrc.at<Vec3b>(i, j)[1])*(smallsrc.at<Vec3b>(i, j)[1]);
//			imageGamma.at<Vec3f>(i, j)[2] = (smallsrc.at<Vec3b>(i, j)[2])*(smallsrc.at<Vec3b>(i, j)[2])*(smallsrc.at<Vec3b>(i, j)[2]);
//		}
//	}
//
//	normalize(imageGamma, imageGamma, 0, 255, CV_MINMAX);
//	//转换成8bit图像显示    
//	convertScaleAbs(imageGamma, imageGamma);
//
//
//	InitControlBoard();
//	cvtColor(imageGamma, binary, CV_RGB2GRAY);
//	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
//
//	while (1)
//	{
//		
//		//imshow("hehe", imageGamma);
//		imshow("src", binary);
//		threshold(binary, theraimg, threadshold, 255, CV_THRESH_BINARY);
//		imshow("bugjj", theraimg);
//
//		if (element_type > 7) {
//		}
//		else {
//			morphologyEx(theraimg, elemimg, element_type, element);
//			imshow("niubi", elemimg);
//		}
//
//		num = waitKey(5);
//		if (num == 'q' || num == 'Q')
//		{
//			destroyAllWindows();
//			break;
//		}
//	}
//
//	getchar();
//	return 0;
//
//}
//
////
//
//int getupmain()
//{
//	Mat srcimage = imread("right.jpg"); // right.jpg  light.jpg
//	Mat srcsest = imread("rect_31.jpg", 0);
//	Mat on_hash = calPHashCode(srcsest);
//	Mat dst_hash, trdst_hashac;
//	Mat smallsrc;
//	Mat binary, theraimg, elemimg;
//	int num;
//	InitControlBoard();
//	cvtColor(srcimage, binary, CV_RGB2GRAY);
//	Mat element = getStructuringElement(MORPH_RECT, Size(5,5));
//
//	vector<vector<Point>> contours ;
//	vector<Vec4i> hierarchy;
//	CvBox2D rect;
//	CvPoint2D32f Corners[4];
//	char name_path[200];
//	memset(name_path, 0, sizeof(name_path));
//
//	int hanming_distance;
//	while (1)
//	{
//		smallsrc = srcimage.clone();
//		threshold(binary, theraimg, threadshold, 255, CV_THRESH_BINARY);
//		imshow("just do it", theraimg);
//		if (element_type > 7) {
//		}
//		else {
//			morphologyEx(theraimg, elemimg, element_type, element);
//			imshow("niubi", elemimg);
//		}
//
//		findContours(elemimg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
//		vector<Rect> boundRect(contours.size());
//	
//		for (int i = 0; i < contours.size(); i++)
//		{
//			rect = minAreaRect((contours[i]));
//			boundRect[i] = boundingRect(Mat(contours[i]));
//			cvBoxPoints(rect, Corners);
//			if (   rect.size.width > 100 && rect.size.height > 100 &&
//				rect.size.width<250 && rect.size.height <250) {
//
//
//				// for (int n = 0; n < 4; n++) {
//				// if (Corners[n].x<0 || Corners[n].x>img.cols || Corners[n].y<0 || Corners[n].y>img.rows) {
//				//	 inpicture = false;
//				//	 break;
//				//	 }
//
//				//	}
//
//				/*if (inpicture == true) {
//				cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
//				cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
//				for (int n = 0; n < 4; n++) {
//				cout << "n =   " << n << "    x = " << Corners[n].x << "  y = " << Corners[n].y << endl;
//				}*/
//				//transpose(binary(boundRect[i]), trdst_hashac);
//				//dst_hash = calPHashCode(trdst_hashac);
//				//hanming_distance = calHammingDistance(on_hash, dst_hash);
//				//printf(" hanimmn is %d \n", hanming_distance);
//			/*	line(smallsrc, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(smallsrc, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(smallsrc, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(smallsrc, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);*/
//				//if (hanming_distance < 6)
//				//{
//					rectangle(smallsrc, boundRect[i], Scalar(78, 135, 0), 2, 8, 0);
//				//}		
//				//memset(name_path, 0, sizeof(name_path));
//				//sprintf(name_path,"rect_%d.jpg",i);
//				//imwrite(name_path, smallsrc(boundRect[i]));
//				
//			}
//		}
//		imshow("srccouter", smallsrc);
//		num = waitKey(20);
//		if (num =='q' || num=='Q')
//		{
//			destroyAllWindows();
//			break;
//		}
//	}
//
//	getchar();
//	return 0;
//}
//
//int  fuckrtmain()
//{
//	Mat src_one = imread("rect_31.jpg",0);
//	Mat src_two = imread("rect_48.jpg",0);
//	Mat src_test = imread("recttest_84.jpg", 0);
//
//	Mat on_hash = calPHashCode(src_one);
//	Mat two_hash = calPHashCode(src_two);
//	Mat testwhat;
//	transpose(src_test, testwhat);
//	//int num = calHammingDistance(on_hash, two_hash);
//	//printf("distance is %d \n ", num);
//	while (1) {
//		imshow("src_test", src_test);
//		imshow("testwhat", testwhat);
//		waitKey(10);
//	}
//
//	getchar();
//	return 0;
//}
//
//
//int gdh444main()
//{
//	Mat  srimage = imread("getout.jpg"); //getout.jpg
//	Mat  srcimage, binary, theraimg, elemimg;
//	Mat srccopyimg;
//
//	vector<vector<Point>> contours;
//	vector<Vec4i> hierarchy;
//	CvBox2D rect;
//	CvPoint2D32f Corners[4];
//
//	InitControlBoard();
//	cvtColor(srimage, binary, CV_RGB2GRAY);
//	char text_str[200];
//	int waitkey_num = 0;
//	Point center_big;
//	float angle_n;
//	while (1)
//	{
//		srccopyimg = srimage.clone();
//		elemimg = srimage.clone();
//		threshold(binary, theraimg, 65, 255, CV_THRESH_BINARY);
//		findContours(theraimg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
//		for (int i = 0; i < contours.size(); i++)
//		{
//			rect = minAreaRect((contours[i]));
//			//boundRect[i] = boundingRect(Mat(contours[i]));
//			cvBoxPoints(rect, Corners);
//			if (rect.size.width > 300 && rect.size.height > 300) {
//
//
//				// for (int n = 0; n < 4; n++) {
//				// if (Corners[n].x<0 || Corners[n].x>img.cols || Corners[n].y<0 || Corners[n].y>img.rows) {
//				//	 inpicture = false;
//				//	 break;
//				//	 }
//
//				//	}
//
//				/*if (inpicture == true) {
//				cout << "i  =   " << i << "    rect.angle    " << rect.angle << endl;
//				cout << "rect.center.x =  " << rect.center.x << "   rect.center.y = " << rect.center.y << endl;
//				for (int n = 0; n < 4; n++) {
//				cout << "n =   " << n << "    x = " << Corners[n].x << "  y = " << Corners[n].y << endl;
//				}*/
//				//transpose(binary(boundRect[i]), trdst_hashac);
//				//dst_hash = calPHashCode(trdst_hashac);
//				//hanming_distance = calHammingDistance(on_hash, dst_hash);
//				//printf(" hanimmn is %d \n", hanming_distance);
//				line(srccopyimg, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(srccopyimg, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(srccopyimg, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
//				line(srccopyimg, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
//				//if (hanming_distance < 6)
//				//{
//				memset(text_str, 0, sizeof(text_str));
//				sprintf(text_str, "angle: %f ", rect.angle);
//				putText(srccopyimg, text_str, rect.center, FONT_HERSHEY_PLAIN, 2.0, Scalar(78, 0, 160), 2, 8, 0);
//				center_big = rect.center;
//				angle_n = rect.angle;
//				//rectangle(smallsrc, boundRect[i], Scalar(78, 135, 0), 2, 8, 0);
//				//}		
//				//memset(name_path, 0, sizeof(name_path));
//				//sprintf(name_path,"rect_%d.jpg",i);
//				//imwrite(name_path, smallsrc(boundRect[i]));
//
//			}
//		}
//
//		imshow("hehehe", srccopyimg);
//		elemimg = ImageChangle(center_big, angle_n, elemimg);
//		imshow("rotaio", elemimg);
//		waitkey_num = waitKey(5);
//		if (waitkey_num == 'q' || waitkey_num == 'Q')
//		{
//			destroyAllWindows();
//			break;
//		}
//
//		if (waitkey_num == 's' || waitkey_num == 'S')
//		{
//			imwrite("getout.jpg", srcimage);
//		}
//
//
//	}
//	getchar();
//	return 0;
//}


Mat calPHashCode(Mat image)
{
	Mat floatImage, imageDct;
	resize(image, image, Size(32, 32));
	image.convertTo(floatImage, CV_32FC1);
	dct(floatImage, imageDct);
	Rect roi(0, 0, 8, 8);
	Scalar imageMean = mean(imageDct(roi));
	return (imageDct(roi) > imageMean[0]);
}

int calHammingDistance(Mat modelHashCode, Mat testHashCode)
{
	return countNonZero(modelHashCode != testHashCode);
}



Mat ImageChangle(Point center, float angle, Mat src)
{
	Mat dst;
	Mat rotMat = getRotationMatrix2D(center, angle, 1);
	warpAffine(src, dst, rotMat, src.size());
	return dst;
}
//this is test for a lit one

float center_angle = 0;

int fg444main()
{
	Mat hash_image = imread("rect_31.jpg", 0);
	Mat hash_src = calPHashCode(hash_image);
	Mat tra_dst, dst_hash;

	Mat  srcimage, binary, theraimg,elemimg;
	Mat srccopyimg, copyimg;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	//vector<Rect> boundRect(contours.size());

	Mat xunzahng, binarybackof;

	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

	char filepath[200];
	String win_name = "test_1";
	String win_name2 = "test_2";
	int retangle_num = 0;
	int waitkey_num = 0;
	Point bigrect_center;
	Point smallrect_center;
	int hanming_distance = 0;
	int transp_hanming_distance = 0;
	Point center_big;
	
	bool change_flage = false;

	char nam_path[200];
	while (1) {
		printf("please in put one jpg  and q or Q is exit \n");
		memset(filepath, 0, sizeof(filepath));
		scanf("%s", filepath);
		if (strcmp(filepath, "q") == 0 || strcmp(filepath, "Q") == 0)
		{
			printf("ready to exit \n");
			break;
		}
		else
		{
			printf("filepath is %s \n", filepath);
			srcimage = imread(filepath);
			cvtColor(srcimage, binary, CV_RGB2GRAY);
			InitControlBoard();
			//namedWindow(win_name2, 0);
			
			while (1)
			{
				srccopyimg = srcimage.clone();
				binarybackof = binary.clone();
				threshold(binarybackof, theraimg, 65, 255, CV_THRESH_BINARY);
				//imshow(win_name, theraimg);
				findContours(theraimg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
				vector<Rect> boundRect(contours.size());
				retangle_num = 0;
				for (int i = 0; i < contours.size(); i++)
				{
					boundRect[i] = boundingRect(Mat(contours[i]));
					rect = minAreaRect((contours[i]));
					//boundRect[i] = boundingRect(Mat(contours[i]));
					cvBoxPoints(rect, Corners);
					
					if (boundRect[i].width > 400 && boundRect[i].height > 400 )
					{
						rectangle(srccopyimg, boundRect[i], Scalar(78, 135, 0), 2, 8, 0);
						retangle_num++;
						bigrect_center.x = boundRect[i].x + boundRect[i].width / 2;
						bigrect_center.y = boundRect[i].y + boundRect[i].height / 2;
						center_big = rect.center;
						center_angle = rect.angle;
					}				
				 }//for
				if (retangle_num != 1)
				{
					printf("param set error \n");
					break;
				}

				//copyimg = srccopyimg.clone();
				
				if (   !(abs(center_angle) < 5 || (90 - abs(center_angle)) <5)  ) {
					binarybackof = ImageChangle(center_big, center_angle, binarybackof.clone());
					srccopyimg = ImageChangle(center_big, center_angle, srccopyimg.clone());
				}
			   
				threshold(binarybackof, theraimg, 200, 255, CV_THRESH_BINARY);
				if (element_type > 7) {
				}
				else {
					morphologyEx(theraimg, elemimg, element_type, element);
					imshow("niubi", elemimg);
				}
				findContours(elemimg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
				vector<Rect> boundRect2(contours.size());
				for (int i = 0; i < contours.size(); i++)
				{
					boundRect2[i] = boundingRect(Mat(contours[i]));

					if (boundRect2[i].width > 80 && boundRect2[i].height > 80 && boundRect2[i].width < 200 && boundRect2[i].height <200)
					{
						dst_hash = calPHashCode(binarybackof(boundRect2[i]));
						hanming_distance = calHammingDistance(hash_src, dst_hash);
						transpose(binarybackof(boundRect2[i]), tra_dst);
						dst_hash = calPHashCode(tra_dst);
						transp_hanming_distance= calHammingDistance(hash_src, dst_hash);
						if (hanming_distance < 8 || transp_hanming_distance<8)
						{
							smallrect_center.x = boundRect2[i].x + boundRect2[i].width / 2;
							smallrect_center.y = boundRect2[i].y + boundRect2[i].height / 2;
							rectangle(srccopyimg, boundRect2[i], Scalar(78, 0, 160), 2, 8, 0);
							memset(nam_path, 0, sizeof(nam_path));
							if (smallrect_center.x < center_big.x && smallrect_center.y > center_big.y)
							{
								sprintf(nam_path,"true %f ", center_angle);
								putText(srccopyimg, nam_path, center_big,FONT_HERSHEY_PLAIN,2.0, Scalar(78, 0, 160), 2, 8, 0);
							}
							else if(smallrect_center.x < center_big.x && smallrect_center.y < center_big.y)
							{
								sprintf(nam_path, "top left  %f ", center_angle);
								putText(srccopyimg, nam_path, center_big,FONT_HERSHEY_PLAIN, 2.0, Scalar(78, 0, 160), 2, 8, 0);
							}
							else if (smallrect_center.x > center_big.x && smallrect_center.y < center_big.y)
							{
								sprintf(nam_path, "top right %f ", center_angle);
								putText(srccopyimg, nam_path, center_big,FONT_HERSHEY_PLAIN, 2.0, Scalar(78, 0, 160), 2, 8, 0);
							}
							else if (smallrect_center.x > center_big.x && smallrect_center.y > center_big.y)
							{
								sprintf(nam_path, "bottom right %f ", center_angle);
								putText(srccopyimg, nam_path, center_big,FONT_HERSHEY_PLAIN, 2.0, Scalar(78, 0, 160), 2, 8, 0);
							}
							else
							{
								putText(srccopyimg, " ERROR", center_big,FONT_HERSHEY_PLAIN, 2.0, Scalar(78, 0, 160), 2, 8, 0);
							}
						}//if hanming_distance
					}
			     }//for

				imshow("ratio", srccopyimg);
				waitkey_num = waitKey(50);
				if (waitkey_num == 'q' || waitkey_num=='Q')
				{
					destroyAllWindows();
					break;
				}
			
			
			}//while
		
			
		}//else
	}//while

	getchar();
	getchar();
	return 0;
}



int main()
{
	Mat hash_image = imread("rect_48.jpg", 0);
	Mat hash_src = calPHashCode(hash_image);
	Mat tra_dst, dst_hash;

	Mat  srcimage, binary, theraimg, elemimg;
	Mat binarycopyimg;
	Mat srccopyimg;
	char filepath[200];

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	int waitkey_num = 0;
	int retangle_num = 0;

	Point center_big;
	Point small_cenetr;
	char nam_path[200];
	Rect small_rect;
	int hanming_distance = 0;

	while (1) {
		printf("please in put one jpg  and q or Q is exit \n");
		memset(filepath, 0, sizeof(filepath));
		scanf("%s", filepath);
		if (strcmp(filepath, "q") == 0 || strcmp(filepath, "Q") == 0)
		{
			printf("ready to exit \n");
			break;
		}
		else
		{
			printf("filepath is %s \n", filepath);
			srcimage = imread(filepath);
			cvtColor(srcimage, binarycopyimg, CV_RGB2GRAY);
			InitControlBoard();
			Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
			while (1) {
				srccopyimg = srcimage.clone();
				binary = binarycopyimg.clone();
				threshold(binary, theraimg, 65, 255, CV_THRESH_BINARY);
				findContours(theraimg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
				vector<Rect> boundRect(contours.size());
				retangle_num = 0;
				for (int i = 0; i < contours.size(); i++)
				{
					boundRect[i] = boundingRect(Mat(contours[i]));
					rect = minAreaRect((contours[i]));
					//boundRect[i] = boundingRect(Mat(contours[i]));
					cvBoxPoints(rect, Corners);

					if (boundRect[i].width > 400 && boundRect[i].height > 400)
					{
						rectangle(srccopyimg, boundRect[i], Scalar(78, 135, 0), 2, 8, 0);
						retangle_num++;
						//bigrect_center.x = boundRect[i].x + boundRect[i].width / 2;
						//bigrect_center.y = boundRect[i].y + boundRect[i].height / 2;
						center_big = rect.center;
						center_angle = rect.angle;
					}
				}//for

				if (retangle_num != 1)
				{
					printf("param set error \n");
					break;
				}
				threshold(binary, theraimg, threadshold, 255, CV_THRESH_BINARY);
				//imshow("ninary", binary);
				if (element_type > 7) {
				}
				else {
					morphologyEx(theraimg, elemimg, element_type, element);
					imshow("niubi", elemimg);
				}
				findContours(elemimg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
				vector<Rect> boundRect2(contours.size());
				retangle_num = 0;
				for (int i = 0; i < contours.size(); i++)
				{
					boundRect2[i] = boundingRect(Mat(contours[i]));
					rect = minAreaRect((contours[i]));
					//boundRect[i] = boundingRect(Mat(contours[i]));
					cvBoxPoints(rect, Corners);
										
					if (boundRect2[i].width > 80 && boundRect2[i].height > 80 && boundRect2[i].width < 200 && boundRect2[i].height <200)
					{
						retangle_num++;
						rectangle(srccopyimg, boundRect2[i], Scalar(78, 135, 0), 2, 8, 0);
					/*	retangle_num++;
						bigrect_center.x = boundRect[i].x + boundRect[i].width / 2;
						bigrect_center.y = boundRect[i].y + boundRect[i].height / 2;
						center_big = rect.center;
						center_angle = rect.angle;*/
						small_cenetr.x = boundRect2[i].x + boundRect2[i].width / 2;
						small_cenetr.y = boundRect2[i].y + boundRect2[i].height / 2;
						small_rect = boundRect2[i];
					}				
					}//for
				if (retangle_num != 1)
				{
					printf("error \n");
					break;
				}

				memset(nam_path, 0,sizeof(nam_path));
				if (small_cenetr.x > center_big.x && small_cenetr.y > center_big.y)
				{
					//sprintf(nam_path, " True " );
					dst_hash = calPHashCode(binary(small_rect));
					hanming_distance = calHammingDistance(hash_src, dst_hash);
					if (hanming_distance < 8)
					{
						sprintf(nam_path, " True ");
					}
					else
					{
						sprintf(nam_path, " True  error ");
					}
					//putText(srccopyimg, nam_path, center_big, FONT_HERSHEY_PLAIN, 2.0, Scalar(78, 0, 160), 2, 8, 0);
				}
				else if (small_cenetr.x < center_big.x && small_cenetr.y > center_big.y)
				{
					tra_dst = binary(small_rect);
					transpose(tra_dst, tra_dst);
					flip(tra_dst, tra_dst, 0);
					dst_hash = calPHashCode(tra_dst);
					if (hanming_distance < 8)
					{
						sprintf(nam_path, " bottom left  270 ");
					}
					else
					{
						sprintf(nam_path, " bottom left  error ");
					}
				}
				else if (small_cenetr.x < center_big.x && small_cenetr.y < center_big.y)
				{
					tra_dst = binary(small_rect);
					flip(tra_dst, tra_dst, 0);
					flip(tra_dst, tra_dst, 1);
					dst_hash = calPHashCode(tra_dst);
					if (hanming_distance < 8)
					{
						sprintf(nam_path, " top left  180 ");
					}
					else
					{
						sprintf(nam_path, " top left  error ");
					}
				}
				else if (small_cenetr.x > center_big.x && small_cenetr.y < center_big.y)
				{
					tra_dst = binary(small_rect);
					transpose(tra_dst, tra_dst);
					dst_hash = calPHashCode(tra_dst);
					if (hanming_distance < 8)
					{
						sprintf(nam_path, " top right  90  ");
					}
					else
					{
						sprintf(nam_path, " top right  error  ");
					}
				}
				else {
					sprintf(nam_path, " detect   error  ");
				}

				putText(srccopyimg, nam_path, center_big, FONT_HERSHEY_PLAIN, 2.0, Scalar(78, 0, 160), 2, 8, 0);


				imshow("ratio", srccopyimg);
				waitkey_num = waitKey(10);
				if (waitkey_num == 'q' || waitkey_num == 'Q')
				{
					destroyAllWindows();
					break;
				}
			}
		}

		

	}

	getchar();
	getchar();
	return 0;
}


int dgremain()
{
	Mat srcimage = imread("rect_48.jpg");
	Mat dst_one, dst_two, dst_three;

	int waitkey_num = 0;
	while (1)
	{

		imshow("srcimage", srcimage);

		transpose(srcimage, dst_one);
		imshow("night90", dst_one);

		flip(srcimage,dst_two,0);
		flip(dst_two, dst_two, 1);
		imshow("eghi180", dst_one);

		transpose(srcimage, dst_three);
		flip(dst_three, dst_three, 0);
		imshow("twight180", dst_three);

		waitkey_num = waitKey(10);
		if (waitkey_num == 'q' || waitkey_num == 'Q') {
			destroyAllWindows();
			break;
		}
	}

	return 0;
	
}


