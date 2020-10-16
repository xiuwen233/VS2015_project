
// MFCApplicationDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MFCApplication.h"
#include "MFCApplicationDlg.h"
#include "afxdialogex.h"
#include "SerialPort.h"

//network
#include <WinSock2.h>  
#include <stdio.h>  
#include<WS2tcpip.h>
#include<Windows.h>
#pragma comment(lib, "ws2_32.lib")  


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Point center(92, 120);//Ҫ΢���������ĵ�
int mask_radio_r = 143;//ģ��ƥ��Բ�뾶
int dis_box_and_den = 2064;//��һ���ŵ浽���Ӿ���
int match_type = 2;//ģ��ƥ�䷽��
int thresh = 10;//��ֵ����ֵ
int minErrValue = 0;//�����������������
int maxErrValue = 10;//������
int desti_position = 0;//Ŀ��λ�ã�0��1��������λ��
float hole_distent = 785.5;//����������֮�����

float coordinate_y = 0;//ʵʱ��¼������

Mat mask_img;//ƥ��ģ��

int erase = 0;//Ĩȥ��С�����������ص������൱��ȥ���ȶ���

CSerialPort m_Port;  //CSerialPort�����

bool is_thread_running = false;

int m_slider_num = 1;//΢������

bool b_allow_send_cmd = true;

bool is_cmd_running = true;//��λ��ָ������״̬

bool b_allow_sucking = false;//��ʼ����

CString com_recv_cstr;//���յ����ַ���
bool is_recv_com_data = false;//���ձ�־λ�����ڿ��Ƶȴ���һ��ָ���ִ�У�

bool is_all_thread_running = false;

int suck_add_x = 0;//����΢��
int suck_add_y = 176;

int suck_num = 0;//��ȡ�Ľŵ�����

int cols_num = 40;//һ���Ͻŵ�����

CStatic CStatic_sunck_num;

char _t_HexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - 0x30;
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	else
		return 0x10;
}

int _t_Str2Hex(CString str, char* data)
{
	int t, t1;
	int rlen = 0, len = str.GetLength();
	//data.SetSize(len/2);
	for (int i = 0; i<len;)
	{
		char l, h = str[i];
		if (h == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len)
			break;
		l = str[i];
		t = _t_HexChar(h);
		t1 = _t_HexChar(l);
		if ((t == 16) || (t1 == 16))
			break;
		else
			t = t * 16 + t1;
		i++;
		data[rlen] = (char)t;
		rlen++;
	}
	return rlen;
}

//����һ��ָ��
bool send_cmd(char * str)
{
	if (m_Port.m_hComm == NULL)
	{
		//MessageBox(_T("����û�д򿪣���򿪴���"));
		AfxMessageBox("����û�д򿪣���򿪴���");
		return 1;
	}

	if (!b_allow_send_cmd)
	{
		char ch[128];
		sprintf(ch, "�ϸ�ָ��δ��ɣ�����̫��:%s\r\n", str);
		CString cstr(ch);
		AfxMessageBox(cstr);
		b_allow_send_cmd = true;
		return 1;
	}

	while (!is_cmd_running)
	{
		Sleep(1);
	}

	char data[512];
	b_allow_send_cmd = false;

	int len = _t_Str2Hex(str, data);
	m_Port.WriteToPort(data, len);

	char c2[3] = { str[2],str[3],' ' };
	while (1)
	{
		if (is_recv_com_data && com_recv_cstr == c2)
		{
			is_recv_com_data = false;
			break;
		}
		Sleep(1);
	}

	b_allow_send_cmd = true;

	return 0;
}



//ȫ�ֱ���
int port = 8234;
SOCKET s_server;

//����˵�ַ�ͻ��˵�ַ  
SOCKADDR_IN server_addr;
CString  back_true = "EB30";
CString  back_false = "EB31";

DWORD WINAPI FunSend(LPVOID lpParameter) {

	CString psSend = *(CString *)(lpParameter);
	char buffs[4];
	int len = _t_Str2Hex(psSend, buffs);
	int isend = send(s_server, buffs, sizeof(buffs), 0);
	if (isend == SOCKET_ERROR) {

		return 0;
	}
	return 0;
}

DWORD WINAPI IsSuckOkThread(LPVOID pParam)
{
	send_cmd("EB20");//���ӽ�
	send_cmd("EB0C");//�н�

	b_allow_sucking = true;

	int num = suck_num;

	while (true)
	{
		if ((suck_num - num) == 2)
		{
			b_allow_sucking = false;
			break;
		}
		Sleep(1);
	}

	send_cmd("EB0D");//�ɿ�

	send_cmd("EB21");//���ӳ�

	Sleep(2000);

	CreateThread(NULL, 0, FunSend, &back_true, 0, NULL);

	return 0;
}

//ǿ�Ʒ���һ��ָ����ܷ���״̬��
int fore_cend_cmd(char * str)
{
	if (m_Port.m_hComm == NULL)
	{
		AfxMessageBox("����û�д򿪣���򿪴���");
		return -1;
	}

	char data[512];
	int len = _t_Str2Hex(str, data);
	m_Port.WriteToPort(data, len);

	return 0;
}

//һЩ��Ҫ�ĳ�ʼ��
bool sys_init()
{
	//��ʼ��ģ��ƥ��ģ��ͼ��
	Mat mask = Mat(Size(mask_radio_r + 2, mask_radio_r + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(mask_radio_r / 2 + 1, mask_radio_r / 2 + 1), mask_radio_r / 2, Scalar(255, 255, 255), 1);
	mask_img = mask.clone();

	return 0;
}

//opencv���������Ӧ����
static void ContrastAndBright(int, void *)
{
	Mat m = Mat(600, 200, 0);
	imshow("��������塿", m);

	Mat mask = Mat(Size(mask_radio_r + 2, mask_radio_r + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(mask_radio_r / 2 + 1, mask_radio_r / 2 + 1), mask_radio_r / 2, Scalar(255, 255, 255), 1);
	//imshow("mask", mask);
	mask_img = mask.clone();
}

//����opencv�������
void initControlBoard()
{
	namedWindow("��������塿", 0);

	createTrackbar("mask_radio_r��", "��������塿", &mask_radio_r, 200, ContrastAndBright);
	createTrackbar("erase��", "��������塿", &erase, 100, ContrastAndBright);
	createTrackbar("thresh��", "��������塿", &thresh, 100, ContrastAndBright);
	createTrackbar("match_type��", "��������塿", &match_type, 5, ContrastAndBright);
}

//ģ��ƥ��
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

	int x = matchLocation.x + mask_radio_r / 2;
	int y = matchLocation.y + mask_radio_r / 2;

	if (x > 72 && x < 112 && y > 70 && y < 170)//����
	{
		center.x = x;
		center.y = y;
	}

	//rectangle(srcImage, matchLocation, Point(matchLocation.x + templ.cols, matchLocation.y + templ.rows), Scalar(128, 128, 128), 2, 8, 0);
	//resize(g_resultImage, g_resultImage, Size(g_resultImage.cols * 2, g_resultImage.rows * 2));
	//imshow("ƥ�����", g_resultImage);

	//int radius = mask_radio_r / 2;
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
void RemoveSmallRegion(Mat& Src, Mat& Dst, int AreaLimit, int CheckMode, int NeihborMode)
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
				if (GrowBuffer.size()>AreaLimit) CheckResult = 2; //�жϽ�����Ƿ񳬳��޶��Ĵ�С����1Ϊδ������2Ϊ����  
				else { CheckResult = 1;   RemoveCount++; }
				for (int z = 0; z<GrowBuffer.size(); z++)                         //����Label��¼  
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

DWORD WINAPI DetectThread(LPVOID pParam)
{
	VideoCapture captue(1);
	Mat image;
	Mat bin_img1;
	Mat bin_img2;
	Mat bin_img3;

	//Mat  templ = imread("mask.jpg", 0);

	while (1)
	{
		captue >> image;

		if (!image.data)
		{
			MessageBox(NULL, "����ͷ��ʧ�ܣ�����", "��ʾ", MB_OK);
			break;
		}

		if (!is_thread_running)
		{
			break;
		}

		image = image(Rect(image.cols / 4 + 90, image.rows / 4, image.cols / 2 - 135, image.rows / 2));

		Mat atd = image.clone();

		Mat color_img = image.clone();

		Mat gray_img;
		Mat canny;

		//--------------------------------------------------------------log
		Mat imageLog(image.size(), CV_32FC3);
		//	 printf("it is ok \n");
		for (int i = 0; i < image.rows; i++)
		{
			for (int j = 0; j < image.cols; j++)
			{
				imageLog.at<Vec3f>(i, j)[0] = log(1 + image.at<Vec3b>(i, j)[0]);
				imageLog.at<Vec3f>(i, j)[1] = log(1 + image.at<Vec3b>(i, j)[1]);
				imageLog.at<Vec3f>(i, j)[2] = log(1 + image.at<Vec3b>(i, j)[2]);
			}
		}
		//��һ����0~255    
		normalize(imageLog, imageLog, 0, 255, CV_MINMAX);
		convertScaleAbs(imageLog, imageLog);
		image = imageLog.clone();
		line(imageLog, Point(0, 120), Point(185, 120), Scalar(0, 0, 255));
		line(imageLog, Point(92, 0), Point(92, 240), Scalar(0, 0, 255));
		imshow("imageLog", imageLog);
		//----------------------------------

		cvtColor(color_img, gray_img, CV_BGR2GRAY);

		int nr = gray_img.rows;
		int nc = gray_img.cols;
		Mat bitwise_out = Mat(gray_img.size(), CV_8UC1, Scalar::all(0));
		for (int j = 0; j < nr; j++)
		{
			uchar* least_mat_data = gray_img.ptr<uchar>(j);
			uchar* bitwise_out_data = bitwise_out.ptr<uchar>(j);
			for (int i = 0; i < nc; i++)
			{
				if (least_mat_data[i] < thresh)
				{
					bitwise_out_data[i] = 255;
				}
			}
		}

		Rect ccomp;

		int i = 1, j = 1;
		do {
			int are1 = floodFill(bitwise_out, Point(i++, j++), Scalar(0, 0, 0), &ccomp, Scalar(), Scalar(), 8);
			int are2 = floodFill(bitwise_out, Point(bitwise_out.cols - i, j), Scalar(0, 0, 0), &ccomp, Scalar(), Scalar(), 8);
			if (are1 > 10 && are2 > 10)
			{
				break;
			}

		} while (1);


		RemoveSmallRegion(bitwise_out, bitwise_out, erase, 1, 1);

		Mat m_temp = bitwise_out.clone();
		imshow("bin_img", m_temp);
		match_temp(m_temp, mask_img);

		Mat detect_out_img = Mat(gray_img.size(), CV_8UC3, Scalar::all(0));

		//Բ����  
		circle(detect_out_img, center, 3, Scalar(0, 255, 255), -1, 8, 0);

		//Բ����  
		circle(detect_out_img, center, mask_radio_r / 2, Scalar(0, 255, 255), 1, 8, 0);
		String str = "[" + to_string(center.x) + "," + to_string(center.y) + "]";
		putText(detect_out_img, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);

		imshow("detect_out", detect_out_img);
		waitKey(1);
	}
	return 0;
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
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


// CMFCApplicationDlg �Ի���
CMFCApplicationDlg::CMFCApplicationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCAPPLICATION_DIALOG, pParent)
{
	m_ReceiveData = _T("");
	m_strSendData = _T("");

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bStopDispRXData = FALSE;
	m_bOpenPort = FALSE;
}

void CMFCApplicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_TXCOUNT, m_ctrlTXCount);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ctrlPortStatus);
	DDX_Control(pDX, IDC_STATIC_RXCOUNT, m_ctrlRXCOUNT);
	DDX_Control(pDX, IDC_BUTTON_MANUALSEND, m_ctrlManualSend);
	DDX_Control(pDX, IDC_BUTTON_CLEARRECASENDA, m_ctrlClearTXData);
	DDX_Control(pDX, IDC_BUTTON_COUNTRESET, m_ctrlCounterReset);
	DDX_Control(pDX, IDC_EDIT_SEND, m_ctrlEditSend);
	DDX_Control(pDX, IDC_EDIT_RECIVE, m_ctrlReceiveData);
	DDX_Control(pDX, IDC_CHECK_AUTOCLEAR, m_ctrlAutoClear);
	DDX_Control(pDX, IDC_STATIC_OPENOFF, m_ctrlIconOpenoff);
	DDX_Control(pDX, IDC_COMBO_STOPBITS, m_StopBits);
	DDX_Control(pDX, IDC_COMBO_DATABITS, m_DataBits);
	DDX_Control(pDX, IDC_COMBO_PARITY, m_Parity);
	DDX_Control(pDX, IDC_CHECK_HEXSEND, m_ctrlHexSend);
	DDX_Control(pDX, IDC_BUTTON_STOPDISP, m_ctrlStopDisp);
	DDX_Control(pDX, IDC_BUTTON_OPENPORT, m_ctrlOpenPort);
	DDX_Control(pDX, IDC_CHECK_HEXRECIEVE, m_ctrlHexReceieve);
	DDX_Control(pDX, IDC_COMBO_SPEED, m_Speed);
	DDX_Control(pDX, IDC_COMBO_COMSELECT, m_Com);
	DDX_Text(pDX, IDC_EDIT_RECIVE, m_ReceiveData);
	DDX_Text(pDX, IDC_EDIT_SEND, m_strSendData);
	DDX_Control(pDX, IDC_SLIDER_STEP_NUM, m_slider);
	DDX_Control(pDX, IDC_SLIDER_STEP_NUM2, m_slider2);
	DDX_Control(pDX, IDC_STATIC_TXCOUNT_Y, static_coordinate_y);
	DDX_Control(pDX, IDC_SLIDER_STEP_NUM3, m_slider3);
	DDX_Control(pDX, IDC_SLIDER_STEP_NUM4, m_slider4);
	DDX_Control(pDX, IDC_STATIC_TXCOUNT_Y2, CStatic_sunck_num);
}

BEGIN_MESSAGE_MAP(CMFCApplicationDlg, CDialogEx)
	ON_MESSAGE(WM_COMM_RXCHAR, OnCommunication)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_RECI_AREA, OnButtonClearReciArea)
	ON_BN_CLICKED(IDC_BUTTON_OPENPORT, OnButtonOpenport)
	ON_BN_CLICKED(IDC_BUTTON_STOPDISP, OnButtonStopdisp)
	ON_BN_CLICKED(IDC_BUTTON_MANUALSEND, OnButtonManualsend)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_EDIT_SEND, OnChangeEditSend)
	ON_CBN_SELENDOK(IDC_COMBO_COMSELECT, OnSelendokComboComselect)
	ON_CBN_SELENDOK(IDC_COMBO_SPEED, OnSelendokComboSpeed)
	ON_CBN_SELENDOK(IDC_COMBO_PARITY, OnSelendokComboParity)
	ON_CBN_SELENDOK(IDC_COMBO_DATABITS, OnSelendokComboDatabits)
	ON_CBN_SELENDOK(IDC_COMBO_STOPBITS, OnSelendokComboStopbits)
	ON_BN_CLICKED(IDC_BUTTON_COUNTRESET, OnButtonCountreset)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_DETECT, &CMFCApplicationDlg::OnBnClickedBtnDetect)
	ON_BN_CLICKED(IDC_BTN_STOP_DETECT, &CMFCApplicationDlg::OnBnClickedBtnStopDetect)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_CONBOARD, &CMFCApplicationDlg::OnBnClickedBtnConboard)
	ON_BN_CLICKED(IDC_BTN_TEST, &CMFCApplicationDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_UP, &CMFCApplicationDlg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BTN_DOWN, &CMFCApplicationDlg::OnBnClickedBtnDown)
	ON_BN_CLICKED(IDC_BTN_LEFT, &CMFCApplicationDlg::OnBnClickedBtnLeft)
	ON_BN_CLICKED(IDC_BTN_RIGHT, &CMFCApplicationDlg::OnBnClickedBtnRight)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_PUMP_UP, &CMFCApplicationDlg::OnBnClickedBtnPumpUp)
	ON_BN_CLICKED(IDC_BTN__PUMP_DOWN, &CMFCApplicationDlg::OnBnClickedBtnPumpDown)
	ON_BN_CLICKED(IDC_BTN_AIR_UP, &CMFCApplicationDlg::OnBnClickedBtnAirUp)
	ON_BN_CLICKED(IDC_BTN_AIR_DOWN, &CMFCApplicationDlg::OnBnClickedBtnAirDown)
	ON_BN_CLICKED(IDC_BTN_NEXT_BOX, &CMFCApplicationDlg::OnBnClickedBtnNextBox)
	ON_BN_CLICKED(IDC_BTN_NEXT_BOX_OUT, &CMFCApplicationDlg::OnBnClickedBtnNextBoxOut)
	ON_BN_CLICKED(IDC_BTN_BAND_UP, &CMFCApplicationDlg::OnBnClickedBtnBandUp)
	ON_BN_CLICKED(IDC_BTN_BAND_DOWN, &CMFCApplicationDlg::OnBnClickedBtnBandDown)
	ON_BN_CLICKED(IDC_BTN_SET_UP, &CMFCApplicationDlg::OnBnClickedBtnSetUp)
	ON_BN_CLICKED(IDC_BTN_SET_DOWN, &CMFCApplicationDlg::OnBnClickedBtnSetDown)
	ON_BN_CLICKED(IDC_BTN_FIX_UP, &CMFCApplicationDlg::OnBnClickedBtnFixUp)
	ON_BN_CLICKED(IDC_BTN_FIX_DOWN, &CMFCApplicationDlg::OnBnClickedBtnFixDown)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CMFCApplicationDlg::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_BAND_UP2, &CMFCApplicationDlg::OnBnClickedBtnBandUp2)
	ON_BN_CLICKED(IDC_BTN_BAND_DOWN2, &CMFCApplicationDlg::OnBnClickedBtnBandDown2)
	ON_BN_CLICKED(IDC_BTN_TEST3, &CMFCApplicationDlg::OnBnClickedBtnTest3)
	ON_BN_CLICKED(IDC_BTN_CONBOARD2, &CMFCApplicationDlg::OnBnClickedBtnConboard2)
	ON_BN_CLICKED(IDC_BTN_CONNECT_SERVER, &CMFCApplicationDlg::OnBnClickedBtnConnectServer)
END_MESSAGE_MAP()


// CMFCApplicationDlg ��Ϣ�������

//ʵʱ���Y����
DWORD WINAPI detect_coordinate_y(LPVOID pParam)
{
	CStatic* cst = (CStatic*)pParam;
	while (1) {
		if (!is_all_thread_running)
		{
			break;
		}
		string str = to_string(coordinate_y);
		CString cstr(str.c_str());
		cst->SetWindowText(cstr);

		str = to_string(suck_num);
		cstr.Format(str.c_str());
		CStatic_sunck_num.SetWindowText(cstr);
		Sleep(30);
	}
	return 0;
}


DWORD WINAPI FunRsever(LPVOID lpParameter) {

	char recvBuf[10];
	unsigned char recvBuf1[10];
	while (1)
	{
		memset(recvBuf, 0, sizeof(recvBuf));

		int m_len = recv(s_server, recvBuf, sizeof(recvBuf), 0);

		if (strlen(recvBuf) != 0) {
			memcpy(recvBuf1, recvBuf, m_len);

			if ((recvBuf1[0] == 0xEB) && (recvBuf1[1] == 0x30))
			{
				CreateThread(NULL, 0, IsSuckOkThread, NULL, 0, NULL);
			}
		}
	}
	closesocket(s_server);
}


bool  connect_server()
{
	WSADATA wsadata;
	int err;
	err = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (err != 0) {

		AfxMessageBox(_T("Failed to load Winsock"));
		return false;
	}

	s_server = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;

	char *ServerIP = "192.168.201.250";
	inet_pton(AF_INET, ServerIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_port = htons(port);

	int nn = 0;
	bool s_connect = false;
	while (nn<5) {
		if (connect(s_server, (struct  sockaddr*)&server_addr, sizeof(server_addr)) == INVALID_SOCKET)
		{
			Sleep(100);
		}
		else {

			AfxMessageBox(_T("���ӳɹ�"));
			s_connect = true;
			nn = 1;
			break;
		}
		nn++;
	}
	if (s_connect == true)
	{
		CreateThread(NULL, 0, FunRsever, NULL, 0, NULL);
	}
	else {
		AfxMessageBox(_T("����ʧ��"));
	}

	return true;
}


BOOL CMFCApplicationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�
	BOOL b = CDialog::OnInitDialog();
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
	m_Com.SetCurSel(0);
	m_Speed.SetCurSel(5);
	m_Parity.SetCurSel(0);
	m_DataBits.SetCurSel(0);
	m_StopBits.SetCurSel(0);

	m_hIconRed = AfxGetApp()->LoadIcon(IDI_ICON_RED);
	m_hIconOff = AfxGetApp()->LoadIcon(IDI_ICON_OFF);
	m_hIconGreen = AfxGetApp()->LoadIcon(IDI_ICON_GREEN);

	m_nBaud = 9600;
	m_nCom = 1;
	m_cParity = 'N';
	m_nDatabits = 8;
	m_nStopbits = 1;
	m_dwCommEvents = EV_RXFLAG | EV_RXCHAR;

	//ʮ�����Ʒ��͡���ʾ
	((CButton *)GetDlgItem(IDC_CHECK_HEXSEND))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_CHECK_HEXRECIEVE))->SetCheck(TRUE);

	//�Զ����
	m_ctrlIconOpenoff.SetIcon(m_hIconOff);

	sys_init();

	//����������
	m_slider.SetRange(1, 255);
	m_slider.SetTicFreq(1);
	m_slider.SetPos(m_slider_num);

	//�ŵ浽���ӻ�����
	m_slider2.SetRange(2000, 2100);
	m_slider2.SetTicFreq(1);
	m_slider2.SetPos(dis_box_and_den);
	String str2 = to_string(dis_box_and_den);
	CString cstr2(str2.c_str());
	GetDlgItem(IDC_STATIC_STEP5)->SetWindowText(cstr2);

	//����΢��X������
	m_slider3.SetRange(-20, 20);
	m_slider3.SetTicFreq(1);
	m_slider3.SetPos(suck_add_x);
	String str3 = to_string(suck_add_x);
	CString cstr3(str3.c_str());
	GetDlgItem(IDC_STATIC_STEP6)->SetWindowText(cstr3);

	//����Y������
	m_slider4.SetRange(150, 200);
	m_slider4.SetTicFreq(1);
	m_slider4.SetPos(suck_add_y);
	String str4 = to_string(suck_add_y);
	CString cstr4(str4.c_str());
	GetDlgItem(IDC_STATIC_STEP7)->SetWindowText(cstr4);

	m_strSendData = "";

	m_ctrlAutoClear.SetCheck(1);

	SetTimer(4, 200, NULL);

	UpdateData(FALSE);

	ShowWindow(SW_SHOW);

	is_all_thread_running = true;
	CreateThread(NULL, 0, detect_coordinate_y, &static_coordinate_y, 0, NULL);

	return b;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMFCApplicationDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCApplicationDlg::OnPaint()
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
HCURSOR CMFCApplicationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static long rxdatacount = 0;  //�ñ������ڽ����ַ�����WPARAM wParam,LPARAM lParam
LRESULT CMFCApplicationDlg::OnCommunication(WPARAM ch, LPARAM port)
{
	if (port <= 0 || port > 11)
		return -1;
	rxdatacount++;   //���յ��ֽڼ���
	CString strTemp;
	strTemp.Format("%ld", rxdatacount);
	strTemp = "RX:" + strTemp;
	m_ctrlRXCOUNT.SetWindowText(strTemp);  //��ʾ���ռ���

	if (m_bStopDispRXData)   //���ѡ���ˡ�ֹͣ��ʾ���������ݣ��򷵻�
		return -1;          //ע�⣬��������£��������ڼ�����ֻ�ǲ���ʾ
							//�������ˡ��Զ���ա�����ﵽ50�к��Զ���ս��ձ༭������ʾ������
	if ((m_ctrlAutoClear.GetCheck()) && (m_ctrlReceiveData.GetLineCount() >= 50))
	{
		m_ReceiveData.Empty();
		UpdateData(FALSE);
	}
	//���û�С��Զ���ա��������дﵽ400��Ҳ�Զ����
	//��Ϊ���ݹ��࣬Ӱ������ٶȣ���ʾ�����CPUʱ��Ĳ���
	if (m_ctrlReceiveData.GetLineCount()>400)
	{
		m_ReceiveData.Empty();
		m_ReceiveData = "***The Length of the Text is too long, Emptied Automaticly!!!***\r\n";
		UpdateData(FALSE);
	}

	//���ѡ����"ʮ��������ʾ"������ʾʮ������ֵ
	CString str;
	if (m_ctrlHexReceieve.GetCheck())
	{
		str.Format("%02X ", ch);
		is_recv_com_data = true;
		com_recv_cstr = str;
	}
	else
	{
		str.Format("%c", ch);
	}
	//�����ǽ����յ��ַ������ַ�������������ʱ�ܶ�
	//�����ǵ�������Ҫ������ļ�������û����List Control
	int nLen = m_ctrlReceiveData.GetWindowTextLength();
	m_ctrlReceiveData.SetSel(nLen, nLen);
	m_ctrlReceiveData.ReplaceSel(str);
	nLen += str.GetLength();

	m_ReceiveData += str;
	return 0;
}

//��ս�����
void CMFCApplicationDlg::OnButtonClearReciArea()
{
	// TODO: Add your control notification handler code here
	m_ReceiveData.Empty();
	UpdateData(FALSE);
}

//��/�رմ���
void CMFCApplicationDlg::OnButtonOpenport()
{
	if (m_bOpenPort)  //�رմ���
	{
		m_ctrlOpenPort.SetWindowText("�򿪴���");
		m_Port.ClosePort();//�رմ���
		m_ctrlPortStatus.SetWindowText("״̬�������ѹر�");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);
		m_bOpenPort = false;
	}
	else  //�򿪴���
	{
		CString strStatus;
		if (m_Port.InitPort(this, m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits, m_dwCommEvents, 512))
		{
			m_Port.StartMonitoring();
			m_ctrlIconOpenoff.SetIcon(m_hIconRed);
			strStatus.Format("״̬��COM%d �򿪣�%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
			//"��ǰ״̬�����ڴ򿪣�����żУ�飬8����λ��1ֹͣλ");
			//send_cmd("EB26");
		}
		else
		{
			AfxMessageBox("û�з��ִ˴��ڻ�ռ��");
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			return;
		}
		m_ctrlOpenPort.SetWindowText("�رմ���");
		m_ctrlPortStatus.SetWindowText(strStatus);
		m_bOpenPort = true;
	}
}

//ֹͣ/������ʾ��������
void CMFCApplicationDlg::OnButtonStopdisp()
{
	m_bStopDispRXData = !m_bStopDispRXData;
	if (m_bStopDispRXData)
		m_ctrlStopDisp.SetWindowText("������ʾ");
	else
		m_ctrlStopDisp.SetWindowText("ֹͣ��ʾ");
}

char CMFCApplicationDlg::HexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - 0x30;
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	else
		return 0x10;
}

//��һ���ַ�����Ϊʮ�����ƴ�ת��Ϊһ���ֽ����飬�ֽڼ���ÿո�ָ���
//����ת������ֽ����鳤�ȣ�ͬʱ�ֽ����鳤���Զ����á�
int CMFCApplicationDlg::Str2Hex(CString str, char* data)
{
	int t, t1;
	int rlen = 0, len = str.GetLength();
	//data.SetSize(len/2);
	for (int i = 0; i<len;)
	{
		char l, h = str[i];
		if (h == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len)
			break;
		l = str[i];
		t = HexChar(h);
		t1 = HexChar(l);
		if ((t == 16) || (t1 == 16))
			break;
		else
			t = t * 16 + t1;
		i++;
		data[rlen] = (char)t;
		rlen++;
	}
	return rlen;

}

long TX_count = 0;
//�ֶ�����
void CMFCApplicationDlg::OnButtonManualsend()
{
	if (m_Port.m_hComm == NULL)
	{
		AfxMessageBox("����û�д򿪣���򿪴���");
		return;
	}
	else
	{
		UpdateData(TRUE);

		if (m_ctrlHexSend.GetCheck())
		{
			char data[512];
			int len = Str2Hex(m_strSendData, data);
			m_Port.WriteToPort(data, len);
			TX_count += (long)((m_strSendData.GetLength() + 1) / 3);
			//m_Port.WriteToPort(hexdata);	
		}
		else
		{
			m_Port.WriteToPort((LPCTSTR)m_strSendData);	//��������
			TX_count += m_strSendData.GetLength();
		}
		CString strTemp;
		strTemp.Format("TX:%d", TX_count);
		m_ctrlTXCount.SetWindowText(strTemp);
	}
}

void CMFCApplicationDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CString strStatus;
	switch (nIDEvent)
	{
	case 1:
		OnButtonManualsend();
		break;
	default:
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

void CMFCApplicationDlg::OnChangeEditSend()
{
	UpdateData(TRUE);
}

//�����������
void CMFCApplicationDlg::OnSelendokComboComselect()
{
	m_nCom = m_Com.GetCurSel() + 1;
}

//����������
void CMFCApplicationDlg::OnSelendokComboSpeed()
{
	// TODO: Add your control notification handler code here
	CString temp;
	int i = m_Speed.GetCurSel();
	switch (i)
	{
	case 0:
		i = 300;
		break;
	case 1:
		i = 600;
		break;
	case 2:
		i = 1200;
		break;
	case 3:
		i = 2400;
		break;
	case 4:
		i = 4800;
		break;
	case 5:
		i = 9600;
		break;
	case 6:
		i = 19200;
		break;
	case 7:
		i = 38400;
		break;
	case 8:
		i = 43000;
		break;
	case 9:
		i = 56000;
		break;
	case 10:
		i = 57600;
		break;
	case 11:
		i = 115200;
		break;
	default:
		break;

	}
	m_nBaud = i;
	CString strStatus;
	if (m_Port.InitPort(this, m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits, m_dwCommEvents, 512))
	{
		if (!m_bOpenPort)
		{
			m_Port.StartMonitoring();
			m_ctrlIconOpenoff.SetIcon(m_hIconRed);


			strStatus.Format("״̬��COM%d �򿪣�%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		else
		{
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			strStatus.Format("״̬��COM%d CLOSED��%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		m_ctrlPortStatus.SetWindowText(strStatus);

	}
	else
	{
		AfxMessageBox("û�гɹ���������");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);

	}


}

//У��λ����
void CMFCApplicationDlg::OnSelendokComboParity()
{
	// TODO: Add your control notification handler code here
	char temp;
	int i = m_Parity.GetCurSel();
	switch (i)
	{
	case 0:
		temp = 'N';
		break;
	case 1:
		temp = 'O';
		break;
	case 2:
		temp = 'E';
		break;
	}
	m_cParity = temp;
	CString strStatus;
	if (m_Port.InitPort(this, m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits, m_dwCommEvents, 512))
	{
		if (!m_bOpenPort)
		{
			m_Port.StartMonitoring();
			m_ctrlIconOpenoff.SetIcon(m_hIconRed);
			strStatus.Format("״̬��COM%d �򿪣�%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		else
		{
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			strStatus.Format("״̬��COM%d CLOSED��%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		m_ctrlPortStatus.SetWindowText(strStatus);

	}
	else
	{
		AfxMessageBox("û�гɹ���������");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);
	}


}

//����λ����
void CMFCApplicationDlg::OnSelendokComboDatabits()
{
	// TODO: Add your control notification handler code here
	//char temp;
	int i = m_DataBits.GetCurSel();
	switch (i)
	{
	case 0:
		i = 8;
		break;
	case 1:
		i = 7;
		break;
	case 2:
		i = 6;
		break;
	}
	m_nDatabits = i;
	CString strStatus;
	if (m_Port.InitPort(this, m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits, m_dwCommEvents, 512))
	{
		if (!m_bOpenPort)
		{
			m_Port.StartMonitoring();
			m_ctrlIconOpenoff.SetIcon(m_hIconRed);
			strStatus.Format("״̬��COM%d �򿪣�%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		else
		{
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			strStatus.Format("״̬��COM%d CLOSED��%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		m_ctrlPortStatus.SetWindowText(strStatus);

	}
	else
	{
		AfxMessageBox("û�гɹ���������");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);
	}


}

//ֹͣλ����
void CMFCApplicationDlg::OnSelendokComboStopbits()
{
	// TODO: Add your control notification handler code here
	int i = m_StopBits.GetCurSel();
	switch (i)
	{
	case 0:
		i = 1;
		break;
	case 1:
		i = 2;
		break;
	}
	m_nStopbits = i;
	CString strStatus;
	if (m_Port.InitPort(this, m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits, m_dwCommEvents, 512))
	{
		if (!m_bOpenPort)
		{
			m_Port.StartMonitoring();
			m_ctrlIconOpenoff.SetIcon(m_hIconRed);
			strStatus.Format("״̬��COM%d �򿪣�%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		else
		{
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			strStatus.Format("״̬��COM%d CLOSED��%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		m_ctrlPortStatus.SetWindowText(strStatus);

	}
	else
	{
		AfxMessageBox("û�гɹ���������");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);
	}

}

//��ռ�����
void CMFCApplicationDlg::OnButtonCountreset()
{
	rxdatacount = 0;
	CString strTemp;
	strTemp.Format("%ld", rxdatacount);
	strTemp = "RX:" + strTemp;
	m_ctrlRXCOUNT.SetWindowText(strTemp);
	TX_count = 0;
	strTemp.Format("%ld", TX_count);
	strTemp = "TX:" + strTemp;
	m_ctrlTXCount.SetWindowText(strTemp);
	coordinate_y = 0;
}

void CMFCApplicationDlg::OnDestroy()
{
	CDialog::OnDestroy();
	KillTimer(1);   //�رն�ʱ��
	KillTimer(4);
	m_Port.ClosePort();  //�رմ���
	m_ReceiveData.Empty();  //��ս��������ַ���
}

//ֹͣ���
void CMFCApplicationDlg::OnBnClickedBtnStopDetect()
{
	is_thread_running = false;
	destroyAllWindows();
}

//�رճ���
void CMFCApplicationDlg::OnClose()
{
	if (::MessageBox(NULL, "  ȷ��Ҫ�˳���", "��ʾ", MB_YESNO) == IDYES)
	{
		is_all_thread_running = false;
		is_thread_running = false;
		destroyAllWindows();
		CMFCApplicationDlg::OnOK();
	}
}

//��ѹ���
bool check_suck()
{
	if (m_Port.m_hComm == NULL)
	{
		AfxMessageBox("����û�д򿪣���򿪴���");
		return 1;
	}

	if (!b_allow_send_cmd)
	{
		char ch[128];
		sprintf(ch, "�ϸ�ָ��δ��ɣ�����̫��!");
		CString cstr(ch);
		AfxMessageBox(cstr);
		return 1;
	}

	while (!is_cmd_running)
	{
		Sleep(1);
	}

	b_allow_send_cmd = false;

	char * str = "EB05";
	char data[512];
	int len = _t_Str2Hex(str, data);
	m_Port.WriteToPort(data, len);

	char c_success[3] = { '0','5',' ' };
	//char c_false[3] = { '5','0',' ' };
	while (1)
	{
		if (is_recv_com_data)
		{
			is_recv_com_data = false;
			b_allow_send_cmd = true;
			return (com_recv_cstr == c_success);
		}
		Sleep(1);
	}

	b_allow_send_cmd = true;
	return false;

}


void CMFCApplicationDlg::OnBnClickedBtnConboard()
{
	initControlBoard();
}

DWORD WINAPI sendCmdThread(LPVOID pParam)
{
	char * str = (char*)pParam;
	send_cmd(str);
	return 0;
}


void CMFCApplicationDlg::OnBnClickedBtnDetect()
{
	is_thread_running = true;
	CreateThread(NULL, 0, DetectThread, NULL, 0, NULL);
	initControlBoard();
}

void CMFCApplicationDlg::OnBnClickedBtnUp()
{
	char str[6];
	sprintf(str, "EB14%02x", m_slider_num);
	coordinate_y += m_slider_num;
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnDown()
{
	char str[6];
	sprintf(str, "EB13%02x", m_slider_num);
	coordinate_y -= m_slider_num;
	CString cstr(str);
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnLeft()
{
	char str[6];
	sprintf(str, "EB16%02x", m_slider_num);
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnRight()
{
	char str[6];
	sprintf(str, "EB15%02x", m_slider_num);
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_slider_num = m_slider.GetPos();//ȡ�õ�ǰλ��ֵ 
	String str = to_string(m_slider_num);
	CString cstr(str.c_str());
	GetDlgItem(IDC_STATIC_STEP)->SetWindowText(cstr);


	dis_box_and_den = m_slider2.GetPos();//ȡ�õ�ǰλ��ֵ 
	String str2 = to_string(dis_box_and_den);
	CString cstr2(str2.c_str());
	GetDlgItem(IDC_STATIC_STEP5)->SetWindowText(cstr2);


	suck_add_x = m_slider3.GetPos();//ȡ�õ�ǰλ��ֵ 
	String str3 = to_string(suck_add_x);
	CString cstr3(str3.c_str());
	GetDlgItem(IDC_STATIC_STEP6)->SetWindowText(cstr3);

	suck_add_y = m_slider4.GetPos();//ȡ�õ�ǰλ��ֵ 
	String str4 = to_string(suck_add_y);
	CString cstr4(str4.c_str());
	GetDlgItem(IDC_STATIC_STEP7)->SetWindowText(cstr4);


	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMFCApplicationDlg::OnBnClickedBtnPumpUp()
{
	char * str = "EB02";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnPumpDown()
{
	char * str = "EB01";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnAirUp()
{
	char * str = "EB09";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnAirDown()
{
	char * str = "EB0A";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnNextBox()
{
	char * str = "EB20";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnNextBoxOut()
{
	char * str = "EB21";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnBandUp()
{
	char str[6];
	sprintf(str, "EB23%02x", m_slider_num);
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnBandDown()
{
	char str[6];
	sprintf(str, "EB22%02x", m_slider_num);
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnSetUp()
{
	char * str = "EB03";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnSetDown()
{
	char * str = "EB04";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnFixUp()
{
	char * str = "EB0D";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnFixDown()
{
	char * str = "EB0C";
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnPause()
{
	if (is_cmd_running)
	{
		is_cmd_running = false;
		GetDlgItem(IDC_BTN_PAUSE)->SetWindowText("����");
		//�Ƶƿ����ȴ�����
		//fore_cend_cmd("EB26");

	}
	else {
		is_cmd_running = true;
		GetDlgItem(IDC_BTN_PAUSE)->SetWindowText("��ͣ");
		//�̵ƿ�,����
		//fore_cend_cmd("EB25");
	}
}

void CMFCApplicationDlg::OnBnClickedBtnBandUp2()
{
	char str[6];
	sprintf(str, "EB18%02x", m_slider_num);
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

void CMFCApplicationDlg::OnBnClickedBtnBandDown2()
{
	char str[6];
	sprintf(str, "EB17%02x", m_slider_num);
	CreateThread(NULL, 0, sendCmdThread, str, 0, NULL);
}

DWORD WINAPI fipPointThread2(LPVOID pParam)
{
	int cycles = 0;//���ڣ�ÿ11���ŵ�1�����ڣ�

	for (int i = 0; i < cols_num; i++)
	{
		//��һ��6��
		for (int cols_6 = 0; cols_6 < 6; cols_6++)
		{
			while (true)
			{
				if (b_allow_sucking)
				{
					break;
				}
				Sleep(1);
			}

			Sleep(220);

			int err_x = center.x - 92;
			int err_y = center.y - 120;
			int err_trans_x = err_x / 0.65;
			int err_trans_y = err_y / 1.8;
			char str[6];
			if (abs(err_x) > minErrValue && abs(err_x) < maxErrValue)
			{
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
			}

			if (abs(err_y) > minErrValue && abs(err_y) < maxErrValue)
			{
				char str[6];
				char str_d[6];//�¶�΢��
				if (err_trans_y < 0)
				{
					sprintf(str, "EB13%02x", -err_trans_y);
					sprintf(str_d, "EB17%02x", -err_trans_y);
					send_cmd(str);
					send_cmd(str_d);
				}
				else if (err_trans_y > 0)
				{
					sprintf(str, "EB14%02x", err_trans_y);
					sprintf(str_d, "EB18%02x", err_trans_y);
					send_cmd(str);
					send_cmd(str_d);
				}
			}//��׼���

			send_cmd("EB1100CC");//��ͷ�ƶ����ŵ�λ��

			send_cmd("EB01");//��ͷ��ѹ
			send_cmd("EB09");//��ͷ����
			send_cmd("EB03");//���涥��
			send_cmd("EB02");//��ͷ��ѹ
			send_cmd("EB04");//���涥��

			int fast_dist;
			if (desti_position == 0)
			{
				fast_dist = (dis_box_and_den + err_trans_y - cols_6 * 163) * 4;
			}
			else if (desti_position == 1)
			{
				fast_dist = (dis_box_and_den + err_trans_y - cols_6 * 163 + hole_distent) * 4;
			}

			sprintf(str, "EB12%04x", fast_dist);
			send_cmd(str);

			//�������
			send_cmd("EB01");//��ͷ��ѹ
			send_cmd("EB0A");//��ͷ����
			send_cmd("EB02");//��ͷ��ѹ

			if (cols_6 < 5)
			{
				send_cmd("EB07A1");
			}

			if (desti_position == 0)
			{
				fast_dist = (dis_box_and_den + err_trans_y - 176 - cols_6 * 163 - 163) * 4;
				desti_position = 1;
			}
			else if (desti_position == 1)
			{
				fast_dist = (dis_box_and_den + err_trans_y - 176 - cols_6 * 163 - 163 + hole_distent) * 4;
				desti_position = 0;
			}

			sprintf(str, "EB11%04x", fast_dist);
			send_cmd(str);

			if (cols_6 == 5)
			{
				Sleep(10);
				send_cmd("EB1101DD");//��һ�ţ�5����
				send_cmd("EB1583");

				send_cmd("EB1850");//�¶�����

			}

			suck_num++;

			Sleep(5);
		}

		//�ڶ���5��
		for (int cols_5 = 0; cols_5 < 5; cols_5++)
		{
			while (true)
			{
				if (b_allow_sucking)
				{
					break;
				}
				Sleep(1);
			}

			Sleep(220);

			int err_x = center.x - 92;
			int err_y = center.y - 120;
			int err_trans_x = err_x / 0.65;
			int err_trans_y = err_y / 1.8;
			char str[6];
			if (abs(err_x) > minErrValue && abs(err_x) < maxErrValue)
			{
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
			}

			if (abs(err_y) > minErrValue && abs(err_y) < maxErrValue)
			{
				char str[6];
				char str_d[6];//�¶�΢��
				if (err_trans_y < 0)
				{
					sprintf(str, "EB13%02x", -err_trans_y);
					sprintf(str_d, "EB17%02x", -err_trans_y);
					send_cmd(str);
					send_cmd(str_d);
				}
				else if (err_trans_y > 0)
				{
					sprintf(str, "EB14%02x", err_trans_y);
					sprintf(str_d, "EB18%02x", err_trans_y);
					send_cmd(str);
					send_cmd(str_d);
				}
			}//��׼���

			send_cmd("EB1100CC");//��ͷ�ƶ����ŵ�λ��

			send_cmd("EB01");//��ͷ��ѹ
			send_cmd("EB09");//��ͷ����
			send_cmd("EB03");//���涥��
			send_cmd("EB02");//��ͷ��ѹ
			send_cmd("EB04");//���涥��

			int fast_dist;
			if (desti_position == 0)
			{
				fast_dist = (dis_box_and_den - 5 * 163 + 81 + err_trans_y + cols_5 * 163) * 4;
			}
			else if (desti_position == 1)
			{			
				fast_dist = (dis_box_and_den - 5 * 163 + 81 + err_trans_y + cols_5 * 163 + hole_distent) * 4;
			}

			sprintf(str, "EB12%04x", fast_dist);
			send_cmd(str);

			//�������
			send_cmd("EB01");//��ͷ��ѹ
			send_cmd("EB0A");//��ͷ����
			send_cmd("EB02");//��ͷ��ѹ

			if (cols_5 < 4)
			{
				send_cmd("EB06A1");
			}

			fast_dist = (dis_box_and_den - 5 * 163 + 81 + err_trans_y - 176 + cols_5 * 163 + 163) * 4;

			if (desti_position == 0)
			{
				fast_dist = (dis_box_and_den - 5 * 163 + 81 + err_trans_y - 176 + cols_5 * 163 + 163) * 4;
				desti_position = 1;
			}
			else if (desti_position == 1)
			{
				fast_dist = (dis_box_and_den - 5 * 163 + 81 + err_trans_y - 176 + cols_5 * 163 + 163 + hole_distent) * 4;
				desti_position = 0;
			}

			sprintf(str, "EB11%04x", fast_dist);
			send_cmd(str);

			if (cols_5 == 2 && cycles % 5 == 0 && cycles != 0)//�����¶�λ�ã��˶��������
			{
				//AfxMessageBox("����");
				send_cmd("EB180B");
			}

			if (cols_5 == 4)
			{
				send_cmd("EB1350");//��ͷ����ͷ������
				send_cmd("EB1583");//���ϵ���
				send_cmd("EB1850");//�¶�����
			}

			suck_num++;

			Sleep(5);

		}

		cycles++;
	}
	return 0;
}

void CMFCApplicationDlg::OnBnClickedBtnTest3()
{
	b_allow_sucking = true;
}

void CMFCApplicationDlg::OnBnClickedBtnConboard2()
{
	CreateThread(NULL, 0, IsSuckOkThread, NULL, 0, NULL);
}

DWORD WINAPI TestThread(LPVOID pParam)
{
	send_cmd("EB02");//��ͷ��ѹ
	send_cmd("EB10");//��λ
	send_cmd("EB112EAD");//�ƶ�������
	coordinate_y += 3107;

	send_cmd("EB08");//�¶���λ

	Sleep(30);

	send_cmd("EB1764");//����һ��

	Sleep(30);

	send_cmd("EB08");//�ٸ�λ

	Sleep(30);

	send_cmd("EB1741");//�¶���׼��ͷ

	Sleep(20);

	CreateThread(NULL, 0, fipPointThread2, NULL, 0, NULL);//��ʼ��ȡ����


	

	return 0;
}

void CMFCApplicationDlg::OnBnClickedBtnTest()
{
	coordinate_y = 0;
	CreateThread(NULL, 0, TestThread, NULL, 0, NULL);
}


void CMFCApplicationDlg::OnBnClickedBtnConnectServer()
{
	connect_server();
}
