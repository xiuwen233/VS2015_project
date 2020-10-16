
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
Mat sliderImg[50];//ÿ����ǩ
Mat maskImg;
float rotateMinAngle = 0.3;//��ת����������̽Ƕȣ����ڸ�ֵ����У������
int slideMinPx = 400;//��ǩ�ߴ緶Χ�����أ�
int slideMaxPx = 700;

int silideNum = 0;
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
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CMFCApplication1Dlg::OnNMCustomdrawSlider1)
	ON_BN_CLICKED(IDC_BUTTON5, &CMFCApplication1Dlg::OnBnClickedButton5)
	ON_WM_DROPFILES()
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
	pSlidCtrl->SetPos(42);
	slider1 = 42;
	
	GetDlgItem(IDC_EDIT1)->SetWindowText(_T("��ͼƬ�϶����˴���Ȼ������ʼʶ��"));
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


//��תͼ�񣨽�����
void imRoTate(Mat& img, Mat& newIm, double angle) {
	int len = max(img.cols, img.rows);
	Point2f pt(len / 2., len / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(img, newIm, r, Size(img.cols, img.rows));
}

//��һ��ƥ�䣬Ѱ��Դͼ��Ŀ��ͼ��֮���͸�ӱ任
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
	//ͨ��ɸѡƥ��ؼ���֮���ת�����ҵ�����ƽ�棨ͼ��֮���͸�ӱ任
	homography = findHomography(srcPoints, dstPoints, CV_FM_RANSAC, reprojectionThreshold, inliersMask);
	vector<DMatch> inliers;
	//���ң��õ���ȷ��ƥ����  
	for (size_t i = 0; i<inliersMask.size(); i++)
	{
		if (inliersMask[i])
			inliers.push_back(matches[i]);
	}
	matches.swap(inliers);
	return matches.size() > minNumberMatchesAllowed;
}

//��ȡ����n���ַ�
char* substrend(char *str, int n)
{
	char * substr = (char*)malloc(n + 1);
	int length = strlen(str);
	if (n >= length)//����ȡ���ȴ����ַ������ȣ���ֱ�ӽ�ȡȫ���ַ���
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
	int perPixelValue;//ÿ�����ص�ֵ

	int* projectValArrys = new int[height];
	memset(projectValArrys, 0, height * 4);//�����ʼ������

										   //����ͳ��
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

	/*�½�һ��Mat���ڴ���ͶӰֱ��ͼ����������Ϊ��ɫ*/
	Mat verticalProjectionMats(height, width, CV_8UC1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			verticalProjectionMats.at<uchar>(i, j) = 255;
		}
	}

	/*��ֱ��ͼ��������Ϊ��ɫ*/
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

//ʶ��Mac
string recoMacForEachCharater(Mat image)
{
	cvtColor(image, image, CV_RGB2GRAY);
	int blockSize = 7;
	int constValue = 11;
	Mat local;
	adaptiveThreshold(image, local, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, blockSize, constValue);

	int width = local.cols;
	int height = local.rows;
	int perPixelValue;//ÿ�����ص�ֵ

	int* projectValArrys = new int[height];
	memset(projectValArrys, 0, height * 4);//�����ʼ������

	//����ͳ��
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

	//�½�һ��Mat���ڴ���ͶӰֱ��ͼ����������Ϊ��ɫ
	Mat verticalProjectionMats(height, width, CV_8UC1);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			verticalProjectionMats.at<uchar>(i, j) = 255;
		}
	}

	//��ֱ��ͼ��������Ϊ��ɫ
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
	for (i = 0; i < out.cols; i++)//��
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
		for (; i < out.cols; i++)//��߽�
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

		for (; i < out.cols; i++)//����
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

//ʶ��ÿ���и�õı�ǩ
string reconiseSlideForMac(Mat slideOutImg, Mat maskImg)
{
	Mat img_1 = maskImg.clone();
	Mat img_2 = slideOutImg.clone();
	//���Ŀ���Լ�ģ����Ե�������
	vector<KeyPoint> keypoints_1, keypoints_2;
	Ptr<FastFeatureDetector> fast = FastFeatureDetector::create(slider1, true, 1);
	fast->detect(img_1, keypoints_1);
	fast->detect(img_2, keypoints_2);
	//������Ե�������������������������
	Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	Mat descriptors_1, descriptors_2;
	f2d->compute(img_1, keypoints_1, descriptors_1);
	f2d->compute(img_2, keypoints_2, descriptors_2);
	//ʹ��BFMatcherƥ��������
	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptors_1, descriptors_2, matches);
	//��һ��ƥ�䣬ɸѡƥ����
	Mat img_matches;	
	refineMatchesWithHomography(keypoints_1, keypoints_2, 3, matches, img_matches);
	//����ƥ����Ĺؼ���
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_matches);
	//��λ׼ȷ��ƥ���
	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < matches.size(); i++)
	{
		//DMathch������queryIdx��ָmatch�е�һ�����������,keyPoint������ptָ���ǵ�ǰ������
		obj.push_back(keypoints_1[matches[i].queryIdx].pt);
		scene.push_back(keypoints_2[matches[i].trainIdx].pt);
	}
	//�ҵ�Ŀ����ͼƬ�е�ӳ�����
	Mat H = findHomography(obj, scene, CV_RANSAC);
	vector<Point2f> obj_corners(4), scene_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint(img_1.cols, 0);
	obj_corners[2] = cvPoint(img_1.cols, img_1.rows);
	obj_corners[3] = cvPoint(0, img_1.rows);
	//����ӳ��������͸�ӱ任
	perspectiveTransform(obj_corners, scene_corners, H);
	//�õ�ROI
	Rect silidOutRect;
	silidOutRect.x = scene_corners[1].x;
	silidOutRect.y = scene_corners[1].y + Point2f(img_1.cols, 0).y;
	silidOutRect.width = img_1.cols * 2.3;
	silidOutRect.height = img_1.rows;
	//�и�����и�ʱ���߽絼���ڴ���ʳ���
	if (silidOutRect.x + silidOutRect.width < img_2.cols
		&& silidOutRect.y + silidOutRect.height < img_2.rows
		&& silidOutRect.width * silidOutRect.height > 10000)
	{
		//TessBaseAPI��ʶ���������
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
		return "ʶ��ʧ��";
	}
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
	//imwrite("Canny.jpg", canny_output);

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
	//imwrite("drawingAllRect.jpg", drawingAllRect);


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
		//ɸѡ���������ı�ǩ�����Σ���������ֵ��slideMinPx��slideMaxPx֮��
		if (minRect[i].size.width > slideMinPx &&
			minRect[i].size.width < slideMaxPx &&
			minRect[i].size.height > slideMinPx &&
			minRect[i].size.height < slideMaxPx)
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
			//imwrite("slide_" + to_string(sliderNum) + ".jpg", sliderOutImg);
		}
	}
	return  sliderNum;
}

UINT ThreadFun(LPVOID pParam) {
	
	int* pNum = (int*)pParam;
	string str = "�߳�" + to_string((int)*pNum)+"�Ѿ����С�\r\n";
	CString cstring(str.c_str());
	//reconiseStr += recoSiliderMac(sliderImg[(int)*pNum], maskImg);
	reconiseStr += str;
	return 0;
}


void CMFCApplication1Dlg::OnBnClickedButton5()
{
	long tStart;//��ʱ
	long tEnd;
	char timeStrBuf[64];
	int readcont = 0;
	tStart = GetTickCount();
	CString edit_text;//��ʾ���༭����ַ���
	Mat inImage = imread(imgPath);
	maskImg = imread(maskImgPathStr);
	cvtColor(maskImg, maskImg, CV_RGB2GRAY);

	int Num = slideImg(inImage);
	if (Num == -1)
	{
		MessageBox(_T("ͼƬ��ȡʧ�ܣ�"));
		return;
	}
	if (Num == -2)
	{
		MessageBox(_T("��ͼƬ�и�����쳣��"));
		return;
	}

	reconiseStr = "";
	CWinThread* pThread;
	int reconisImgNum = 0;
	for (int i = 0; i < Num; i++)
	{		
		int* pNum = &i; 
		//pThread = AfxBeginThread(ThreadFun, pNum, THREAD_PRIORITY_HIGHEST, 0, 0);
		reconiseStr += ("MAC��ַ��" + reconiseSlideForMac(sliderImg[i], maskImg) + "\r\n");
		silideNum++;
	}

	if (reconiseStr.length() > 0)
	{
		edit_text = reconiseStr.c_str();
		tEnd = GetTickCount();
		sprintf(timeStrBuf, "\r\n��ʱ��%ld����\r\n", tEnd - tStart);
		edit_text += timeStrBuf;
	}else{
		sprintf(timeStrBuf, "�и%d�ű�ǩ������ԭͼ", Num);
		edit_text = timeStrBuf;
	}
	GetDlgItem(IDC_EDIT1)->SetWindowText(edit_text);
}

