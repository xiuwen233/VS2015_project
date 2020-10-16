
// MFCApplicationDlg.cpp : 实现文件
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

Point center(92, 120);//要微调到的中心点
int mask_radio_r = 143;//模板匹配圆半径
int dis_box_and_den = 2064;//第一个脚垫到盒子距离
int match_type = 2;//模板匹配方法
int thresh = 10;//二值化阈值
int minErrValue = 0;//消除不连续点得数量
int maxErrValue = 10;//最大误差
int desti_position = 0;//目标位置，0和1代表两个位置
float hole_distent = 785.5;//盒子两个孔之间距离

float coordinate_y = 0;//实时记录的坐标

Mat mask_img;//匹配模版

int erase = 0;//抹去最小连续区域像素点数（相当于去噪稳定）

CSerialPort m_Port;  //CSerialPort类对象

bool is_thread_running = false;

int m_slider_num = 1;//微调步进

bool b_allow_send_cmd = true;

bool is_cmd_running = true;//下位机指令运行状态

bool b_allow_sucking = false;//开始吸放

CString com_recv_cstr;//接收到的字符串
bool is_recv_com_data = false;//接收标志位（用于控制等待下一条指令的执行）

bool is_all_thread_running = false;

int suck_add_x = 0;//吸嘴微调
int suck_add_y = 176;

int suck_num = 0;//吸取的脚垫数量

int cols_num = 40;//一张料脚垫排数

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

//发送一条指令
bool send_cmd(char * str)
{
	if (m_Port.m_hComm == NULL)
	{
		//MessageBox(_T("串口没有打开，请打开串口"));
		AfxMessageBox("串口没有打开，请打开串口");
		return 1;
	}

	if (!b_allow_send_cmd)
	{
		char ch[128];
		sprintf(ch, "上个指令未完成，操作太快:%s\r\n", str);
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



//全局变量
int port = 8234;
SOCKET s_server;

//服务端地址客户端地址  
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
	send_cmd("EB20");//盒子进
	send_cmd("EB0C");//夹紧

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

	send_cmd("EB0D");//松开

	send_cmd("EB21");//盒子出

	Sleep(2000);

	CreateThread(NULL, 0, FunSend, &back_true, 0, NULL);

	return 0;
}

//强制发送一条指令（不管返回状态）
int fore_cend_cmd(char * str)
{
	if (m_Port.m_hComm == NULL)
	{
		AfxMessageBox("串口没有打开，请打开串口");
		return -1;
	}

	char data[512];
	int len = _t_Str2Hex(str, data);
	m_Port.WriteToPort(data, len);

	return 0;
}

//一些必要的初始化
bool sys_init()
{
	//初始化模板匹配模板图像
	Mat mask = Mat(Size(mask_radio_r + 2, mask_radio_r + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(mask_radio_r / 2 + 1, mask_radio_r / 2 + 1), mask_radio_r / 2, Scalar(255, 255, 255), 1);
	mask_img = mask.clone();

	return 0;
}

//opencv控制面板响应函数
static void ContrastAndBright(int, void *)
{
	Mat m = Mat(600, 200, 0);
	imshow("【控制面板】", m);

	Mat mask = Mat(Size(mask_radio_r + 2, mask_radio_r + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(mask_radio_r / 2 + 1, mask_radio_r / 2 + 1), mask_radio_r / 2, Scalar(255, 255, 255), 1);
	//imshow("mask", mask);
	mask_img = mask.clone();
}

//创建opencv控制面板
void initControlBoard()
{
	namedWindow("【控制面板】", 0);

	createTrackbar("mask_radio_r：", "【控制面板】", &mask_radio_r, 200, ContrastAndBright);
	createTrackbar("erase：", "【控制面板】", &erase, 100, ContrastAndBright);
	createTrackbar("thresh：", "【控制面板】", &thresh, 100, ContrastAndBright);
	createTrackbar("match_type：", "【控制面板】", &match_type, 5, ContrastAndBright);
}

//模版匹配
void match_temp(Mat bitwise_out, Mat templ)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//进行匹配
	matchTemplate(bitwise_out, templ, g_resultImage, match_type);

	//标准化
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;

	//定位最匹配的位置
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

	if (x > 72 && x < 112 && y > 70 && y < 170)//更新
	{
		center.x = x;
		center.y = y;
	}

	//rectangle(srcImage, matchLocation, Point(matchLocation.x + templ.cols, matchLocation.y + templ.rows), Scalar(128, 128, 128), 2, 8, 0);
	//resize(g_resultImage, g_resultImage, Size(g_resultImage.cols * 2, g_resultImage.rows * 2));
	//imshow("匹配过程", g_resultImage);

	//int radius = mask_radio_r / 2;
	////圆中心  
	//circle(srcImage, center, 3, Scalar(128, 128, 128), -1, 8, 0);
	////圆轮廓  
	//circle(srcImage, center, radius, Scalar(128, 128, 128), 2, 8, 0);
	//String str = "[" + to_string(center.x) + "," + to_string(center.y) + "]";
	//putText(srcImage, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);

	////imshow("out", srcImage);

	//return srcImage;
}

//CheckMode: 0代表去除黑区域，1代表去除白区域; NeihborMode：0代表4邻域，1代表8邻域;  
void RemoveSmallRegion(Mat& Src, Mat& Dst, int AreaLimit, int CheckMode, int NeihborMode)
{
	int RemoveCount = 0;       //记录除去的个数  
							   //记录每个像素点检验状态的标签，0代表未检查，1代表正在检查,2代表检查不合格（需要反转颜色），3代表检查合格或不需检查  
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

	vector<Point2i> NeihborPos;  //记录邻域点位置  
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
	//开始检测  
	for (int i = 0; i < Src.rows; ++i)
	{
		uchar* iLabel = Pointlabel.ptr<uchar>(i);
		for (int j = 0; j < Src.cols; ++j)
		{
			if (iLabel[j] == 0)
			{
				//********开始该点处的检查**********  
				vector<Point2i> GrowBuffer;  //堆栈，用于存储生长点  
				GrowBuffer.push_back(Point2i(j, i));
				Pointlabel.at<uchar>(i, j) = 1;
				int CheckResult = 0;//用于判断结果（是否超出大小），0为未超出，1为超出  

				for (int z = 0; z<GrowBuffer.size(); z++)
				{

					for (int q = 0; q<NeihborCount; q++)//检查四个邻域点  
					{
						CurrX = GrowBuffer.at(z).x + NeihborPos.at(q).x;
						CurrY = GrowBuffer.at(z).y + NeihborPos.at(q).y;
						if (CurrX >= 0 && CurrX<Src.cols&&CurrY >= 0 && CurrY<Src.rows) //防止越界  
						{
							if (Pointlabel.at<uchar>(CurrY, CurrX) == 0)
							{
								GrowBuffer.push_back(Point2i(CurrX, CurrY)); //邻域点加入buffer  
								Pointlabel.at<uchar>(CurrY, CurrX) = 1; //更新邻域点的检查标签，避免重复检查  
							}
						}
					}
				}
				if (GrowBuffer.size()>AreaLimit) CheckResult = 2; //判断结果（是否超出限定的大小），1为未超出，2为超出  
				else { CheckResult = 1;   RemoveCount++; }
				for (int z = 0; z<GrowBuffer.size(); z++)                         //更新Label记录  
				{
					CurrX = GrowBuffer.at(z).x;
					CurrY = GrowBuffer.at(z).y;
					Pointlabel.at<uchar>(CurrY, CurrX) += CheckResult;
				}
			}
		}
	}

	CheckMode = 255 * (1 - CheckMode);
	//开始反转面积过小的区域  
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
			MessageBox(NULL, "摄像头打开失败！！！", "提示", MB_OK);
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
		//归一化到0~255    
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

		//圆中心  
		circle(detect_out_img, center, 3, Scalar(0, 255, 255), -1, 8, 0);

		//圆轮廓  
		circle(detect_out_img, center, mask_radio_r / 2, Scalar(0, 255, 255), 1, 8, 0);
		String str = "[" + to_string(center.x) + "," + to_string(center.y) + "]";
		putText(detect_out_img, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);

		imshow("detect_out", detect_out_img);
		waitKey(1);
	}
	return 0;
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


// CMFCApplicationDlg 对话框
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


// CMFCApplicationDlg 消息处理程序

//实时检测Y坐标
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

			AfxMessageBox(_T("连接成功"));
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
		AfxMessageBox(_T("连接失败"));
	}

	return true;
}


BOOL CMFCApplicationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。
	BOOL b = CDialog::OnInitDialog();
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

									// TODO: 在此添加额外的初始化代码
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

	//十六进制发送、显示
	((CButton *)GetDlgItem(IDC_CHECK_HEXSEND))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_CHECK_HEXRECIEVE))->SetCheck(TRUE);

	//自动清空
	m_ctrlIconOpenoff.SetIcon(m_hIconOff);

	sys_init();

	//步进滑动条
	m_slider.SetRange(1, 255);
	m_slider.SetTicFreq(1);
	m_slider.SetPos(m_slider_num);

	//脚垫到盒子滑动条
	m_slider2.SetRange(2000, 2100);
	m_slider2.SetTicFreq(1);
	m_slider2.SetPos(dis_box_and_den);
	String str2 = to_string(dis_box_and_den);
	CString cstr2(str2.c_str());
	GetDlgItem(IDC_STATIC_STEP5)->SetWindowText(cstr2);

	//吸嘴微调X滑动条
	m_slider3.SetRange(-20, 20);
	m_slider3.SetTicFreq(1);
	m_slider3.SetPos(suck_add_x);
	String str3 = to_string(suck_add_x);
	CString cstr3(str3.c_str());
	GetDlgItem(IDC_STATIC_STEP6)->SetWindowText(cstr3);

	//吸嘴Y滑动条
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

	return b;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplicationDlg::OnPaint()
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
HCURSOR CMFCApplicationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static long rxdatacount = 0;  //该变量用于接收字符计数WPARAM wParam,LPARAM lParam
LRESULT CMFCApplicationDlg::OnCommunication(WPARAM ch, LPARAM port)
{
	if (port <= 0 || port > 11)
		return -1;
	rxdatacount++;   //接收的字节计数
	CString strTemp;
	strTemp.Format("%ld", rxdatacount);
	strTemp = "RX:" + strTemp;
	m_ctrlRXCOUNT.SetWindowText(strTemp);  //显示接收计数

	if (m_bStopDispRXData)   //如果选择了“停止显示”接收数据，则返回
		return -1;          //注意，这种情况下，计数仍在继续，只是不显示
							//若设置了“自动清空”，则达到50行后，自动清空接收编辑框中显示的数据
	if ((m_ctrlAutoClear.GetCheck()) && (m_ctrlReceiveData.GetLineCount() >= 50))
	{
		m_ReceiveData.Empty();
		UpdateData(FALSE);
	}
	//如果没有“自动清空”，数据行达到400后，也自动清空
	//因为数据过多，影响接收速度，显示是最费CPU时间的操作
	if (m_ctrlReceiveData.GetLineCount()>400)
	{
		m_ReceiveData.Empty();
		m_ReceiveData = "***The Length of the Text is too long, Emptied Automaticly!!!***\r\n";
		UpdateData(FALSE);
	}

	//如果选择了"十六进制显示"，则显示十六进制值
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
	//以下是将接收的字符加在字符串的最后，这里费时很多
	//但考虑到数据需要保存成文件，所以没有用List Control
	int nLen = m_ctrlReceiveData.GetWindowTextLength();
	m_ctrlReceiveData.SetSel(nLen, nLen);
	m_ctrlReceiveData.ReplaceSel(str);
	nLen += str.GetLength();

	m_ReceiveData += str;
	return 0;
}

//清空接收区
void CMFCApplicationDlg::OnButtonClearReciArea()
{
	// TODO: Add your control notification handler code here
	m_ReceiveData.Empty();
	UpdateData(FALSE);
}

//打开/关闭串口
void CMFCApplicationDlg::OnButtonOpenport()
{
	if (m_bOpenPort)  //关闭串口
	{
		m_ctrlOpenPort.SetWindowText("打开串口");
		m_Port.ClosePort();//关闭串口
		m_ctrlPortStatus.SetWindowText("状态：串口已关闭");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);
		m_bOpenPort = false;
	}
	else  //打开串口
	{
		CString strStatus;
		if (m_Port.InitPort(this, m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits, m_dwCommEvents, 512))
		{
			m_Port.StartMonitoring();
			m_ctrlIconOpenoff.SetIcon(m_hIconRed);
			strStatus.Format("状态：COM%d 打开，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
			//"当前状态：串口打开，无奇偶校验，8数据位，1停止位");
			//send_cmd("EB26");
		}
		else
		{
			AfxMessageBox("没有发现此串口或被占用");
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			return;
		}
		m_ctrlOpenPort.SetWindowText("关闭串口");
		m_ctrlPortStatus.SetWindowText(strStatus);
		m_bOpenPort = true;
	}
}

//停止/继续显示接收数据
void CMFCApplicationDlg::OnButtonStopdisp()
{
	m_bStopDispRXData = !m_bStopDispRXData;
	if (m_bStopDispRXData)
		m_ctrlStopDisp.SetWindowText("继续显示");
	else
		m_ctrlStopDisp.SetWindowText("停止显示");
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

//将一个字符串作为十六进制串转化为一个字节数组，字节间可用空格分隔，
//返回转换后的字节数组长度，同时字节数组长度自动设置。
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
//手动发送
void CMFCApplicationDlg::OnButtonManualsend()
{
	if (m_Port.m_hComm == NULL)
	{
		AfxMessageBox("串口没有打开，请打开串口");
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
			m_Port.WriteToPort((LPCTSTR)m_strSendData);	//发送数据
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

//串口序号下拉
void CMFCApplicationDlg::OnSelendokComboComselect()
{
	m_nCom = m_Com.GetCurSel() + 1;
}

//波特率下拉
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


			strStatus.Format("状态：COM%d 打开，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		else
		{
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			strStatus.Format("状态：COM%d CLOSED，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		m_ctrlPortStatus.SetWindowText(strStatus);

	}
	else
	{
		AfxMessageBox("没有成功，请重试");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);

	}


}

//校验位下拉
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
			strStatus.Format("状态：COM%d 打开，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		else
		{
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			strStatus.Format("状态：COM%d CLOSED，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		m_ctrlPortStatus.SetWindowText(strStatus);

	}
	else
	{
		AfxMessageBox("没有成功，请重试");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);
	}


}

//数据位下拉
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
			strStatus.Format("状态：COM%d 打开，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		else
		{
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			strStatus.Format("状态：COM%d CLOSED，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		m_ctrlPortStatus.SetWindowText(strStatus);

	}
	else
	{
		AfxMessageBox("没有成功，请重试");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);
	}


}

//停止位下拉
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
			strStatus.Format("状态：COM%d 打开，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		else
		{
			m_ctrlIconOpenoff.SetIcon(m_hIconOff);
			strStatus.Format("状态：COM%d CLOSED，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
		}
		m_ctrlPortStatus.SetWindowText(strStatus);

	}
	else
	{
		AfxMessageBox("没有成功，请重试");
		m_ctrlIconOpenoff.SetIcon(m_hIconOff);
	}

}

//清空计数器
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
	KillTimer(1);   //关闭定时器
	KillTimer(4);
	m_Port.ClosePort();  //关闭串口
	m_ReceiveData.Empty();  //清空接收数据字符串
}

//停止检测
void CMFCApplicationDlg::OnBnClickedBtnStopDetect()
{
	is_thread_running = false;
	destroyAllWindows();
}

//关闭程序
void CMFCApplicationDlg::OnClose()
{
	if (::MessageBox(NULL, "  确定要退出？", "提示", MB_YESNO) == IDYES)
	{
		is_all_thread_running = false;
		is_thread_running = false;
		destroyAllWindows();
		CMFCApplicationDlg::OnOK();
	}
}

//负压检查
bool check_suck()
{
	if (m_Port.m_hComm == NULL)
	{
		AfxMessageBox("串口没有打开，请打开串口");
		return 1;
	}

	if (!b_allow_send_cmd)
	{
		char ch[128];
		sprintf(ch, "上个指令未完成，操作太快!");
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
	m_slider_num = m_slider.GetPos();//取得当前位置值 
	String str = to_string(m_slider_num);
	CString cstr(str.c_str());
	GetDlgItem(IDC_STATIC_STEP)->SetWindowText(cstr);


	dis_box_and_den = m_slider2.GetPos();//取得当前位置值 
	String str2 = to_string(dis_box_and_den);
	CString cstr2(str2.c_str());
	GetDlgItem(IDC_STATIC_STEP5)->SetWindowText(cstr2);


	suck_add_x = m_slider3.GetPos();//取得当前位置值 
	String str3 = to_string(suck_add_x);
	CString cstr3(str3.c_str());
	GetDlgItem(IDC_STATIC_STEP6)->SetWindowText(cstr3);

	suck_add_y = m_slider4.GetPos();//取得当前位置值 
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
		GetDlgItem(IDC_BTN_PAUSE)->SetWindowText("继续");
		//黄灯开，等待物料
		//fore_cend_cmd("EB26");

	}
	else {
		is_cmd_running = true;
		GetDlgItem(IDC_BTN_PAUSE)->SetWindowText("暂停");
		//绿灯开,运行
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
	int cycles = 0;//周期（每11个脚垫1个周期）

	for (int i = 0; i < cols_num; i++)
	{
		//第一排6个
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
				char str_d[6];//下顶微调
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
			}//对准完毕

			send_cmd("EB1100CC");//吸头移动到脚垫位置

			send_cmd("EB01");//吸头下压
			send_cmd("EB09");//吸头吸气
			send_cmd("EB03");//下面顶起
			send_cmd("EB02");//吸头上压
			send_cmd("EB04");//下面顶下

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

			//到达盒子
			send_cmd("EB01");//吸头下压
			send_cmd("EB0A");//吸头放气
			send_cmd("EB02");//吸头上压

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
				send_cmd("EB1101DD");//下一排（5个）
				send_cmd("EB1583");

				send_cmd("EB1850");//下顶回来

			}

			suck_num++;

			Sleep(5);
		}

		//第二排5个
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
				char str_d[6];//下顶微调
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
			}//对准完毕

			send_cmd("EB1100CC");//吸头移动到脚垫位置

			send_cmd("EB01");//吸头下压
			send_cmd("EB09");//吸头吸气
			send_cmd("EB03");//下面顶起
			send_cmd("EB02");//吸头上压
			send_cmd("EB04");//下面顶下

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

			//到达盒子
			send_cmd("EB01");//吸头下压
			send_cmd("EB0A");//吸头放气
			send_cmd("EB02");//吸头上压

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

			if (cols_5 == 2 && cycles % 5 == 0 && cycles != 0)//调整下顶位置（运动误差引起）
			{
				//AfxMessageBox("调整");
				send_cmd("EB180B");
			}

			if (cols_5 == 4)
			{
				send_cmd("EB1350");//镜头（吸头）调整
				send_cmd("EB1583");//送料调整
				send_cmd("EB1850");//下顶调整
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
	send_cmd("EB02");//吸头上压
	send_cmd("EB10");//复位
	send_cmd("EB112EAD");//移动到物料
	coordinate_y += 3107;

	send_cmd("EB08");//下顶复位

	Sleep(30);

	send_cmd("EB1764");//出来一点

	Sleep(30);

	send_cmd("EB08");//再复位

	Sleep(30);

	send_cmd("EB1741");//下顶对准镜头

	Sleep(20);

	CreateThread(NULL, 0, fipPointThread2, NULL, 0, NULL);//开始吸取服务


	

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
