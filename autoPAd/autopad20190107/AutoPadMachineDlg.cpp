
// AutoPadMachineDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "AutoPadMachine.h"
#include "AutoPadMachineDlg.h"
#include "afxdialogex.h"
#include"SerialPort.h"
#include"Common.h"
#include <fstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<math.h>
using namespace cv;
using namespace std;

CSerialPort serialport;//���ڶ���
bool b_CheckCircle = true;  //��ʼĬ�ϵļ��Բ
bool b_JudgePostion = false;
bool b_correct = true;  //�������ϵ�
bool b_third_test = false;
bool b_brightness = false;
bool b_CheckSmall = false;   //���������Ƿ���СС�ľ���
bool b_CoutCK = true;   //�����ж��Ƿ�ʼ����CK
bool b_ER_Send = false; //�����ж��Ƿ�����ER
bool b_ER_Send_Rect = false; //�ж���һ���Ƿ�����ER
bool b_PAGet = false;  //��������Ƿ���Ҫ����
bool b_ChangeMaterial = false;  //����  ����ȷ������ST��
bool b_OVGet = false;  //�����ж��Ƿ���ܵ���OV
bool b_detect = false;   //�����̵߳ı�־
bool b_ST_Get = false;  //���ܵ�ST��ʼ��
bool b_other250 = false; 
bool b_firstaj = true;

int b_cout_cknum = 0;
int count_AJnum = 0;
int cout_ck_time = 0;  //�ж�CK���ܵĴ���  7��֮�����Բ���

int i_threshold = 180;
int i_threshold_third = 200;

CFont   font;
CBrush m_brush;

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

int Str2ToHex(CString str, char* data)
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


void SaveSendLog(char * str);
//ǿ�Ʒ���һ��ָ��
int ForceSendCmd(char * str)
{
	if (serialport.m_hComm == NULL)
	{
		AfxMessageBox("����û�д򿪣���򿪴���");
		return -1;
	}

	char data[512];
	int len = Str2ToHex(str, data);
	serialport.WriteToPort(data, len);
	//SaveSendLog(str);

	return 0;
}

//���ò����ʵĺ���
int BaudSet(int m)
{
	int i = 0;
	switch (m)
	{
	case 300:
		i = 0;
		break;
	case 600:
		i = 1;
		break;
	case 1200:
		i = 2;
		break;
	case 2400:
		i = 3;
		break;
	case 4800:
		i = 4;
		break;
	case 9600:
		i = 5;
		break;
	case 19200:
		i = 6;
		break;
	case 38400:
		i = 7;
		break;
	case 43000:
		i = 8;
		break;
	case 56000:
		i = 9;
		break;
	case 57600:
		i = 10;
		break;
	case 115200:
		i = 11;
		break;
	default:
		break;
	}
	return i;
}

//ģ��ƥ�� Բ
Point MatchCircleTemplate(Mat bitwise_out, Mat templ, int circle_d)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//����ƥ��
	matchTemplate(bitwise_out, templ, g_resultImage, 4);

	//��׼��
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;

	//��λ��ƥ���λ��
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

//ģ��ƥ�� rectangle ����
Point MatchRectangleTemplate(Mat bitwise_out, Mat templ)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//����ƥ��
	matchTemplate(bitwise_out, templ, g_resultImage, 4);

	//��׼��
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;

	//��λ��ƥ���λ��
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

std::ofstream ofs_send_log;
//��������
void SaveSendLog(char * str)
{
	char tmpBufSend[256];
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(tmpBufSend, "%4d/%02d/%02d %02d:%02d:%02d.%03d SD %s\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, str);
	ofs_send_log << tmpBufSend;
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


// CAutoPadMachineDlg �Ի���

CAutoPadMachineDlg::CAutoPadMachineDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_AUTOPADMACHINE_DIALOG, pParent)
	, m_DataShowStr(_T(""))
	, m_SendDATA(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoPadMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OPEN_PORT, m_OpenPort);
	DDX_Control(pDX, IDC_STATIC_PIcture, m_Picture);
	DDX_Control(pDX, IDC_COM, m_SerialPort);
	DDX_Control(pDX, IDC_SPEED, m_BoteRate);
	DDX_Control(pDX, IDC_DATA_ACCPET, m_DataShow);
	DDX_Text(pDX, IDC_DATA_ACCPET, m_DataShowStr);
	DDX_Text(pDX, IDC_SendData, m_SendDATA);
	DDX_Control(pDX, IDC_STATIC_TXT, m_statictxt);
	DDX_Control(pDX, IDC_MATERIAL_JU, m_Material);
}

BEGIN_MESSAGE_MAP(CAutoPadMachineDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_COMM_RXCHAR, OnCommunication)
	ON_BN_CLICKED(IDC_OPEN_PORT, &CAutoPadMachineDlg::OnBnClickedOpenPort)
	ON_CBN_SELENDOK(IDC_COM, &CAutoPadMachineDlg::OnCbnSelendokCom)
	ON_CBN_SELENDOK(IDC_SPEED, &CAutoPadMachineDlg::OnCbnSelendokSpeed)
	ON_BN_CLICKED(IDC_VIDEO_START, &CAutoPadMachineDlg::OnBnClickedVideoStart)
	ON_BN_CLICKED(IDC_AutoSend, &CAutoPadMachineDlg::OnBnClickedAutosend)
	ON_BN_CLICKED(IDC_Start_HH, &CAutoPadMachineDlg::OnBnClickedStartHh)
	ON_BN_CLICKED(IDC_TIAOSHI, &CAutoPadMachineDlg::OnBnClickedTiaoshi)
	ON_BN_CLICKED(IDC_CHANGE_MATE, &CAutoPadMachineDlg::OnBnClickedChangeMate)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECHover, &CAutoPadMachineDlg::OnBnClickedChechover)
	ON_BN_CLICKED(IDC_FORCESTART, &CAutoPadMachineDlg::OnBnClickedForcestart)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_MATERIAL_JU, &CAutoPadMachineDlg::OnBnClickedMaterialJu)
END_MESSAGE_MAP()


// CAutoPadMachineDlg ��Ϣ�������

BOOL CAutoPadMachineDlg::OnInitDialog()
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

	//ShowWindow(SW_MAXIMIZE);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	font.CreatePointFont(250, "����");
	m_brush.CreateSolidBrush(RGB(188, 243, 188));//��ˢΪ��ɫ

	m_cParity = 'N';
	m_nDatabits = 8;
	m_nStopbits = 1;
    m_nBaud = 9600;       //������
    m_nCom = 6 ;         //���ں�
	m_BoteRate.SetCurSel(BaudSet(m_nBaud));
	m_SerialPort.SetCurSel(m_nCom -1);

	m_hIconRed = AfxGetApp()->LoadIcon(IDI_ICON2);
	m_hIconOff = AfxGetApp()->LoadIcon(IDI_ICON1);
	m_Picture.SetIcon(m_hIconOff);

	GetDlgItem(IDC_MATERIAL_JU)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECHover)->EnableWindow(FALSE);

	/*SYSTEMTIME sys;
	GetLocalTime(&sys);
	char chatrecv[256];
	sprintf(chatrecv, "%4d_%02d_%02d_%02d_%02d_%02d_Send.txt", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
	ofs_send_log.open(chatrecv, std::ofstream::app);*/

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CAutoPadMachineDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAutoPadMachineDlg::OnPaint()
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
HCURSOR CAutoPadMachineDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAutoPadMachineDlg::OnBnClickedOpenPort()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//if (m_bOpenPort)  //�رմ���
	//{
	//	m_OpenPort.SetWindowText("�򿪴���");
	//	serialport.ClosePort();//�رմ���
	//						   //m_ctrlPortStatus.SetWindowText("״̬�������ѹر�");
	//	m_Picture.SetIcon(m_hIconOff);
	//	m_bOpenPort = false;
	//}
	//else  //�򿪴���
	//{
		CString strStatus;
		if (serialport.InitPort(this, m_nCom, 9600, m_cParity, m_nDatabits, m_nStopbits, EV_RXFLAG | EV_RXCHAR, 512))
		{
			serialport.StartMonitoring();
			m_Picture.SetIcon(m_hIconRed);
			strStatus.Format("״̬��COM%d �򿪣�%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
			//"��ǰ״̬�����ڴ򿪣�����żУ�飬8����λ��1ֹͣλ");
		}
		else
		{
			AfxMessageBox("û�з��ִ˴��ڻ�ռ��");
			m_Picture.SetIcon(m_hIconOff);
			return;
		}
		m_OpenPort.SetWindowText("�رմ���");
		//m_ctrlPortStatus.SetWindowText(strStatus);
		m_bOpenPort = true;
	//}

}

//comѡ��
void CAutoPadMachineDlg::OnCbnSelendokCom()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_nCom = m_SerialPort.GetCurSel() + 1;
}

//������ѡ��
void CAutoPadMachineDlg::OnCbnSelendokSpeed()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int i = m_BoteRate.GetCurSel();
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
}

//���ǽ������ݵĽӿ�
LRESULT CAutoPadMachineDlg::OnCommunication(WPARAM ch, LPARAM port)
{
	
	if (port <= 0 ) //|| port > 11
		return -1;

	//i_rx_count++;   //���յ��ֽڼ���
	//CString strTemp;
	//strTemp.Format("%ld", i_rx_count);
	//strTemp = "RX:" + strTemp;
	//m_ctrlRXCOUNT.SetWindowText(strTemp);  //��ʾ���ռ���

	//if (m_bStopDispRXData)   //���ѡ���ˡ�ֹͣ��ʾ���������ݣ��򷵻�
	//	return -1;          //ע�⣬��������£��������ڼ�����ֻ�ǲ���ʾ
	//						//�������ˡ��Զ���ա�����ﵽ8�к��Զ���ս��ձ༭������ʾ������
	if (m_DataShow.GetLineCount() >= 10)
	{
		m_DataShowStr.Empty();
		UpdateData(FALSE);
	}
	//���û�С��Զ���ա��������дﵽ400��Ҳ�Զ����
	//��Ϊ���ݹ��࣬Ӱ������ٶȣ���ʾ�����CPUʱ��Ĳ���
	if (m_DataShow.GetLineCount()>400)
	{
		m_DataShowStr.Empty();
		UpdateData(FALSE);
	}

	//���ѡ����"ʮ��������ʾ"������ʾʮ������ֵ
	CString str;

	str.Format("%02X ", ch);

	//CK 67 75 43 48 ���ͼ��оƬ������ ��ʼ�����ж�ʲôʱ��ʼ���
	if (ch == 75  && b_CoutCK == true)
	{
		cout_ck_time++;
	}
	else if (ch == 75 && b_CoutCK == false)
	{
		b_CheckSmall = true;
	}

	// AJ  ʮ���� 65 74  ʮ������ 41 4A  ��ʼ��������̵��ĸ��ǵ�оƬ
	if (ch == 74)
	{
		b_JudgePostion = true;
		count_AJnum++;
	}

	//�жϽ��ܵ��Ļ��� PA 80 65 50 41
	if (ch == 80)
	{
		b_PAGet = true;
	}

	//OV 79 86 4F 56 �ж��Ƿ���ܵ�OV
	if (ch == 86)
	{
		b_OVGet = true;
		//b_CheckSmall = false;
		//b_other250 = true;
	}

	//ST ��ʼ  83 84 53 54 
	if (ch == 84)
	{
		b_ST_Get = true;
	}
	//is_com_recv_data = true;
	//if (ch == 80 || ch == 235 || ch == 196 || ch == 195 || ch == 193 || ch == 197)
	//{
	//	//cstr_com_recv_string = str;
	//}
	//else
	//{
	//	cstr_com_recv_string = str;
	//}

	//�����ǽ����յ��ַ������ַ�������������ʱ�ܶ�
	//�����ǵ�������Ҫ������ļ�������û����List Control
	int nLen = m_DataShow.GetWindowTextLength();
	m_DataShow.SetSel(nLen, nLen);
	m_DataShow.ReplaceSel(str);
	nLen += str.GetLength();

	m_DataShowStr += str;
	return 0;
}


Mat third_mask_circle;
int  third_mask_circle_d = 30;

//��������������ͷ������
void Third_mask_made()
{
	Mat mask;
	mask = Mat(Size(third_mask_circle_d + 2, third_mask_circle_d + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(third_mask_circle_d / 2, third_mask_circle_d / 2), third_mask_circle_d / 2, Scalar(255), 1);
	third_mask_circle = mask.clone();
	//imwrite("third.jpg", third_mask_circle);
}

Mat black_recentangle_mask;
int black_recentangle_mask_x = 37; //44
int black_recentangle_mask_y = 9;  //9
//��������������ͷ������
void Third_rentangle_mask_made()
{
	Mat mask;
	mask = Mat(Size(black_recentangle_mask_x + 2, black_recentangle_mask_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(0, 0), Point(black_recentangle_mask_x + 1, black_recentangle_mask_y + 1), Scalar(255), 1);
	//rectangle(mask, Point(1, 1), Point(black_recentangle_mask_x + 2, black_recentangle_mask_y + 2), Scalar(255), 1);
	black_recentangle_mask = mask.clone();
	//imwrite("rtgffFFF.jpg", black_recentangle_mask);
}

int  circle_center_x = 183; //180
int circle_center_y = 189; //180
int  small_third_x = 240;  //228
int  small_third_y = 93;   //84

//���㲢����һ��ͼ�������
void GetBrightness(Mat InputImg, float& cast, float& f_dark_quantity)
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
	f_dark_quantity = a / float(GRAYimg.rows*InputImg.cols);
	float D = abs(f_dark_quantity);
	float Ma = 0;
	for (int i = 0; i<256; i++)
	{
		Ma += abs(i - 128 - f_dark_quantity)*Hist[i];
	}
	Ma /= float((GRAYimg.rows*GRAYimg.cols));
	float M = abs(Ma);
	float K = D / M;
	cast = K;
	return;
}

void InitControlBoard();

//����������ͷ���߳�
DWORD WINAPI DetecAtThird(LPVOID pParam)
{
	Mat videoimage;
	Mat videoimage2;
	Mat filter, canny, wavefilter;
	Mat blue, out;
	Point center;
	float cast;
	float f_dark_quantity;
	int Send_ER_Once = 0;  //��ER������һ��
	int circle_error_num = 0;  //������������
	int Detect_First_ERROR = 0;
	VideoCapture  capture2(0);
	vector<Vec4i> lines;

	Third_mask_made();
	Third_rentangle_mask_made();
	int true_threshold = 0;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

	int count_time = 0;
	theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("���������λ��");
	int m_coutnn = 0;
	char str[30];
	while (b_detect)
	{
		capture2 >> videoimage2;
		videoimage2 = videoimage2(Rect(190, 30, 380, 380));   //110 50 360 360
		out = videoimage2.clone();
	/*	if (true == b_brightness)
		{
			GetBrightness(out, cast, f_dark_quantity);
		}*/
		cvtColor(videoimage2, videoimage2, CV_RGB2GRAY);

		if (b_ER_Send == true)
		{
			circle(out, Point(circle_center_x, circle_center_y), third_mask_circle_d / 2, Scalar(0, 255, 255), 2);
		}

		//�жϵڶ�������������  ���ܵ�OV  �ж�4��AJ�Ƿ������ ��һ�εı�־
		//Ŀ���� �������С�׵��߳���������
		if ( true == b_OVGet && b_other250 == true && b_firstaj == false)
		{
			b_CheckCircle = true;
			b_other250 = false;
			b_OVGet = false;
			b_CoutCK = true;
			//b_checkSmall = false;
		}

		if ( true == b_CheckCircle)
		{ 
			//printf("start  b_CheckCircle \n");
			medianBlur(videoimage2, videoimage2, 5);
			//imshow("videoimage2", videoimage2);
			adaptiveThreshold(videoimage2, blue, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 3, 5);
		  //  imshow("blue", blue);
			center = MatchCircleTemplate(blue, third_mask_circle, third_mask_circle_d);
			circle(out, Point(center.x, center.y), third_mask_circle_d / 2, Scalar(255, 0, 255), 1);
			if (abs(center.x - circle_center_x) < 8     &&   abs(center.y - circle_center_y) < 15)
			{ 

				//printf("it is 0k \n");
				theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("���λ�ü����ȷ");
				theApp.GetMainWnd()->GetDlgItem(IDC_CHECHover)->EnableWindow(FALSE);
				Detect_First_ERROR = 0; //���ü���һ�δ���ĺ�����һֱ����һ�����������
				b_CheckCircle = false;
				//b_brightness = true; 
				//�����������г�ͻ
				if(b_ER_Send == true)
				{ 
					ForceSendCmd(ST);
					b_ER_Send = false;
				}
				
				//����ÿ�ʼ������ȷ�� b_ST_Get ����ĳ�false 
				//�ܵ� else error�����򲻻�ִ����һ��
				if (b_ST_Get ==  true)
				{
					ForceSendCmd(ST);
					b_ST_Get = false;
				}
			
			}
			else  //if (abs(center.y - 179)<10)
			{  
				//printf("it is error \n");
				circle_error_num++;
				if (circle_error_num >= 2)
				{   
					//printf("it is error 2 \n");
					theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("������ô��� ���Ƶ�ͼ�л�Ȧλ��,�����ťУ׼���");//SetWindowText("ֹͣ���");
					theApp.GetMainWnd()->GetDlgItem(IDC_CHECHover)->EnableWindow(TRUE);
					//imshow("out", out);
					//��һ���жϵ�һ�μ��͵ڶ��μ��
					if (Detect_First_ERROR ==  0)
					{  
						while (b_ST_Get == false)  //������ܵ� ST �Ĳ��� ��Ϊ̫��Ļ�4��AJ����û�����ж�
						{
							Sleep(50);
						}		
					}
					//printf("I am ready for again \n");
				//	capture2 >> videoimage2;					
					Detect_First_ERROR = 1;
					b_ST_Get = false;
					ForceSendCmd(ER);
					b_ER_Send = true;
					circle_error_num = 0;
					b_CheckCircle = false;    //���� ֪�������ť��ʼ���
				}
				Sleep(50);
				capture2 >> videoimage2;		
				printf("it is error  Sleep  \n");
			}	
		}
		else if( 7 == cout_ck_time)  //cout_ck_time (int)f_dark_quantity > 10
		{
			b_CheckSmall = true;
			//b_brightness = false;
			b_CoutCK = false;
			cout_ck_time = 0;
		}
	//	printf("error 3\n");

	// ���̵õ�ST ������ER ����	b_CheckCircle Ϊfalse
	if (b_ST_Get == true && b_ER_Send == false && b_CheckCircle == false)
		{
			ForceSendCmd(ST);
			b_ST_Get = false;
		}

		//�жϼ��С��ɫ��־
		if ( true == b_CheckSmall)
		{   
			//InitControlBoard();
			true_threshold = i_threshold_third;
			if (true_threshold <= 0 || true_threshold >= 255)
			{
				true_threshold = 180;
			}
			threshold(videoimage2, blue, true_threshold, 255, CV_THRESH_BINARY);
			//imshow("blue", blue);
			morphologyEx(blue, wavefilter, MORPH_DILATE, element);
			//imshow("wavefilter", wavefilter);
			center = MatchRectangleTemplate(wavefilter, black_recentangle_mask);
			rectangle(out, center, Point(center.x + black_recentangle_mask_x, center.y + black_recentangle_mask_y), Scalar(255, 0, 255), 1);
			if (abs(center.x - small_third_x) < 15 && abs(center.y - small_third_y) < 15)
			{  			
				theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("��������ȷ ����");
				if (b_ER_Send_Rect == true)
				{  
					ForceSendCmd(ST);
					Sleep(100);
					capture2 >> videoimage2;
					b_ER_Send_Rect = false;
					Send_ER_Once = 0;
				}
				b_CheckSmall = false;
			}
			else {
				if (Send_ER_Once == 0)
				{
					//sprintf(str,"error_%d.jpg", m_coutnn);
					//imwrite(str, out);
					//m_coutnn++;
					theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("���оƬ���ó��������  ");
					Send_ER_Once = 1;
					ForceSendCmd(ER);
					b_ER_Send_Rect = true;
				}
				Sleep(100);
				capture2 >> videoimage2;
			
			//	ForceSendCmd(ER);
			}
		}
		
		imshow("out", out);
		waitKey(10);
	}
	return 0;
}

int i_rect_max_x = 170;  //204
int i_rect_max_y = 100;   //76
int i_rect_max_w = 256;  //260
int i_rect_max_h = 256;  //260

Mat out_mask_img;
int i_img_mask_max_rect_x = 180;  //175
int i_img_mask_max_rect_y = 185;  // 181

//����һ����������
void Max_Rectrectangle_mask()
{
	Mat mask;
	mask = Mat(Size(i_img_mask_max_rect_x + 2, i_img_mask_max_rect_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(1, 1), Point(i_img_mask_max_rect_x+1 , i_img_mask_max_rect_y+1 ), Scalar(255), 1);
	out_mask_img = mask.clone();
}

Mat back_small;
int back_small_x = 37; //35
int back_samll_y = 9; //8

//����һ��С�ڵľ���
void Back_mask()
{
	Mat mask;
	mask = Mat(Size(back_small_x + 2, back_samll_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(0, 0), Point(back_small_x+1 , back_samll_y+1 ), Scalar(255), 1);
	back_small = mask.clone();
}

//opencv���������Ӧ����
static void ContrastAndBright(int, void *)
{
	Mat m = Mat(800, 600, 0);
	Max_Rectrectangle_mask();
	imshow("��������塿", m);
}

//����һ��������壬�����϶�������Ӧֵ
void InitControlBoard()
{
	namedWindow("��������塿", 0);
	createTrackbar("��ֵ����ֵ��", "��������塿", &i_threshold, 256, ContrastAndBright);
	createTrackbar("�ڶ�����ֵ����", "��������塿", &i_threshold_third, 256, ContrastAndBright);
	createTrackbar("��ʼ��x��", "��������塿", &i_rect_max_x, 300, ContrastAndBright);
	createTrackbar("��ʼ��y��", "��������塿", &i_rect_max_y, 300, ContrastAndBright);
	createTrackbar("���ο�ȣ�", "��������塿", &i_img_mask_max_rect_x, 256, ContrastAndBright);
	createTrackbar("���θ߶ȣ�", "��������塿", &i_img_mask_max_rect_y, 256, ContrastAndBright);

}

//���оƬλ�õ����
void Caclulation_error(int difference_x, int difference_y, int src_x, int src_y)
{
	char str[32];
	if (difference_x > 10)
	{
		if (difference_y  >10)
		{
			//	sprintf(str, "EBB501%02XBE", -err_trans_x);
			sprintf(str, "414A00%02X00%02X", difference_x, (int)(difference_y * 0.8));
		}
		else if (difference_y < -10)
		{
			sprintf(str, "414A00%02X01%02X", difference_x, -((int)(difference_y * 0.8)));
		}
		else {
			sprintf(str, "414A00%02X0000", difference_x);
		}
	}
	else if (difference_x < -10)
	{
		if (difference_y  >10)
		{
			//	sprintf(str, "EBB501%02XBE", -err_trans_x);
			sprintf(str, "414A01%02X00%02X", -(difference_x), (int)(difference_y * 0.8));
		}
		else if (difference_y < -10)
		{
			sprintf(str, "414A01%02X01%02X", -(difference_x), -((int)(difference_y * 0.8)));
		}
		else {
			sprintf(str, "414A01%02X0000", -(difference_x));
		}
	}
	else
	{
		if (difference_y >10)
		{
			//	sprintf(str, "EBB501%02XBE", -err_trans_x);
			sprintf(str, "414A000000%02X", (int)(difference_y * 0.8));
		}
		else if (difference_y < -10)
		{
			sprintf(str, "414A000001%02X", -((int)(difference_y * 0.8)));
		}
		else {
			sprintf(str, "414A00000000");
		}
	}
	ForceSendCmd(str);
}

//��һ������ͷ�ļ��
DWORD WINAPI DetectFirstThread(LPVOID pParam)
{
	Mat  VideoImage, binaryimage, threImage, out, canny_image;
	Mat  resultImage, restIm_binary, resuIm_threshold;
	Point  center, src_location;
	int true_threshold = 0;
	VideoCapture capture(1);

	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	Max_Rectrectangle_mask();
	Back_mask();

	//��һ������ͷȷ���ı�ǩλ��
	int src_x = 48;    //168
	int src_y = 185;     //10

	int  point_src_x = 0;
	int  point_src_y = 0;

	int difference_x = 0;
	int difference_y = 0;

	ForceSendCmd(ST);
	Sleep(500);
	bool first_wait_material = true; //�����жϵ�һ�����ϵļ�� ��ʼ
	bool b_GetAJ = false;   //��ʾ��ʼ���оƬ ���¼�
	int error_cout = 0; //�������Ĵ���
	int diff_x[2] = { 0,0 };
	int diff_y[2] = { 0,0 };
	int time_detect = 0; //������Ĵ���

	while (b_detect)
	{  
		if (true == b_ChangeMaterial)
		{
			ForceSendCmd(ST);
			Sleep(500);
			b_ChangeMaterial = false;
			first_wait_material = true;
		}

		capture >> VideoImage;
		if (!VideoImage.data)
		{
			AfxMessageBox("����ͷ��ʧ�ܣ���������ͷ���������м�⣡");
			break;
		}

		//��һ���ŵ���Ҫ�ȴ�ʱ���һ��
		if (first_wait_material == true)
		{
			Sleep(200);
			capture >> VideoImage;
			first_wait_material = false;
		}

	   //��ʾ���Դ��ڵ�
		if ( true == b_third_test )
		{
			InitControlBoard();
		}

		VideoImage = VideoImage(Rect(i_rect_max_x, i_rect_max_y, i_rect_max_w, i_rect_max_h));
		cvtColor(VideoImage, binaryimage, CV_RGB2GRAY);
		true_threshold = i_threshold;
		if (true_threshold <= 0 || true_threshold >= 255)
		{
			true_threshold = 180;
		}
		threshold(binaryimage, threImage, true_threshold, 255, CV_THRESH_BINARY);
		resultImage = threImage.clone();
		morphologyEx(threImage, out, MORPH_ERODE, element);
		//imshow("threshold", out);
		Canny(out, canny_image, 50, 100, 3);
		//imshow("src_xytr", canny_image);
		center = MatchRectangleTemplate(canny_image, out_mask_img);
		//rectangle(VideoImage, center, Point(center.x + i_img_mask_max_rect_x, center.y + i_img_mask_max_rect_y), Scalar(255, 0, 255), 1);
		resultImage = resultImage(Rect(center.x, center.y, i_img_mask_max_rect_x, i_img_mask_max_rect_y));
		src_location = MatchRectangleTemplate(resultImage, back_small);
		point_src_x = src_location.x + center.x;
		point_src_y = src_location.y + center.y;
		rectangle(VideoImage, Point(point_src_x, point_src_y), Point(point_src_x + back_small_x, point_src_y + back_samll_y), Scalar(255, 0, 255), 1);
		
		imshow("src", VideoImage);
	    //x�� 10������1mm   y��10������0.8mm
		difference_x = src_x - point_src_x;
		difference_y = src_y - point_src_y;

		if ( false == b_correct)
		{
			waitKey(10);
			continue;
		}

		if (true == b_JudgePostion)
		{
			Sleep(500);
			capture >> VideoImage;
			b_GetAJ = true;
			b_JudgePostion = false;
		}
		else if ( true == b_GetAJ)
		{   
			//�ж����Ƿ����
			if (abs(difference_x) >20 || abs(difference_y) >20)
			{
				if (2 <= error_cout)
				{
					theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("��ȷ���������Ƿ������ȷ");
					theApp.GetMainWnd()->GetDlgItem(IDC_MATERIAL_JU)->EnableWindow(TRUE);
					b_correct = false;
					error_cout = 0;
				}
				else {
					error_cout++;
					Sleep(40);
					capture >> VideoImage;
				}
			}
			else {
				Caclulation_error(difference_x, difference_y, src_x, src_y);
				error_cout = 0;
				b_GetAJ = false;
			}
		}

		//ֻҪ����һ�ξͿ�����
		if (count_AJnum == 4 )
		{  
			count_AJnum = 0;
			b_other250 = true;
			if (b_firstaj == true)
			{
				CreateThread(NULL, 0, DetecAtThird, NULL, 0, NULL);  //DetectThirdThread 
				b_firstaj = false;
				b_other250 = false;  //��һ�ε�trueû��������
			}		   
		}
		waitKey(10);
	}
	return 0;
}

//��������ͷ���
void CAutoPadMachineDlg::OnBnClickedVideoStart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	b_detect = true;
	CreateThread(NULL, 0, DetectFirstThread, NULL, 0, NULL);  //DetectThirdThread  DetectThirdThread  DetectFirstThread
}

//�ֶ���������
void CAutoPadMachineDlg::OnBnClickedAutosend()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (serialport.m_hComm == NULL)
	{
		AfxMessageBox("����û�д򿪣���򿪴���");
		return;
	}
	else
	{
		UpdateData(TRUE);

		char data[512];
		int len = Str2ToHex(m_SendDATA, data); // _t_Str2Hex  Str2Hex
		serialport.WriteToPort(data, len);
		//i_tx_count += (long)((m_strSendData.GetLength() + 1) / 3);
		//SaveSendLog(m_strSendData.GetBuffer());
		//m_DATDSEND.ReleaseBuffer();

		/*	CString strTemp;
		strTemp.Format("TX:%d", i_tx_count);
		cst_ctrl_i_tx_count.SetWindowText(strTemp);*/
	}

}


void CAutoPadMachineDlg::OnBnClickedStartHh()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	b_JudgePostion = true;
}

void CAutoPadMachineDlg::OnBnClickedTiaoshi()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if ( true  ==  b_third_test )
	{
		b_third_test = false;
	}
	else {
		b_third_test = true;
	}

}

void CAutoPadMachineDlg::OnBnClickedChangeMate()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	b_ChangeMaterial = true;
	b_PAGet = false;
}

void CAutoPadMachineDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (::MessageBox(NULL, "�رճ���", "��ʾ", MB_YESNO) == IDNO)
	{
		return;
	}
	b_detect = false;
   //	ofs_send_log.close();
	destroyAllWindows();
	CDialogEx::OnClose();
}

void CAutoPadMachineDlg::OnBnClickedChechover()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	b_CheckCircle = true;
	GetDlgItem(IDC_CHECHover)->EnableWindow(FALSE);
}

void CAutoPadMachineDlg::OnBnClickedForcestart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ForceSendCmd(ST);
}


void CAutoPadMachineDlg::OnBnClickedForcestop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ForceSendCmd(ER);
}


HBRUSH CAutoPadMachineDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_TXT)
			{
				pDC->SetBkColor(RGB(188, 243, 188));//����ɫΪ��ɫ
				pDC->SetTextColor(RGB(0, 0, 0));//����Ϊ��ɫ
				pDC->SelectObject(&font);//����Ϊ15�����壬�����п�
				return m_brush;
			}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}


void CAutoPadMachineDlg::OnBnClickedMaterialJu()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	b_correct = true;
	GetDlgItem(IDC_MATERIAL_JU)->EnableWindow(FALSE);
}
