
// MFCApplication1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include "opencv2\xfeatures2d\nonfree.hpp" 
#include <opencv2\features2d\features2d.hpp>  
#include "opencv2\calib3d\calib3d_c.h"
#include "opencv2\calib3d.hpp"
#include <tesseract/baseapi.h> 
 

using std::vector;
using namespace cv;
using namespace std;
using namespace tesseract;


CSliderCtrl   *pSlidCtrl;
int slider1;
string reconiseStr;
string imgPath;
string maskImgPathStr = "mask_mac_max.jpg";
TessBaseAPI tessBaseAPI;
Mat sliderImg[50];//每个标签
Mat maskImg;
float rotateMinAngle = 0.3;//旋转矫正最低容忍角度（低于该值不做校正处理）
int slideMinPx = 400;//标签尺寸范围（像素）
int slideMaxPx = 700;

int silideNum = 0;
float silieMul = 1.1;//实际切割大小(倍)

// CMFCApplication1Dlg 对话框
CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CMFCApplication1Dlg::OnNMCustomdrawSlider1)
	ON_BN_CLICKED(IDC_BUTTON5, &CMFCApplication1Dlg::OnBnClickedButton5)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CMFCApplication1Dlg 消息处理程序
BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_NORMAL);

	// TODO: 在此添加额外的初始化代码

	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	pSlidCtrl->SetRange(0, 65, TRUE);
	pSlidCtrl->SetPos(42);
	slider1 = 42;
	
	GetDlgItem(IDC_EDIT1)->SetWindowText(_T("将图片拖动到此处，然后点击开始识别。"));
	//tranning库选择
	tessBaseAPI.Init(NULL, "num", OEM_DEFAULT);
	tessBaseAPI.SetPageSegMode(PSM_SINGLE_BLOCK);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMFCApplication1Dlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	slider1 = ((CSliderCtrl *)GetDlgItem(IDC_SLIDER1))->GetPos();
	CString str;
	str.Format(_T("%d"), slider1);
	GetDlgItem(IDC_STATIC)->SetWindowText(str);
	*pResult = 0;

}



string TCHAR2STRING(TCHAR *STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen*sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
	std::string str(chRtn);
	return str;
}

void CMFCApplication1Dlg::OnDropFiles(HDROP hDropInfo)
{
	UINT nCount;
	TCHAR szPath[MAX_PATH];

	nCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	if (nCount)
	{
		for (UINT nIndex = 0; nIndex < nCount; ++nIndex)
		{
			DragQueryFile(hDropInfo, nIndex, szPath, _countof(szPath));
			imgPath = TCHAR2STRING(szPath);
			CString cstr(imgPath.c_str());
			GetDlgItem(IDC_EDIT1)->SetWindowText(cstr);
		}
	}
	DragFinish(hDropInfo);
	CDialogEx::OnDropFiles(hDropInfo);
}


//旋转图像（矫正）
void imRoTate(Mat& img, Mat& newIm, double angle) {
	int len = max(img.cols, img.rows);
	Point2f pt(len / 2., len / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(img, newIm, r, Size(img.cols, img.rows));
}

//进一步匹配，寻找源图与目标图像之间的透视变换
bool refineMatchesWithHomography(const vector<KeyPoint>& queryKeypoints, const vector<KeyPoint>& trainKeypoints,
	float reprojectionThreshold, vector<DMatch>& matches, Mat& homography)
{
	const int minNumberMatchesAllowed = 8;
	if (matches.size() < minNumberMatchesAllowed)
	{
		return false;
	}

	vector<Point2f> srcPoints(matches.size());
	vector<Point2f> dstPoints(matches.size());
	for (size_t i = 0; i < matches.size(); i++)
	{
		srcPoints[i] = trainKeypoints[matches[i].trainIdx].pt;
		dstPoints[i] = queryKeypoints[matches[i].queryIdx].pt;
		//srcPoints[i] = trainKeypoints[i].pt;  
		//dstPoints[i] = queryKeypoints[i].pt;  
	}
 
	std::vector<unsigned char> inliersMask(srcPoints.size());
	//通过筛选匹配关键点之间的转换，找到两个平面（图像）之间的透视变换
	homography = findHomography(srcPoints, dstPoints, CV_FM_RANSAC, reprojectionThreshold, inliersMask);
	vector<DMatch> inliers;
	//查找，得到正确的匹配结果  
	for (size_t i = 0; i<inliersMask.size(); i++)
	{
		if (inliersMask[i])
			inliers.push_back(matches[i]);
	}
	matches.swap(inliers);
	return matches.size() > minNumberMatchesAllowed;
}

//截取最后的n个字符
char* substrend(char *str, int n)
{
	char * substr = (char*)malloc(n + 1);
	int length = strlen(str);
	if (n >= length)//若截取长度大于字符串长度，则直接截取全部字符串
	{
		strcpy(substr, str);
		return substr;
	}
	int k = 0;
	for (int i = strlen(str) - n - 1; i < strlen(str); i++)
	{
		substr[k++] = str[i];
	}
	substr[k] = '\0';
	return substr;
}


Mat slidMac(Mat image)
{
	cvtColor(image, image, CV_BGR2GRAY);
	int blockSize = 7;
	int constValue = 11;
	Mat local;
	adaptiveThreshold(image, local, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, blockSize, constValue);

	int width = local.cols;
	int height = local.rows;
	int perPixelValue;//每个像素的值

	int* projectValArrys = new int[height];
	memset(projectValArrys, 0, height * 4);//必须初始化数组

										   //横向统计
	for (int row = 0; row < height; ++row)
	{
		for (int col = 0; col < width; ++col)
		{
			perPixelValue = local.at<uchar>(row, col);
			if (perPixelValue == 255)
			{
				projectValArrys[row]++;
			}
		}
	}

	/*新建一个Mat用于储存投影直方图并将背景置为白色*/
	Mat verticalProjectionMats(height, width, CV_8UC1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			verticalProjectionMats.at<uchar>(i, j) = 255;
		}
	}

	/*将直方图的曲线设为黑色*/
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < projectValArrys[i]; j++)
		{
			verticalProjectionMats.at<uchar>(i, j) = 0;
		}
	}

	int startPosition;
	int endPosition;
	int startPositions;
	int endPositions;
	int wid = 0;
	int widMax = 0;
	int sub = 0;
	for (int i = 0; i < height; i++)
	{
		sub += projectValArrys[i];
	}
	sub /= height;
	sub *= 0.8;
	for (int i = 0; i < height; i++)
	{
		if (verticalProjectionMats.at<uchar>(i, sub) == 0)
		{
			wid++;
			startPosition = i;
		}
		else {
			wid = 0;
			endPosition = i;
		}

		if (widMax <= wid)
		{
			widMax = wid;
			startPositions = startPosition;
			endPositions = endPosition;
		}
	}
	Rect rc;
	rc.x = 0;
	rc.y = endPositions;
	rc.width = width;
	rc.height = startPositions - endPositions + 1;
	delete[] projectValArrys;
	return  image(rc);
}

//识别Mac
string recoMacForEachCharater(Mat image)
{
	cvtColor(image, image, CV_RGB2GRAY);
	int blockSize = 7;
	int constValue = 11;
	Mat local;
	adaptiveThreshold(image, local, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, blockSize, constValue);

	int width = local.cols;
	int height = local.rows;
	int perPixelValue;//每个像素的值

	int* projectValArrys = new int[height];
	memset(projectValArrys, 0, height * 4);//必须初始化数组

	//横向统计
	for (int row = 0; row < height; ++row)
	{
		for (int col = 0; col < width; ++col)
		{
			perPixelValue = local.at<uchar>(row, col);
			if (perPixelValue == 255)
			{
				projectValArrys[row]++;
			}
		}
	}

	//新建一个Mat用于储存投影直方图并将背景置为白色
	Mat verticalProjectionMats(height, width, CV_8UC1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			verticalProjectionMats.at<uchar>(i, j) = 255;
		}
	}

	//将直方图的曲线设为黑色
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < projectValArrys[i]; j++)
		{
			verticalProjectionMats.at<uchar>(i, j) = 0;
		}
	}

	int startPosition;
	int endPosition;
	int startPositions;
	int endPositions;
	int wid = 0;
	int widMax = 0;
	int sub = 0;
	for (int i = 0; i < height; i++)
	{
		sub += projectValArrys[i];
	}
	sub /= height;
	sub *= 0.8;
	for (int i = 0; i < height; i++)
	{
		if (verticalProjectionMats.at<uchar>(i, sub) == 0)
		{
			wid++;
			startPosition = i;
		}
		else {
			wid = 0;
			endPosition = i;
		}

		if (widMax <= wid)
		{
			widMax = wid;
			startPositions = startPosition;
			endPositions = endPosition;
		}
	}
	Rect rc;
	rc.x = 0;
	rc.y = endPositions;
	rc.width = width;
	rc.height = startPositions - endPositions + 1;
	Mat out = local(rc);
	delete[] projectValArrys;

	string outStr;
	Mat macImg[32];
	int symbolNum = 0;
	int imgleft = 0;
	int imgrigth = 0;

	int i;
	for (i = 0; i < out.cols; i++)//空
	{
		long colValue = 0;
		for (int j = 0; j < out.rows; j++)
		{
			colValue += out.at<uchar>(j, i);
		}
		if (colValue == 0)
		{
			break;
		}
	}

	int colValue;
	while (i < out.cols)
	{
		for (; i < out.cols; i++)//左边界
		{
			colValue = 0;
			for (int j = 0; j < out.rows; j++)
			{
				colValue += out.at<uchar>(j, i);
			}
			if (colValue != 0)
			{
				imgleft = i;
				break;
			}
		}

		for (; i < out.cols; i++)//中心
		{
			colValue = 0;
			for (int j = 0; j < out.rows; j++)
			{
				colValue += out.at<uchar>(j, i);
			}
			if (colValue == 0)
			{
				imgrigth = i;
				break;
			}
		}

		if (imgrigth - imgleft > 2)
		{
			Rect rec = Rect(Point2d(imgleft, 0), Size(imgrigth - imgleft, out.rows));
			Mat roiImg = out(rec);
			macImg[symbolNum] = roiImg.clone();
			symbolNum++;
		}
	}

	symbolNum--;

	if (symbolNum < 16)
	{
		return "null";
	}

	Mat macImgOut[17];
	if (symbolNum == 16)
	{
		for (int i = 0; i < 17; i++)
		{
			macImgOut[i] = macImg[symbolNum--].clone();
		}
	}
	else
	{
		for (int i = 0; i < 17; i++)
		{
			macImgOut[i] = macImg[--symbolNum].clone();
		}

	}

	Mat macImgDouble[6];
	for (int i = 0; i < 6; i++)
	{
		Mat logo1 = macImgOut[i * 3].clone();
		Mat logo2 = macImgOut[i * 3 + 1].clone();
		Mat image(out.rows, logo1.cols + logo2.cols, CV_8UC1);
		Mat imageROI1 = image(Rect(0, 0, logo2.cols, logo2.rows));
		logo2.copyTo(imageROI1);

		Mat imageROI2 = image(Rect(logo2.cols, 0, logo1.cols, logo1.rows));
		logo1.copyTo(imageROI2);
		macImgDouble[5 - i] = image.clone();
	}

	for (int i = 0; i < 6; i++)
	{
		tessBaseAPI.SetImage((uchar*)macImgDouble[i].data, macImgDouble[i].cols, macImgDouble[i].rows, 1, macImgDouble[i].cols);
		outStr += tessBaseAPI.GetUTF8Text();
		if (i < 5)
		{
			outStr += ":";
		}
	}
	return outStr;
}

//识别每个切割好的标签
string reconiseSlideForMac(Mat slideOutImg, Mat maskImg)
{
	Mat img_1 = maskImg.clone();
	Mat img_2 = slideOutImg.clone();
	//检测目标以及模板各自的特征点
	vector<KeyPoint> keypoints_1, keypoints_2;
	Ptr<FastFeatureDetector> fast = FastFeatureDetector::create(slider1, true, 1);
	fast->detect(img_1, keypoints_1);
	fast->detect(img_2, keypoints_2);
	//计算各自的特征点描述符（特征向量）
	Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	Mat descriptors_1, descriptors_2;
	f2d->compute(img_1, keypoints_1, descriptors_1);
	f2d->compute(img_2, keypoints_2, descriptors_2);
	//使用BFMatcher匹配描述符
	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptors_1, descriptors_2, matches);
	//进一步匹配，筛选匹配结果
	Mat img_matches;	
	refineMatchesWithHomography(keypoints_1, keypoints_2, 3, matches, img_matches);
	//绘制匹配出的关键点
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_matches);
	//定位准确的匹配点
	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < matches.size(); i++)
	{
		//DMathch类型中queryIdx是指match中第一个数组的索引,keyPoint类型中pt指的是当前点坐标
		obj.push_back(keypoints_1[matches[i].queryIdx].pt);
		scene.push_back(keypoints_2[matches[i].trainIdx].pt);
	}
	//找到目标在图片中的映射矩阵
	Mat H = findHomography(obj, scene, CV_RANSAC);
	vector<Point2f> obj_corners(4), scene_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint(img_1.cols, 0);
	obj_corners[2] = cvPoint(img_1.cols, img_1.rows);
	obj_corners[3] = cvPoint(0, img_1.rows);
	//根据映射矩阵进行透视变换
	perspectiveTransform(obj_corners, scene_corners, H);
	//得到ROI
	Rect silidOutRect;
	silidOutRect.x = scene_corners[1].x;
	silidOutRect.y = scene_corners[1].y + Point2f(img_1.cols, 0).y;
	silidOutRect.width = img_1.cols * 2.3;
	silidOutRect.height = img_1.rows;
	//切割，避免切割时超边界导致内存访问出错
	if (silidOutRect.x + silidOutRect.width < img_2.cols
		&& silidOutRect.y + silidOutRect.height < img_2.rows
		&& silidOutRect.width * silidOutRect.height > 10000)
	{
		//TessBaseAPI，识别字体操作
		string perRectStr = recoMacForEachCharater(img_2(silidOutRect));
		if (perRectStr.length() == 29 )
		{
			return perRectStr;
		}
		else {
			Mat apiGrayImg = slidMac(img_2(silidOutRect));
			tessBaseAPI.SetImage((uchar*)apiGrayImg.data, apiGrayImg.cols, apiGrayImg.rows, 1, apiGrayImg.cols);
			char* out = tessBaseAPI.GetUTF8Text();
			char outMac[18];
			for (int i = 0; i < strlen(out); i++)
			{
				if (out[i] == ':')
				{
					strncpy(outMac, out + i - 2, 18);
					outMac[2] = outMac[5] = outMac[8] = outMac[11] = outMac[14] = ':';
					outMac[17] = '\0';
					break;
				}
			}
			return outMac;
		}
	}
	else {
		return "识别失败";
	}
}


//预处理，切割标签，每个标签存到sliderImg中
int slideImg(Mat inImage)
{
	int sliderNum = 0;
	Mat srcImage = inImage.clone();  //源图像
	if (!srcImage.data)
	{
		return -1;
	}

	//预处理
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

	//Mat openImg = specifiImg(srcImage);

	Mat openImg = srcImage.clone();

	morphologyEx(openImg, openImg, MORPH_OPEN, element);//开运算  

	Mat canny_output;
	Canny(openImg, canny_output, 50, 200, 3);//进行canny边缘检测  	

	//Mat dst = canny_output<100;
	//imwrite("Canny.jpg", canny_output);

	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;
	//矩形检测
	findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	//对每个找到的轮廓创建可倾斜的边界框 
	vector<RotatedRect> minRect(contours.size());
	for (unsigned int i = 0; i < contours.size(); i++)
	{
		minRect[i] = minAreaRect(Mat(contours[i]));
	}
	//绘出倾斜的边界框  
	CvBox2D  End_Rage2D;

	//画出每个矩形框，just for test...
	Mat drawingAllRect = srcImage.clone();
	unsigned int rectNum;
	for (rectNum = 0; rectNum< contours.size(); rectNum++)
	{
		Point2f rect_points[4]; minRect[rectNum].points(rect_points);
		for (char j = 0; j < 4; j++)
		{
			line(drawingAllRect, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 255), 1, 8);
		}
	}
	//imwrite("drawingAllRect.jpg", drawingAllRect);


	for (unsigned int i = 0; i< contours.size(); i++)
	{
		Point2f rect_points[4]; minRect[i].points(rect_points);
		
		//检测到矩形长和宽的比
		float ratio;
		if (minRect[i].size.width > minRect[i].size.height)
		{
			ratio = minRect[i].size.width / minRect[i].size.height;
		}
		else {
			ratio = minRect[i].size.height / minRect[i].size.width;
		}
		//筛选符合条件的标签（矩形）：长宽数值在slideMinPx、slideMaxPx之间
		if (minRect[i].size.width > slideMinPx &&
			minRect[i].size.width < slideMaxPx &&
			minRect[i].size.height > slideMinPx &&
			minRect[i].size.height < slideMaxPx)
		{
			string positionStr[4];

			End_Rage2D = minAreaRect(Mat(contours[i]));
			float angle = End_Rage2D.angle;

			//顺时针倾斜：End_Rage2D.angle = 实际倾斜度数
			//逆时针倾斜：End_Rage2D.angle = -90 + 实际倾斜度数，奇葩
			if (angle < -45)
			{
				angle += 90;
			}
			//小于0度(包括0)的时候各坐标点要变换才能得出ROI区域silidRect，很奇葩
			if (angle <= 0)
			{
				Point2f p2fTemp = rect_points[3];
				rect_points[3] = rect_points[2];
				rect_points[2] = rect_points[1];
				rect_points[1] = rect_points[0];
				rect_points[0] = p2fTemp;
			}

			//切割1.1倍标签大小，防止倾斜大的标签内容会被截掉
			Rect silidRect;
			silidRect.width = (int)sqrt((pow((rect_points[0].x - rect_points[1].x), 2) + pow((rect_points[0].y - rect_points[1].y), 2)));
			silidRect.height = (int)sqrt((pow((rect_points[0].x - rect_points[3].x), 2) + pow((rect_points[0].y - rect_points[3].y), 2)));
			silidRect.x = (int)(rect_points[2].x - silidRect.width*(silieMul - 1) / 2);
			silidRect.y = (int)(rect_points[2].y - silidRect.height*(silieMul - 1) / 2);
			silidRect.width *= silieMul;
			silidRect.height *= silieMul;

			//切割超出边界会导致崩溃
			if (silidRect.x + silidRect.width > srcImage.cols || silidRect.y + silidRect.height > srcImage.rows || silidRect.area() < 2000)
			{
				return -2;
			}

			Mat sliderOutImg = srcImage(silidRect);

			if (abs(angle) > rotateMinAngle)
			{
				imRoTate(sliderOutImg, sliderOutImg, angle);
			}
			sliderImg[sliderNum] = sliderOutImg.clone();
			sliderNum++;
			//imwrite("slide_" + to_string(sliderNum) + ".jpg", sliderOutImg);
		}
	}
	return  sliderNum;
}

UINT ThreadFun(LPVOID pParam) {
	
	int* pNum = (int*)pParam;
	string str = "线程" + to_string((int)*pNum)+"已经运行。\r\n";
	CString cstring(str.c_str());
	//reconiseStr += recoSiliderMac(sliderImg[(int)*pNum], maskImg);
	reconiseStr += str;
	return 0;
}


void CMFCApplication1Dlg::OnBnClickedButton5()
{
	long tStart;//计时
	long tEnd;
	char timeStrBuf[64];
	int readcont = 0;
	tStart = GetTickCount();
	CString edit_text;//显示到编辑框的字符串
	Mat inImage = imread(imgPath);
	maskImg = imread(maskImgPathStr);
	cvtColor(maskImg, maskImg, CV_RGB2GRAY);

	int Num = slideImg(inImage);
	if (Num == -1)
	{
		MessageBox(_T("图片读取失败！"));
		return;
	}
	if (Num == -2)
	{
		MessageBox(_T("此图片切割出现异常！"));
		return;
	}

	reconiseStr = "";
	CWinThread* pThread;
	int reconisImgNum = 0;
	for (int i = 0; i < Num; i++)
	{		
		int* pNum = &i; 
		//pThread = AfxBeginThread(ThreadFun, pNum, THREAD_PRIORITY_HIGHEST, 0, 0);
		reconiseStr += ("MAC地址：" + reconiseSlideForMac(sliderImg[i], maskImg) + "\r\n");
		silideNum++;
	}

	if (reconiseStr.length() > 0)
	{
		edit_text = reconiseStr.c_str();
		tEnd = GetTickCount();
		sprintf(timeStrBuf, "\r\n耗时：%ld毫秒\r\n", tEnd - tStart);
		edit_text += timeStrBuf;
	}else{
		sprintf(timeStrBuf, "切割：%d张标签，请检查原图", Num);
		edit_text = timeStrBuf;
	}
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);
}

