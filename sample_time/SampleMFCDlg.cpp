
// SampleMFCDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SampleMFC.h"
#include "SampleMFCDlg.h"
#include "afxdialogex.h"
#include"SerialPort.h"
#include"HDEXtoDC.h"

#include<iostream>
#include<string>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include <fstream>
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace cv;
//����һЩȫ�ֱ���
bool is_cmd_running = true;//������ͣ����
bool is_send_cmd = true;  //���ڷ��Ϳ���
bool is_recv_com_data = false;//���ձ�־λ�����ڿ��Ƶȴ���һ��ָ���ִ�У�
bool is_detect_running = false; //��ʼ���Լ��
bool videostop = false;
CString com_recv_cstr;//���յ����ַ���

CSerialPort m_Port;  //���ڶ���
int TX_count = 0;  //���㷢�͵�����
int socket_port ;
SOCKET s_server;// = socket(AF_INET, SOCK_STREAM, 0);
				//����˵�ַ�ͻ��˵�ַ  
SOCKADDR_IN server_addr;
SOCKET *psClintt = new SOCKET;
WSADATA wsadata;
BOOL m_isOpenSocket; //�������ӱ�־

Mat mask_img;//ƥ��ģ��ͼ��

int sleep_ms = 400; 
//---------------------------�ļ�����------------------------------------
FILE *file;
char filename[] = {"recorddata.txt"};
char othername[] = { "whathappen" };
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

//-----------------------------------------------------------------------------------------------------------opencv---------------------------------------------------------------------------
 // ------------------------- ��ȡ����ͷ�ķ�Χ-----------------------------------
int rect_img_x = 254; 
int rect_img_y = 180;
int rect_img_w = 140;
int rect_img_h = 204;

std::ofstream com_log_file;
std::ofstream point_log_file;

int bin_thresh = 23;//��ֵ����ֵ
int match_type = 4;//ģ��ƥ�䷽��
int erase_max = 700;//Ĩȥ�����������ص������൱��ȥ���ȶ���
int erase_min = 0;

int mask_img_r = 113;//ģ��ƥ��Բ�뾶
float coordinate_y = 0;//ʵʱ��¼������
int suck_num = 0;//��ȡ�Ľŵ����
int cols_num = 38;//һ���Ͻŵ�����
int suck_add_x = 360;//���쵽����ͷ����
int dis_box_pad = 306;//���ӽŵ�����
bool is_waited = true;
bool draw_gbr_img = false;

Mat capture_img; //���ڼ��ͼ��Ҷ�ֵ
int m_center_x = 70;
int m_center_y = 102;
Point center(m_center_x, m_center_y);//���ĵ�
//---------------------------------
//һЩ��Ҫ�ĳ�ʼ��
bool sys_init()
{
	//��ʼ��ģ��ƥ��ģ��ͼ��
	Mat mask = Mat(Size(mask_img_r + 2, mask_img_r + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(mask_img_r / 2 + 1, mask_img_r / 2 + 1), mask_img_r / 2, Scalar(255, 255, 255), 1);
	mask_img = mask.clone();

	return 0;
}
// --------------------------------
static void ContrastAndBright(int, void *)
{
	Mat m = Mat(600, 200, 0);
	imshow("��������塿", m);

	Mat mask = Mat(Size(mask_img_r + 2, mask_img_r + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(mask_img_r / 2 + 1, mask_img_r / 2 + 1), mask_img_r / 2, Scalar(255, 255, 255), 1);
	//imshow("mask", mask);
	mask_img = mask.clone();
}
									 //����opencv�������
void initControlBoard()
{
	namedWindow("��������塿", 0);

	createTrackbar("mask_img_r��", "��������塿", &mask_img_r, 180, ContrastAndBright);
	createTrackbar("erase_max��", "��������塿", &erase_max, 1000, ContrastAndBright);
	createTrackbar("erase_min��", "��������塿", &erase_min, 500, ContrastAndBright);

	createTrackbar("bin_thresh��", "��������塿", &bin_thresh, 200, ContrastAndBright);
	createTrackbar("match_type��", "��������塿", &match_type, 5, ContrastAndBright);

	createTrackbar("rect_img_x��", "��������塿", &rect_img_x, 500, ContrastAndBright);
	createTrackbar("rect_img_y��", "��������塿", &rect_img_y, 500, ContrastAndBright);
	createTrackbar("rect_img_w��", "��������塿", &rect_img_w, 500, ContrastAndBright);
	createTrackbar("rect_img_h��", "��������塿", &rect_img_h, 500, ContrastAndBright);

	createTrackbar("sleep_ms��", "��������塿", &sleep_ms, 400, ContrastAndBright);
}
//----------------------------------------------------���ϻҶ�ֵ���-----------------------------
int HistGet(Mat img)
{
	MatND hist;       // ��cv����CvHistogram *hist = cvCreateHist  
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };   //������ҪΪconst����  
	int size = 256;
	int channels = 0;
	//����ͼ���ֱ��ͼ  
	calcHist(&img, 1, &channels, Mat(), hist, dims, &size, ranges);    //cv����cvCalcHist  
	int hpt = saturate_cast<int>(0.9 * size);

	double minVal = 0;
	double maxVal = 0;
	int scale = 1;
	Mat imageShow(size * scale, size, CV_8U, Scalar(0));

	minMaxLoc(hist, &minVal, &maxVal, 0, 0);    //cv���õ���cvGetMinMaxHistValue  
	int max_realValue = 0;
	int max_position;
	for (int i = 0; i < 256; i++)
	{
		float value = hist.at<float>(i);           // ע��hist����float����    cv����cvQueryHistValue_1D  
		int realValue = saturate_cast<int>(value * hpt / maxVal);
		if (realValue  >  max_realValue)
		{
			max_realValue = realValue;
			max_position = i;
		}
		rectangle(imageShow, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255));
	}
	//imshow("ֱ��ͼ", imageShow);
	return max_position;
}
//----------------------------------------------------------------------------------------

//--------------ģ��ƥ��---
void match_temp(Mat bitwise_out, Mat templ)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//����ƥ��
	matchTemplate(bitwise_out, templ, g_resultImage, match_type);

	//��׼��
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;

	//��λ��ƥ���λ��
	minMaxLoc(g_resultImage, &minValue, &maxValue, &minLocation, &maxLocation);

	if (match_type == TM_SQDIFF || match_type == CV_TM_SQDIFF_NORMED)
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}

	int x = matchLocation.x + mask_img_r / 2;
	int y = matchLocation.y + mask_img_r / 2;


	center.x = x;
	center.y = y;


	//rectangle(srcImage, matchLocation, Point(matchLocation.x + templ.cols, matchLocation.y + templ.rows), Scalar(128, 128, 128), 2, 8, 0);
	//resize(g_resultImage, g_resultImage, Size(g_resultImage.cols * 2, g_resultImage.rows * 2));
	//imshow("ƥ�����", g_resultImage);

	//int radius = mask_img_r / 2;
	////Բ����  
	//circle(srcImage, center, 3, Scalar(128, 128, 128), -1, 8, 0);
	////Բ����  
	//circle(srcImage, center, radius, Scalar(128, 128, 128), 2, 8, 0);
	//String str = "[" + to_string(center.x) + "," + to_string(center.y) + "]";
	//putText(srcImage, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);

	////imshow("out", srcImage);

	//return srcImage;
}
					//CheckMode: 0����ȥ��������1����ȥ��������; NeihborMode��0����4����1����8����;  
void RemoveRegion(Mat& Src, Mat& Dst, int AreaLimitMin, int AreaLimitMax, int CheckMode, int NeihborMode)
{
	int RemoveCount = 0;       //��¼��ȥ�ĸ���  
							   //��¼ÿ�����ص����״̬�ı�ǩ��0����δ��飬1�������ڼ��,2�����鲻�ϸ���Ҫ��ת��ɫ����3������ϸ������  
	Mat Pointlabel = Mat::zeros(Src.size(), CV_8UC1);

	if (CheckMode == 1)
	{
		for (int i = 0; i < Src.rows; ++i)
		{
			uchar* iData = Src.ptr<uchar>(i);
			uchar* iLabel = Pointlabel.ptr<uchar>(i);
			for (int j = 0; j < Src.cols; ++j)
			{
				if (iData[j] < 10)
				{
					iLabel[j] = 3;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < Src.rows; ++i)
		{
			uchar* iData = Src.ptr<uchar>(i);
			uchar* iLabel = Pointlabel.ptr<uchar>(i);
			for (int j = 0; j < Src.cols; ++j)
			{
				if (iData[j] > 10)
				{
					iLabel[j] = 3;
				}
			}
		}
	}

	vector<Point2i> NeihborPos;  //��¼�����λ��  
	NeihborPos.push_back(Point2i(-1, 0));
	NeihborPos.push_back(Point2i(1, 0));
	NeihborPos.push_back(Point2i(0, -1));
	NeihborPos.push_back(Point2i(0, 1));
	if (NeihborMode == 1)
	{
		NeihborPos.push_back(Point2i(-1, -1));
		NeihborPos.push_back(Point2i(-1, 1));
		NeihborPos.push_back(Point2i(1, -1));
		NeihborPos.push_back(Point2i(1, 1));
	}
	int NeihborCount = 4 + 4 * NeihborMode;
	int CurrX = 0, CurrY = 0;
	//��ʼ���  
	for (int i = 0; i < Src.rows; ++i)
	{
		uchar* iLabel = Pointlabel.ptr<uchar>(i);
		for (int j = 0; j < Src.cols; ++j)
		{
			if (iLabel[j] == 0)
			{
				//********��ʼ�õ㴦�ļ��**********  
				vector<Point2i> GrowBuffer;  //��ջ�����ڴ洢������  
				GrowBuffer.push_back(Point2i(j, i));
				Pointlabel.at<uchar>(i, j) = 1;
				int CheckResult = 0;//�����жϽ�����Ƿ񳬳���С����0Ϊδ������1Ϊ����  

				for (int z = 0; z<GrowBuffer.size(); z++)
				{

					for (int q = 0; q<NeihborCount; q++)//����ĸ������  
					{
						CurrX = GrowBuffer.at(z).x + NeihborPos.at(q).x;
						CurrY = GrowBuffer.at(z).y + NeihborPos.at(q).y;
						if (CurrX >= 0 && CurrX<Src.cols&&CurrY >= 0 && CurrY<Src.rows) //��ֹԽ��  
						{
							if (Pointlabel.at<uchar>(CurrY, CurrX) == 0)
							{
								GrowBuffer.push_back(Point2i(CurrX, CurrY)); //��������buffer  
								Pointlabel.at<uchar>(CurrY, CurrX) = 1; //���������ļ���ǩ�������ظ����  
							}
						}
					}
				}
				if (GrowBuffer.size()<AreaLimitMax && GrowBuffer.size() > AreaLimitMin)
				{
					CheckResult = 2; //�жϽ�����Ƿ񳬳��޶��Ĵ�С����1Ϊδ������2Ϊ���� 
				}
				else { CheckResult = 1;   RemoveCount++; }
				for (int z = 0; z<GrowBuffer.size(); z++)//����Label��¼  
				{
					CurrX = GrowBuffer.at(z).x;
					CurrY = GrowBuffer.at(z).y;
					Pointlabel.at<uchar>(CurrY, CurrX) += CheckResult;
				}
			}
		}
	}

	CheckMode = 255 * (1 - CheckMode);
	//��ʼ��ת�����С������  
	for (int i = 0; i < Src.rows; ++i)
	{
		uchar* iData = Src.ptr<uchar>(i);
		uchar* iDstData = Dst.ptr<uchar>(i);
		uchar* iLabel = Pointlabel.ptr<uchar>(i);
		for (int j = 0; j < Src.cols; ++j)
		{
			if (iLabel[j] == 2)
			{
				iDstData[j] = CheckMode;
			}
			else if (iLabel[j] == 3)
			{
				iDstData[j] = iData[j];
			}
		}
	}

}

/*
*����������  brightnessException     ���㲢����һ��ͼ���ɫƫ���Լ���ɫƫ����
*����������  InputImg   ��Ҫ�����ͼƬ��BGR��Ÿ�ʽ����ɫ��3ͨ�������Ҷ�ͼ��Ч
*            cast       �������ƫ��ֵ��С��1��ʾ�Ƚ�����������1��ʾ���������쳣��
��cast�쳣ʱ��da����0��ʾ������daС��0��ʾ����
*��������ֵ�� ����ֵͨ��cast��da�������÷��أ�����ʽ����ֵ
*/
void brightnessException(Mat InputImg, float& cast, float& da)
{
	Mat GRAYimg;
	cvtColor(InputImg, GRAYimg, CV_BGR2GRAY);
	float a = 0;
	int Hist[256];
	for (int i = 0; i<256; i++)
		Hist[i] = 0;
	for (int i = 0; i<GRAYimg.rows; i++)
	{
		for (int j = 0; j<GRAYimg.cols; j++)
		{
			a += float(GRAYimg.at<uchar>(i, j) - 128);//�ڼ�������У�����128Ϊ���Ⱦ�ֵ��  
			int x = GRAYimg.at<uchar>(i, j);
			Hist[x]++;
		}
	}
	da = a / float(GRAYimg.rows*InputImg.cols);
	float D = abs(da);
	float Ma = 0;
	for (int i = 0; i<256; i++)
	{
		Ma += abs(i - 128 - da)*Hist[i];
	}
	Ma /= float((GRAYimg.rows*GRAYimg.cols));
	float M = abs(Ma);
	float K = D / M;
	cast = K;
	return;
}



//��һ���̼߳��----------------------------------------------------------------------------
DWORD WINAPI DetectThread(LPVOID pParam)
{
	VideoCapture capture(1);

	Mat show_image;

	VideoWriter write;
	//д����Ƶ�ļ���  
	//const string outFlie = "fcq.avi";

	char chats[256];
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(chats, "%4d_%02d_%02d_%02d_%02d_%02d_%03d.avi", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	const string outFlie = chats;
	int w = 140;
	int h = 204;
	Size S(w, h);
	bool b_write_video = true;
	//����Ƶ�ļ���׼��д��  
	if (write.open(outFlie, -1, 25.0, S, true) == false)
	{
		MessageBox(NULL, "write open ʧ�ܣ�", "��ʾ", MB_OK);
		b_write_video = false;
	}

	while (1)
	{
		if (!is_detect_running)
		{
			break;
		}

		double t = (double)cv::getTickCount();

		capture >> show_image;

		if (!show_image.data)
		{
			MessageBox(NULL, "����ͷ1��ʧ�ܣ�����������ͷ0������", "��ʾ", MB_OK);
			capture.open(0);
			capture >> show_image;
			if (!show_image.data)
			{
				is_cmd_running = false;
				//is_comm_action = false;
				MessageBox(NULL, "����ͷ0��ʧ�ܣ�����", "��ʾ", MB_OK);
				break;
			}
		}

		//��ȡ��������
		show_image = show_image(Rect(rect_img_x, rect_img_y, rect_img_w, rect_img_h));
		Mat color_img = show_image.clone();
		capture_img = show_image.clone();

		float cast, da;
		brightnessException(show_image, cast, da);
		String strs = "ca:" + to_string(cast);
		putText(show_image, strs, Point(0, 10), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);
		strs = "da:" + to_string(da);
		putText(show_image, strs, Point(0, 23), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);

		line(show_image, Point(0, show_image.rows / 2), Point(show_image.cols, show_image.rows / 2), Scalar(0, 0, 255));
		line(show_image, Point(show_image.cols / 2, 0), Point(show_image.cols / 2, show_image.rows), Scalar(0, 0, 255));

		Mat gray_img;
		cvtColor(color_img, gray_img, CV_BGR2GRAY);

		//�Ҷ�С��һ��ֵ��ʱ��������ȵ���
		int b_thresh = bin_thresh;
		if (da < -40.0)
		{
			b_thresh += abs(da + 40) / 3;
		}

		Mat m_threshold;
		threshold(gray_img, m_threshold, b_thresh, 255, 1);

		RemoveRegion(m_threshold, m_threshold, erase_min, erase_max, 1, 1);

		imshow("bin_img", m_threshold);

		match_temp(m_threshold, mask_img);

		Mat detect_out_img = Mat(gray_img.size(), CV_8UC3, Scalar::all(0));

		//Բ����  
		circle(detect_out_img, center, 3, Scalar(0, 255, 255), -1, 8, 0);
		//Բ����  
		circle(detect_out_img, center, mask_img_r / 2, Scalar(0, 255, 255), 1, 8, 0);
		String str = "[" + to_string(center.x) + "," + to_string(center.y) + "]";
		putText(detect_out_img, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);

		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();

		putText(detect_out_img, to_string(t * 1000), Point(10, 10), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);

		imshow("detect_out", detect_out_img);

		if (draw_gbr_img)
		{
			//Բ����
			circle(show_image, center, 3, Scalar(0, 255, 255), -1, 8, 0);

			//Բ����
			circle(show_image, Point(center.x, center.y), mask_img_r / 2 - 2, Scalar(0, 255, 255), 4, 8, 0);

			String str = "[" + to_string(center.x) + "," + to_string(center.y) + "]";
			putText(show_image, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255), 1);

			strs = "dx:" + to_string(center.x - m_center_x);
			putText(show_image, strs, Point(0, 36), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);

			strs = "dy:" + to_string(center.y - m_center_y);
			putText(show_image, strs, Point(0, 49), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);
		}

		imshow("show_image", show_image);

		if (b_write_video)
		{
			write.write(show_image);
		}
		waitKey(1);
	}

	write.release();
	return 0;
}

//-----------------------------------------------------------------------------------------------opencv------------------------------------------------------------------------------------
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSampleMFCDlg �Ի���



CSampleMFCDlg::CSampleMFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SAMPLEMFC_DIALOG, pParent)
	, m_dataShow(_T(""))
	, m_dataport(_T(""))
	, m_datasocket(_T(""))
	, m_socketip(_T(""))
	, m_socketport_int(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSampleMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PORTCOM, m_portset);
	DDX_Control(pDX, IDC_BOTELV, m_botelvset);
	DDX_Control(pDX, IDC_CONNECT_PORT, m_connect_port);
	DDX_Control(pDX, IDC_STATUE, m_statue);
	DDX_Control(pDX, IDC_TIMEcout, m_count);
	DDX_Control(pDX, IDC_RECVDATA_DATA, m_recvdata);
	DDX_Text(pDX, IDC_RECVDATA_DATA, m_dataShow);
	DDX_Text(pDX, IDC_DATAport, m_dataport);
	DDX_Text(pDX, IDC_DATAsocket, m_datasocket);
	DDX_Control(pDX, IDC_SOCKETIP, m_SOCKET);
	DDX_Text(pDX, IDC_SOCKETIP, m_socketip);
	DDX_Control(pDX, IDC_SOCKET_PORT, m_socketport);
	//  DDX_Text(pDX, IDC_SOCKET_PORT, m_socketportstr);
	DDX_Text(pDX, IDC_SOCKET_PORT, m_socketport_int);
	DDX_Control(pDX, IDC_PHOT0_1, m_photo_1);
	DDX_Control(pDX, IDC_serialPORT, m_SERIAPORT);
	DDX_Control(pDX, IDC_OPENCVTEST, m_opencvstart);
}

BEGIN_MESSAGE_MAP(CSampleMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT_PORT, &CSampleMFCDlg::OnBnClickedConnectPort)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_TEST, &CSampleMFCDlg::OnBnClickedTest)
	ON_MESSAGE(WM_COMM_RXCHAR, OnCommunication)
	ON_BN_CLICKED(IDC_SENDDATA_PORT, &CSampleMFCDlg::OnBnClickedSenddataPort)
	ON_BN_CLICKED(IDC_CONNECT_SOCKET, &CSampleMFCDlg::OnBnClickedConnectSocket)
	ON_BN_CLICKED(IDC_SENDDATA_SOCKET, &CSampleMFCDlg::OnBnClickedSenddataSocket)
	ON_BN_CLICKED(IDC_StopSOCKET, &CSampleMFCDlg::OnBnClickedStopsocket)
	ON_BN_CLICKED(IDC_OPENCVTEST, &CSampleMFCDlg::OnBnClickedOpencvtest)
	ON_BN_CLICKED(IDC_CONTROLWID, &CSampleMFCDlg::OnBnClickedControlwid)
	ON_BN_CLICKED(IDC_FILEWRITE, &CSampleMFCDlg::OnBnClickedFilewrite)
	ON_BN_CLICKED(IDC_DOESFILE, &CSampleMFCDlg::OnBnClickedDoesfile)
	ON_BN_CLICKED(IDC_FILEREAD, &CSampleMFCDlg::OnBnClickedFileread)
	ON_BN_CLICKED(IDC_BINARYwrite, &CSampleMFCDlg::OnBnClickedBinarywrite)
	ON_BN_CLICKED(IDC_binaryreadu, &CSampleMFCDlg::OnBnClickedbinaryreadu)
	ON_BN_CLICKED(IDC_videocapture, &CSampleMFCDlg::OnBnClickedvideocapture)
	ON_BN_CLICKED(IDC_stopviseo, &CSampleMFCDlg::OnBnClickedstopviseo)
	ON_BN_CLICKED(IDC_GETTOFIRST, &CSampleMFCDlg::OnBnClickedGettofirst)
	ON_BN_CLICKED(IDC_STOPPROCESS, &CSampleMFCDlg::OnBnClickedStopprocess)
END_MESSAGE_MAP()


// CSampleMFCDlg ��Ϣ�������

BOOL CSampleMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	sys_init();
	m_portset.SetCurSel(3);
	m_botelvset.SetCurSel(1);

	m_hIconOff = AfxGetApp()->LoadIcon(IDI_ICON2);
	m_hIconOPcv = AfxGetApp()->LoadIcon(IDI_ICON1);

	com_log_file.open("com_log.txt", std::ofstream::app);
	com_log_file << "-----------------------------------------" << endl;
	point_log_file.open("adjus_log.txt", std::ofstream::app);
	point_log_file << "-----------------------------------------" << endl;

	m_SERIAPORT.SetIcon(m_hIconOff);
	//��ʼ��һЩ���ڲ���
	m_photo_1.SetIcon(m_hIconOff);
	m_cParity = 'N';
	m_nDatabits = 8;
	m_nStopbits = 1;
	m_dwCommEvents = EV_RXFLAG | EV_RXCHAR;
	m_bOpenPort = false;
	m_isOpenSocket = false;
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSampleMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSampleMFCDlg::OnPaint()
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
HCURSOR CSampleMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//-------------------------------------�Զ�����Ϣ  ���ܴ�����Ϣ-------------------------------------------------
int rxdatacount = 0; //������ܵ�����  �ñ������ڽ����ַ�����
void seveLog(char * str, int type);
LRESULT CSampleMFCDlg::OnCommunication(WPARAM ch, LPARAM port)
{
	if (port <= 0 || port > 11)
		return -1;
	rxdatacount++;   //���յ��ֽڼ���
	CString strTemp;
	strTemp.Format("%ld", rxdatacount);
	strTemp = "RX:" + strTemp;
	m_count.SetWindowText(strTemp);  //��ʾ���ռ���

	if (m_recvdata.GetLineCount() >= 20)
	{
		m_dataShow.Empty();
		UpdateData(false);
	}
	//if (m_bStopDispRXData)   //���ѡ���ˡ�ֹͣ��ʾ���������ݣ��򷵻�
	//	return -1;          //ע�⣬��������£��������ڼ�����ֻ�ǲ���ʾ
	//�������ˡ��Զ���ա�����ﵽ50�к��Զ���ս��ձ༭������ʾ������
	if  (m_recvdata.GetLineCount() >= 50)
		{
			m_dataShow.Empty();
		    UpdateData(FALSE);
	}
	//���û�С��Զ���ա��������дﵽ400��Ҳ�Զ����
	//��Ϊ���ݹ��࣬Ӱ������ٶȣ���ʾ�����CPUʱ��Ĳ���
	//if (m_ctrlReceiveData.GetLineCount()>400)
	//{
	//	m_ReceiveData.Empty();
	//	m_ReceiveData = "***The Length of the Text is too long, Emptied Automaticly!!!***\r\n";
	//UpdateData(FALSE);
	//}

	//���ѡ����"ʮ��������ʾ"������ʾʮ������ֵ
	CString str;
	//	if (m_ctrlHexReceieve.GetCheck())
	str.Format("%02X ", ch);
	com_recv_cstr = str;
	is_recv_com_data = true;
	seveLog(str.GetBuffer(), 2);
	str.ReleaseBuffer();
	
	//else
	//	str.Format("%c", ch);
	//�����ǽ����յ��ַ������ַ�������������ʱ�ܶ�
	//�����ǵ�������Ҫ������ļ�������û����List Control
	int nLen = m_recvdata.GetWindowTextLength();
	m_recvdata.SetSel(nLen, nLen);
	m_recvdata.ReplaceSel(str);
	nLen += str.GetLength();

	m_dataShow += str;
	return 0;
}

//���Ӵ��ڰ�ť
void CSampleMFCDlg::OnBnClickedConnectPort()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_bOpenPort)  //�رմ���
	{
		m_connect_port.SetWindowText("�򿪴���");
		m_Port.ClosePort();//�رմ���
		m_statue.SetWindowText("�����ѹر�");
		m_SERIAPORT.SetIcon(m_hIconOff);
	}
	else  //�򿪴���
	{
		m_connect_port.SetWindowText("�رմ���");

		int menth = m_botelvset.GetCurSel();
		CString str;
		m_botelvset.GetLBText(menth, str);
		m_nBaud = atoi(str);

		m_nCom = m_portset.GetCurSel() + 1;
		//int menth = m_ipslect.GetCurSel();

		CString strStatus;
		if (m_Port.InitPort(this, m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits, m_dwCommEvents, 512))
		{
			m_Port.StartMonitoring();
			//m_ctrlIconOpenoff.SetIcon(m_hIconRed);
			m_SERIAPORT.SetIcon(m_hIconOPcv);
			strStatus.Format("STATUS��COM%d OPENED��%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
			//"��ǰ״̬�����ڴ򿪣�����żУ�飬8����λ��1ֹͣλ");
		}
		else
		{
			AfxMessageBox("û�з��ִ˴��ڻ�ռ��");
			//m_ctrlIconOpenoff.SetIcon(m_hIconOff);
		}
		m_statue.SetWindowText(strStatus);
	}

	m_bOpenPort = !m_bOpenPort;

}

// �رյ�һЩ���� ȥ��
void CSampleMFCDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (MessageBox(_T("ȷ���˳���"), _T("��ʾ"), MB_YESNO | MB_ICONWARNING)
		== IDNO)
	{ 

		is_detect_running = false;
		videostop = false;
		if (m_bOpenPort == true)
		{
			m_Port.ClosePort();
		}
		if (m_isOpenSocket == true)
		{
			closesocket(s_server);
		}
		point_log_file.close();
		com_log_file.close();
		return;
	}
	CDialogEx::OnClose();
}

//�������Եĸ���С��demo
void CSampleMFCDlg::OnBnClickedTest()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	/*int menth = m_botelvset.GetCurSel();
	CString str;
	m_botelvset.GetLBText(menth, str);
	m_nBaud = atoi(str);
	CString statue;
	statue.Format("%d",m_nBaud);
	m_statue.SetWindowTextA(statue);*/
	is_detect_running = true;
	CreateThread(NULL, 0, DetectThread, NULL, 0, NULL);

}

//----------------------------------------------------�ļ�����-------------------------------
void seveLog(char * str, int type)
{

	char tmpBuf[256];

	SYSTEMTIME sys;
	GetLocalTime(&sys);

	switch (type)
	{
	case 1:
		sprintf(tmpBuf, "%4d/%02d/%02d %02d:%02d:%02d.%03d TX %s\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, str);
		break;
	case 2:
		sprintf(tmpBuf, "%4d/%02d/%02d %02d:%02d:%02d.%03d RX %s\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, str);
		break;
	default:
		break;
	}

	//log_str += tmpBuf;

	com_log_file << tmpBuf;
	//free(tmpBuf);
}
// ------------------------------�ֶ�����ǿ�Ʒ���һ��ָ����ܷ���״̬��-----------------------------------
int force_send_cmd(char * str)
{
	if (m_Port.m_hComm == NULL)
	{
		AfxMessageBox("����û�д򿪣���򿪴���");
		return -1;
	}

	seveLog(str, 1);

	char data[512];
	int len = _t_Str2Hex(str, data);
	m_Port.WriteToPort(data, len);
	return 0;
}
//----------------------------�жϷ���ֵ�����
bool send_cmd(char * str)
{
	while (!is_send_cmd)
	{
		char ch[128];
		sprintf(ch, "�ϸ�ָ��δ��ɣ�����̫��:%s\r\n", str);
		CString cstr(ch);
		AfxMessageBox(cstr);
		is_send_cmd = true;
	}

	while (!is_cmd_running)
	{
		Sleep(1);
	}

	is_send_cmd = false;

	force_send_cmd(str);

	//�ȴ����ڷ��غ�˵����ָ���������ڽ���
	char c3[3] = { str[2], str[3], ' ' };

	while (1)
	{
		if (is_recv_com_data && com_recv_cstr == c3)
		{
			is_recv_com_data = false;
			is_send_cmd = true;
			break;
		}
		Sleep(1);
	}
	return 0;
}

// -----------------------------------������ʹ�������---------------------
void CSampleMFCDlg::OnBnClickedSenddataPort()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_Port.m_hComm == NULL)
	{
		//m_ctrlAutoSend.SetCheck(0);
		AfxMessageBox("����û�д򿪣���򿪴���");
		return;
	}
	else
	{
		UpdateData(TRUE);
		//д������16��������
		char data[512];
		int len = _t_Str2Hex(m_dataport, data);
		if (len == 0)
		{
			AfxMessageBox("������ϢΪ��");
			return;
		}
		m_Port.WriteToPort(data, len);

		TX_count++;	

		CString strTemp;
		strTemp.Format("TX:%d  %s", TX_count, m_dataport);
		m_statue.SetWindowText(strTemp);
	}
}


//����������Ϣ���߳� ����CString���͵�ֵ
DWORD WINAPI FunSend(LPVOID lpParameter) {

	CString psSend = *(CString *)(lpParameter);

	if (m_isOpenSocket == false)
	{
		AfxMessageBox("�����������");
		return 0;
	}
	char buffs[4];
	// >> buffs;
	int len = _t_Str2Hex(psSend, buffs);
	int isend = send(s_server, buffs, sizeof(buffs), 0);
	if (isend == SOCKET_ERROR) {
		AfxMessageBox("��������ʧ��");
		return 0;
	}
	return 0;
}


//�����߳����ڽ�������
DWORD WINAPI FunRsever(LPVOID lpParameter) {

	CEdit* thrRse = (CEdit *)(lpParameter);
	
	char recvBuf[10];
	unsigned char recvBuf1[10];
	char changeBuf[10];
	int num_1 = 0;
	int num_2 = 0;
	while (1)
	{
		memset(recvBuf, 0, sizeof(recvBuf));
		memset(changeBuf, 0, sizeof(changeBuf));
		int m_len = recv(s_server, recvBuf, sizeof(recvBuf), 0);
		if (strlen(recvBuf) != 0) {
			memcpy(recvBuf1, recvBuf, m_len);
			datatoHEx(m_len, changeBuf, recvBuf1);
			CString str;		
			str.Format("%s", changeBuf);
			//thrRse->SetWindowTextA(str);
			int nLen = thrRse->GetWindowTextLength();
			thrRse->SetSel(nLen, nLen);
			thrRse->ReplaceSel(str);
			nLen += str.GetLength();
		}
		else {
			thrRse->SetWindowTextA("�������  ����������");
			 m_isOpenSocket = false; 
			closesocket(s_server);
			return 0;
		}
	}
}

//-------------------------------------------------��֪��ʲô��-------------------------
int HistGetMaxVal(Mat img)
{
	MatND hist;
	int dims = 1;
	float hranges[] = { 0, 255 };
	const float *ranges[] = { hranges };
	int size = 256;
	int channels = 0;
	calcHist(&img, 1, &channels, Mat(), hist, dims, &size, ranges);
	int hpt = saturate_cast<int>(0.9 * size);

	double minVal = 0;
	double maxVal = 0;
	int scale = 1;
	Mat imageShow(size * scale, size, CV_8U, Scalar(0));

	minMaxLoc(hist, &minVal, &maxVal, 0, 0);
	return maxVal;
}

//------------------------------------------�ֵ�------------------------------------------------
bool correctFirst()
{
	Sleep(sleep_ms);

	int err_x = center.x - m_center_x;
	int err_y = center.y - m_center_y;

	int err_trans_x = err_x / 0.4667;//�ŵ淽��
	int err_trans_y = err_y / 1.2666;//��ͷ����

	char str[32];
	char str_d[32];//�¶�΢��

				   //��������
	if (err_trans_x < 0)
	{
		sprintf(str, "EB15%02x", -err_trans_x);
		send_cmd(str);
	}
	else if (err_trans_x > 0)
	{
		sprintf(str, "EB16%02x", err_trans_x);
		send_cmd(str);
	}
	seveLog("----------34-----------", 1);
	//y��΢��
	if (err_trans_y < 0)
	{
		sprintf(str, "EB13%02x", -err_trans_y);
		sprintf(str_d, "EB17%02x", -err_trans_y);
		send_cmd(str);
		send_cmd(str_d);
		coordinate_y -= (-err_trans_y);
	}
	else if (err_trans_y > 0)
	{
		sprintf(str, "EB14%02x", err_trans_y);
		sprintf(str_d, "EB18%02x", err_trans_y);
		send_cmd(str);
		send_cmd(str_d);
		coordinate_y += err_trans_y;
	}
	seveLog("-------25-------------", 1);
	return true;
}

//-------------------------------�Ӿ�΢��--------------------------------------------------------
bool correctLittle()
{
	Sleep(sleep_ms / 2);
	draw_gbr_img = true;
	Sleep(sleep_ms / 2);

	int err_x = center.x - m_center_x;
	int err_y = center.y - m_center_y;

	char log_str[256];
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(log_str, "%4d/%02d/%02d %02d:%02d:%02d.%03d err_x:%02d, err_y:%02d\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, err_x, err_y);
	point_log_file << log_str;

	char str[32];
	char str_d[32];//�¶�΢��
	// ---------------------------�����ж�λ���Ƿ���Ҫ����
	Point center_3[4];
	if (err_x > 15 || err_y > 20)
	{   
		center_3[0] = center;
		Sleep(sleep_ms);
		err_x = center.x - m_center_x;
		err_y = center.y - m_center_y;
		center_3[1] = center;
		if (err_x > 15 || err_y > 20)
		{ 
			Sleep(sleep_ms);
			err_x = center.x - m_center_x;
			err_y = center.y - m_center_y;
			center_3[2] = center;
			if (err_x > 15 || err_y > 20)
			{  
				if (center_3[0].x == center_3[1].x && center_3[0].x == center_3[2].x  && center_3[0].y == center_3[1].y && center_3[0].y == center_3[2].y)
				{

				}
				else {
					AfxMessageBox(_T("�쳣��ͣ��"));
				}
				//AfxMessageBox(_T("�쳣��ͣ��"));
				//err_x = 0;
				//err_y = 0;
			}
		}
	}

	int err_trans_x = err_x / 0.4667;//�ŵ淽��
	int err_trans_y = err_y / 1.2666;//��ͷ����

	if (err_trans_x < 0)
	{
		sprintf(str, "EB15%02x", -err_trans_x);
		send_cmd(str);
	}
	else if (err_trans_x > 0)
	{
		sprintf(str, "EB16%02x", err_trans_x);
		send_cmd(str);
	}


	if (err_trans_y < 0)
	{
		sprintf(str, "EB13%02x", -err_trans_y);
		sprintf(str_d, "EB17%02x", -err_trans_y);
		send_cmd(str);
		send_cmd(str_d);
		coordinate_y -= (-err_trans_y);
	}
	else if (err_trans_y > 0)
	{
		sprintf(str, "EB14%02x", err_trans_y);
		sprintf(str_d, "EB18%02x", err_trans_y);
		send_cmd(str);
		send_cmd(str_d);
		coordinate_y += err_trans_y;
	}

	draw_gbr_img = false;
	return true;
}

//-------------------------------------------------------------------------------
// �������ӷ�����
//��������
void CSampleMFCDlg::OnBnClickedConnectSocket()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//--------------------------------��ʼ��--------------------------------
	if (m_isOpenSocket == true)
	{
		AfxMessageBox("�����Ѿ�����");
		return;
	}
	int err;
	err = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (err != 0) {
		//cout << "Failed to load Winsock" << endl;
		m_statue.SetWindowText(_T("Failed to load Winsock"));
		return;
	}

	s_server = socket(AF_INET, SOCK_STREAM, 0);  
	server_addr.sin_family = AF_INET;

	UpdateData(true);
	socket_port = m_socketport_int;
	if (socket_port == 0)
	{
		AfxMessageBox("������˿ں�");
		return;
	}
	CString str = m_socketip;
	
	char *ServerIP = (LPSTR)(LPCSTR)str;
	inet_pton(AF_INET, ServerIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_port = htons(socket_port);
	*psClintt = s_server;
	m_statue.SetWindowText(_T("׼������"));

	int nn = 0;	
	while (nn<5) {
		if (connect(s_server, (struct  sockaddr*)&server_addr, sizeof(server_addr)) == INVALID_SOCKET)
		{	
			Sleep(100);
		}
		else {
			m_statue.SetWindowText(_T("���ӳɹ�"));
			m_isOpenSocket = true;
			m_photo_1.SetIcon(m_hIconOPcv);
			nn = 1;
			break;
		}
		nn++;
	}

	if (m_isOpenSocket == true)
	{
		m_statue.SetWindowText(_T("׼���������ݣ�����IP %s   "+ str));
	      CreateThread(NULL, 0, FunRsever, &m_recvdata, 0, NULL);
	}
	else {
		m_statue.SetWindowText(_T("����ʧ��"));
		AfxMessageBox("��ȷ������IP���˿���ȷ���Լ�����˿���");
	}
}

//����ֶ�����SOcket����
void CSampleMFCDlg::OnBnClickedSenddataSocket()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	CreateThread(NULL, 0, FunSend, &m_datasocket, 0, NULL);
}

//�ر���������
void CSampleMFCDlg::OnBnClickedStopsocket()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_isOpenSocket == true)
	{  
		m_isOpenSocket =false;
		closesocket(s_server);
	
	}
}


//-----------------------------------------------
DWORD WINAPI JUSTgoANDBACK(LPVOID pParam)
{
	for (int i = 0; i < 20; i++)
	{
		for (int n = 5; n < 10; n++)
		{
			send_cmd("EB122D34");
			send_cmd("EB112D34");
			Sleep(20);
		}
	}
}

//---------------------------------------------------�Լ�д���̼߳��-------------------------------
DWORD WINAPI USEMYSELF(LPVOID pParam)
{
	while (true)
	{
		Sleep(1000);
		//�ŵ����
		while (true)
		{
			if (HistGet(capture_img) < 128)
			{
				Sleep(sleep_ms);
				if (HistGet(capture_img) < 128)
				{
					Sleep(sleep_ms);
					if (HistGet(capture_img) < 128)
					{
						break;
					}
				}
			}
			send_cmd("EB1515");
		}

		//�Զ�����������λ��
		while (true)
		{
			//����n֡���귽��жϼ�����Ƿ��ȶ�
			int  frame_n = 3;
			int sum_x = 0;
			int sum_x_2 = 0;
			int sum_y = 0;
			int sum_y_2 = 0;
			int average_x = 0;
			int average_y = 0;
			int variance_x = 0;
			int variance_y = 0;

			for (int i = 0; i < frame_n; i++)
			{
				Sleep(sleep_ms);
				sum_x += (center.x - m_center_x);
				sum_x_2 += (center.x - m_center_x) *(center.x - m_center_x);
				sum_y += (center.y - m_center_y);
				sum_y_2 += (center.y - m_center_y) *(center.y - m_center_y);

				average_x = sum_x / (i + 1);
				average_y = sum_y / (i + 1);
				variance_x = sum_x_2 / (i + 1) - (sum_x / (i + 1)) * (sum_x / (i + 1));
				variance_y = sum_y_2 / (i + 1) - (sum_y / (i + 1)) * (sum_y / (i + 1));
				if (variance_x>10 || variance_y>50)
				{
					send_cmd("EB1515");
					break;
				}
			}

			average_x = sum_x / frame_n;
			average_y = sum_y / frame_n;
			variance_x = sum_x_2 / frame_n - (sum_x / frame_n) * (sum_x / frame_n);
			variance_y = sum_y_2 / frame_n - (sum_y / frame_n) * (sum_y / frame_n);

			if (variance_x<10 && variance_y<50 && center.x > 65 && center.x<75)
			{
				send_cmd("EB15FA");
				break;
			}
			else {
				send_cmd("EB1515");
			}

		}

		//�ֵ�
		correctFirst();
		seveLog("----------------------", 1);
		//�������ŵ�
		char str[32];
		int cols_6;
		int cols_5;
		float fast_dist = 0.0;
		

		for (int i = 0; i < cols_num; i++)
		{
			//��һ��6��
			for (cols_6 = 0; cols_6 < 6; cols_6++)
			{
				correctLittle();
				sprintf(str, "EB11%04x", suck_add_x);
				send_cmd(str);
				coordinate_y += (125 + suck_add_x / 4);

				//if (suck_add_y > 0)
				//{
				//	sprintf(str, "EB16%02x", suck_add_y);
				//	send_cmd(str);
				//}else if(suck_add_y < 0)
				//{
				//	sprintf(str, "EB15%02x", -suck_add_y);
				//	send_cmd(str);
				//}

				//send_cmd("EB09");//��ͷ����
				//send_cmd("EB01");//��ͷ��ѹ
				//send_cmd("EB03");//���涥��
				//send_cmd("EB02");//��ͷ��ѹ
				//send_cmd("EB04");//���涥��

				//if (suck_add_y > 0)
				//{
				//	sprintf(str, "EB15%02x", suck_add_y);
				//	send_cmd(str);
				//}
				//else if (suck_add_y < 0)
				//{
				//	sprintf(str, "EB16%02x", -suck_add_y);
				//	send_cmd(str);
				//}

				fast_dist = coordinate_y - dis_box_pad - 125;

				sprintf(str, "EB12%04x", (int)(fast_dist * 4));
				send_cmd(str);
				coordinate_y -= (fast_dist + 125);

				////�������
				//while (is_waited)
				//{
				//	Sleep(1);
				//}

				//send_cmd("EB01");//��ͷ��ѹ
				//send_cmd("EB0A");//��ͷ����
				//send_cmd("EB02");//��ͷ��ѹ

				//foce_send_cmd("EB80");
				//Sleep(50);
				suck_num++;

				if (cols_6 == 5)
				{
					send_cmd("EB157D");
					send_cmd("EB0652");//�¶�
					fast_dist -= (suck_add_x / 4 + 125 - 81);
					sprintf(str, "EB11%04x", (int)(fast_dist * 4));
					send_cmd(str);
					coordinate_y += (fast_dist + 125);
				}
				else {
					//send_cmd("EB12009B");
					send_cmd("EB07A5");
					fast_dist -= (suck_add_x / 4 + 125 + 155 / 4 + 125);
					sprintf(str, "EB11%04x", (int)(fast_dist * 4));
					send_cmd(str);
					coordinate_y += (fast_dist + 125);
				}
				is_waited = true;
			}

			//�ڶ���5��
			for (cols_5 = 0; cols_5 < 5; cols_5++)
			{
				correctLittle();
				sprintf(str, "EB11%04x", suck_add_x);
				send_cmd(str);
				coordinate_y += (125 + suck_add_x / 4);

				//if (suck_add_y > 0)
				//{
				//	sprintf(str, "EB16%02x", suck_add_y);
				//	send_cmd(str);
				//}
				//else if (suck_add_y < 0)
				//{
				//	sprintf(str, "EB15%02x", -suck_add_y);
				//	send_cmd(str);
				//}

				//send_cmd("EB09");//��ͷ����
				//send_cmd("EB01");//��ͷ��ѹ
				//send_cmd("EB03");//���涥��
				//send_cmd("EB02");//��ͷ��ѹ
				//send_cmd("EB04");//���涥��

				//if (suck_add_y > 0)
				//{
				//	sprintf(str, "EB15%02x", suck_add_y);
				//	send_cmd(str);
				//}
				//else if (suck_add_y < 0)
				//{
				//	sprintf(str, "EB16%02x", 0-suck_add_y);
				//	send_cmd(str);
				//}

				fast_dist = coordinate_y - dis_box_pad - 125;

				sprintf(str, "EB12%04x", (int)(fast_dist * 4));
				send_cmd(str);
				coordinate_y -= (fast_dist + 125);

				////�������
				//while (is_waited)
				//{
				//	Sleep(1);
				//}

				//send_cmd("EB01");//��ͷ��ѹ
				//send_cmd("EB0A");//��ͷ����
				//send_cmd("EB02");//��ͷ��ѹ

				//foce_send_cmd("EB80");
				//Sleep(50);
				suck_num++;

				if (cols_5 == 4)
				{
					send_cmd("EB157D");
					//send_cmd("EB1451");
					send_cmd("EB0652");//�¶�
					fast_dist -= (suck_add_x / 4 + 125 - 81);
					sprintf(str, "EB11%04x", (int)(fast_dist * 4));
					send_cmd(str);
					coordinate_y += (fast_dist + 125);

					//if (((i+1)%3)!=0)
					//{
					//	send_cmd("EB1701");
					//}
				}
				else {
					send_cmd("EB06A5");
					//send_cmd("EB11009B");
					fast_dist -= (suck_add_x / 4 - 155 / 4);
					sprintf(str, "EB11%04x", (int)(fast_dist * 4));
					send_cmd(str);
					coordinate_y += (fast_dist + 125);
				}
				is_waited = true;
			}
		}

		//�ŵ����
		while (true)
		{
			send_cmd("EB15FF");
			if (HistGetMaxVal(capture_img) > 3500 && HistGet(capture_img) > 250)
			{
				Sleep(sleep_ms);
				if (HistGetMaxVal(capture_img) > 3500 && HistGet(capture_img) > 250)
				{
					Sleep(sleep_ms);
					if (HistGetMaxVal(capture_img) > 3500 && HistGet(capture_img) > 250)
					{
						break;
					}
				}
			}
		}

	}
	return 0;
}

// ���ڿ���opencv�̲߳��Ե�
void CSampleMFCDlg::OnBnClickedOpencvtest()
{
	//if (is_detect_running)
	//{
	//	is_detect_running = false;
	//	destroyAllWindows();
	//	 m_opencvstart.SetWindowText("��ʼ���");
	//}
	//else {
		//is_detect_running = true;
		CreateThread(NULL, 0, USEMYSELF, NULL, 0, NULL);
		//CreateThread(NULL, 0, DetectThread, NULL, 0, NULL); JUSTgoANDBACK mthresh
		//GetDlgItem(IDC_BTN_STOP_DETECT)->SetWindowText("ֹͣ���");
		//m_opencvstart.SetWindowText("ֹͣ���");
	//}
}


void CSampleMFCDlg::OnBnClickedControlwid()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	initControlBoard();
}


void CSampleMFCDlg::OnBnClickedFilewrite()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if ((file = fopen(filename,"w+")) == NULL)
	{
		m_statue.SetWindowText("�ļ���ʧ��");
		fclose(file);
		return;
	}
	for (int i = 0; i < 5;i++)
	{
	   if (fputs(filename,file) == EOF  || fputs("\n", file) == EOF)
	      	{
		     	m_statue.SetWindowText("��������ʧ��");
		     	fclose(file);
		     	return;
           	}
	}
	fclose(file);
	m_statue.SetWindowText("�����������");
}


void CSampleMFCDlg::OnBnClickedDoesfile()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int num= access(filename, 0);
	if (num == -1)
	{
		m_statue.SetWindowText("�ļ�������");
	}
	else {
		m_statue.SetWindowText("�ļ�����");
	
	}
}


void CSampleMFCDlg::OnBnClickedFileread()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if ((file = fopen(filename, "r")) == NULL)
	{
		m_statue.SetWindowText("�ļ�������");
		fclose(file);
		return;
	}
	char databuff[1024];
	CString sttr;
	while (fgets(databuff,1024,file)  != NULL)
	{
		int nLen = m_recvdata.GetWindowTextLength();
		m_recvdata.SetSel(nLen, nLen);
		sttr.Format("%s",databuff);
		m_recvdata.ReplaceSel(sttr);
		nLen += sttr.GetLength();
		m_dataShow += sttr;
	}
	m_statue.SetWindowText("�ļ���ȡ���");
	fclose(file);
}


void CSampleMFCDlg::OnBnClickedBinarywrite()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if ((file = fopen(othername, "wb+")) == NULL)
	{
		m_statue.SetWindowText("�ļ���ʧ��");
		fclose(file);
		return;
	}
	for (int n = 0; n < 5;n++)
	{
		if (fwrite(&othername,sizeof(othername),1,file) !=1)
		{
			m_statue.SetWindowText("�������ļ�д��ʧ��");
			fclose(file);
			return;
		}
	}
	m_statue.SetWindowText("�������ļ�д��ɹ�");
	fclose(file);
}


void CSampleMFCDlg::OnBnClickedbinaryreadu()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if ((file = fopen(othername, "rb")) == NULL)
	{
		m_statue.SetWindowText("�ļ���ʧ��");
		fclose(file);
		return;
	}
	char databuff[1024];
	CString sttr;
	while (fread(&databuff,sizeof(othername),1,file) ==1)
	{
		int nLen = m_recvdata.GetWindowTextLength();
		m_recvdata.SetSel(nLen, nLen);
		sttr.Format("%s", databuff);
		m_recvdata.ReplaceSel(sttr);
		nLen += sttr.GetLength();
		m_dataShow += sttr;
	}
	m_statue.SetWindowText("�ļ���ȡ���");
	fclose(file);
}

DWORD WINAPI videoRECORD(LPVOID lpParameter) {

	// TODO: �ڴ���ӿؼ�֪ͨ����������
	VideoCapture captrue(0);
	//��Ƶд�����  
	VideoWriter write;
	//д����Ƶ�ļ���  
	string outFlie = "E://fcq.avi";
	//���֡�Ŀ��  
	int w = static_cast<int>(captrue.get(CV_CAP_PROP_FRAME_WIDTH));
	int h = static_cast<int>(captrue.get(CV_CAP_PROP_FRAME_HEIGHT));
	/*int w = 140;
	int h = 204;*/
	Size S(w, h);
	//���֡��  
	double r = captrue.get(CV_CAP_PROP_FPS);
	//����Ƶ�ļ���׼��д��  
	//write.open(outFlie, -1, 30.0, S, true);
	write.open("E://fcq.avi", -1, 30.0, S, true);
	//��ʧ��  
	if (!captrue.isOpened())
	{
		return 1;
	}
	bool stop = false;
	Mat frame;
	videostop = true;
	while (1)
	{
		captrue >> frame;
		//frame = frame(Rect(258, 97, 140, 204));
		imshow("src_img", frame);
		write.write(frame);
		if (waitKey(20) == 'q')
		{
			break;
		}
		if (videostop == false)
		{
			break;
		}
	}
	
	write.release();
	return 0;
}


void CSampleMFCDlg::OnBnClickedvideocapture()
{
	CreateThread(NULL, 0, videoRECORD,NULL, 0, NULL);
}


void CSampleMFCDlg::OnBnClickedstopviseo()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	videostop = false;
}

//��λ����
DWORD WINAPI PositionReset(LPVOID pParam)
{
	send_cmd("EB10");//��λ
	send_cmd("EB112E19");//�ƶ�������
	send_cmd("EB08");//�¶���λ
	Sleep(30);

	send_cmd("EB1764");//��΢�����ƶ�
	Sleep(30);

	send_cmd("EB08");//�ٸ�λ
	Sleep(30);

	send_cmd("EB1737");//�¶���׼��ͷ
	Sleep(20);
	coordinate_y += 3075.25;
	return 0;
}

//DWORD WINAPI JUSTgoANDBACK(LPVOID pParam)
//{
//	for (int i = 0; i < 20; i++)
//	{
//		for (int n = 5; n < 10;n++)
//		{
//			send_cmd("EB122D34");
//			send_cmd("EB112D34");
//			Sleep(20);
//		}
//	}
//}


void CSampleMFCDlg::OnBnClickedGettofirst()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CreateThread(NULL, 0, PositionReset, NULL, 0, NULL);//��ʼ��ȡ����
}


void CSampleMFCDlg::OnBnClickedStopprocess()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (is_cmd_running)
	{
		is_cmd_running = false;
		//is_comm_action = false;
	}
}
