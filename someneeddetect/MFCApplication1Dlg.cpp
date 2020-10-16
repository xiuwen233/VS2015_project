
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
#include "zbar.h"   
#include <tesseract/baseapi.h> 
 

using std::vector;
using namespace cv;
using namespace std;
using namespace zbar;
using namespace tesseract;

int slidetwo(Mat inImage);

CSliderCtrl   *pSlidCtrl;
int slider1;
string imgname;
string imgPath;
string maskImgPathStr = "mask_mac_max.jpg";
TessBaseAPI tessBaseAPI;
Mat sliderImg[50];//每个标签
float rotateMinAngle = 0.3;//旋转矫正最低容忍角度（低于该值不做校正处理）
int slideMinPx = 300;//标签尺寸范围（像素）
int slideMaxPx = 500;
float ratioMin = 1.0;//标签长和宽的比例
float ratioMax = 1.2;

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
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CMFCApplication1Dlg::OnNMCustomdrawSlider1)
	ON_BN_CLICKED(IDC_BUTTON5, &CMFCApplication1Dlg::OnBnClickedButton5)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON20, &CMFCApplication1Dlg::OnBnClickedButton20)
	ON_BN_CLICKED(IDC_BUTTON10, &CMFCApplication1Dlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON13, &CMFCApplication1Dlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON22, &CMFCApplication1Dlg::OnBnClickedButton22)
	ON_BN_CLICKED(IDC_BUTTON44, &CMFCApplication1Dlg::OnBnClickedButton44)
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
	pSlidCtrl->SetPos(60);
	slider1 = 60;
	CString str;
	str.Format(_T("%d"), slideMinPx);
	GetDlgItem(IDC_EDIT2)->SetWindowText(str);
	str.Format(_T("%d"), slideMaxPx);
	GetDlgItem(IDC_EDIT3)->SetWindowText(str);

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


bool Cal_Hist(Mat Gray_img, MatND &hist) {
	int bins = 256;
	int hist_size[] = { bins };
	float range[] = { 0, 256 };
	const float* ranges[] = { range };

	int channels[] = { 0 };
	//计算直方图    
	calcHist(&Gray_img, 1, channels, Mat(), // do not use mask      
		hist, 1, hist_size, ranges,
		true, // the histogram is uniform      
		false);
	if (hist.data == 0) return false;
	return true;
}

//单通道 
void one_channel_hist_specify(Mat input_img, Mat dst_img, Mat &output_img) 
{
	int i, j;

	//计算输入，规定图像的直方图  
	MatND input_hist, dst_hist;
	Cal_Hist(input_img, input_hist);
	Cal_Hist(dst_img, dst_hist);

	//计算概率直方图  
	MatND input_p_hist, dst_p_hist;
	input_p_hist = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//原始概率直方图  
	dst_p_hist = MatND::zeros(dst_hist.size[0], dst_hist.size[1], CV_32FC1);//规定概率直方图  
	float input_totalnum = 0;
	float dst_totalnum = 0;
	for (i = 0; i < input_hist.rows; i++)
		input_totalnum += input_hist.at<float>(i);
	for (i = 0; i < dst_hist.rows; i++)
		dst_totalnum += dst_hist.at<float>(i);
	for (i = 0; i < input_hist.rows; i++)
		input_p_hist.at<float>(i) = input_hist.at<float>(i) / input_totalnum;
	for (i = 0; i < dst_hist.rows; i++)
		dst_p_hist.at<float>(i) = dst_hist.at<float>(i) / dst_totalnum;

	//计算累计直方图  
	MatND input_c_hist, dst_c_hist;
	input_c_hist = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//原始累计直方图  
	dst_c_hist = MatND::zeros(dst_hist.size[0], dst_hist.size[1], CV_32FC1);//规定累计直方图  
	float input_accum_p = 0;
	float dst_accum_p = 0;
	for (i = 0; i < input_hist.rows; i++)
	{
		input_accum_p += input_p_hist.at<float>(i);
		input_c_hist.at<float>(i) = input_accum_p;
	}
	for (i = 0; i < dst_hist.rows; i++)
	{
		dst_accum_p += dst_p_hist.at<float>(i);
		dst_c_hist.at<float>(i) = dst_accum_p;
	}

	//计算单映射规则  
	MatND SML = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//SML单映射规则  
	for (i = 0; i < input_c_hist.rows; i++)
	{
		int minind = 0;
		float minval = 1;
		for (j = 0; j < dst_c_hist.rows; j++)
		{
			float abssub = abs(input_c_hist.at<float>(i) - dst_c_hist.at<float>(j));
			if (abssub < minval)
			{
				minval = abssub;
				minind = j;
			}
		}
		SML.at<float>(i) = minind;
	}

	//计算输出图像  
	Mat outimg = Mat::zeros(input_img.size[0], input_img.size[1], CV_8U);
	for (i = 0; i < input_img.rows; i++)
	{
		for (j = 0; j < input_img.cols; j++)
		{
			outimg.at<uchar>(i, j) = SML.at<float>(input_img.at<uchar>(i, j));
		}
	}
	outimg.copyTo(output_img);
}

//规定化
Mat specifiImg(Mat inImg) 
{
	Mat src, gray, src2, gray2;

	src = inImg.clone();
	cvtColor(src, gray, CV_RGB2GRAY);  //转换成灰度图    
	src2 = imread("136.jpg");
	cvtColor(src2, gray2, CV_RGB2GRAY);  //转换成灰度图    
	Mat output_img;
	one_channel_hist_specify(gray, gray2, output_img);
	return output_img;
}

//旋转图像（矫正）
void imRoTate(Mat& img, Mat& newIm, double angle) {
	int len = max(img.cols, img.rows);
	Point2f pt(len / 2., len / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(img, newIm, r, Size(img.cols, img.rows));
}


//进一步匹配，寻找源图与目标图像之间的透视变换，目前为止没有发现匹配不对的点
bool refineMatchesWithHomography(const std::vector<cv::KeyPoint>& queryKeypoints, const std::vector<cv::KeyPoint>& trainKeypoints,
	float reprojectionThreshold, std::vector<cv::DMatch>& matches, cv::Mat& homography)
{
	const int minNumberMatchesAllowed = 8;
	if (matches.size() < minNumberMatchesAllowed)
		return false;

	std::vector<cv::Point2f> srcPoints(matches.size());
	std::vector<cv::Point2f> dstPoints(matches.size());
	for (size_t i = 0; i < matches.size(); i++)
	{
		srcPoints[i] = trainKeypoints[matches[i].trainIdx].pt;
		dstPoints[i] = queryKeypoints[matches[i].queryIdx].pt;
		//srcPoints[i] = trainKeypoints[i].pt;  
		//dstPoints[i] = queryKeypoints[i].pt;  
	}
  
	std::vector<unsigned char> inliersMask(srcPoints.size());
	//查找匹配关键点之间的转换，找到两个平面（图像）之间的透视变换
	homography = findHomography(srcPoints, dstPoints, CV_FM_RANSAC, reprojectionThreshold, inliersMask);
	std::vector<cv::DMatch> inliers;
	//查找，得到正确的匹配结果  
	for (size_t i = 0; i<inliersMask.size(); i++)
	{
		if (inliersMask[i])
			inliers.push_back(matches[i]);
	}
	matches.swap(inliers);
	return matches.size() > minNumberMatchesAllowed;
}


//识别每个切割好的标签的条码
string recoSiliderBinCode(Mat inImg) {
	string str;
	Mat out_img = inImg.clone();
	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
	Mat imageGray;
	cvtColor(out_img, imageGray, CV_RGB2GRAY);
	int width = imageGray.cols;
	int height = imageGray.rows;
	uchar *raw = (uchar *)imageGray.data;
	Image imageZbar(width, height, "Y800", raw, width * height);
	scanner.scan(imageZbar); //扫描条码      
	Image::SymbolIterator symbol = imageZbar.symbol_begin();

	for (; symbol != imageZbar.symbol_end(); ++symbol)
	{
		if ((symbol->get_data().length() == 15)) {//"CODE - 128"
			str = symbol->get_data();
		}
	}
	imageZbar.set_data(NULL, 0);
	return str;
}

//识别每个切割好的标签的MAC地址
string recoSiliderMac(Mat sliderOutImg)
{
	//“MAC地址”模板图片
	Mat img_2 = sliderOutImg.clone();
	if (img_2.cols < 600)
	{
		maskImgPathStr = "mask_mac_min.jpg";	
		if (slider1 > 40 )
		{
			pSlidCtrl->SetPos(35);
			slider1 = 35;
		}
	}
	else {
		maskImgPathStr = "mask_mac_max.jpg";
	}

	Mat img_1 = imread(maskImgPathStr);
	cvtColor(img_1, img_1, CV_RGB2GRAY);

	//检测特征点
	vector<KeyPoint> keypoints_1, keypoints_2;
	Ptr<FastFeatureDetector> fast = FastFeatureDetector::create(slider1, true, 1);
	//f2d->detect(img_1, keypoints_1);//用SIFT来检测特别慢
	//f2d->detect(img_2, keypoints_2);
	fast->detect(img_1, keypoints_1);
	fast->detect(img_2, keypoints_2);
	//计算描述符（特征向量）
	Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	Mat descriptors_1, descriptors_2;
	f2d->compute(img_1, keypoints_1, descriptors_1);
	f2d->compute(img_2, keypoints_2, descriptors_2);
	//使用BFMatcher匹配描述符
	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptors_1, descriptors_2, matches);

	Mat img_matches;
	//进一步匹配，筛选匹配结果
	refineMatchesWithHomography(keypoints_1, keypoints_2, 3, matches, img_matches);
	//绘制匹配出的关键点
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_matches);
	//imshow("match.jpg", img_matches);

	//定位“好”的匹配点
	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < matches.size(); i++)
	{
		//DMathch类型中queryIdx是指match中第一个数组的索引,keyPoint类型中pt指的是当前点坐标
		obj.push_back(keypoints_1[matches[i].queryIdx].pt);
		scene.push_back(keypoints_2[matches[i].trainIdx].pt);
	}

	Mat H = findHomography(obj, scene, CV_RANSAC);
	vector<Point2f> obj_corners(4), scene_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint(img_1.cols, 0);
	obj_corners[2] = cvPoint(img_1.cols, img_1.rows);
	obj_corners[3] = cvPoint(0, img_1.rows);

	perspectiveTransform(obj_corners, scene_corners, H);

	//绘制角点之间的直线
	Rect silidOutRect;
	silidOutRect.x = scene_corners[1].x;
	silidOutRect.y = scene_corners[1].y + Point2f(img_1.cols, 0).y;
	silidOutRect.width = img_1.cols*2.3;
	silidOutRect.height = img_1.rows;

	if (silidOutRect.x + silidOutRect.width > img_2.cols || silidOutRect.y + silidOutRect.height > img_2.rows)
	{
		return nullptr;
	}
	//TessBaseAPI，识别字体操作
	Mat apiGrayImg;
	cvtColor(img_2(silidOutRect), apiGrayImg, CV_BGR2GRAY);
	tessBaseAPI.SetImage((uchar*)apiGrayImg.data, apiGrayImg.cols, apiGrayImg.rows, 1, apiGrayImg.cols);
	return tessBaseAPI.GetUTF8Text();
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
	imwrite("Canny.jpg", canny_output);

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
	imwrite("drawingAllRect.jpg", drawingAllRect);


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
		//筛选符合条件的标签（矩形）：长宽数值在slideMinPx、slideMaxPx之间，长宽比值在ratioMin、ratioMax之间
		if (minRect[i].size.width > slideMinPx &&
			minRect[i].size.width < slideMaxPx &&
			minRect[i].size.height > slideMinPx &&
			minRect[i].size.height < slideMaxPx &&
			ratio < ratioMax &&
			ratio > ratioMin)
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
			//直方图均衡化
			//cvtColor(sliderOutImg, sliderOutImg, CV_BGR2GRAY);
			//equalizeHist(sliderOutImg, sliderOutImg);		
			imwrite("slider_" + to_string(sliderNum) + ".jpg", sliderOutImg);
		}
	}
	return  sliderNum;
}

void CMFCApplication1Dlg::OnBnClickedButton1()
{
	long tStart;//计时
	long tEnd;
	char timeStrBuf[64];
	int readcont = 0;
	 tStart = GetTickCount();
	CString edit_text;//显示到编辑框的字符串


	GetDlgItemText(IDC_EDIT2, edit_text); 
	int i = _ttoi(edit_text);
	if (i > 100)
	{
		slideMinPx = i;
	}

	GetDlgItemText(IDC_EDIT3, edit_text);
	i = _ttoi(edit_text);
	if (i > 100)
	{
		slideMaxPx = i;
	}

	Mat inImage = imread(imgPath);
	string str;
	int slideNum = slideImg(inImage);

	for (int i = 0; i < slideNum; i++)
		str += ("S/N："+ recoSiliderBinCode(sliderImg[i]) + "\r\n");

	edit_text = str.c_str();
	tEnd = GetTickCount();
	sprintf(timeStrBuf, "\r\n耗时：%ld毫秒\r\n", tEnd - tStart);
	edit_text += timeStrBuf;
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);
	edit_text.Format(_T("%d"), slideNum);
	GetDlgItem(IDC_STATIC_CONT)->SetWindowText(edit_text + " 个");

}


void CMFCApplication1Dlg::OnBnClickedButton5()
{
	long tStart;//计时
	long tEnd;
	char timeStrBuf[64];
	int readcont = 0;
	tStart = GetTickCount();
	CString edit_text;//显示到编辑框的字符串

	GetDlgItemText(IDC_EDIT2, edit_text);
	int i = _ttoi(edit_text);
	if (i > 100)
	{
		slideMinPx = i;
	}

	GetDlgItemText(IDC_EDIT3, edit_text);
	i = _ttoi(edit_text);
	if (i > 100)
	{
		slideMaxPx = i;
	}


	Mat inImage = imread(imgPath);
	string str;
	int slideNum = slideImg(inImage);

	for (int i = 0; i < slideNum; i++)
		str += ("MAC地址："+ recoSiliderMac(sliderImg[i]) + "\r\n");

	edit_text = str.c_str();
	tEnd = GetTickCount();
	sprintf(timeStrBuf, "\r\n耗时：%ld毫秒\r\n", tEnd - tStart);
	edit_text += timeStrBuf;
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);
	edit_text.Format(_T("%d"), slideNum);
	GetDlgItem(IDC_STATIC_CONT)->SetWindowText(edit_text + " 个");
}

//----------------------------------------------------------特征匹配方法----------------------------------------------------------
//（1）与ORB结合使用，效果较好
void match_features_knn(Mat& query, Mat& train, vector<DMatch>& matches)
{
	flann::Index flannIndex(query, flann::LshIndexParams(12, 20, 2), cvflann::FLANN_DIST_HAMMING);
	Mat matchindex(train.rows, 2, CV_32SC1);
	Mat matchdistance(train.rows, 2, CV_32FC1);
	flannIndex.knnSearch(train, matchindex, matchdistance, 2, flann::SearchParams());
	//根据劳氏算法  
	for (int i = 0; i < matchdistance.rows; i++)
	{
		if (matchdistance.at<float>(i, 0) < 0.6*matchdistance.at<float>(i, 1))
		{
			DMatch dmatches(matchindex.at<int>(i, 0), i, matchdistance.at<float>(i, 0));
			matches.push_back(dmatches);
		}
	}
}

//（2）个人感觉这种方法，效果与暴力匹配法没啥区别，但是被注释掉的方法，效果不好
void match_features_FLANN(Mat& query, Mat& train, vector<DMatch>& matches)
{
	FlannBasedMatcher matcher;

	/*vector<DMatch> match;
	matcher.match(query, train, match);
	double max_dist = 0;
	double min_dist = 100;
	for (int i = 0; i < match.size(); i++)
	{
	double dist = match[i].distance;
	if (dist < min_dist) min_dist = dist;
	if (dist > max_dist) max_dist = dist;
	}
	for (int i = 0; i < match.size(); i++)
	{
	if (match[i].distance < 2 * min_dist) matches.push_back(match[i]);
	}*/

	vector<vector<DMatch>> knn_matches;
	matcher.knnMatch(query, train, knn_matches, 2);

	//获取满足Ratio Test的最小匹配的距离  
	float min_dist = FLT_MAX;
	for (int r = 0; r < knn_matches.size(); ++r)
	{
		//Ratio Test  
		if (knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance)
			continue;

		float dist = knn_matches[r][0].distance;
		if (dist < min_dist) min_dist = dist;
	}

	matches.clear();
	for (size_t r = 0; r < knn_matches.size(); ++r)
	{
		//排除不满足Ratio Test的点和匹配距离过大的点  
		if (
			knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance ||
			knn_matches[r][0].distance > 5 * max(min_dist, 10.0f)
			)
			continue;

		//保存匹配点  
		matches.push_back(knn_matches[r][0]);
	}

}

//（3）也叫暴力匹配法，此种方法结合sift、surf用的比较多  大部分点都是匹配正确的。但是，依然有少部分点匹配的明显不正确
void match_features(Mat& query, Mat& train, vector<DMatch>& matches)
{
	vector<vector<DMatch>> knn_matches;
	BFMatcher matcher(NORM_L2);

	matcher.knnMatch(query, train, knn_matches, 2);

	//获取满足Ratio Test的最小匹配的距离  
	float min_dist = FLT_MAX;
	for (int r = 0; r < knn_matches.size(); ++r)
	{
		//Ratio Test  
		if (knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance)
			continue;

		float dist = knn_matches[r][0].distance;
		if (dist < min_dist) min_dist = dist;
	}

	matches.clear();
	for (size_t r = 0; r < knn_matches.size(); ++r)
	{
		//排除不满足Ratio Test的点和匹配距离过大的点  
		if (
			knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance ||
			knn_matches[r][0].distance > 5 * max(min_dist, 10.0f)
			)
			continue;

		//保存匹配点  
		matches.push_back(knn_matches[r][0]);
	}
}


void CMFCApplication1Dlg::OnBnClickedButton20()
{
	//	Mat img_1 = imread("1.jpg");
	//	Mat img_2 = imread(imgPath);
	//	Mat descriptor1, descriptor2;
	//	Mat homography;
	//	Mat outimg;
	//	vector<DMatch> matches;
	//	vector<KeyPoint> key_points1, key_points2;
	//
	////得到特征点与特征点描述（SIFT SURF ORB  AKAZE）---------------------------------------------------①
	//	//（1）SIFT
	//	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);
	//	sift->detectAndCompute(img_1, noArray(), key_points1, descriptor1);
	//	sift->detectAndCompute(img_2, noArray(), key_points2, descriptor2);
	//
	//	//（2）SURF（可以认为是尺度不变特征变换sift的加速版）
	//	//Ptr<Feature2D> surf = xfeatures2d::SURF::create();
	//	//surf->detectAndCompute(img_1, noArray(), key_points1, descriptor1);
	//	//surf->detectAndCompute(img_2, noArray(), key_points2, descriptor2);
	//
	//	//（3）ORB（实际来看，速度很快，但效果并不一定好）
	//	/*Ptr<ORB> orb = ORB::create(5000);
	//	orb->detectAndCompute(img_1, noArray(), key_points1, descriptor1);
	//	orb->detectAndCompute(img_2, noArray(), key_points2, descriptor2);*/
	//
	//	//（4）AKAZE（与ORB在同一个hpp中）
	//	//Ptr<AKAZE> akaze = AKAZE::create();
	//	//akaze->detectAndCompute(img_1, noArray(), key_points1, descriptor1);
	//	//akaze->detectAndCompute(img_2, noArray(), key_points2, descriptor2);
	//
	////特征点匹配的几种方法 -----------------------------------------------------------------------------②
	//	//（1）与ORB结合使用，效果较好
	//	match_features_knn(descriptor1, descriptor2, matches);
	//
	//	//（2）个人感觉这种方法，效果与暴力匹配法没啥区别，但是被注释掉的方法，效果不好
	//	//match_features_FLANN(descriptor1, descriptor2, matches);
	//
	//	//（3）也叫暴力匹配法，此种方法结合sift、surf用的比较多
	//	//match_features(descriptor1, descriptor2, matches);
	//
	////进一步匹配（寻找源图与目标图像之间的透视变换）----------------------------------------------------③
	//	refineMatchesWithHomography(key_points1, key_points2,3, matches,homography);
	//
	//	drawMatches(img_1, key_points1, img_2, key_points2, matches, outimg);
	//	imshow("matchingResult.jpg", outimg);//显示匹配结果  

	CString edit_text;
	GetDlgItemText(IDC_EDIT2, edit_text);
	int i = _ttoi(edit_text);
	if (i > 100)
	{
		slideMinPx = i;
	}

	GetDlgItemText(IDC_EDIT3, edit_text);
	i = _ttoi(edit_text);
	if (i > 100)
	{
		slideMaxPx = i;
	}


	long tStart, tStartTemp;//计时
	long tEnd;
	char timeStrBuf[64];
	int readcont = 0;
	tStartTemp = tStart = GetTickCount();


	Mat img_2 = imread(imgPath);
	if (img_2.cols < 600)
	{
		maskImgPathStr = "mask_mac_min.jpg";
		if (slider1 > 40)
		{
			pSlidCtrl->SetPos(35);
			slider1 = 35;
		}
	}
	else {
		maskImgPathStr = "mask_mac_max.jpg";
	}

	Mat img_1 = imread(maskImgPathStr);
	cvtColor(img_1, img_1, CV_RGB2GRAY);
	cvtColor(img_2, img_2, CV_RGB2GRAY);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "读入图片耗时：%ld毫秒\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;

	//检测特征点
	vector<KeyPoint> keypoints_1, keypoints_2;
	Ptr<FastFeatureDetector> fast = FastFeatureDetector::create(slider1, true, 1);
	fast->detect(img_1, keypoints_1);
	fast->detect(img_2, keypoints_2);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "检测特征点耗时：%ld毫秒\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;


	//计算描述符（特征向量）
	Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	Mat descriptors_1, descriptors_2;
	f2d->compute(img_1, keypoints_1, descriptors_1);
	f2d->compute(img_2, keypoints_2, descriptors_2);

	//fast->detectAndCompute(img_1, descriptors_1, keypoints_1, descriptors_1);
	//fast->detectAndCompute(img_2, descriptors_2, keypoints_2, descriptors_2);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "计算描述符（特征向量）耗时：%ld毫秒\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;


	//使用BFMatcher匹配描述符
	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptors_1, descriptors_2, matches);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "使用BFMatcher匹配描述符耗时：%ld毫秒\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;

	Mat img_matches;
	//进一步匹配
	refineMatchesWithHomography(keypoints_1, keypoints_2, 3, matches, img_matches);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "进一步匹配耗时：%ld毫秒\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;


	//绘制匹配出的关键点
	drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_matches);
	imwrite("match.jpg", img_matches);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "绘制匹配出的关键点耗时：%ld毫秒\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;

	//定位“好”的匹配点
	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < matches.size(); i++)
	{
		//DMathch类型中queryIdx是指match中第一个数组的索引,keyPoint类型中pt指的是当前点坐标
		obj.push_back(keypoints_1[matches[i].queryIdx].pt);
		scene.push_back(keypoints_2[matches[i].trainIdx].pt);
	}

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "定位“好”的匹配点耗时：%ld毫秒\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;

	Mat H = findHomography(obj, scene, CV_RANSAC);
	vector<Point2f> obj_corners(4), scene_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint(img_1.cols, 0);
	obj_corners[2] = cvPoint(img_1.cols, img_1.rows);
	obj_corners[3] = cvPoint(0, img_1.rows);

	perspectiveTransform(obj_corners, scene_corners, H);

	//绘制角点之间的直线
	Rect silidOutRect;
	silidOutRect.x = scene_corners[1].x;
	silidOutRect.y = scene_corners[1].y + Point2f(img_1.cols, 0).y;
	silidOutRect.width = img_1.cols*2.3;
	silidOutRect.height = img_1.rows;

	if (silidOutRect.area() < 2000)
		return;
	//输出图像
	//imshow("匹配图像.jpg", img_matches);
	imshow("ss.jpg", img_2(silidOutRect));

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "\r\n总耗时：%ld毫秒\r\n", tEnd - tStart);
	edit_text += timeStrBuf;
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);
}


void CMFCApplication1Dlg::OnBnClickedButton10()
{
	Mat src, dst;
	src = imread(imgPath);

	Mat  gray_src;
	cvtColor(src, gray_src, CV_BGR2GRAY);


	//转换为二值图像
	Mat binimage;
	adaptiveThreshold(~gray_src, binimage, 255, ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 15, -2);

	Mat hline = getStructuringElement(MORPH_RECT, Size(src.cols / 16, 1), Point(-1, -1));
	Mat yline = getStructuringElement(MORPH_RECT, Size(1, src.rows / 16), Point(-1, -1));
	Mat sline = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	Mat temp;
	erode(binimage, temp, sline);
	dilate(temp, dst, sline);
	//morphologyEx(binimage, dst, CV_MOP_OPEN, hline);
	//blur(dst, dst, Size(3, 3), Point(-1, -1));
	bitwise_not(dst, dst);

	imwrite("水平线.jpg", dst);

}


void CMFCApplication1Dlg::OnBnClickedButton13()
{
	//MessageBox(_T("你好，um！"));
	Mat  imme = imread("E://photo_opencv/pic_temp/temp_2359.jpg");

	MatND hist;       // 在cv中用CvHistogram *hist = cvCreateHist  
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };   //这里需要为const类型  
	int size = 256;
	int channels = 0;
	//计算图像的直方图  
	calcHist(&imme, 1, &channels, Mat(), hist, dims, &size, ranges);    //cv中是cvCalcHist  
	int scale = 1;
	Mat imageShow(size * scale, size, CV_8U, Scalar(0));
	//获取最大值和最小值  
	double minVal = 0;
	double maxVal = 0;
	minMaxLoc(hist, &minVal, &maxVal, 0, 0);    //cv中用的是cvGetMinMaxHistValue  
												//显示直方图的图像  
	int hpt = saturate_cast<int>(0.9 * size);

	for (int i = 0; i < 256; i++)
	{
		float value = hist.at<float>(i);           // 注意hist中是float类型    cv中用cvQueryHistValue_1D  
		int realValue = saturate_cast<int>(value * hpt / maxVal);
		rectangle(imageShow, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255));
	}
	namedWindow("Hist");
	imshow("Hist", imageShow);
	//imwrite("hist.jpg", imageShow);  

	waitKey(0);
}




//bool Cal_Hist(Mat Gray_img, MatND &hist) {
//	int bins = 256;
//	int hist_size[] = { bins };
//	float range[] = { 0, 256 };
//	const float* ranges[] = { range };
//
//	int channels[] = { 0 };
//	//计算直方图    
//	calcHist(&Gray_img, 1, channels, Mat(), // do not use mask      
//		hist, 1, hist_size, ranges,
//		true, // the histogram is uniform      
//		false);
//	if (hist.data == 0) return false;
//	return true;
//}
//
//void one_channel_hist_specify(Mat input_img, Mat dst_img, Mat &output_img)//单通道  
//{
//	int i, j;
//
//	//计算输入，规定图像的直方图  
//	MatND input_hist, dst_hist;
//	Cal_Hist(input_img, input_hist);
//	Cal_Hist(dst_img, dst_hist);
//
//	//计算概率直方图  
//	MatND input_p_hist, dst_p_hist;
//	input_p_hist = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//原始概率直方图  
//	dst_p_hist = MatND::zeros(dst_hist.size[0], dst_hist.size[1], CV_32FC1);//规定概率直方图  
//	float input_totalnum = 0;
//	float dst_totalnum = 0;
//	for (i = 0; i < input_hist.rows; i++)
//		input_totalnum += input_hist.at<float>(i);
//	for (i = 0; i < dst_hist.rows; i++)
//		dst_totalnum += dst_hist.at<float>(i);
//	for (i = 0; i < input_hist.rows; i++)
//		input_p_hist.at<float>(i) = input_hist.at<float>(i) / input_totalnum;
//	for (i = 0; i < dst_hist.rows; i++)
//		dst_p_hist.at<float>(i) = dst_hist.at<float>(i) / dst_totalnum;
//
//	//计算累计直方图  
//	MatND input_c_hist, dst_c_hist;
//	input_c_hist = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//原始累计直方图  
//	dst_c_hist = MatND::zeros(dst_hist.size[0], dst_hist.size[1], CV_32FC1);//规定累计直方图  
//	float input_accum_p = 0;
//	float dst_accum_p = 0;
//	for (i = 0; i < input_hist.rows; i++)
//	{
//		input_accum_p += input_p_hist.at<float>(i);
//		input_c_hist.at<float>(i) = input_accum_p;
//	}
//	for (i = 0; i < dst_hist.rows; i++)
//	{
//		dst_accum_p += dst_p_hist.at<float>(i);
//		dst_c_hist.at<float>(i) = dst_accum_p;
//	}
//
//	//计算单映射规则  
//	MatND SML = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//SML单映射规则  
//	for (i = 0; i < input_c_hist.rows; i++)
//	{
//		int minind = 0;
//		float minval = 1;
//		for (j = 0; j < dst_c_hist.rows; j++)
//		{
//			float abssub = abs(input_c_hist.at<float>(i) - dst_c_hist.at<float>(j));
//			if (abssub < minval)
//			{
//				minval = abssub;
//				minind = j;
//			}
//		}
//		SML.at<float>(i) = minind;
//	}
//
//	//计算输出图像  
//	Mat outimg = Mat::zeros(input_img.size[0], input_img.size[1], CV_8U);
//	for (i = 0; i < input_img.rows; i++)
//	{
//		for (j = 0; j < input_img.cols; j++)
//		{
//			outimg.at<uchar>(i, j) = SML.at<float>(input_img.at<uchar>(i, j));
//		}
//	}
//	outimg.copyTo(output_img);
//}


void CMFCApplication1Dlg::OnBnClickedButton22()
{
	// TODO: 在此添加控件通知处理程序代码
	Mat src, gray, src2, gray2;
	src = imread("E://photo_opencv/pic_temp/");
	cvtColor(src, gray, CV_RGB2GRAY);  //转换成灰度图    
	src2 = imread("136.jpg");
	cvtColor(src2, gray2, CV_RGB2GRAY);  //转换成灰度图    
	Mat output_img;
	//three_channel_hist_specify(src, src2, output_img);
	one_channel_hist_specify(gray, gray2, output_img);
	imshow("哈哈哈哈哈", output_img);
	waitKey(0);
	//imwrite("out.jpg", output_img);
}


void CMFCApplication1Dlg::OnBnClickedButton44()
{
	// TODO: 在此添加控件通知处理程序代码
	Mat imme = imread("E://photo_opencv/pic_temp/136.jpg");
	CString edit_text;
	char timeStrBuf[64];
	memset(timeStrBuf, 0, 64);
	int num = slidetwo(imme);
	sprintf(timeStrBuf, "\r\n切出的图片：%d 张\r\n", num);
	edit_text += timeStrBuf ;
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);

}

int slidetwo(Mat inImage)
{
	int sliderNum = 0;
	Mat srcImage = inImage.clone();  //源图像
	if (!srcImage.data)
	{
		return -1;
	}

	int slider1 = 163;

	for (int i = 0; i < srcImage.rows; i++)
	{
		uchar* p = srcImage.ptr(i);//获取Mat某一行的首地址  

		for (int j = 0; j < srcImage.cols; j++)
		{
			int R = *(p + j * 3);
			int G = *(p + j * 3 + 1);
			int B = *(p + j * 3 + 2);

			if (!(R>slider1&& G>slider1 && B>slider1))
			{
				*(p + j * 3) = 0;
				*(p + j * 3 + 1) = 0;
				*(p + j * 3 + 2) = 0;
			}
			else {
				*(p + j * 3) = 255;
				*(p + j * 3 + 1) = 255;
				*(p + j * 3 + 2) = 255;
			}
		}
	}
	namedWindow("2233");
	imshow("2233", srcImage);
	waitKey(3000);
	//预处理
	//Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

	//Mat openImg = specifiImg(srcImage);

	//Mat openImg = srcImage.clone();

	//morphologyEx(openImg, openImg, MORPH_OPEN, element);//开运算  

	Mat canny_output;
	cvtColor(openImg, openImg, CV_BGR2GRAY);
	Canny(openImg, canny_output, 50, 200, 3);//进行canny边缘检测  	

											 //Mat dst = canny_output<100;
	imwrite("Canny.jpg", canny_output);

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
	imwrite("drawingAllRect.jpg", drawingAllRect);


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
		//筛选符合条件的标签（矩形）：长宽数值在slideMinPx、slideMaxPx之间，长宽比值在ratioMin、ratioMax之间
		if (minRect[i].size.width > slideMinPx &&
			minRect[i].size.width < slideMaxPx &&
			minRect[i].size.height > slideMinPx &&
			minRect[i].size.height < slideMaxPx &&
			ratio < ratioMax &&
			ratio > ratioMin)
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
			//直方图均衡化
			//cvtColor(sliderOutImg, sliderOutImg, CV_BGR2GRAY);
			//equalizeHist(sliderOutImg, sliderOutImg);		
			imwrite("slider_" + to_string(sliderNum) + ".jpg", sliderOutImg);
		}
	}
	return  sliderNum;
}
