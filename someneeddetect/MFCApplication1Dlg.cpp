
// MFCApplication1Dlg.cpp : ʵ���ļ�
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
Mat sliderImg[50];//ÿ����ǩ
float rotateMinAngle = 0.3;//��ת����������̽Ƕȣ����ڸ�ֵ����У������
int slideMinPx = 300;//��ǩ�ߴ緶Χ�����أ�
int slideMaxPx = 500;
float ratioMin = 1.0;//��ǩ���Ϳ�ı���
float ratioMax = 1.2;

float silieMul = 1.1;//ʵ���и��С(��)

// CMFCApplication1Dlg �Ի���
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


// CMFCApplication1Dlg ��Ϣ�������
BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_NORMAL);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	pSlidCtrl->SetRange(0, 65, TRUE);
	pSlidCtrl->SetPos(60);
	slider1 = 60;
	CString str;
	str.Format(_T("%d"), slideMinPx);
	GetDlgItem(IDC_EDIT2)->SetWindowText(str);
	str.Format(_T("%d"), slideMaxPx);
	GetDlgItem(IDC_EDIT3)->SetWindowText(str);

	//tranning��ѡ��
	tessBaseAPI.Init(NULL, "num", OEM_DEFAULT);
	tessBaseAPI.SetPageSegMode(PSM_SINGLE_BLOCK);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
	//����ֱ��ͼ    
	calcHist(&Gray_img, 1, channels, Mat(), // do not use mask      
		hist, 1, hist_size, ranges,
		true, // the histogram is uniform      
		false);
	if (hist.data == 0) return false;
	return true;
}

//��ͨ�� 
void one_channel_hist_specify(Mat input_img, Mat dst_img, Mat &output_img) 
{
	int i, j;

	//�������룬�涨ͼ���ֱ��ͼ  
	MatND input_hist, dst_hist;
	Cal_Hist(input_img, input_hist);
	Cal_Hist(dst_img, dst_hist);

	//�������ֱ��ͼ  
	MatND input_p_hist, dst_p_hist;
	input_p_hist = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//ԭʼ����ֱ��ͼ  
	dst_p_hist = MatND::zeros(dst_hist.size[0], dst_hist.size[1], CV_32FC1);//�涨����ֱ��ͼ  
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

	//�����ۼ�ֱ��ͼ  
	MatND input_c_hist, dst_c_hist;
	input_c_hist = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//ԭʼ�ۼ�ֱ��ͼ  
	dst_c_hist = MatND::zeros(dst_hist.size[0], dst_hist.size[1], CV_32FC1);//�涨�ۼ�ֱ��ͼ  
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

	//���㵥ӳ�����  
	MatND SML = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//SML��ӳ�����  
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

	//�������ͼ��  
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

//�涨��
Mat specifiImg(Mat inImg) 
{
	Mat src, gray, src2, gray2;

	src = inImg.clone();
	cvtColor(src, gray, CV_RGB2GRAY);  //ת���ɻҶ�ͼ    
	src2 = imread("136.jpg");
	cvtColor(src2, gray2, CV_RGB2GRAY);  //ת���ɻҶ�ͼ    
	Mat output_img;
	one_channel_hist_specify(gray, gray2, output_img);
	return output_img;
}

//��תͼ�񣨽�����
void imRoTate(Mat& img, Mat& newIm, double angle) {
	int len = max(img.cols, img.rows);
	Point2f pt(len / 2., len / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(img, newIm, r, Size(img.cols, img.rows));
}


//��һ��ƥ�䣬Ѱ��Դͼ��Ŀ��ͼ��֮���͸�ӱ任��ĿǰΪֹû�з���ƥ�䲻�Եĵ�
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
	//����ƥ��ؼ���֮���ת�����ҵ�����ƽ�棨ͼ��֮���͸�ӱ任
	homography = findHomography(srcPoints, dstPoints, CV_FM_RANSAC, reprojectionThreshold, inliersMask);
	std::vector<cv::DMatch> inliers;
	//���ң��õ���ȷ��ƥ����  
	for (size_t i = 0; i<inliersMask.size(); i++)
	{
		if (inliersMask[i])
			inliers.push_back(matches[i]);
	}
	matches.swap(inliers);
	return matches.size() > minNumberMatchesAllowed;
}


//ʶ��ÿ���и�õı�ǩ������
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
	scanner.scan(imageZbar); //ɨ������      
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

//ʶ��ÿ���и�õı�ǩ��MAC��ַ
string recoSiliderMac(Mat sliderOutImg)
{
	//��MAC��ַ��ģ��ͼƬ
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

	//���������
	vector<KeyPoint> keypoints_1, keypoints_2;
	Ptr<FastFeatureDetector> fast = FastFeatureDetector::create(slider1, true, 1);
	//f2d->detect(img_1, keypoints_1);//��SIFT������ر���
	//f2d->detect(img_2, keypoints_2);
	fast->detect(img_1, keypoints_1);
	fast->detect(img_2, keypoints_2);
	//����������������������
	Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	Mat descriptors_1, descriptors_2;
	f2d->compute(img_1, keypoints_1, descriptors_1);
	f2d->compute(img_2, keypoints_2, descriptors_2);
	//ʹ��BFMatcherƥ��������
	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptors_1, descriptors_2, matches);

	Mat img_matches;
	//��һ��ƥ�䣬ɸѡƥ����
	refineMatchesWithHomography(keypoints_1, keypoints_2, 3, matches, img_matches);
	//����ƥ����Ĺؼ���
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_matches);
	//imshow("match.jpg", img_matches);

	//��λ���á���ƥ���
	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < matches.size(); i++)
	{
		//DMathch������queryIdx��ָmatch�е�һ�����������,keyPoint������ptָ���ǵ�ǰ������
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

	//���ƽǵ�֮���ֱ��
	Rect silidOutRect;
	silidOutRect.x = scene_corners[1].x;
	silidOutRect.y = scene_corners[1].y + Point2f(img_1.cols, 0).y;
	silidOutRect.width = img_1.cols*2.3;
	silidOutRect.height = img_1.rows;

	if (silidOutRect.x + silidOutRect.width > img_2.cols || silidOutRect.y + silidOutRect.height > img_2.rows)
	{
		return nullptr;
	}
	//TessBaseAPI��ʶ���������
	Mat apiGrayImg;
	cvtColor(img_2(silidOutRect), apiGrayImg, CV_BGR2GRAY);
	tessBaseAPI.SetImage((uchar*)apiGrayImg.data, apiGrayImg.cols, apiGrayImg.rows, 1, apiGrayImg.cols);
	return tessBaseAPI.GetUTF8Text();
}


//Ԥ�����и��ǩ��ÿ����ǩ�浽sliderImg��
int slideImg(Mat inImage)
{
	int sliderNum = 0;
	Mat srcImage = inImage.clone();  //Դͼ��
	if (!srcImage.data)
	{
		return -1;
	}


	//Ԥ����
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

	//Mat openImg = specifiImg(srcImage);

	Mat openImg = srcImage.clone();

	morphologyEx(openImg, openImg, MORPH_OPEN, element);//������  

	Mat canny_output;
	Canny(openImg, canny_output, 50, 200, 3);//����canny��Ե���  	

	//Mat dst = canny_output<100;
	imwrite("Canny.jpg", canny_output);

	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;
	//���μ��
	findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	//��ÿ���ҵ���������������б�ı߽�� 
	vector<RotatedRect> minRect(contours.size());
	for (unsigned int i = 0; i < contours.size(); i++)
	{
		minRect[i] = minAreaRect(Mat(contours[i]));
	}
	//�����б�ı߽��  
	CvBox2D  End_Rage2D;

	//����ÿ�����ο�just for test...
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
		
		//��⵽���γ��Ϳ�ı�
		float ratio;
		if (minRect[i].size.width > minRect[i].size.height)
		{
			ratio = minRect[i].size.width / minRect[i].size.height;
		}
		else {
			ratio = minRect[i].size.height / minRect[i].size.width;
		}
		//ɸѡ���������ı�ǩ�����Σ���������ֵ��slideMinPx��slideMaxPx֮�䣬�����ֵ��ratioMin��ratioMax֮��
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

			//˳ʱ����б��End_Rage2D.angle = ʵ����б����
			//��ʱ����б��End_Rage2D.angle = -90 + ʵ����б����������
			if (angle < -45)
			{
				angle += 90;
			}
			//С��0��(����0)��ʱ��������Ҫ�任���ܵó�ROI����silidRect��������
			if (angle <= 0)
			{
				Point2f p2fTemp = rect_points[3];
				rect_points[3] = rect_points[2];
				rect_points[2] = rect_points[1];
				rect_points[1] = rect_points[0];
				rect_points[0] = p2fTemp;
			}

			//�и�1.1����ǩ��С����ֹ��б��ı�ǩ���ݻᱻ�ص�
			Rect silidRect;
			silidRect.width = (int)sqrt((pow((rect_points[0].x - rect_points[1].x), 2) + pow((rect_points[0].y - rect_points[1].y), 2)));
			silidRect.height = (int)sqrt((pow((rect_points[0].x - rect_points[3].x), 2) + pow((rect_points[0].y - rect_points[3].y), 2)));
			silidRect.x = (int)(rect_points[2].x - silidRect.width*(silieMul - 1) / 2);
			silidRect.y = (int)(rect_points[2].y - silidRect.height*(silieMul - 1) / 2);
			silidRect.width *= silieMul;
			silidRect.height *= silieMul;

			//�и���߽�ᵼ�±���
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
			//ֱ��ͼ���⻯
			//cvtColor(sliderOutImg, sliderOutImg, CV_BGR2GRAY);
			//equalizeHist(sliderOutImg, sliderOutImg);		
			imwrite("slider_" + to_string(sliderNum) + ".jpg", sliderOutImg);
		}
	}
	return  sliderNum;
}

void CMFCApplication1Dlg::OnBnClickedButton1()
{
	long tStart;//��ʱ
	long tEnd;
	char timeStrBuf[64];
	int readcont = 0;
	 tStart = GetTickCount();
	CString edit_text;//��ʾ���༭����ַ���


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
		str += ("S/N��"+ recoSiliderBinCode(sliderImg[i]) + "\r\n");

	edit_text = str.c_str();
	tEnd = GetTickCount();
	sprintf(timeStrBuf, "\r\n��ʱ��%ld����\r\n", tEnd - tStart);
	edit_text += timeStrBuf;
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);
	edit_text.Format(_T("%d"), slideNum);
	GetDlgItem(IDC_STATIC_CONT)->SetWindowText(edit_text + " ��");

}


void CMFCApplication1Dlg::OnBnClickedButton5()
{
	long tStart;//��ʱ
	long tEnd;
	char timeStrBuf[64];
	int readcont = 0;
	tStart = GetTickCount();
	CString edit_text;//��ʾ���༭����ַ���

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
		str += ("MAC��ַ��"+ recoSiliderMac(sliderImg[i]) + "\r\n");

	edit_text = str.c_str();
	tEnd = GetTickCount();
	sprintf(timeStrBuf, "\r\n��ʱ��%ld����\r\n", tEnd - tStart);
	edit_text += timeStrBuf;
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);
	edit_text.Format(_T("%d"), slideNum);
	GetDlgItem(IDC_STATIC_CONT)->SetWindowText(edit_text + " ��");
}

//----------------------------------------------------------����ƥ�䷽��----------------------------------------------------------
//��1����ORB���ʹ�ã�Ч���Ϻ�
void match_features_knn(Mat& query, Mat& train, vector<DMatch>& matches)
{
	flann::Index flannIndex(query, flann::LshIndexParams(12, 20, 2), cvflann::FLANN_DIST_HAMMING);
	Mat matchindex(train.rows, 2, CV_32SC1);
	Mat matchdistance(train.rows, 2, CV_32FC1);
	flannIndex.knnSearch(train, matchindex, matchdistance, 2, flann::SearchParams());
	//���������㷨  
	for (int i = 0; i < matchdistance.rows; i++)
	{
		if (matchdistance.at<float>(i, 0) < 0.6*matchdistance.at<float>(i, 1))
		{
			DMatch dmatches(matchindex.at<int>(i, 0), i, matchdistance.at<float>(i, 0));
			matches.push_back(dmatches);
		}
	}
}

//��2�����˸о����ַ�����Ч���뱩��ƥ�䷨ûɶ���𣬵��Ǳ�ע�͵��ķ�����Ч������
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

	//��ȡ����Ratio Test����Сƥ��ľ���  
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
		//�ų�������Ratio Test�ĵ��ƥ��������ĵ�  
		if (
			knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance ||
			knn_matches[r][0].distance > 5 * max(min_dist, 10.0f)
			)
			continue;

		//����ƥ���  
		matches.push_back(knn_matches[r][0]);
	}

}

//��3��Ҳ�б���ƥ�䷨�����ַ������sift��surf�õıȽ϶�  �󲿷ֵ㶼��ƥ����ȷ�ġ����ǣ���Ȼ���ٲ��ֵ�ƥ������Բ���ȷ
void match_features(Mat& query, Mat& train, vector<DMatch>& matches)
{
	vector<vector<DMatch>> knn_matches;
	BFMatcher matcher(NORM_L2);

	matcher.knnMatch(query, train, knn_matches, 2);

	//��ȡ����Ratio Test����Сƥ��ľ���  
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
		//�ų�������Ratio Test�ĵ��ƥ��������ĵ�  
		if (
			knn_matches[r][0].distance > 0.6*knn_matches[r][1].distance ||
			knn_matches[r][0].distance > 5 * max(min_dist, 10.0f)
			)
			continue;

		//����ƥ���  
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
	////�õ���������������������SIFT SURF ORB  AKAZE��---------------------------------------------------��
	//	//��1��SIFT
	//	Ptr<Feature2D> sift = xfeatures2d::SIFT::create(0, 3, 0.04, 10);
	//	sift->detectAndCompute(img_1, noArray(), key_points1, descriptor1);
	//	sift->detectAndCompute(img_2, noArray(), key_points2, descriptor2);
	//
	//	//��2��SURF��������Ϊ�ǳ߶Ȳ��������任sift�ļ��ٰ棩
	//	//Ptr<Feature2D> surf = xfeatures2d::SURF::create();
	//	//surf->detectAndCompute(img_1, noArray(), key_points1, descriptor1);
	//	//surf->detectAndCompute(img_2, noArray(), key_points2, descriptor2);
	//
	//	//��3��ORB��ʵ���������ٶȺܿ죬��Ч������һ���ã�
	//	/*Ptr<ORB> orb = ORB::create(5000);
	//	orb->detectAndCompute(img_1, noArray(), key_points1, descriptor1);
	//	orb->detectAndCompute(img_2, noArray(), key_points2, descriptor2);*/
	//
	//	//��4��AKAZE����ORB��ͬһ��hpp�У�
	//	//Ptr<AKAZE> akaze = AKAZE::create();
	//	//akaze->detectAndCompute(img_1, noArray(), key_points1, descriptor1);
	//	//akaze->detectAndCompute(img_2, noArray(), key_points2, descriptor2);
	//
	////������ƥ��ļ��ַ��� -----------------------------------------------------------------------------��
	//	//��1����ORB���ʹ�ã�Ч���Ϻ�
	//	match_features_knn(descriptor1, descriptor2, matches);
	//
	//	//��2�����˸о����ַ�����Ч���뱩��ƥ�䷨ûɶ���𣬵��Ǳ�ע�͵��ķ�����Ч������
	//	//match_features_FLANN(descriptor1, descriptor2, matches);
	//
	//	//��3��Ҳ�б���ƥ�䷨�����ַ������sift��surf�õıȽ϶�
	//	//match_features(descriptor1, descriptor2, matches);
	//
	////��һ��ƥ�䣨Ѱ��Դͼ��Ŀ��ͼ��֮���͸�ӱ任��----------------------------------------------------��
	//	refineMatchesWithHomography(key_points1, key_points2,3, matches,homography);
	//
	//	drawMatches(img_1, key_points1, img_2, key_points2, matches, outimg);
	//	imshow("matchingResult.jpg", outimg);//��ʾƥ����  

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


	long tStart, tStartTemp;//��ʱ
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
	sprintf(timeStrBuf, "����ͼƬ��ʱ��%ld����\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;

	//���������
	vector<KeyPoint> keypoints_1, keypoints_2;
	Ptr<FastFeatureDetector> fast = FastFeatureDetector::create(slider1, true, 1);
	fast->detect(img_1, keypoints_1);
	fast->detect(img_2, keypoints_2);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "����������ʱ��%ld����\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;


	//����������������������
	Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	Mat descriptors_1, descriptors_2;
	f2d->compute(img_1, keypoints_1, descriptors_1);
	f2d->compute(img_2, keypoints_2, descriptors_2);

	//fast->detectAndCompute(img_1, descriptors_1, keypoints_1, descriptors_1);
	//fast->detectAndCompute(img_2, descriptors_2, keypoints_2, descriptors_2);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "������������������������ʱ��%ld����\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;


	//ʹ��BFMatcherƥ��������
	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptors_1, descriptors_2, matches);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "ʹ��BFMatcherƥ����������ʱ��%ld����\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;

	Mat img_matches;
	//��һ��ƥ��
	refineMatchesWithHomography(keypoints_1, keypoints_2, 3, matches, img_matches);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "��һ��ƥ���ʱ��%ld����\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;


	//����ƥ����Ĺؼ���
	drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_matches);
	imwrite("match.jpg", img_matches);

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "����ƥ����Ĺؼ����ʱ��%ld����\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;

	//��λ���á���ƥ���
	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < matches.size(); i++)
	{
		//DMathch������queryIdx��ָmatch�е�һ�����������,keyPoint������ptָ���ǵ�ǰ������
		obj.push_back(keypoints_1[matches[i].queryIdx].pt);
		scene.push_back(keypoints_2[matches[i].trainIdx].pt);
	}

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "��λ���á���ƥ����ʱ��%ld����\r\n", tEnd - tStartTemp);
	tStartTemp = GetTickCount();
	edit_text += timeStrBuf;

	Mat H = findHomography(obj, scene, CV_RANSAC);
	vector<Point2f> obj_corners(4), scene_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint(img_1.cols, 0);
	obj_corners[2] = cvPoint(img_1.cols, img_1.rows);
	obj_corners[3] = cvPoint(0, img_1.rows);

	perspectiveTransform(obj_corners, scene_corners, H);

	//���ƽǵ�֮���ֱ��
	Rect silidOutRect;
	silidOutRect.x = scene_corners[1].x;
	silidOutRect.y = scene_corners[1].y + Point2f(img_1.cols, 0).y;
	silidOutRect.width = img_1.cols*2.3;
	silidOutRect.height = img_1.rows;

	if (silidOutRect.area() < 2000)
		return;
	//���ͼ��
	//imshow("ƥ��ͼ��.jpg", img_matches);
	imshow("ss.jpg", img_2(silidOutRect));

	tEnd = GetTickCount();
	memset(timeStrBuf, 0, 64);
	sprintf(timeStrBuf, "\r\n�ܺ�ʱ��%ld����\r\n", tEnd - tStart);
	edit_text += timeStrBuf;
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);
}


void CMFCApplication1Dlg::OnBnClickedButton10()
{
	Mat src, dst;
	src = imread(imgPath);

	Mat  gray_src;
	cvtColor(src, gray_src, CV_BGR2GRAY);


	//ת��Ϊ��ֵͼ��
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

	imwrite("ˮƽ��.jpg", dst);

}


void CMFCApplication1Dlg::OnBnClickedButton13()
{
	//MessageBox(_T("��ã�um��"));
	Mat  imme = imread("E://photo_opencv/pic_temp/temp_2359.jpg");

	MatND hist;       // ��cv����CvHistogram *hist = cvCreateHist  
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };   //������ҪΪconst����  
	int size = 256;
	int channels = 0;
	//����ͼ���ֱ��ͼ  
	calcHist(&imme, 1, &channels, Mat(), hist, dims, &size, ranges);    //cv����cvCalcHist  
	int scale = 1;
	Mat imageShow(size * scale, size, CV_8U, Scalar(0));
	//��ȡ���ֵ����Сֵ  
	double minVal = 0;
	double maxVal = 0;
	minMaxLoc(hist, &minVal, &maxVal, 0, 0);    //cv���õ���cvGetMinMaxHistValue  
												//��ʾֱ��ͼ��ͼ��  
	int hpt = saturate_cast<int>(0.9 * size);

	for (int i = 0; i < 256; i++)
	{
		float value = hist.at<float>(i);           // ע��hist����float����    cv����cvQueryHistValue_1D  
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
//	//����ֱ��ͼ    
//	calcHist(&Gray_img, 1, channels, Mat(), // do not use mask      
//		hist, 1, hist_size, ranges,
//		true, // the histogram is uniform      
//		false);
//	if (hist.data == 0) return false;
//	return true;
//}
//
//void one_channel_hist_specify(Mat input_img, Mat dst_img, Mat &output_img)//��ͨ��  
//{
//	int i, j;
//
//	//�������룬�涨ͼ���ֱ��ͼ  
//	MatND input_hist, dst_hist;
//	Cal_Hist(input_img, input_hist);
//	Cal_Hist(dst_img, dst_hist);
//
//	//�������ֱ��ͼ  
//	MatND input_p_hist, dst_p_hist;
//	input_p_hist = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//ԭʼ����ֱ��ͼ  
//	dst_p_hist = MatND::zeros(dst_hist.size[0], dst_hist.size[1], CV_32FC1);//�涨����ֱ��ͼ  
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
//	//�����ۼ�ֱ��ͼ  
//	MatND input_c_hist, dst_c_hist;
//	input_c_hist = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//ԭʼ�ۼ�ֱ��ͼ  
//	dst_c_hist = MatND::zeros(dst_hist.size[0], dst_hist.size[1], CV_32FC1);//�涨�ۼ�ֱ��ͼ  
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
//	//���㵥ӳ�����  
//	MatND SML = MatND::zeros(input_hist.size[0], input_hist.size[1], CV_32FC1);//SML��ӳ�����  
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
//	//�������ͼ��  
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Mat src, gray, src2, gray2;
	src = imread("E://photo_opencv/pic_temp/");
	cvtColor(src, gray, CV_RGB2GRAY);  //ת���ɻҶ�ͼ    
	src2 = imread("136.jpg");
	cvtColor(src2, gray2, CV_RGB2GRAY);  //ת���ɻҶ�ͼ    
	Mat output_img;
	//three_channel_hist_specify(src, src2, output_img);
	one_channel_hist_specify(gray, gray2, output_img);
	imshow("����������", output_img);
	waitKey(0);
	//imwrite("out.jpg", output_img);
}


void CMFCApplication1Dlg::OnBnClickedButton44()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Mat imme = imread("E://photo_opencv/pic_temp/136.jpg");
	CString edit_text;
	char timeStrBuf[64];
	memset(timeStrBuf, 0, 64);
	int num = slidetwo(imme);
	sprintf(timeStrBuf, "\r\n�г���ͼƬ��%d ��\r\n", num);
	edit_text += timeStrBuf ;
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);

}

int slidetwo(Mat inImage)
{
	int sliderNum = 0;
	Mat srcImage = inImage.clone();  //Դͼ��
	if (!srcImage.data)
	{
		return -1;
	}

	int slider1 = 163;

	for (int i = 0; i < srcImage.rows; i++)
	{
		uchar* p = srcImage.ptr(i);//��ȡMatĳһ�е��׵�ַ  

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
	//Ԥ����
	//Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

	//Mat openImg = specifiImg(srcImage);

	//Mat openImg = srcImage.clone();

	//morphologyEx(openImg, openImg, MORPH_OPEN, element);//������  

	Mat canny_output;
	cvtColor(openImg, openImg, CV_BGR2GRAY);
	Canny(openImg, canny_output, 50, 200, 3);//����canny��Ե���  	

											 //Mat dst = canny_output<100;
	imwrite("Canny.jpg", canny_output);

	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;
	//���μ��
	findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	//��ÿ���ҵ���������������б�ı߽�� 
	vector<RotatedRect> minRect(contours.size());
	for (unsigned int i = 0; i < contours.size(); i++)
	{
		minRect[i] = minAreaRect(Mat(contours[i]));
	}
	//�����б�ı߽��  
	CvBox2D  End_Rage2D;

	//����ÿ�����ο�just for test...
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

		//��⵽���γ��Ϳ�ı�
		float ratio;
		if (minRect[i].size.width > minRect[i].size.height)
		{
			ratio = minRect[i].size.width / minRect[i].size.height;
		}
		else {
			ratio = minRect[i].size.height / minRect[i].size.width;
		}
		//ɸѡ���������ı�ǩ�����Σ���������ֵ��slideMinPx��slideMaxPx֮�䣬�����ֵ��ratioMin��ratioMax֮��
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

			//˳ʱ����б��End_Rage2D.angle = ʵ����б����
			//��ʱ����б��End_Rage2D.angle = -90 + ʵ����б����������
			if (angle < -45)
			{
				angle += 90;
			}
			//С��0��(����0)��ʱ��������Ҫ�任���ܵó�ROI����silidRect��������
			if (angle <= 0)
			{
				Point2f p2fTemp = rect_points[3];
				rect_points[3] = rect_points[2];
				rect_points[2] = rect_points[1];
				rect_points[1] = rect_points[0];
				rect_points[0] = p2fTemp;
			}

			//�и�1.1����ǩ��С����ֹ��б��ı�ǩ���ݻᱻ�ص�
			Rect silidRect;
			silidRect.width = (int)sqrt((pow((rect_points[0].x - rect_points[1].x), 2) + pow((rect_points[0].y - rect_points[1].y), 2)));
			silidRect.height = (int)sqrt((pow((rect_points[0].x - rect_points[3].x), 2) + pow((rect_points[0].y - rect_points[3].y), 2)));
			silidRect.x = (int)(rect_points[2].x - silidRect.width*(silieMul - 1) / 2);
			silidRect.y = (int)(rect_points[2].y - silidRect.height*(silieMul - 1) / 2);
			silidRect.width *= silieMul;
			silidRect.height *= silieMul;

			//�и���߽�ᵼ�±���
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
			//ֱ��ͼ���⻯
			//cvtColor(sliderOutImg, sliderOutImg, CV_BGR2GRAY);
			//equalizeHist(sliderOutImg, sliderOutImg);		
			imwrite("slider_" + to_string(sliderNum) + ".jpg", sliderOutImg);
		}
	}
	return  sliderNum;
}
