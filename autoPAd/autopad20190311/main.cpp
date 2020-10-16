
#include <opencv2\core\core.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<opencv2\highgui\highgui.hpp>
#include "windows.h"
#include "dshow.h"
#include <iostream>

using namespace cv;
using namespace std;


int i_threshold = 180;
int threashold_two = 200;


int rect_x = 190;
int rect_y = 170;

static void ContrastAndBright(int, void *)
{
	Mat m = Mat(800, 600, 0);
	imshow("【控制面板】", m);
}

int small_back_rect_new_x = 36;
int small_back_rect_new_y = 9;


void InitControlBoard()
{
	namedWindow("【控制面板】", 0);
	createTrackbar("二值化阈值：", "【控制面板】", &i_threshold, 256, ContrastAndBright);
	createTrackbar("rect_x：", "【控制面板】", &rect_x, 256, ContrastAndBright);
	createTrackbar("rect_y：", "【控制面板】", &rect_y, 256, ContrastAndBright);
	createTrackbar("small_back_rect_new_x：", "【控制面板】", &small_back_rect_new_x, 80, ContrastAndBright);
	createTrackbar("small_back_rect_new_y：", "【控制面板】", &small_back_rect_new_y, 20, ContrastAndBright);
}

Mat third_mask_circle;
int  third_mask_circle_d = 32;

//创建第三个摄像头的掩码
void Third_mask_made()
{
	Mat mask;
	mask = Mat(Size(third_mask_circle_d + 2, third_mask_circle_d + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(third_mask_circle_d / 2, third_mask_circle_d / 2), third_mask_circle_d / 2, Scalar(255), 1);
	third_mask_circle = mask.clone();
	//imwrite("third.jpg", third_mask_circle);
}

//模版匹配 圆
Point MatchCircleTemplate(Mat bitwise_out, Mat templ, int circle_d)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//进行匹配
	matchTemplate(bitwise_out, templ, g_resultImage, 4);

	//标准化
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;

	//定位最匹配的位置
	minMaxLoc(g_resultImage, &minValue, &maxValue, &minLocation, &maxLocation);
	if (4 == TM_SQDIFF || 4 == CV_TM_SQDIFF_NORMED)
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}

	Point p_re;
	p_re.x = matchLocation.x + circle_d / 2;
	p_re.y = matchLocation.y + circle_d / 2;
	return p_re;
}

int CIRclemain()
{
	Mat videoimage;
	Mat videoimage2;
	Mat filter, canny, wavefilter;
	Mat blue, out;
	Point center;

	InitControlBoard();
	VideoCapture  capture2(1);
	vector<Vec4i> lines;

	Third_mask_made();

	int true_threshold = 0;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	while (1)
	{
		//capture >> videoimage; 
		capture2 >> videoimage2;
		//imshow("videoimage", videoimage);
		if (!videoimage2.data)
		{
			printf("error \n");
			break;
		}
		
		videoimage2 = videoimage2(Rect(190, 30, 390, 390));
		out = videoimage2.clone();

		cvtColor(videoimage2, videoimage2, CV_RGB2GRAY);
		medianBlur(videoimage2, videoimage2, 5);
		//imshow("videoimage2", videoimage2);

		adaptiveThreshold(videoimage2, blue, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 3, 5);
		imshow("blue", blue);

		center = MatchCircleTemplate(blue, third_mask_circle, third_mask_circle_d);
		circle(out, Point(center.x, center.y), third_mask_circle_d / 2, Scalar(255, 0, 255), 1);
		printf("center.x =  %d, center.y = %d \n", center.x, center.y);
		imshow("out",out);

		waitKey(10);
	}
	getchar();
	return 0;
}

Mat black_recentangle_mask;
int black_recentangle_mask_x = 48;
int black_recentangle_mask_y = 10;

//创建第三个摄像头的掩码
void Rectangle_mask_made()
{
	Mat mask;
	mask = Mat(Size(black_recentangle_mask_x + 2, black_recentangle_mask_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(0, 0), Point(black_recentangle_mask_x + 1, black_recentangle_mask_y + 1), Scalar(255), 1);
	black_recentangle_mask = mask.clone();
}

//模板匹配 rectangle 矩形
Point MatchRectangleTemplate(Mat bitwise_out, Mat templ)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//进行匹配
	matchTemplate(bitwise_out, templ, g_resultImage, 4);

	//标准化
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;

	//定位最匹配的位置
	minMaxLoc(g_resultImage, &minValue, &maxValue, &minLocation, &maxLocation);
	if (4 == TM_SQDIFF || 4 == CV_TM_SQDIFF_NORMED)
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}

	Point p_re;
	p_re.x = matchLocation.x;
	p_re.y = matchLocation.y;
	return p_re;
}

int Rtmain()
{
	Mat videoimage;
	Mat videoimage2;
	Mat filter, canny, wavefilter;
	Mat blue, out;
	Point center;

	InitControlBoard();
	VideoCapture  capture2(1);
	vector<Vec4i> lines;

	Third_mask_made();
	Rectangle_mask_made();
	int true_threshold = 0;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	while (1)
	{
		//capture >> videoimage; 
		capture2 >> videoimage2;
		//imshow("videoimage", videoimage);
		if (!videoimage2.data)
		{
			printf("error \n");
			break;
		}

		videoimage2 = videoimage2(Rect(190, 30, 390, 390));
		out = videoimage2.clone();

		cvtColor(videoimage2, videoimage2, CV_RGB2GRAY);

		true_threshold = threashold_two;
		if (true_threshold <= 0 || true_threshold >= 255)
		{
			true_threshold = 180;
		}
		threshold(videoimage2, blue, true_threshold, 255, CV_THRESH_BINARY);
		imshow("blue", blue);
		//morphologyEx(blue, wavefilter, MORPH_DILATE, element);
		//imshow("wavefilter", wavefilter);
		center = MatchRectangleTemplate(blue, black_recentangle_mask);
		///chip_center_pointer = center;
		printf("center,x = %d, center.y = %d \n");
		rectangle(out, center, Point(center.x + black_recentangle_mask_x, center.y + black_recentangle_mask_y), Scalar(255, 0, 255), 1);
		imshow("out",out);
		waitKey(10);
	}
	getchar();
	return 0;
}

//模板匹配 rectangle 矩形
Point MatchRectangleTemplateOne(Mat bitwise_out, Mat templ)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//进行匹配
	matchTemplate(bitwise_out, templ, g_resultImage, 4);

	//标准化
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;

	//定位最匹配的位置
	minMaxLoc(g_resultImage, &minValue, &maxValue, &minLocation, &maxLocation);
	if (4 == TM_SQDIFF || 4 == CV_TM_SQDIFF_NORMED)
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}

	Point p_re;
	p_re.x = matchLocation.x;
	p_re.y = matchLocation.y;
	return p_re;
}

Mat out_mask_img;
int out_mask_img_x = 180;  //175
int out_mask_img_y = 185;  //181

Mat back_small;
int back_small_x = 37; // 35
int back_samll_y = 9;  //8

//创建一个最大外矩形
void Max_Rectrectangle_mask()
{
	Mat mask;
	mask = Mat(Size(out_mask_img_x + 2, out_mask_img_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(1, 1), Point(out_mask_img_x + 1, out_mask_img_y + 1), Scalar(255), 1);
	out_mask_img = mask.clone();
}

//创建一个小黑的矩形
void Back_mask()
{
	Mat mask;
	mask = Mat(Size(back_small_x + 2, back_samll_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(0, 0), Point(back_small_x + 1, back_samll_y + 1), Scalar(255), 1);
	back_small = mask.clone();
	imwrite("matchpiature.jpg",back_small);
}

int FRmain()
{
	Mat videoimage;
	VideoCapture capture(1);
	Mat  binaryimage , threImage, resultImage, out, canny_image;
	int true_threshold = 0;
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	Point center;
	Point src_location;
	Max_Rectrectangle_mask();
	Back_mask();
	while (1)
	{
		capture >> videoimage;
		if (!videoimage.data)
		{
			printf("error \n");
			break;
		}
		imshow("src", videoimage);
		videoimage = videoimage(Rect(204, 76, 260, 260));
		cvtColor(videoimage, binaryimage, CV_RGB2GRAY);
		true_threshold = i_threshold;
		if (true_threshold <= 0 || true_threshold >= 255)
		{
			true_threshold = 180;
		}
		threshold(binaryimage, threImage, true_threshold, 255, CV_THRESH_BINARY);
		resultImage = threImage.clone();
		//imshow("resultImage", resultImage);
		morphologyEx(threImage, out, MORPH_ERODE, element);
		//imshow("threshold", out);
		Canny(out, canny_image, 50, 100, 3);
		//imshow("src_xytr", canny_image);
		center = MatchRectangleTemplateOne(canny_image, out_mask_img);
		//rectangle(VideoImage, center, Point(center.x + i_img_mask_max_rect_x, center.y + i_img_mask_max_rect_y), Scalar(255, 0, 255), 1);
		resultImage = resultImage(Rect(center.x, center.y, out_mask_img_x, out_mask_img_y));
		src_location = MatchRectangleTemplateOne(resultImage, back_small);
		/*point_src.x = src_location.x + center.x;
		point_src.y = src_location.y + center.y;
		center_point = point_src;*/
		waitKey(20);
	}
	getchar();
	return 0;
}

int HEmain()
{
	Mat iamge;
	VideoCapture capture(1);
	while (1)
	{
		capture >> iamge;
		imshow("niu_bi", iamge);
		waitKey(10);
	}
	return 0;
}

Mat animal_mask_image;
int animal_mask_image_x = 34;
int animal_mask_image_y = 9;

void Rectangle_animal_mask_made()
{
	Mat mask;
	mask = Mat(Size(animal_mask_image_x + 2, animal_mask_image_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(0, 0), Point(animal_mask_image_x + 1, animal_mask_image_y + 1), Scalar(255), 1);
	animal_mask_image = mask.clone();
}

int FFmain()
{
	Mat image; // = imread("xyz600.jpg")
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	CvBox2D rect;
	CvPoint2D32f Corners[4];
	Mat out;
	Mat  linary,mthresd, luo;
	Point center;
	Rectangle_animal_mask_made();
	VideoCapture capture(1);
	InitControlBoard();
	while (1)
	{  
		capture >> image;
		if (!image.data)
		{
			printf("error  please check you video \n");
			break;
		}
		image = image(Rect(rect_x, rect_y, 260, 230));
		out = image.clone();
		cvtColor(image, linary, CV_RGB2GRAY);
		imshow("image", linary);
		threshold(linary, mthresd, 180, 255, CV_THRESH_BINARY);
		imshow("mthresd", mthresd);
		Mat element = getStructuringElement(MORPH_RECT, Size(5,5));
		morphologyEx(mthresd, luo, MORPH_DILATE, element);
		imshow("luo", luo);
		findContours(luo, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point()); //CV_RETR_EXTERNAL
		for (int i = 0; i < contours.size(); i++)
		{
			rect = minAreaRect((contours[i]));

			cvBoxPoints(rect, Corners);
			if (rect.size.width  * rect.size.height > 10000 &&
				rect.size.width > 100 &&
				rect.size.height > 100) {
				//angle = rect.angle;
				//center = rect.center;
				cout << "矩形中心点   rect.center.x  =    " << rect.center.x << "     rect.center.y = " << rect.center.y << endl;
				line(out, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[1].x, (int)Corners[1].y), Scalar(255, 0, 255), 2, 8, 0);
				line(out, Point((int)Corners[1].x, (int)Corners[1].y), Point((int)Corners[2].x, (int)Corners[2].y), Scalar(255, 0, 255), 2, 8, 0);
				line(out, Point((int)Corners[2].x, (int)Corners[2].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
				line(out, Point((int)Corners[0].x, (int)Corners[0].y), Point((int)Corners[3].x, (int)Corners[3].y), Scalar(255, 0, 255), 2, 8, 0);
			}
		}

		center = MatchRectangleTemplate(mthresd, animal_mask_image);
		rectangle(out, center, Point(center.x + animal_mask_image_x, center.y + animal_mask_image_y), Scalar(255, 0, 255), 1);
		cout << "芯片标志位  center.x  =    " << center.x << "   center.y  =    " << center.y << endl;
		imshow("out",out);
		waitKey(20);
	}

	getchar();
	return 0;
}


int listDevices(vector<string>& list) {

	//COM Library Initialization
	//comInit();

	//if (!silent) DebugPrintOut("\nVIDEOINPUT SPY MODE!\n\n");

	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;
	CoInitialize(NULL);

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));


	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK) {

			printf("SETUP: Looking For Capture Devices\n");
			IMoniker *pMoniker = NULL;

			while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {

				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)) {
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}


				// Find the description or friendly name.
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"Description", &varName, 0);

				if (FAILED(hr)) hr = pPropBag->Read(L"FriendlyName", &varName, 0);

				if (SUCCEEDED(hr))
				{

					hr = pPropBag->Read(L"FriendlyName", &varName, 0);

					int count = 0;
					char tmp[255] = { 0 };
					//int maxLen = sizeof(deviceNames[0]) / sizeof(deviceNames[0][0]) - 2;
					while (varName.bstrVal[count] != 0x00 && count < 255)
					{
						tmp[count] = (char)varName.bstrVal[count];
						count++;
					}
					list.push_back(tmp);
					//deviceNames[deviceCounter][count] = 0;

					//if (!silent) DebugPrintOut("SETUP: %i) %s\n", deviceCounter, deviceNames[deviceCounter]);
				}

				pPropBag->Release();
				pPropBag = NULL;

				pMoniker->Release();
				pMoniker = NULL;

				deviceCounter++;
			}

			pDevEnum->Release();
			pDevEnum = NULL;

			pEnum->Release();
			pEnum = NULL;
		}

		//if (!silent) DebugPrintOut("SETUP: %i Device(s) found\n\n", deviceCounter);
	}

	//comUnInit();

	return deviceCounter;
}

int FindVideomain()
{
	vector<string> list;
	listDevices(list);
	cout <<  "	找到摄像头设备数量    =   " << list.size() << endl;
	for (int i = 0; i<list.size(); i++)
	{
		cout << "	设备ID	=	" << i <<"	设备名称	=  " << list[i] << endl;
	}
	getchar();
	return 0;
}

int Whatmain()
{
	Mat image = imread("xyz600.jpg");
	image = image(Rect(180,180,260,220));
	Mat binaryimage, threImage,out;
	Mat canny_image;
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	while (1)
	{
		cvtColor(image, binaryimage, CV_RGB2GRAY);
		/*true_threshold = threashold_one;
		if (true_threshold <= 0 || true_threshold >= 255)
		{
			true_threshold = 180;
		}*/
		threshold(binaryimage, threImage, 180, 255, CV_THRESH_BINARY);
		imshow("threImage", threImage);
		//resultImage = threImage.clone();
		morphologyEx(threImage, out, MORPH_ERODE, element);
		imshow("threshold", out);
		Canny(out, canny_image, 50, 100, 3);
		imshow("src_xytr", canny_image);
		//center = MatchRectangleTemplateOne(canny_image, out_mask_img);
		waitKey(20);
	}
	return 0;
}

int threashold_one = 180;
//这个是用来测试摄像头1的区域限定范围
int BUGmain()
{
	Mat src = imread("XYZ.jpg");
	Mat src1;
	Mat binaryimage, threImage , resultImage, out, canny_image;
	int index;
	int true_threshold;
	Point center, src_location, point_src;
	/*printf("please  input  zhe camera  device  index  \n");
	scanf("%d", &index);
	VideoCapture capture(index);*/
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	Max_Rectrectangle_mask();
	Back_mask();
	while (1)
	{
		//capture >> src;
		//if (!src.data)
		//{
		//	//如何报警没有摄像头
		//	printf("error    摄像头打开错误     \n");
		//	break;
		//}
		src1 = src.clone();
		src1 = src1(Rect(170,130,256,256));
		cvtColor(src1, binaryimage, CV_RGB2GRAY);
		true_threshold = threashold_one;
		if (true_threshold <= 0 || true_threshold >= 255)
		{
			true_threshold = 180;
		}
		threshold(binaryimage, threImage, 180, 255, CV_THRESH_BINARY);
		imshow("threImage", threImage);
		resultImage = threImage.clone();
	/*	morphologyEx(threImage, out, MORPH_ERODE, element);
		imshow("out",out);
		Canny(out, canny_image, 50, 100, 3);
		imshow("canny_image", canny_image);
		center = MatchRectangleTemplateOne(canny_image, out_mask_img);
		rectangle(src1, center, Point(center.x + out_mask_img_x, center.y + out_mask_img_y), Scalar(255, 0, 255), 1);
		resultImage = resultImage(Rect(center.x, center.y, out_mask_img_x, out_mask_img_y));*/
		src_location = MatchRectangleTemplateOne(resultImage, back_small);
		//point_src.x = src_location.x + center.x;
		//point_src.y = src_location.y + center.y;
		rectangle(src1, src_location, Point(src_location.x + back_small_x, src_location.y + back_samll_y), Scalar(255, 0, 255), 1);

		imshow("src1",src1);
		waitKey(20);
	}
	getchar();
	return 0;
}

Mat  small_back_rect_new;
int matchtemple;

//不知道为什么的问题
void Back_Mask_Small()
{
	Mat mask;
	mask = Mat(Size(small_back_rect_new_x + 2, small_back_rect_new_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(0, 0), Point(small_back_rect_new_x + 1, small_back_rect_new_y + 1), Scalar(255), 1);
	small_back_rect_new = mask.clone();
	//imwrite("matchpiature.jpg", back_small);
}

//这个是用来检测模板匹配的
int main()
{
	Mat src = imread("HELP_Mth.jpg");
	//src = src(Rect(150, 130, 280, 280));
	Mat  binaryimage, threImage, resultImage;
	Mat out;
	Point src_location;
	InitControlBoard();
	while (1)
	{
		out = src.clone();
		cvtColor(src, binaryimage, CV_RGB2GRAY);
		Back_Mask_Small();
		threshold(binaryimage, threImage, 180, 255, CV_THRESH_BINARY);
		imshow("threImage", threImage);
		resultImage = threImage.clone();
		src_location = MatchRectangleTemplateOne(threImage, small_back_rect_new);
		rectangle(out, src_location, Point(src_location.x + small_back_rect_new_x, src_location.y + small_back_rect_new_y), Scalar(255, 0, 255), 1);
		imshow("out",out);
		waitKey(20);
	}
	getchar();
	return 0;
}
