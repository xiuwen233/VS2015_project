
// AutoPadMachineDlg.cpp : 实现文件
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

CSerialPort serialport;//串口对象
bool b_CheckCircle = true;  //开始默认的检测圆
bool b_JudgePostion = false;
bool b_correct = true;  //纠正物料的
bool b_third_test = false;
bool b_brightness = false;
bool b_CheckSmall = false;   //用来开启是否检测小小的矩形
bool b_CoutCK = true;   //用来判断是否开始计数CK
bool b_ER_Send = false; //用来判断是否发送了ER
bool b_ER_Send_Rect = false; //判断下一个是否发送了ER
bool b_PAGet = false;  //用来检测是否需要换盘
bool b_ChangeMaterial = false;  //换盘  用来确定发送ST的
bool b_OVGet = false;  //用来判断是否接受到了OV
bool b_detect = false;   //结束线程的标志
bool b_ST_Get = false;  //接受到ST开始标
bool b_other250 = false; 
bool b_firstaj = true;

int b_cout_cknum = 0;
int count_AJnum = 0;
int cout_ck_time = 0;  //判断CK接受的次数  7次之后进行圆检测

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
//强制发送一条指令
int ForceSendCmd(char * str)
{
	if (serialport.m_hComm == NULL)
	{
		AfxMessageBox("串口没有打开，请打开串口");
		return -1;
	}

	char data[512];
	int len = Str2ToHex(str, data);
	serialport.WriteToPort(data, len);
	//SaveSendLog(str);

	return 0;
}

//设置波特率的函数
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

std::ofstream ofs_send_log;
//保存数据
void SaveSendLog(char * str)
{
	char tmpBufSend[256];
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(tmpBufSend, "%4d/%02d/%02d %02d:%02d:%02d.%03d SD %s\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, str);
	ofs_send_log << tmpBufSend;
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CAutoPadMachineDlg 对话框

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


// CAutoPadMachineDlg 消息处理程序

BOOL CAutoPadMachineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//ShowWindow(SW_MAXIMIZE);

	// TODO: 在此添加额外的初始化代码

	font.CreatePointFont(250, "宋体");
	m_brush.CreateSolidBrush(RGB(188, 243, 188));//画刷为绿色

	m_cParity = 'N';
	m_nDatabits = 8;
	m_nStopbits = 1;
    m_nBaud = 9600;       //波特率
    m_nCom = 6 ;         //串口号
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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAutoPadMachineDlg::OnPaint()
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
HCURSOR CAutoPadMachineDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAutoPadMachineDlg::OnBnClickedOpenPort()
{
	// TODO: 在此添加控件通知处理程序代码
	//if (m_bOpenPort)  //关闭串口
	//{
	//	m_OpenPort.SetWindowText("打开串口");
	//	serialport.ClosePort();//关闭串口
	//						   //m_ctrlPortStatus.SetWindowText("状态：串口已关闭");
	//	m_Picture.SetIcon(m_hIconOff);
	//	m_bOpenPort = false;
	//}
	//else  //打开串口
	//{
		CString strStatus;
		if (serialport.InitPort(this, m_nCom, 9600, m_cParity, m_nDatabits, m_nStopbits, EV_RXFLAG | EV_RXCHAR, 512))
		{
			serialport.StartMonitoring();
			m_Picture.SetIcon(m_hIconRed);
			strStatus.Format("状态：COM%d 打开，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
			//"当前状态：串口打开，无奇偶校验，8数据位，1停止位");
		}
		else
		{
			AfxMessageBox("没有发现此串口或被占用");
			m_Picture.SetIcon(m_hIconOff);
			return;
		}
		m_OpenPort.SetWindowText("关闭串口");
		//m_ctrlPortStatus.SetWindowText(strStatus);
		m_bOpenPort = true;
	//}

}

//com选择
void CAutoPadMachineDlg::OnCbnSelendokCom()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nCom = m_SerialPort.GetCurSel() + 1;
}

//波特率选择
void CAutoPadMachineDlg::OnCbnSelendokSpeed()
{
	// TODO: 在此添加控件通知处理程序代码
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

//这是接受数据的接口
LRESULT CAutoPadMachineDlg::OnCommunication(WPARAM ch, LPARAM port)
{
	
	if (port <= 0 ) //|| port > 11
		return -1;

	//i_rx_count++;   //接收的字节计数
	//CString strTemp;
	//strTemp.Format("%ld", i_rx_count);
	//strTemp = "RX:" + strTemp;
	//m_ctrlRXCOUNT.SetWindowText(strTemp);  //显示接收计数

	//if (m_bStopDispRXData)   //如果选择了“停止显示”接收数据，则返回
	//	return -1;          //注意，这种情况下，计数仍在继续，只是不显示
	//						//若设置了“自动清空”，则达到8行后，自动清空接收编辑框中显示的数据
	if (m_DataShow.GetLineCount() >= 10)
	{
		m_DataShowStr.Empty();
		UpdateData(FALSE);
	}
	//如果没有“自动清空”，数据行达到400后，也自动清空
	//因为数据过多，影响接收速度，显示是最费CPU时间的操作
	if (m_DataShow.GetLineCount()>400)
	{
		m_DataShowStr.Empty();
		UpdateData(FALSE);
	}

	//如果选择了"十六进制显示"，则显示十六进制值
	CString str;

	str.Format("%02X ", ch);

	//CK 67 75 43 48 发送检测芯片的命令 开始计数判断什么时候开始检测
	if (ch == 75  && b_CoutCK == true)
	{
		cout_ck_time++;
	}
	else if (ch == 75 && b_CoutCK == false)
	{
		b_CheckSmall = true;
	}

	// AJ  十进制 65 74  十六进制 41 4A  开始检测物料盘的四个角的芯片
	if (ch == 74)
	{
		b_JudgePostion = true;
		count_AJnum++;
	}

	//判断接受到的换盘 PA 80 65 50 41
	if (ch == 80)
	{
		b_PAGet = true;
	}

	//OV 79 86 4F 56 判断是否接受到OV
	if (ch == 86)
	{
		b_OVGet = true;
		//b_CheckSmall = false;
		//b_other250 = true;
	}

	//ST 开始  83 84 53 54 
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

	//以下是将接收的字符加在字符串的最后，这里费时很多
	//但考虑到数据需要保存成文件，所以没有用List Control
	int nLen = m_DataShow.GetWindowTextLength();
	m_DataShow.SetSel(nLen, nLen);
	m_DataShow.ReplaceSel(str);
	nLen += str.GetLength();

	m_DataShowStr += str;
	return 0;
}


Mat third_mask_circle;
int  third_mask_circle_d = 30;

//创建第三个摄像头的掩码
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
//创建第三个摄像头的掩码
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

//计算并返回一幅图像的亮度
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
			a += float(GRAYimg.at<uchar>(i, j) - 128);//在计算过程中，考虑128为亮度均值点  
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

//第三个摄像头的线程
DWORD WINAPI DetecAtThird(LPVOID pParam)
{
	Mat videoimage;
	Mat videoimage2;
	Mat filter, canny, wavefilter;
	Mat blue, out;
	Point center;
	float cast;
	float f_dark_quantity;
	int Send_ER_Once = 0;  //让ER至发送一次
	int circle_error_num = 0;  //计算错误检测次数
	int Detect_First_ERROR = 0;
	VideoCapture  capture2(0);
	vector<Vec4i> lines;

	Third_mask_made();
	Third_rentangle_mask_made();
	int true_threshold = 0;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));

	int count_time = 0;
	theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("即将检测编带位置");
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

		//判断第二次重启的条件  接受到OV  判断4个AJ是否接受完 第一次的标志
		//目的是 开启检测小孔的线程其他不管
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
				theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("编带位置检测正确");
				theApp.GetMainWnd()->GetDlgItem(IDC_CHECHover)->EnableWindow(FALSE);
				Detect_First_ERROR = 0; //重置检测第一次错误的函数，一直到下一次跑这个函数
				b_CheckCircle = false;
				//b_brightness = true; 
				//这两个不会有冲突
				if(b_ER_Send == true)
				{ 
					ForceSendCmd(ST);
					b_ER_Send = false;
				}
				
				//如果该开始就是正确的 b_ST_Get 不会改成false 
				//跑到 else error那里则不会执行这一步
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
					theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("编带放置错误 请移到图中黄圈位置,点击按钮校准完毕");//SetWindowText("停止检测");
					theApp.GetMainWnd()->GetDlgItem(IDC_CHECHover)->EnableWindow(TRUE);
					//imshow("out", out);
					//加一个判断第一次检测和第二次检测
					if (Detect_First_ERROR ==  0)
					{  
						while (b_ST_Get == false)  //这里接受到 ST 的参数 因为太快的话4个AJ计算没有做中断
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
					b_CheckCircle = false;    //置零 知道点击按钮开始检测
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

	// 换盘得到ST 不发送ER 并且	b_CheckCircle 为false
	if (b_ST_Get == true && b_ER_Send == false && b_CheckCircle == false)
		{
			ForceSendCmd(ST);
			b_ST_Get = false;
		}

		//判断检测小黑色标志
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
				theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("检测放置正确 继续");
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
					theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("检测芯片放置出错，请调整  ");
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

//创建一个最大外矩形
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

//创建一个小黑的矩形
void Back_mask()
{
	Mat mask;
	mask = Mat(Size(back_small_x + 2, back_samll_y + 2), CV_8UC1, Scalar::all(0));
	rectangle(mask, Point(0, 0), Point(back_small_x+1 , back_samll_y+1 ), Scalar(255), 1);
	back_small = mask.clone();
}

//opencv控制面板响应函数
static void ContrastAndBright(int, void *)
{
	Mat m = Mat(800, 600, 0);
	Max_Rectrectangle_mask();
	imshow("【控制面板】", m);
}

//创建一个控制面板，用来拖动控制响应值
void InitControlBoard()
{
	namedWindow("【控制面板】", 0);
	createTrackbar("二值化阈值：", "【控制面板】", &i_threshold, 256, ContrastAndBright);
	createTrackbar("第二个二值化：", "【控制面板】", &i_threshold_third, 256, ContrastAndBright);
	createTrackbar("初始点x：", "【控制面板】", &i_rect_max_x, 300, ContrastAndBright);
	createTrackbar("初始点y：", "【控制面板】", &i_rect_max_y, 300, ContrastAndBright);
	createTrackbar("矩形宽度：", "【控制面板】", &i_img_mask_max_rect_x, 256, ContrastAndBright);
	createTrackbar("矩形高度：", "【控制面板】", &i_img_mask_max_rect_y, 256, ContrastAndBright);

}

//检测芯片位置的误差
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

//第一个摄像头的检测
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

	//第一个摄像头确定的标签位置
	int src_x = 48;    //168
	int src_y = 185;     //10

	int  point_src_x = 0;
	int  point_src_y = 0;

	int difference_x = 0;
	int difference_y = 0;

	ForceSendCmd(ST);
	Sleep(500);
	bool first_wait_material = true; //用于判断第一个物料的检测 开始
	bool b_GetAJ = false;   //提示开始检测芯片 是下级
	int error_cout = 0; //计算出错的次数
	int diff_x[2] = { 0,0 };
	int diff_y[2] = { 0,0 };
	int time_detect = 0; //计算检测的次数

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
			AfxMessageBox("摄像头打开失败，请检查摄像头后重新运行检测！");
			break;
		}

		//第一个脚垫需要等待时间久一点
		if (first_wait_material == true)
		{
			Sleep(200);
			capture >> VideoImage;
			first_wait_material = false;
		}

	   //显示调试窗口的
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
	    //x轴 10个像素1mm   y轴10个像素0.8mm
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
			//判断其是否放歪
			if (abs(difference_x) >20 || abs(difference_y) >20)
			{
				if (2 <= error_cout)
				{
					theApp.GetMainWnd()->GetDlgItem(IDC_STATIC_TXT)->SetWindowText("请确认物料盘是否放置正确");
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

		//只要开启一次就可以了
		if (count_AJnum == 4 )
		{  
			count_AJnum = 0;
			b_other250 = true;
			if (b_firstaj == true)
			{
				CreateThread(NULL, 0, DetecAtThird, NULL, 0, NULL);  //DetectThirdThread 
				b_firstaj = false;
				b_other250 = false;  //第一次的true没有人置零
			}		   
		}
		waitKey(10);
	}
	return 0;
}

//开启摄像头检测
void CAutoPadMachineDlg::OnBnClickedVideoStart()
{
	// TODO: 在此添加控件通知处理程序代码
	b_detect = true;
	CreateThread(NULL, 0, DetectFirstThread, NULL, 0, NULL);  //DetectThirdThread  DetectThirdThread  DetectFirstThread
}

//手动发送数据
void CAutoPadMachineDlg::OnBnClickedAutosend()
{
	// TODO: 在此添加控件通知处理程序代码
	if (serialport.m_hComm == NULL)
	{
		AfxMessageBox("串口没有打开，请打开串口");
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
	// TODO: 在此添加控件通知处理程序代码
	b_JudgePostion = true;
}

void CAutoPadMachineDlg::OnBnClickedTiaoshi()
{
	// TODO: 在此添加控件通知处理程序代码
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
	// TODO: 在此添加控件通知处理程序代码
	b_ChangeMaterial = true;
	b_PAGet = false;
}

void CAutoPadMachineDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (::MessageBox(NULL, "关闭程序？", "提示", MB_YESNO) == IDNO)
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
	// TODO: 在此添加控件通知处理程序代码
	b_CheckCircle = true;
	GetDlgItem(IDC_CHECHover)->EnableWindow(FALSE);
}

void CAutoPadMachineDlg::OnBnClickedForcestart()
{
	// TODO: 在此添加控件通知处理程序代码
	ForceSendCmd(ST);
}


void CAutoPadMachineDlg::OnBnClickedForcestop()
{
	// TODO: 在此添加控件通知处理程序代码
	ForceSendCmd(ER);
}


HBRUSH CAutoPadMachineDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_TXT)
			{
				pDC->SetBkColor(RGB(188, 243, 188));//背景色为绿色
				pDC->SetTextColor(RGB(0, 0, 0));//文字为红色
				pDC->SelectObject(&font);//文字为15号字体，华文行楷
				return m_brush;
			}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CAutoPadMachineDlg::OnBnClickedMaterialJu()
{
	// TODO: 在此添加控件通知处理程序代码
	b_correct = true;
	GetDlgItem(IDC_MATERIAL_JU)->EnableWindow(FALSE);
}
