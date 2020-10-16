
// NewAutoPatDlg.cpp : 实现文件
/*
* These codes are written for the automatic foot rest machine
* maybe you will find that it is not perfect
*we We'd love you to improve it
*if you find somewhere that you are not andstand
*you can You can turn to the founder wangxiuwen
*if you can still find he in this company
*over 20200320
*/

#include "stdafx.h"
#include "NewAutoPat.h"
#include "NewAutoPatDlg.h"
#include "afxdialogex.h"

#include<iostream>
#include<string>
#include<thread>
#include"common.h"
#include "HiKanAPI.h"
#include "global_data.h"
#include"fun_opencv.h"
#include"Datauser.h"
#include"datachange.h"
#include<math.h>
#include"Csetting.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CONFIG_FILE_PATH "cfg/config.ini"
#define CONFIG_APP_NAME  "umAppConfig"

using namespace std;

//------------------------------------------------------------------------------

CString cpu_ip[4];
CString state_record;

int auto_slider_bj = 1;

HikAPI *myhikclass = NULL;
MV_CC_DEVICE_INFO_LIST m_stDevList; 

Mat img_mask;
int element_ght = 1;

BOOL b_test_stop = FALSE;

struct SocketTest
{
	SOCKET  socket_cpu;
	BOOL is_connect;
	BOOL is_cmd_sending;
};

SocketTest  u1_cpu = { NULL, FALSE, FALSE };
SocketTest  u2_cpu = { NULL, FALSE, FALSE };
SocketTest  u3_cpu = { NULL, FALSE, FALSE };
SocketTest  host_ip = { NULL, FALSE, FALSE };
int host_port;

struct SocketInfo
{
	CString u1_ip;
	CString u2_ip;
	CString u3_ip;
	CString host_ip;
	CString host_port;
};

SocketInfo  socketinfo = { "192.168.201.1", "192.168.201.2",
											"192.168.201.3", "192.168.1.118",
											 "10010" };


struct ImageAndCoordinateData
{
	int  adjust_x ;
	int  adjust_y ;
	int tem_len;
	int temu1_save_len;
	int temu2_save_len;
	int userd_cols;
	int tem_cols;
	int tem_rowa;
	int postion_mat;
	float dark_quantity ;
	int error_number;
	bool circle_judge_exceed;
	Point tem_center;
};

ImageAndCoordinateData  templedata = {0, 0, 0, 0, 0, 0, -2, -2, 0};


SaveData savedata = { 496, 339, 1.02, 2.78, 10.108, 654, 1006, 318, 466,
									   928, 45058, 1124, 14840, 18310, 38, 0, 214, 318,
									   0, 0, 0, 0, 0, 0
};

Golbal_Bool_State globalstate = { false, false, false, false, false,
                                                           false, false, false, false, false,
														   false, false, false, false, false
};


 //**************************************************************
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


// CNewAutoPatDlg 对话框

CNewAutoPatDlg::CNewAutoPatDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_NEWAUTOPAT_DIALOG, pParent)
	, m_CPU1_IP(_T(""))
	, m_CPU2__IP(_T(""))
	, m_CPU3_IP(_T(""))
	, m_RECEDATA_STR(_T(""))
	, m_Data_Send_Str(_T(""))
	, m_numDeviceComb(0)
	, m_datadigital(_T(""))
	, m_HomeIp(_T(""))
	, m_host_port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNewAutoPatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CPU1_IP, m_CPU1_IP);
	DDX_Text(pDX, IDC_CPU2_IP, m_CPU2__IP);
	DDX_Text(pDX, IDC_CPU3_IP, m_CPU3_IP);
	DDX_Control(pDX, IDC_DATA_RECEIVE, m_RECEIVER_DATA);
	DDX_Text(pDX, IDC_DATA_RECEIVE, m_RECEDATA_STR);
	DDX_Control(pDX, IDC_SLIDER1, m_sliderBJ);
	DDX_Control(pDX, IDC_DATA_SEND, m_Data_Send);
	DDX_Text(pDX, IDC_DATA_SEND, m_Data_Send_Str);
	DDX_Control(pDX, IDC_CPU1_IP, m_U1_IP_edit);
	DDX_Control(pDX, IDC_CPU2_IP, m_U2_IP_edit);
	DDX_Control(pDX, IDC_CPU3_IP, m_U3_IP_edit);
	DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrCombox);
	DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_numDeviceComb);
	DDX_Control(pDX, IDC_MAT_POSTION, m_MatPostion);
	DDX_Control(pDX, IDC_TEST_DEBUG, m_Debug_check);
	DDX_Control(pDX, IDC_DATA_CHANGE, m_DataChange);
	DDX_Control(pDX, IDC_SELECT_ITEM, m_Dataselect);
	DDX_Text(pDX, IDC_DATA_CHANGE, m_datadigital);
	DDX_Text(pDX, IDC_HOME_IP, m_HomeIp);
	DDX_Control(pDX, IDC_HOME_IP, m_HostIP_Edit);
	DDX_Control(pDX, IDC_EDIT3, m_HostPort);
	DDX_Text(pDX, IDC_EDIT3, m_host_port);
}

BEGIN_MESSAGE_MAP(CNewAutoPatDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_MESSAGE(WM_MyMessage, OnMyMessage)
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_IP_SURE, &CNewAutoPatDlg::OnBnClickedIpSure)
	ON_BN_CLICKED(IDC_TEST_ONE, &CNewAutoPatDlg::OnBnClickedTestOne)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_DIRECT_UP, &CNewAutoPatDlg::OnBnClickedDirectUp)
	ON_BN_CLICKED(IDC_DIRECT_DOWN, &CNewAutoPatDlg::OnBnClickedDirectDown)
	ON_BN_CLICKED(IDC_DIREC_LEFT, &CNewAutoPatDlg::OnBnClickedDirecLeft)
	ON_BN_CLICKED(IDC_DIREC_RIGHT, &CNewAutoPatDlg::OnBnClickedDirecRight)
	ON_BN_CLICKED(IDC_PADS_UP, &CNewAutoPatDlg::OnBnClickedPadsUp)
	ON_BN_CLICKED(IDC_PADS_DOWN, &CNewAutoPatDlg::OnBnClickedPadsDown)
	ON_BN_CLICKED(IDC_AIR_OPEN, &CNewAutoPatDlg::OnBnClickedAirOpen)
	ON_BN_CLICKED(IDC_AIR_CLOSE, &CNewAutoPatDlg::OnBnClickedAirClose)
	ON_BN_CLICKED(IDC_MAT_UP, &CNewAutoPatDlg::OnBnClickedMatUp)
	ON_BN_CLICKED(IDC_MAT_DOWN, &CNewAutoPatDlg::OnBnClickedMatDown)
	ON_BN_CLICKED(IDC_BT_DATA_SEND, &CNewAutoPatDlg::OnBnClickedBtDataSend)
	ON_BN_CLICKED(IDC_BT_DATA_SEND2, &CNewAutoPatDlg::OnBnClickedBtDataSend2)
	ON_BN_CLICKED(IDC_BT_DATA_SEND3, &CNewAutoPatDlg::OnBnClickedBtDataSend3)
	ON_BN_CLICKED(IDC_CODE_TEST, &CNewAutoPatDlg::OnBnClickedCodeTest)
	ON_BN_CLICKED(IDC_DATA_CLEAN, &CNewAutoPatDlg::OnBnClickedDataClean)
	ON_BN_CLICKED(IDC_U3_PRESS, &CNewAutoPatDlg::OnBnClickedU3Press)
	ON_BN_CLICKED(IDC_RESET_U1, &CNewAutoPatDlg::OnBnClickedResetU1)
	ON_BN_CLICKED(IDC_BOTTOM_RESET, &CNewAutoPatDlg::OnBnClickedBottomReset)
	ON_BN_CLICKED(IDC_U3_PROESS_DOWN, &CNewAutoPatDlg::OnBnClickedU3ProessDown)
	ON_BN_CLICKED(IDC_STOP_BT, &CNewAutoPatDlg::OnBnClickedStopBt)
	ON_BN_CLICKED(IDC_FIND_DEVICE_BUTTON, &CNewAutoPatDlg::OnBnClickedFindDeviceButton)
	ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CNewAutoPatDlg::OnBnClickedStartGrabbingButton)
	ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CNewAutoPatDlg::OnBnClickedStopGrabbingButton)
	ON_BN_CLICKED(IDC_THREAD_BUTTON, &CNewAutoPatDlg::OnBnClickedThreadButton)
	ON_BN_CLICKED(IDC_SMALL_BUTTON, &CNewAutoPatDlg::OnBnClickedSmallButton)
	ON_BN_CLICKED(IDC_SOCKET_CONNECT_BUTTON, &CNewAutoPatDlg::OnBnClickedSocketConnectButton)
	ON_BN_CLICKED(IDC_SOCKET_DISCONNECT_BUTTON, &CNewAutoPatDlg::OnBnClickedSocketDisconnectButton)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_MAT_POSTION, &CNewAutoPatDlg::OnSelchangeMatPostion)
	ON_BN_CLICKED(IDC_TEST_DEBUG, &CNewAutoPatDlg::OnBnClickedTestDebug)
	ON_BN_CLICKED(IDC_CHANGE_DATA, &CNewAutoPatDlg::OnBnClickedChangeData)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SELECT_ITEM, &CNewAutoPatDlg::OnNMCustomdrawSelectItem)
	ON_BN_CLICKED(IDC_HOST_SEND, &CNewAutoPatDlg::OnBnClickedHostSend)
	ON_BN_CLICKED(IDC_BOTTOM_CAMERA, &CNewAutoPatDlg::OnBnClickedBottomCamera)
	ON_BN_CLICKED(IDC_SETTING, &CNewAutoPatDlg::OnBnClickedSetting)
END_MESSAGE_MAP()


void CNewAutoPatDlg::SetWinEnable()
{
	GetDlgItem(IDC_DIRECT_UP)->EnableWindow(TRUE);
	GetDlgItem(IDC_DIREC_LEFT)->EnableWindow(TRUE);
	GetDlgItem(IDC_DIREC_RIGHT)->EnableWindow(TRUE);
	GetDlgItem(IDC_DIRECT_DOWN)->EnableWindow(TRUE);
	GetDlgItem(IDC_PADS_UP)->EnableWindow(TRUE);
	GetDlgItem(IDC_PADS_DOWN)->EnableWindow(TRUE);
	GetDlgItem(IDC_AIR_OPEN)->EnableWindow(TRUE);

	GetDlgItem(IDC_AIR_CLOSE)->EnableWindow(TRUE);
	GetDlgItem(IDC_MAT_UP)->EnableWindow(TRUE);
	GetDlgItem(IDC_MAT_DOWN)->EnableWindow(TRUE);
	GetDlgItem(IDC_SLIDER1)->EnableWindow(TRUE);
	GetDlgItem(IDC_SELECT_ITEM)->EnableWindow(TRUE);
	GetDlgItem(IDC_DATA_CHANGE)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHANGE_DATA)->EnableWindow(TRUE);

	GetDlgItem(IDC_BOTTOM_RESET)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOP_BT)->EnableWindow(TRUE);
	GetDlgItem(IDC_U3_PRESS)->EnableWindow(TRUE);
	GetDlgItem(IDC_SMALL_BUTTON)->EnableWindow(TRUE);
	GetDlgItem(IDC_RESET_U1)->EnableWindow(TRUE);   
   
	GetDlgItem(IDC_BT_DATA_SEND)->EnableWindow(TRUE);
	GetDlgItem(IDC_BT_DATA_SEND2)->EnableWindow(TRUE);
	GetDlgItem(IDC_BT_DATA_SEND3)->EnableWindow(TRUE);
	GetDlgItem(IDC_DATA_SEND)->EnableWindow(TRUE);

	GetDlgItem(IDC_HOST_SEND)->EnableWindow(TRUE);
	GetDlgItem(IDC_BOTTOM_CAMERA)->EnableWindow(TRUE);
}

void CNewAutoPatDlg::SetWinUnEnable()
{
	GetDlgItem(IDC_DIRECT_UP)->EnableWindow(FALSE);
	GetDlgItem(IDC_DIREC_LEFT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DIREC_RIGHT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DIRECT_DOWN)->EnableWindow(FALSE);
	GetDlgItem(IDC_PADS_UP)->EnableWindow(FALSE);
	GetDlgItem(IDC_PADS_DOWN)->EnableWindow(FALSE);
	GetDlgItem(IDC_AIR_OPEN)->EnableWindow(FALSE);

	GetDlgItem(IDC_AIR_CLOSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_MAT_UP)->EnableWindow(FALSE);
	GetDlgItem(IDC_MAT_DOWN)->EnableWindow(FALSE);
	GetDlgItem(IDC_SLIDER1)->EnableWindow(FALSE);
	GetDlgItem(IDC_SELECT_ITEM)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATA_CHANGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHANGE_DATA)->EnableWindow(FALSE);

	GetDlgItem(IDC_BOTTOM_RESET)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_BT)->EnableWindow(FALSE);
	GetDlgItem(IDC_U3_PRESS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SMALL_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_RESET_U1)->EnableWindow(FALSE);

	GetDlgItem(IDC_BT_DATA_SEND)->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_DATA_SEND2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BT_DATA_SEND3)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATA_SEND)->EnableWindow(FALSE);

	GetDlgItem(IDC_HOST_SEND)->EnableWindow(FALSE);
	GetDlgItem(IDC_BOTTOM_CAMERA)->EnableWindow(FALSE);
}


// CNewAutoPatDlg 消息处理程序
BOOL CNewAutoPatDlg::OnInitDialog()
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

	// TODO: 在此添加额外的初始化代码

	SetWinUnEnable();
	
	m_sliderBJ.SetRange(1, 255);
	m_sliderBJ.SetTicFreq(1);
	m_sliderBJ.SetPos(20);

	m_Dataselect.SetRange(1, 20);
	m_Dataselect.SetTicFreq(1);
	m_Dataselect.SetPos(1);




	CString str = InitConfig();
	if ("null" != str)
	{
		AfxMessageBox(str);
	}

	m_U1_IP_edit.SetWindowTextA(socketinfo.u1_ip);
	m_U2_IP_edit.SetWindowTextA(socketinfo.u2_ip);
	m_U3_IP_edit.SetWindowTextA(socketinfo.u3_ip);
	m_HostIP_Edit.SetWindowTextA(socketinfo.host_ip);
	m_HostPort.SetWindowTextA(socketinfo.host_port);

	m_MatPostion.SetCurSel(savedata.postion_mat);
	templedata.postion_mat = savedata.postion_mat;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CNewAutoPatDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CNewAutoPatDlg::OnPaint()
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
HCURSOR CNewAutoPatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


DWORD WINAPI CPU1_RECEIVER(LPVOID  lppargram)
{
	SOCKET socket = *(SOCKET *)lppargram;
	int num = 0;
	char msg[512];
	CString str;
	str = "u1 receiver start  ";
	SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
	while (true)
	{
		num = recv(socket, msg, 512, 0);
		if (num>0)
		{
			if (Hex_2_str(msg, num) == "0016")
			{
				globalstate.socket_air_sucess = true;
			}
			u1_cpu.is_cmd_sending = FALSE;
			//printf("u1 receivered \n");
			str = "u1: " + Hex_2_str(msg, num) + " ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			//printf("send message over");
		}
		else if(num == 0)
		{
			str = "u1:  receiver   0 ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			break;
		}
		else if(num < 0 )
		{
			//待处理
			str = "u1:  receiver  -1 ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			u1_cpu.is_connect = FALSE;
			break;
		}
	}

	closesocket(socket);
	return 0;

}

DWORD WINAPI CPU2_RECEIVER(LPVOID  lppargram)
{
	SOCKET socket = *(SOCKET *)lppargram;
	int num = 0;
	char msg[512];
	CString str;
	str = "u2 receiver start  ";
	SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
	while (true)
	{
		num = recv(socket, msg, 512, 0);
		if (num>0)
		{
			u2_cpu.is_cmd_sending = FALSE;
			str = "u2: " + Hex_2_str(msg, num) + " ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		}
		else if (num == 0)
		{
			str = "u2: receiver 0 ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			break;
		}
		else if (num < 0)
		{
			//待处理
			str = "u2: receiver -1 ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			u2_cpu.is_connect = FALSE;
			break;
		}
	}
	closesocket(socket);
	return 0;
}

DWORD WINAPI CPU3_RECEIVER(LPVOID  lppargram)
{
	SOCKET socket = *(SOCKET *)lppargram;
	int num = 0;
	char msg[512];
	CString str;

	str = "u3 receiver start  ";
	SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));

	while (true)
	{
		num = recv(socket, msg, 512, 0);
		if (num>0)
		{
			u3_cpu.is_cmd_sending = FALSE;
			//printf("u3 receivered \n");
			
			str = "u3: "+Hex_2_str(msg, num)+" ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		}
		else if (num == 0)
		{
			str = "u3: receiver 0 ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			break;
		}
		else if (num < 0)
		{
			//待处理
			str = "u3: receiver -1 ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			u3_cpu.is_connect = FALSE;
			break;
		}
	}
	closesocket(socket);
	return 0;
}

int SocketSendCmd(CString str, int cpu_number)
{

	while (globalstate.socket_send_stop)
	{
		Sleep(20);
	}

	char data[512] = { 0 };
	int lengh = 0;
	int ret = 0;
	memset(data, 0, sizeof(data));
	lengh = Str2ToHex(str, data);

	switch (cpu_number)
	{
	case 1:
		if (!u1_cpu.is_connect)
		{
			//没连接退出
			printf("return -2\n");
			return -2;
		}
		u1_cpu.is_cmd_sending = TRUE;
		ret = send(u1_cpu.socket_cpu, data, lengh, 0);
		//printf("u1 send over \n");
		if (ret == SOCKET_ERROR)
		{
			u1_cpu.is_cmd_sending = FALSE;
			return -1;
		}
		while (u1_cpu.is_cmd_sending)
		{
			Sleep(5);
			//printf("wait for stop \n");
		}
		//printf("u1 receiver ok\n");
		break;
	case 2:
		if (!u2_cpu.is_connect)
		{
			//没连接退出
			return -2;
		}
		u2_cpu.is_cmd_sending = TRUE;
		ret = send(u2_cpu.socket_cpu, data, lengh, 0);
		//printf("u2 send over \n");
		if (ret == SOCKET_ERROR)
		{
			u2_cpu.is_cmd_sending = FALSE;
			return -1;
		}
		while (u2_cpu.is_cmd_sending)
		{
			Sleep(5);
		}
		//printf("u2 receiver ok\n");
		break;
	case 3:
		if (!u3_cpu.is_connect)
		{
			//没连接退出
			return -2;
		}
		u3_cpu.is_cmd_sending = TRUE;
		ret = send(u3_cpu.socket_cpu, data, lengh, 0);
		//printf("u3 send over \n");
		if (ret == SOCKET_ERROR)
		{
			u3_cpu.is_cmd_sending = FALSE;
			return -1;
		}
		while (u3_cpu.is_cmd_sending)
		{
			Sleep(5);
		}
		break;
	case 4:
		if (!host_ip.is_connect)
		{
			//没连接退出
			return -2;
		}
		host_ip.is_cmd_sending = TRUE;
		ret = send(host_ip.socket_cpu, data, lengh, 0);
		//printf("u3 send over \n");
		if (ret == SOCKET_ERROR)
		{
			host_ip.is_cmd_sending = FALSE;
			return -1;
		}
		/*while (host_ip.is_cmd_sending)
		{
			Sleep(5);
		}*/
		break;
	default:
		break;
	}

	return 0;
}

void Host_Socket()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	CString str;
	int ret = 0;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(atoi(socketinfo.host_port));
	inet_pton(AF_INET, socketinfo.host_ip, &serAddr.sin_addr);

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		str =  "HOST INVALID FAILD ";
		SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		return;
	}
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //连接失败 
		str = "HOST CONNECT FAILD ";
		SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		closesocket(sclient);
		return;
	}
	host_ip.socket_cpu = sclient;

	char senddata[256] = { 0 };
	memset(senddata, 0, sizeof(senddata));
	int lengh = Str2ToHex(HELLO_HOST, senddata);

	ret = send(sclient, senddata, lengh, 0);
	if (ret == SOCKET_ERROR)
	{
		host_ip.is_connect = FALSE;
		str = "HOST SEND FAILD ";
		SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		return;
	}
	else
	{
		host_ip.is_connect = TRUE;
	}
	char recData[255];
	str = "host receiver start ";
	SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
	globalstate.host_connect = true;
	while (true)
	{
		
		ret = recv(sclient, recData, 255, 0);
		if (ret > 0)
		{
			//recData[ret] = 0x00;

			str = "host: " + Hex_2_str(recData, ret) + " ";
			host_ip.is_cmd_sending = FALSE;
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			if (Hex_2_str(recData, ret) ==  RECEIVE_DISCONNECT)
			{
				str = "host  disconnect activity  ";
				SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
				break;
			}
			if (Hex_2_str(recData, ret) == RECEIVE_STATE)
			{
				globalstate.start_push = true;
			}
			//printf("host str : %s\n", str);

		}
		else if (ret < 0)
		{
			str = "host: receiver  -1  close  ";
			SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
			host_ip.is_connect = FALSE;
			break;
		}
	}

	closesocket(sclient);
	globalstate.host_connect = false;
}

DWORD WINAPI SOCKET_HOST_MAIN(LPVOID  lppargram)
{
	//printf("ready to start thread ! \n");
	Host_Socket();
	return 0;
}

void Manage_Socket()
{
	WSACleanup();
	state_record = "";
	theApp.GetMainWnd()->GetDlgItem(IDC_DATA_RECEIVE)->SetWindowText(state_record);
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsdata;
	SOCKET serverSocket;
	CString  str;

	if (WSAStartup(sockVersion, &wsdata) != 0)
	{
		//待处理
		//printf("WSAStartup failed \n");
		str = "WSAStartup failed ";
		SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		return;
	}

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		//待处理
		str = "ServerSocket  Establish failed ";
		SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		return;
	}
	//绑定套接字
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(666);
	sockAddr.sin_addr.S_un.S_addr = inet_addr("192.168.201.250");   //inet_addr("127.0.0.1")  INADDR_ANY  inet_addr("192.168.1.0");

	if ( ::bind(serverSocket, (sockaddr*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
	{
		//待处理
		str = "ServerSocket  Bind  failed ";
		SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		return;
	}

	//开始监听
	if (listen(serverSocket, 10) == SOCKET_ERROR)
	{
		//待处理
		str = "ServerSocket  Listen failed ";
		SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		return;
	}

	SOCKET clientSocket;
	sockaddr_in client_sin;
	int len = sizeof(client_sin);
	int socketnum = 0;
	int num_acceprt = 0;

	//printf("ready to accept\n");
	globalstate.socket_connect = true;
	while (true)
	{
		clientSocket = accept(serverSocket, (sockaddr*)&client_sin, &len);
		if (clientSocket == INVALID_SOCKET)
		{
			//待处理
			//printf("accepte failed \n");
			return;
		}
		socketnum++;
		if (strcmp(inet_ntoa(client_sin.sin_addr), (char *)socketinfo.u1_ip.GetBuffer()) == 0)
		{ 
			u1_cpu.socket_cpu = *(&clientSocket);
			CreateThread(NULL, 0, CPU1_RECEIVER, (LPVOID)(&u1_cpu.socket_cpu), 0, NULL);
			u1_cpu.is_connect = TRUE;
			num_acceprt++;
		}
		else if (strcmp(inet_ntoa(client_sin.sin_addr), (char *)socketinfo.u2_ip.GetBuffer()) == 0)
		{
			u2_cpu.socket_cpu = *(&clientSocket);
			CreateThread(NULL, 0, CPU2_RECEIVER, (LPVOID)(&u2_cpu.socket_cpu), 0, NULL);
			u2_cpu.is_connect = TRUE;
			num_acceprt++;
		}
		else if (strcmp(inet_ntoa(client_sin.sin_addr), (char *)socketinfo.u3_ip.GetBuffer()) == 0)
		{
	
			u3_cpu.socket_cpu = *(&clientSocket);
			CreateThread(NULL, 0, CPU3_RECEIVER, (LPVOID)(&u3_cpu.socket_cpu), 0, NULL);
		
			u3_cpu.is_connect = TRUE;
			num_acceprt++;
		}

		if (socketnum >=3)
		{
			break;
		}
	}

	//closesocket(serverSocket);

	if (socketnum != num_acceprt)
	{
		str =  "Connect failed ";
		SendMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_MyMessage, 0, LPARAM(&str));
		return;
	}
	CreateThread(NULL, 0, SOCKET_HOST_MAIN, NULL, 0, NULL);
	/*else
	{
		state_record = state_record + " connect  over ";
		theApp.GetMainWnd()->GetDlgItem(IDC_DATA_RECEIVE)->SetWindowText(state_record);
	}*/

}


void CNewAutoPatDlg::OnBnClickedIpSure()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	socketinfo.u1_ip = m_CPU1_IP;
	socketinfo.u2_ip = m_CPU2__IP;
	socketinfo.u3_ip = m_CPU3_IP;
	socketinfo.host_ip = m_HomeIp;
	socketinfo.host_port = m_host_port;
}

DWORD WINAPI MAIN__THREAD(LPVOID  lppargram)
{
	//printf("ready to start thread ! \n");
	Manage_Socket();
	return 0;
}


void send_message()
{
	Sleep(400);
	int number;
	CString strmsg;
	if (-4 <= templedata.adjust_x && templedata.adjust_x <= 4)
	{

	}
	else  if (templedata.adjust_x <-4)
	{
		number = int(abs(templedata.adjust_x)*savedata.u1_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);
	}
	else if (templedata.adjust_x > 4)
	{
		number = int(templedata.adjust_x*savedata.u1_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);
	
	}

	if (templedata.adjust_y > 4)
	{
		number = int(templedata.adjust_y*savedata.u3_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
	else if (templedata.adjust_y<-4)
	{
		number = int(abs(templedata.adjust_y)*savedata.u3_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
}

void synchro_just()
{
	Sleep(300);
	int number;
	int bottom_num;
	CString strmsg;
	if (-4 <= templedata.adjust_x && templedata.adjust_x <= 4)
	{

	}
	else  if (templedata.adjust_x <-4)
	{
		number = int(abs(templedata.adjust_x)*savedata.u1_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);
		
		bottom_num = int(savedata.u1_u2_scale *number);
		strmsg.Format("02%04x%04x", bottom_num, bottom_num);
		SocketSendCmd(strmsg, 2);
		
		templedata.tem_len += number;
		templedata.temu1_save_len -= number;
		templedata.temu2_save_len += bottom_num;
	}
	else if (templedata.adjust_x > 4)
	{
		number = int(templedata.adjust_x*savedata.u1_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);
		
		bottom_num = int(savedata.u1_u2_scale *number);
		strmsg.Format("03%04x%04x", bottom_num, bottom_num);
		SocketSendCmd(strmsg, 2);
		
		templedata.tem_len   -=   number;
		templedata.temu1_save_len += number;
		templedata.temu2_save_len -= bottom_num;
	}

	if (templedata.adjust_y > 4)
	{
		number = int(templedata.adjust_y*savedata.u3_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
	else if (templedata.adjust_y<-4)
	{
		number = int(abs(templedata.adjust_y)*savedata.u3_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
}


bool sync_judge_adjust()
{
	Sleep(300);
	int number, bottom_num ;
	CString strmsg;

	if (abs(templedata.adjust_x) >= 70 || abs(templedata.adjust_y)>=70)
	{
		Sleep(300);
		if (abs(templedata.adjust_x) >= 70 || abs(templedata.adjust_y) >= 50)
		{
			//AfxMessageBox(_T("error circle judge"));
			return false;
		}
	}

	if (-4 <= templedata.adjust_x && templedata.adjust_x <= 4)
	{

	}
	else  if (templedata.adjust_x <-4)
	{
		number = int(abs(templedata.adjust_x)*savedata.u1_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);
		
		bottom_num = int(savedata.u1_u2_scale *number);
		strmsg.Format("02%04x%04x", bottom_num, bottom_num);
		SocketSendCmd(strmsg, 2);
		
		templedata.tem_len  += number;
		templedata.temu1_save_len -= number;
		templedata.temu2_save_len += bottom_num;
	}
	else if (templedata.adjust_x > 4)
	{
		number = int(templedata.adjust_x*savedata.u1_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);
		bottom_num = int(savedata.u1_u2_scale *number);
		strmsg.Format("03%04x%04x", bottom_num, bottom_num);
		SocketSendCmd(strmsg, 2);

		templedata.tem_len -=  number;
		templedata.temu1_save_len += number;
		templedata.temu2_save_len -= bottom_num;
	}

	if (templedata.adjust_y > 4)
	{
		number = int(templedata.adjust_y*savedata.u3_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
	else if (templedata.adjust_y<-4)
	{
		number = int(abs(templedata.adjust_y)*savedata.u3_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
	return true;
}


bool just_circle_stable()
{
	Point temp_center[3];
	Sleep(400);
	temp_center[0].x = templedata.tem_center.x;
	temp_center[0].y = templedata.tem_center.y;
	if (abs(templedata.adjust_x) > 120 || abs(templedata.adjust_y) > 110 )
	{
		return false;
	}
	Sleep(300);
	temp_center[1].x = templedata.tem_center.x;
	temp_center[1].y = templedata.tem_center.y;
	if (abs(temp_center[0].x - temp_center[1].x) >20 ||
		abs(temp_center[0].y - temp_center[1].y) >20)
	{
		return false;
	}
	Sleep(300);
	temp_center[2].x = templedata.tem_center.x;
	temp_center[2].y = templedata.tem_center.y;
	if (abs(temp_center[0].x - temp_center[2].x) >20 ||
		abs(temp_center[0].y - temp_center[2].y) >20)
	{
		return false;
	}

	return true;
}


int for_cols6( int cols=0)
{
	CString strlen;
	for (int ix = cols; ix < 6; ix++)
	{
		//savedata.save_cols6 = ix;
		templedata.tem_cols = ix;
		if (!sync_judge_adjust())
		{
			templedata.error_number++;
			if (templedata.error_number >= 4)
			{		
				SocketSendCmd(REPORT_STATE_CIRCLE, 4);
				AfxMessageBox(_T("连续检测错误超过4个，请换脚垫重新开始"));
				templedata.circle_judge_exceed = true;
				globalstate.record_valid = false;
				return -1;
			}
			if (ix == 5)
			{
				strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len += savedata.space_cols_pulse;
				strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
				SocketSendCmd(strlen, 3);
				strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				templedata.temu1_save_len -= savedata.space_cols_pulse;
				templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
			}
			else // 不是边缘
			{
				strlen.Format("02%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len -= savedata.cols_mat_pulse;
				strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
				templedata.temu1_save_len += savedata.cols_mat_pulse;
				templedata.temu2_save_len -= int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
			}

			continue;
		}
		//对准之后
		templedata.error_number = 0;
		strlen.Format("02%04x%04x", savedata.vacuumpads_camera_pulse, savedata.vacuumpads_camera_pulse);
		SocketSendCmd(strlen, 1);
		templedata.tem_len -= savedata.vacuumpads_camera_pulse;
		SocketSendCmd(U1_OPENSTEAM, 1);
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(500);
		SocketSendCmd(U1_MATUP, 1);
		Sleep(300);
		SocketSendCmd(U1_MATDOWN, 1);
		Sleep(100);
		SocketSendCmd(U1_UPSUCTION, 1);
		//判断是否成功吸气
		SocketSendCmd(U1_PRESSURE, 1);
		if (globalstate.socket_air_sucess)
		{
			globalstate.socket_air_sucess = false;
		}
		else
		{
			SocketSendCmd(U1_DOWNSUCTION, 1);
			Sleep(400);
			SocketSendCmd(U1_MATUP, 1);
			Sleep(400);
			SocketSendCmd(U1_MATDOWN, 1);
			Sleep(100);
			SocketSendCmd(U1_UPSUCTION, 1);
			SocketSendCmd(U1_PRESSURE, 1);
			if (!globalstate.socket_air_sucess)
			{
				SocketSendCmd(U1_CLOSESTEAM, 1);
				SocketSendCmd(REPORT_STATE_ERROR, 4);
				AfxMessageBox(_T("吸取错误，请清理凹槽"));
				
				if (ix == 5)
				{
					strlen.Format("03%04x%04x", savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse, savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
					SocketSendCmd(strlen, 3);
					strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					templedata.temu1_save_len -= savedata.space_cols_pulse;
					templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);

				}
				else // 不是边缘
				{
					strlen.Format("03%04x%04x", savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse, savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len - savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
					templedata.temu1_save_len += savedata.cols_mat_pulse;
					templedata.temu2_save_len -= int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
				}

				continue;
			}
			else
			{
				globalstate.socket_air_sucess = false;
			}
		}

		/*while (b_test_stop)
		{
		Sleep(100);
		}
		b_test_stop = TRUE;*/
		strlen.Format("02%04x%04x", templedata.tem_len, templedata.tem_len);
		SocketSendCmd(strlen, 1);
		while (b_test_stop)
		{
		Sleep(100);
		}
		//b_test_stop = TRUE;
		
		SocketSendCmd(REPORT_STATE_READY,4);
		while (!globalstate.start_push)
		{
			Sleep(20);
		}
		globalstate.start_push = false;

		if (globalstate.force_quit)
		{
			return -1;
		}
		SocketSendCmd(U1_DOWNSUCTION,1);
		Sleep(200);
		SocketSendCmd(U1_CLOSESTEAM, 1);
		Sleep(500);
		SocketSendCmd(U1_UPSUCTION, 1);
		Sleep(100);

		SocketSendCmd(REPORT_STATE_OVER, 4);

		//边缘考虑
		if (ix == 5)
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.space_cols_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(strlen, 3);
			strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			templedata.temu1_save_len -= savedata.space_cols_pulse;
			templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);

		}
		else // 不是边缘
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
			templedata.temu1_save_len += savedata.cols_mat_pulse;
			templedata.temu2_save_len -= int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
		}
	}
	templedata.tem_cols = -2;
	return 0;
}


int for_cols5(int cols = 0)
{
	CString strlen;
	for (int iy = cols; iy < 5; iy++)
	{
		//savedata.save_cols5 = iy;
		templedata.tem_rowa = iy;
		if (!sync_judge_adjust())
		{
			templedata.error_number++;
			if (templedata.error_number >= 4)
			{
				SocketSendCmd(REPORT_STATE_CIRCLE, 4);
				AfxMessageBox(_T("连续检测错误4个，请重新换张脚垫开始"));
				templedata.circle_judge_exceed = true;
				globalstate.record_valid = false;
				return -1;
			}
			if (iy == 4)
			{
				strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse;
				strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
				SocketSendCmd(strlen, 3);
				strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				templedata.temu1_save_len -= savedata.space_cols_pulse;
				templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
			}
			else
			{
				strlen.Format("03%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len += savedata.cols_mat_pulse;
				strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
				templedata.temu1_save_len -= savedata.cols_mat_pulse;
				templedata.temu2_save_len += int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
			}
			continue;
		}
		//对准之后
		templedata.error_number = 0;
		strlen.Format("02%04x%04x", savedata.vacuumpads_camera_pulse, savedata.vacuumpads_camera_pulse);
		SocketSendCmd(strlen, 1);
		templedata.tem_len -= savedata.vacuumpads_camera_pulse;
		SocketSendCmd(U1_OPENSTEAM, 1);
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(500);
		SocketSendCmd(U1_MATUP, 1);
		Sleep(300);
		SocketSendCmd(U1_MATDOWN, 1);
		Sleep(100);
		SocketSendCmd(U1_UPSUCTION, 1);

		SocketSendCmd(U1_PRESSURE, 1);
		if (globalstate.socket_air_sucess)
		{
			globalstate.socket_air_sucess = false;
		}
		else
		{
			SocketSendCmd(U1_DOWNSUCTION, 1);
			Sleep(400);
			SocketSendCmd(U1_MATUP, 1);
			Sleep(400);
			SocketSendCmd(U1_MATDOWN, 1);
			Sleep(100);
			SocketSendCmd(U1_UPSUCTION, 1);
			SocketSendCmd(U1_PRESSURE, 1);
			if (!globalstate.socket_air_sucess)
			{
				SocketSendCmd(U1_CLOSESTEAM, 1);
				SocketSendCmd(REPORT_STATE_ERROR, 4);
				AfxMessageBox(_T("吸取错误，请清理凹槽"));
				if (iy == 4)
				{
					strlen.Format("03%04x%04x", savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse, savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
					SocketSendCmd(strlen, 3);
					strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					templedata.temu1_save_len -= savedata.space_cols_pulse;
					templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
				}
				else
				{
					strlen.Format("03%04x%04x", savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse, savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
					templedata.temu1_save_len -= savedata.cols_mat_pulse;
					templedata.temu2_save_len += int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
				}

				continue;
			}
			else
			{
				globalstate.socket_air_sucess = false;
			}
		}
		/*while (b_test_stop)
		{
		Sleep(100);
		}
		b_test_stop = TRUE;*/
		strlen.Format("02%04x%04x", templedata.tem_len, templedata.tem_len);
		SocketSendCmd(strlen, 1);
		while (b_test_stop)
		{
		Sleep(100);
		}
		//b_test_stop = TRUE;*/
		SocketSendCmd(REPORT_STATE_READY, 4);
		while (!globalstate.start_push)
		{
			Sleep(20);
		}
		globalstate.start_push = false;
		if (globalstate.force_quit)
		{
			return -1;
		}
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(200);
		SocketSendCmd(U1_CLOSESTEAM, 1);
		Sleep(500);
		SocketSendCmd(U1_UPSUCTION, 1);
		Sleep(100);
		SocketSendCmd(REPORT_STATE_OVER, 4);

		//返回判断边缘
		if (iy == 4)
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.space_cols_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(strlen, 3);
			strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			templedata.temu1_save_len -= savedata.space_cols_pulse;
			templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
		}
		else
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.cols_mat_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
			templedata.temu1_save_len -= savedata.cols_mat_pulse;
			templedata.temu2_save_len += int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
		}
	}
	templedata.tem_rowa = -2;
	return 0;
}


int  bottom_one_test(int valid_cols, int start_cols =0)
{
	int ret = 0;
	templedata.error_number = 0;
	templedata.circle_judge_exceed = false;
	for (int i = start_cols;  i<valid_cols;  i++)
	{
		
		globalstate.record_valid = true;
		templedata.userd_cols = i;
		//savedata.save_surplus_cols = i;
		 ret = for_cols6();
		 if (ret != 0)
		 {
			 return -1;
		 }
		templedata.tem_cols = -2;

		ret = for_cols5();
		if (ret != 0)
		{
			return -1;
		}
		templedata.tem_rowa = -2;
	}
	templedata.userd_cols = -2;
	globalstate.record_valid = false;
	//printf("循环结束");
	return 0;
}


void bottom_sychro_test()
{  
	int ret = 0;
	CString str_pluse;
	globalstate.runing_state = true;
	while (true)
	{
		SocketSendCmd(REPORT_STATE_SWITCH,4);
		SocketSendCmd(U1_UPSUCTION, 1);
		SocketSendCmd(U1_OPENLIGHT, 1);
		SocketSendCmd(U1_MATDOWN, 1);
		SocketSendCmd(U1_RESET, 1);
		SocketSendCmd(U2_RESET, 2);
		str_pluse.Format("02%04x%04x", savedata.first_mat_pulse, savedata.first_mat_pulse);
		SocketSendCmd(str_pluse, 1);
		SocketSendCmd(U3_UPPRESS, 3); //抬起

		templedata.tem_len = 0;
		templedata.temu1_save_len = 0;
		templedata.temu2_save_len = 0;
		templedata.error_number = 0;
		templedata.circle_judge_exceed = false;
		templedata.userd_cols = 0;
		templedata.tem_cols = -2;
		templedata.tem_rowa = -2;

		globalstate.material_send = false;
		while (!globalstate.material_send)
		{
			Sleep(50);
		}
		globalstate.material_send = false; // 进料确定
		SocketSendCmd(U3_DOWNPRESS, 3);
		Sleep(200);
		SocketSendCmd(U1_RESET, 1);
		str_pluse.Format("02%04x%04x", savedata.camera_start_pulse, savedata.camera_start_pulse);
		SocketSendCmd(str_pluse, 1);
		if (templedata.postion_mat == 0)
		{
			templedata.tem_len = savedata.first_mat_pulse - savedata.camera_start_pulse;
		}
		else
		{
			templedata.tem_len = savedata.second_mat_pulse - savedata.camera_start_pulse;
		}
		
		templedata.temu1_save_len += savedata.camera_start_pulse;
		templedata.dark_quantity = 0.0; //清除残留标志

		Sleep(200);
		while (templedata.dark_quantity<30)
		{
			SocketSendCmd("0202000200", 3);
			Sleep(400);
		}

		str_pluse.Format("02%04x%04x", savedata.bottom_jacking_pulse, savedata.bottom_jacking_pulse);
		SocketSendCmd(str_pluse, 2); //下顶到达摄像头下方
		templedata.temu2_save_len += savedata.bottom_jacking_pulse;

		while (TRUE)
		{
			SocketSendCmd("0200b000b0", 3);
			if (just_circle_stable())
			{
				break;
			}
		} //判断圆心是否稳定
		synchro_just(); //微调
		while (TRUE)
		{
			if (just_circle_stable())
			{
				break;
			}
			SocketSendCmd("0200a000a0", 3);
		} //判断圆心是否稳定两次
		synchro_just();
		str_pluse.Format("02%04x%04x", savedata.row_mat_pulse, savedata.row_mat_pulse);
		SocketSendCmd(str_pluse, 3); //跳到下一个开始  row_mat_pulse
	    //sync_judge_adjust();

		ret = bottom_one_test(savedata.valid_mat_cols);
		if (globalstate.force_quit)
		{
			globalstate.force_quit = false;
			break;
		}
	}
	if (!globalstate.record_valid)
	{
		savedata.b_save_data = 0;
	}
	else
	{
		savedata.b_save_data = 1;
		savedata.save_mat_pulse = templedata.temu1_save_len;
		savedata.save_jacking_pulse = templedata.temu2_save_len;
		savedata.save_surplus_cols = templedata.userd_cols;
		savedata.save_cols6 = templedata.tem_cols;
		savedata.save_cols5 = templedata.tem_rowa;
		savedata.postion_mat = templedata.postion_mat;
	}
	globalstate.runing_state = false;
	
}


bool sync_judge_adjust_debug()
{
	Sleep(300);
	int number, bottom_num;
	CString strmsg;

	if (abs(templedata.adjust_x) >= 70 || abs(templedata.adjust_y) >= 70)
	{
		Sleep(300);
		if (abs(templedata.adjust_x) >= 70 || abs(templedata.adjust_y) >= 50)
		{
			AfxMessageBox(_T("error circle judge"));
			return false;
		}
	}

	if (-4 <= templedata.adjust_x && templedata.adjust_x <= 4)
	{

	}
	else  if (templedata.adjust_x <-4)
	{
		number = int(abs(templedata.adjust_x)*savedata.u1_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);

		bottom_num = int(savedata.u1_u2_scale *number);
		strmsg.Format("02%04x%04x", bottom_num, bottom_num);
		SocketSendCmd(strmsg, 2);

		templedata.tem_len += number;
	}
	else if (templedata.adjust_x > 4)
	{
		number = int(templedata.adjust_x*savedata.u1_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);
		bottom_num = int(savedata.u1_u2_scale *number);
		strmsg.Format("03%04x%04x", bottom_num, bottom_num);
		SocketSendCmd(strmsg, 2);

		templedata.tem_len -= number;
	}

	if (templedata.adjust_y > 4)
	{
		number = int(templedata.adjust_y*savedata.u3_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
	else if (templedata.adjust_y<-4)
	{
		number = int(abs(templedata.adjust_y)*savedata.u3_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
	return true;
}


int for_cols6_debug(int cols = 0)
{
	CString strlen;
	for (int ix = cols; ix < 6; ix++)
	{
		//savedata.save_cols6 = ix;
		if (!sync_judge_adjust_debug())
		{
			templedata.error_number++;
			if (templedata.error_number >= 4)
			{
				AfxMessageBox(_T("连续检测错误超过4个，请换脚垫重新开始"));
				return -1;
			}
			if (ix == 5)
			{
				strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len += savedata.space_cols_pulse;
				strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
				SocketSendCmd(strlen, 3);
				strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			}
			else // 不是边缘
			{
				strlen.Format("02%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len -= savedata.cols_mat_pulse;
				strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
			}

			continue;
		}
		//对准之后
		templedata.error_number = 0;
		strlen.Format("02%04x%04x", savedata.vacuumpads_camera_pulse, savedata.vacuumpads_camera_pulse);
		SocketSendCmd(strlen, 1);
		templedata.tem_len -= savedata.vacuumpads_camera_pulse;
		SocketSendCmd(U1_OPENSTEAM, 1);
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(500);
		SocketSendCmd(U1_MATUP, 1);
		Sleep(300);
		SocketSendCmd(U1_MATDOWN, 1);
		Sleep(100);
		SocketSendCmd(U1_UPSUCTION, 1);
		//判断是否成功吸气
		SocketSendCmd(U1_PRESSURE, 1);
		if (globalstate.socket_air_sucess)
		{
			globalstate.socket_air_sucess = false;
		}
		else
		{
			SocketSendCmd(U1_DOWNSUCTION, 1);
			Sleep(400);
			SocketSendCmd(U1_MATUP, 1);
			Sleep(400);
			SocketSendCmd(U1_MATDOWN, 1);
			Sleep(100);
			SocketSendCmd(U1_UPSUCTION, 1);
			SocketSendCmd(U1_PRESSURE, 1);
			if (!globalstate.socket_air_sucess)
			{
				SocketSendCmd(U1_CLOSESTEAM, 1);
				AfxMessageBox(_T("吸取错误，请清理凹槽"));
				if (ix == 5)
				{
					strlen.Format("03%04x%04x", savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse, savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
					SocketSendCmd(strlen, 3);
					strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				

				}
				else // 不是边缘
				{
					strlen.Format("03%04x%04x", savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse, savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len - savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));

				}

				continue;
			}
			else
			{
				globalstate.socket_air_sucess = false;
			}
		}

		/*while (b_test_stop)
		{
		Sleep(100);
		}
		b_test_stop = TRUE;*/
		strlen.Format("02%04x%04x", templedata.tem_len, templedata.tem_len);
		SocketSendCmd(strlen, 1);
		while (b_test_stop)
		{
			Sleep(100);
			if (globalstate.force_quit)
			{  
				globalstate.force_quit = false;
				return -1;
			}
		}
		//b_test_stop = TRUE;
	

		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(300);
		SocketSendCmd(U1_CLOSESTEAM, 1);
		Sleep(500);
		SocketSendCmd(U1_UPSUCTION, 1);
		Sleep(100);

		//边缘考虑
		if (ix == 5)
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.space_cols_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(strlen, 3);
			strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));

		}
		else // 不是边缘
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
		}
	}
	return 0;
}

int for_cols5_debug(int cols = 0)
{
	CString strlen;
	for (int iy = cols; iy < 5; iy++)
	{
		//savedata.save_cols5 = iy;
		if (!sync_judge_adjust_debug())
		{
			templedata.error_number++;
			if (templedata.error_number >= 4)
			{
				AfxMessageBox(_T("连续检测错误4个，请重新换张脚垫开始"));
				templedata.circle_judge_exceed = true;
				return -1;
			}
			if (iy == 4)
			{
				strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse;
				strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
				SocketSendCmd(strlen, 3);
				strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			}
			else
			{
				strlen.Format("03%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len += savedata.cols_mat_pulse;
				strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
			}
			continue;
		}
		//对准之后
		templedata.error_number = 0;
		strlen.Format("02%04x%04x", savedata.vacuumpads_camera_pulse, savedata.vacuumpads_camera_pulse);
		SocketSendCmd(strlen, 1);
		templedata.tem_len -= savedata.vacuumpads_camera_pulse;
		SocketSendCmd(U1_OPENSTEAM, 1);
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(500);
		SocketSendCmd(U1_MATUP, 1);
		Sleep(300);
		SocketSendCmd(U1_MATDOWN, 1);
		Sleep(100);
		SocketSendCmd(U1_UPSUCTION, 1);

		SocketSendCmd(U1_PRESSURE, 1);
		if (globalstate.socket_air_sucess)
		{
			globalstate.socket_air_sucess = false;
		}
		else
		{
			SocketSendCmd(U1_DOWNSUCTION, 1);
			Sleep(400);
			SocketSendCmd(U1_MATUP, 1);
			Sleep(400);
			SocketSendCmd(U1_MATDOWN, 1);
			Sleep(100);
			SocketSendCmd(U1_UPSUCTION, 1);
			SocketSendCmd(U1_PRESSURE, 1);
			if (!globalstate.socket_air_sucess)
			{
				SocketSendCmd(U1_CLOSESTEAM, 1);
				AfxMessageBox(_T("吸取错误，请清理凹槽"));
				if (iy == 4)
				{
					strlen.Format("03%04x%04x", savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse, savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
					SocketSendCmd(strlen, 3);
					strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));

				}
				else
				{
					strlen.Format("03%04x%04x", savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse, savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
				}

				continue;
			}
			else
			{
				globalstate.socket_air_sucess = false;
			}
		}
		/*while (b_test_stop)
		{
		Sleep(100);
		}
		b_test_stop = TRUE;*/
		strlen.Format("02%04x%04x", templedata.tem_len, templedata.tem_len);
		SocketSendCmd(strlen, 1);
		while (b_test_stop)
		{
			Sleep(100);
			if (globalstate.force_quit)
			{
				globalstate.force_quit = false;
				return -1;
			}
		}
		//b_test_stop = TRUE;*/
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(300);
		SocketSendCmd(U1_CLOSESTEAM, 1);
		Sleep(500);
		SocketSendCmd(U1_UPSUCTION, 1);
		Sleep(100);
		
		//返回判断边缘
		if (iy == 4)
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.space_cols_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(strlen, 3);
			strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
		}
		else
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.cols_mat_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
		}
	}
	return 0;
}


int  bottom_one_test_dubug(int valid_cols, int start_cols = 0)
{
	int ret = 0;
	templedata.error_number = 0;
	templedata.circle_judge_exceed = false;
	for (int i = start_cols; i<valid_cols; i++)
	{	
		//savedata.save_surplus_cols = i;
		ret = for_cols6_debug();
		if (ret != 0)
		{
			return -1;
		}

		ret = for_cols5_debug();
		if (ret != 0)
		{
			return -1;
		}
	}
	//printf("循环结束");
	return 0;
}


void synchro_just_debug()
{
	Sleep(300);
	int number;
	int bottom_num;
	CString strmsg;
	if (-4 <= templedata.adjust_x && templedata.adjust_x <= 4)
	{

	}
	else  if (templedata.adjust_x <-4)
	{
		number = int(abs(templedata.adjust_x)*savedata.u1_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);

		bottom_num = int(savedata.u1_u2_scale *number);
		strmsg.Format("02%04x%04x", bottom_num, bottom_num);
		SocketSendCmd(strmsg, 2);

		templedata.tem_len += number;
	}
	else if (templedata.adjust_x > 4)
	{
		number = int(templedata.adjust_x*savedata.u1_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 1);

		bottom_num = int(savedata.u1_u2_scale *number);
		strmsg.Format("03%04x%04x", bottom_num, bottom_num);
		SocketSendCmd(strmsg, 2);

		templedata.tem_len -= number;
	}

	if (templedata.adjust_y > 4)
	{
		number = int(templedata.adjust_y*savedata.u3_pulse_pixel);
		strmsg.Format("02%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
	else if (templedata.adjust_y<-4)
	{
		number = int(abs(templedata.adjust_y)*savedata.u3_pulse_pixel);
		strmsg.Format("03%04x%04x", number, number);
		SocketSendCmd(strmsg, 3);
	}
}


void bottom_camera_together()
{
	int ret = 0;
	CString str_pluse;
	globalstate.runing_state = true;
	SocketSendCmd(U1_UPSUCTION, 1);
	SocketSendCmd(U1_OPENLIGHT, 1);
	SocketSendCmd(U1_MATDOWN, 1);
	SocketSendCmd(U1_RESET, 1);
	SocketSendCmd(U2_RESET, 2);
	str_pluse.Format("02%04x%04x", savedata.first_mat_pulse, savedata.first_mat_pulse);
	SocketSendCmd(str_pluse, 1);
	SocketSendCmd(U3_UPPRESS, 3); //抬起

	globalstate.material_send = false;
	while (!globalstate.material_send)
	{
		Sleep(50);
	}
	globalstate.material_send = false; // 进料确定
	SocketSendCmd(U3_DOWNPRESS, 3);
	Sleep(200);
	SocketSendCmd(U1_RESET, 1);
	str_pluse.Format("02%04x%04x", savedata.camera_start_pulse, savedata.camera_start_pulse);
	SocketSendCmd(str_pluse, 1);
	
	templedata.tem_len = savedata.first_mat_pulse - savedata.camera_start_pulse;
	
	templedata.dark_quantity = 0.0; //清除残留标志

	Sleep(200);
	while (templedata.dark_quantity<30)
	{
		SocketSendCmd("0202000200", 3);
		Sleep(400);
	}

	str_pluse.Format("02%04x%04x", savedata.bottom_jacking_pulse, savedata.bottom_jacking_pulse);
	SocketSendCmd(str_pluse, 2); //下顶到达摄像头下方

	while (TRUE)
	{
		SocketSendCmd("0200b000b0", 3);
		if (just_circle_stable())
		{
			break;
		}
	} //判断圆心是否稳定
	synchro_just_debug(); //微调
	while (TRUE)
	{
		if (just_circle_stable())
		{
			break;
		}
		SocketSendCmd("0200a000a0", 3);
	} //判断圆心是否稳定两次
	synchro_just_debug();
	str_pluse.Format("02%04x%04x", savedata.row_mat_pulse, savedata.row_mat_pulse);
	SocketSendCmd(str_pluse, 3); //跳到下一个开始  row_mat_pulse
									//sync_judge_adjust();
	
	ret = bottom_one_test_dubug(savedata.valid_mat_cols);
	templedata.tem_len = 0;
	globalstate.runing_state = false; 
}

DWORD WINAPI JUST_START(LPVOID  lppargram)
{
	//just_first_jiaodian();
	//synchro_just_material();
	bottom_sychro_test();
	return 0;
}


int judge_force_mat()
{
	CString strlen;
	int ret = 0;
	templedata.error_number = 0;
	templedata.circle_judge_exceed = false;
	if (savedata.save_cols6 != 5 && savedata.save_cols6 != -2)
	{
		globalstate.record_valid = true;
		ret = for_cols6(savedata.save_cols6 + 1);
		if (ret != 0)
		{
			return -1;
		}

		ret = for_cols5();
		if (ret != 0)
		{
			return -1;
		}

		if ( templedata.error_number != 0)
		{
			templedata.error_number = 0;
			globalstate.record_valid = false;
			return  -1;
		}

	}
	else if( savedata.save_cols5 != 4)
	{  
		int y_start = 0;
		if (savedata.save_cols6 == 5 && savedata.save_cols5 == -2)
		{
			y_start = 0;
		}
		else
		{
			y_start = savedata.save_cols5 + 1;		
		}
		globalstate.record_valid = true;
		ret = for_cols5(y_start);
		if (ret != 0)
		{
			return -1;
		}

		if (templedata.error_number != 0)
		{
			templedata.error_number = 0;
			globalstate.record_valid = false;
			return  -1;
		}
	}
	return 0;
}


int record_data_start()
{
	//开始到达上一个位置
	CString str_pluse;
	int ret = 0;
	globalstate.runing_state = true;
	SocketSendCmd(REPORT_STATE_START, 4);
	SocketSendCmd(U3_DOWNPRESS, 3);
	SocketSendCmd(U1_UPSUCTION, 1);
	SocketSendCmd(U1_OPENLIGHT, 1);
	SocketSendCmd(U1_MATDOWN, 1);
	SocketSendCmd(U1_RESET, 1);
	SocketSendCmd(U2_RESET, 2);
	str_pluse.Format("02%04x%04x", savedata.save_mat_pulse, savedata.save_mat_pulse);
	SocketSendCmd(str_pluse, 1);
	templedata.tem_len = 0;
	if (templedata.postion_mat == 0)
	{
		templedata.tem_len = savedata.first_mat_pulse - savedata.save_mat_pulse;
	}
	else
	{
		templedata.tem_len = savedata.second_mat_pulse - savedata.save_mat_pulse;
	}

	templedata.temu1_save_len = savedata.save_mat_pulse;
	
	str_pluse.Format("02%04x%04x", savedata.save_jacking_pulse, savedata.save_jacking_pulse);
	SocketSendCmd(str_pluse, 2);
	templedata.temu2_save_len = savedata.save_jacking_pulse;

	//移到下一个
	if (savedata.save_cols6 == -2 )
	{
		if (savedata.save_cols5 == 4)
		{
			str_pluse.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
			SocketSendCmd(str_pluse, 1);
			templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse;
			str_pluse.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(str_pluse, 3);
			str_pluse.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(str_pluse, 2);
			templedata.temu1_save_len -= savedata.space_cols_pulse;
			templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
		}
		else
		{
			str_pluse.Format("03%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
			SocketSendCmd(str_pluse, 1);
			templedata.tem_len += savedata.cols_mat_pulse;
			str_pluse.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(str_pluse, 2);
			templedata.temu1_save_len -= savedata.cols_mat_pulse;
			templedata.temu2_save_len += int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
		}
	}
	else
	{
		if (savedata.save_cols6 == 5)
		{
			str_pluse.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
			SocketSendCmd(str_pluse, 1);
			templedata.tem_len += savedata.space_cols_pulse;
			str_pluse.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(str_pluse, 3);
			str_pluse.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(str_pluse, 2);
			templedata.temu1_save_len -= savedata.space_cols_pulse;
			templedata.temu2_save_len += int(savedata.space_cols_pulse*savedata.u1_u2_scale);
		}
		else
		{
			str_pluse.Format("02%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
			SocketSendCmd(str_pluse, 1);
			templedata.tem_len -= savedata.cols_mat_pulse;
			str_pluse.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(str_pluse, 2);
			templedata.temu1_save_len += savedata.cols_mat_pulse;
			templedata.temu2_save_len -= int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
		}
	}
	Sleep(200);
	if (!sync_judge_adjust)
	{
		SocketSendCmd(REPORT_STATE_CIRCLE, 4);
		AfxMessageBox(_T(" 数据校验不对，重新装填 "));
		CreateThread(NULL, 0, JUST_START, NULL, 0, NULL);
		return -1;
	}
	else
	{
		//AfxMessageBox(_T(" 数据校验正确，现在开始 "));
		globalstate.record_valid = true;
	    ret = judge_force_mat();
		if (globalstate.force_quit)
		{
			globalstate.force_quit = false;
			if (!globalstate.record_valid)
			{
				savedata.b_save_data = 0;
			}
			else
			{
				savedata.b_save_data = 1;
				savedata.save_mat_pulse = templedata.temu1_save_len;
				savedata.save_jacking_pulse = templedata.temu2_save_len;
				savedata.save_surplus_cols = templedata.userd_cols;
				savedata.save_cols6 = templedata.tem_cols;
				savedata.save_cols5 = templedata.tem_rowa;
				savedata.postion_mat = templedata.postion_mat;
			}
			return -1;
		}
		if (ret !=0 )
		{
			SocketSendCmd(REPORT_STATE_CIRCLE, 4);
			AfxMessageBox(_T(" 数据校验不对，重新装填开始 "));
			CreateThread(NULL, 0, JUST_START, NULL, 0, NULL);
			return -1;
		}
		else
		{
			//执行剩下的行数
			ret = bottom_one_test( savedata.valid_mat_cols,  savedata.save_surplus_cols +1);
			if (globalstate.force_quit)
			{
				globalstate.force_quit = false;
				if (!globalstate.record_valid)
				{
					savedata.b_save_data = 0;
				}
				else
				{
					savedata.b_save_data = 1;
					savedata.save_mat_pulse = templedata.temu1_save_len;
					savedata.save_jacking_pulse = templedata.temu2_save_len;
					savedata.save_surplus_cols = templedata.userd_cols;
					savedata.save_cols6 = templedata.tem_cols;
					savedata.save_cols5 = templedata.tem_rowa;
					savedata.postion_mat = templedata.postion_mat;
				}
				return -1;
			}
			if (ret != 0)
			{
				SocketSendCmd(REPORT_STATE_CIRCLE, 4);
				AfxMessageBox(_T(" 执行剩下的脚垫错误，将要重新开始 "));
				CreateThread(NULL, 0, JUST_START, NULL, 0, NULL);
				return -1;
			}
			//执行完再开始循环检测
			globalstate.record_valid = false;
			CreateThread(NULL, 0, JUST_START, NULL, 0, NULL);
			return 0;
		}
	}
	return 0;
}


DWORD WINAPI RECORD_DATA_START(LPVOID  lppargram)
{
	//just_first_jiaodian();
	//synchro_just_material();
	record_data_start();
	return 0;
}

int for_two_cols6(int cols = 0)
{
	CString strlen;
	for (int ix = cols; ix < 6; ix++)
	{
		//savedata.save_cols6 = ix;
		templedata.tem_cols = ix;
		if (!sync_judge_adjust())
		{
			templedata.error_number++;
			if (templedata.error_number >= 4)
			{
				SocketSendCmd(REPORT_STATE_CIRCLE, 4);
				AfxMessageBox(_T("连续检测错误超过4个，请换脚垫重新开始"));
				templedata.circle_judge_exceed = true;
				globalstate.record_valid = false;
				return -1;
			}
			if (ix == 5)
			{
				strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len += savedata.space_cols_pulse;
				strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
				SocketSendCmd(strlen, 3);
				strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				templedata.temu1_save_len -= savedata.space_cols_pulse;
				templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
			}
			else // 不是边缘
			{
				strlen.Format("02%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len -= savedata.cols_mat_pulse;
				strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
				templedata.temu1_save_len += savedata.cols_mat_pulse;
				templedata.temu2_save_len -= int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
			}

			continue;
		}
		//对准之后
		templedata.error_number = 0;
		strlen.Format("02%04x%04x", savedata.vacuumpads_camera_pulse, savedata.vacuumpads_camera_pulse);
		SocketSendCmd(strlen, 1);
		templedata.tem_len -= savedata.vacuumpads_camera_pulse;
		SocketSendCmd(U1_OPENSTEAM, 1);
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(300);
		SocketSendCmd(U1_MATUP, 1);
		Sleep(500);
		SocketSendCmd(U1_MATDOWN, 1);
		Sleep(300);
		SocketSendCmd(U1_UPSUCTION, 1);
		//判断是否成功吸气
		SocketSendCmd(U1_PRESSURE, 1);
		if (globalstate.socket_air_sucess)
		{
			globalstate.socket_air_sucess = false;
		}
		else
		{
			SocketSendCmd(U1_DOWNSUCTION, 1);
			Sleep(300);
			SocketSendCmd(U1_MATUP, 1);
			Sleep(500);
			SocketSendCmd(U1_MATDOWN, 1);
			Sleep(300);
			SocketSendCmd(U1_UPSUCTION, 1);
			SocketSendCmd(U1_PRESSURE, 1);
			if (!globalstate.socket_air_sucess)
			{
				SocketSendCmd(U1_CLOSESTEAM, 1);
				SocketSendCmd(REPORT_STATE_ERROR, 4);
				AfxMessageBox(_T("吸取错误，请清理凹槽"));
				if (ix == 5)
				{
					strlen.Format("03%04x%04x", savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse, savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
					SocketSendCmd(strlen, 3);
					strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					templedata.temu1_save_len -= savedata.space_cols_pulse;
					templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);

				}
				else // 不是边缘
				{
					strlen.Format("03%04x%04x", savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse, savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len - savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
					templedata.temu1_save_len += savedata.cols_mat_pulse;
					templedata.temu2_save_len -= int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
				}

				continue;
			}
			else
			{
				globalstate.socket_air_sucess = false;
			}
		}

		/*while (b_test_stop)
		{
		Sleep(100);
		}
		b_test_stop = TRUE;*/

		if (globalstate.even_num)
		{
			templedata.tem_len += savedata.second_mat_pulse - savedata.first_mat_pulse;
			globalstate.even_num = false;
		}
		else
		{
			if (globalstate.reset_first)
			{
				templedata.tem_len = templedata.tem_len - savedata.second_mat_pulse + savedata.first_mat_pulse;
				globalstate.even_num = true;
			}
			else
			{
				globalstate.reset_first = true;
			}
			
		}

		strlen.Format("02%04x%04x", templedata.tem_len, templedata.tem_len);
		SocketSendCmd(strlen, 1);
		while (b_test_stop)
		{
			Sleep(100);
		}
		//b_test_stop = TRUE;
		SocketSendCmd(REPORT_STATE_READY, 4);
		while (!globalstate.start_push)
		{
			Sleep(20);
		}
		globalstate.start_push = false;
		if (globalstate.force_quit)
		{
			return -1;
		}
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(200);
		SocketSendCmd(U1_CLOSESTEAM, 1);
		Sleep(600);
		SocketSendCmd(U1_UPSUCTION, 1);
		Sleep(100);
	    
		SocketSendCmd(REPORT_STATE_OVER, 4);

		//边缘考虑
		if (ix == 5)
		{  
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.space_cols_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(strlen, 3);
			strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			templedata.temu1_save_len -= savedata.space_cols_pulse;
			templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);

		}
		else // 不是边缘
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse - savedata.cols_mat_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
			templedata.temu1_save_len += savedata.cols_mat_pulse;
			templedata.temu2_save_len -= int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
		}
	}
	templedata.tem_cols = -2;
	return 0;
}

int for_two_cols5(int cols = 0)
{
	CString strlen;
	for (int iy = cols; iy < 5; iy++)
	{
		//savedata.save_cols5 = iy;
		templedata.tem_rowa = iy;
		if (!sync_judge_adjust())
		{
			templedata.error_number++;
			if (templedata.error_number >= 4)
			{
				SocketSendCmd(REPORT_STATE_CIRCLE, 4);
				AfxMessageBox(_T("连续检测错误4个，请重新换张脚垫开始"));
				templedata.circle_judge_exceed = true;
				globalstate.record_valid = false;
				return -1;
			}
			if (iy == 4)
			{
				strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse;
				strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
				SocketSendCmd(strlen, 3);
				strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				templedata.temu1_save_len -= savedata.space_cols_pulse;
				templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
			}
			else
			{
				strlen.Format("03%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len += savedata.cols_mat_pulse;
				strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
				templedata.temu1_save_len -= savedata.cols_mat_pulse;
				templedata.temu2_save_len += int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
			}
			continue;
		}
		//对准之后
		templedata.error_number = 0;
		strlen.Format("02%04x%04x", savedata.vacuumpads_camera_pulse, savedata.vacuumpads_camera_pulse);
		SocketSendCmd(strlen, 1);
		templedata.tem_len -= savedata.vacuumpads_camera_pulse;
		SocketSendCmd(U1_OPENSTEAM, 1);
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(300);
		SocketSendCmd(U1_MATUP, 1);
		Sleep(500);
		SocketSendCmd(U1_MATDOWN, 1);
		Sleep(300);
		SocketSendCmd(U1_UPSUCTION, 1);

		SocketSendCmd(U1_PRESSURE, 1);
		if (globalstate.socket_air_sucess)
		{
			globalstate.socket_air_sucess = false;
		}
		else
		{
			SocketSendCmd(U1_DOWNSUCTION, 1);
			Sleep(300);
			SocketSendCmd(U1_MATUP, 1);
			Sleep(500);
			SocketSendCmd(U1_MATDOWN, 1);
			Sleep(300);
			SocketSendCmd(U1_UPSUCTION, 1);
			SocketSendCmd(U1_PRESSURE, 1);
			if (!globalstate.socket_air_sucess)
			{
				SocketSendCmd(U1_CLOSESTEAM, 1);
				SocketSendCmd(REPORT_STATE_ERROR, 4);
				AfxMessageBox(_T("吸取错误，请清理凹槽"));
				if (iy == 4)
				{
					strlen.Format("03%04x%04x", savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse, savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
					SocketSendCmd(strlen, 3);
					strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
					templedata.temu1_save_len -= savedata.space_cols_pulse;
					templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
				}
				else
				{
					strlen.Format("03%04x%04x", savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse, savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse);
					SocketSendCmd(strlen, 1);
					templedata.tem_len = templedata.tem_len + savedata.cols_mat_pulse + savedata.vacuumpads_camera_pulse;
					strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
					SocketSendCmd(strlen, 2);
					//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
					templedata.temu1_save_len -= savedata.cols_mat_pulse;
					templedata.temu2_save_len += int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
				}

				continue;
			}
			else
			{
				globalstate.socket_air_sucess = false;
			}
		}
		/*while (b_test_stop)
		{
		Sleep(100);
		}
		b_test_stop = TRUE;*/
		if (globalstate.even_num)
		{
			templedata.tem_len += savedata.second_mat_pulse - savedata.first_mat_pulse;
			globalstate.even_num = false;
		}
		else
		{
			if (globalstate.reset_first)
			{
				templedata.tem_len = templedata.tem_len - savedata.second_mat_pulse + savedata.first_mat_pulse;
				globalstate.even_num = true;
			}
			else
			{
				globalstate.reset_first = true;
			}
		}
		strlen.Format("02%04x%04x", templedata.tem_len, templedata.tem_len);
		SocketSendCmd(strlen, 1);
		while (b_test_stop)
		{
			Sleep(100);
		}
		//b_test_stop = TRUE;*/
		SocketSendCmd(REPORT_STATE_READY, 4);
		while (!globalstate.start_push)
		{
			Sleep(20);
		}
		globalstate.start_push = false;
		if (globalstate.force_quit)
		{
			return -1;
		}	
		SocketSendCmd(U1_DOWNSUCTION, 1);
		Sleep(200);
		SocketSendCmd(U1_CLOSESTEAM, 1);
		Sleep(600);
		SocketSendCmd(U1_UPSUCTION, 1);
		Sleep(100);
		SocketSendCmd(REPORT_STATE_OVER, 4);

		//返回判断边缘
		if (iy == 4)
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.space_cols_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(strlen, 3);
			strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			templedata.temu1_save_len -= savedata.space_cols_pulse;
			templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
		}
		else
		{
			templedata.tem_len = templedata.tem_len + savedata.vacuumpads_camera_pulse + savedata.cols_mat_pulse;
			strlen.Format("03%04x%04x", templedata.tem_len, templedata.tem_len);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
			templedata.temu1_save_len -= savedata.cols_mat_pulse;
			templedata.temu2_save_len += int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
		}
	}
	templedata.tem_rowa = -2;
	return 0;
}


int  bottom_two_test(int valid_cols, int start_cols = 0)
{
	int ret = 0;
	templedata.error_number = 0;
	templedata.circle_judge_exceed = false;
	for (int i = start_cols; i<valid_cols; i++)
	{

		globalstate.record_valid = true;
		templedata.userd_cols = i;
		//savedata.save_surplus_cols = i;
		ret = for_two_cols6();
		if (ret != 0)
		{
			return -1;
		}
		templedata.tem_cols = -2;

		ret = for_two_cols5();
		if (ret != 0)
		{
			return -1;
		}
		templedata.tem_rowa = -2;
	}
	templedata.userd_cols = -2;
	globalstate.record_valid = false;
	//printf("循环结束");
	return 0;
}


void two_mat()
{
	int ret = 0;
	CString str_pluse;
	globalstate.runing_state = true;
	while (true)
	{
		SocketSendCmd(REPORT_STATE_SWITCH, 4);
		SocketSendCmd(U1_UPSUCTION, 1);
		SocketSendCmd(U1_OPENLIGHT, 1);
		SocketSendCmd(U1_MATDOWN, 1);
		SocketSendCmd(U1_RESET, 1);
		SocketSendCmd(U2_RESET, 2);
		str_pluse.Format("02%04x%04x", savedata.first_mat_pulse, savedata.first_mat_pulse);
		SocketSendCmd(str_pluse, 1);
		SocketSendCmd(U3_UPPRESS, 3); //抬起

		templedata.tem_len = 0;
		templedata.temu1_save_len = 0;
		templedata.temu2_save_len = 0;
		templedata.error_number = 0;
		templedata.circle_judge_exceed = false;
		templedata.userd_cols = 0;

		globalstate.material_send = false;
		while (!globalstate.material_send)
		{
			Sleep(50);
		}
		globalstate.material_send = false; // 进料确定
		SocketSendCmd(U3_DOWNPRESS, 3);
		Sleep(200);
		SocketSendCmd(U1_RESET, 1);
		str_pluse.Format("02%04x%04x", savedata.camera_start_pulse, savedata.camera_start_pulse);
		SocketSendCmd(str_pluse, 1);
		
		templedata.tem_len = savedata.first_mat_pulse - savedata.camera_start_pulse;
		
		templedata.temu1_save_len += savedata.camera_start_pulse;
		templedata.dark_quantity = 0.0; //清除残留标志

		Sleep(200);
		while (templedata.dark_quantity<30)
		{
			SocketSendCmd("0202000200", 3);
			Sleep(400);
		}

		str_pluse.Format("02%04x%04x", savedata.bottom_jacking_pulse, savedata.bottom_jacking_pulse);
		SocketSendCmd(str_pluse, 2); //下顶到达摄像头下方
		templedata.temu2_save_len += savedata.bottom_jacking_pulse;

		while (TRUE)
		{
			SocketSendCmd("0200b000b0", 3);
			if (just_circle_stable())
			{
				break;
			}
		} //判断圆心是否稳定
		synchro_just(); //微调
		while (TRUE)
		{
			if (just_circle_stable())
			{
				break;
			}
			SocketSendCmd("0200a000a0", 3);
		} //判断圆心是否稳定两次
		synchro_just();
		str_pluse.Format("02%04x%04x", savedata.row_mat_pulse, savedata.row_mat_pulse);
		SocketSendCmd(str_pluse, 3); //跳到下一个开始  row_mat_pulse
									 //sync_judge_adjust();

		ret = bottom_two_test(savedata.valid_mat_cols);
		if (globalstate.force_quit)
		{
			globalstate.force_quit = false;
			break;
		}
		
	
	}
	if (!globalstate.record_valid)
	{
		savedata.b_save_data = 0;
	}
	else
	{
		savedata.b_save_data = 1;
		savedata.save_mat_pulse = templedata.temu1_save_len;
		savedata.save_jacking_pulse = templedata.temu2_save_len;
		savedata.save_surplus_cols = templedata.userd_cols;
		savedata.save_cols6 = templedata.tem_cols;
		savedata.save_cols5 = templedata.tem_rowa;
		savedata.postion_mat = templedata.postion_mat;
	}


	globalstate.runing_state = false;
}

DWORD WINAPI JUST_START_TWO(LPVOID  lppargram)
{
	//just_first_jiaodian();
	//just_first_jiaodian();
	two_mat();
	return 0;
}


int judge_twoforce_mat()
{
	CString strlen;
	int ret = 0;
	templedata.error_number = 0;
	templedata.circle_judge_exceed = false;
	if (savedata.save_cols6 != 5 && savedata.save_cols6 != -2)
	{
		globalstate.record_valid = true;
		ret = for_two_cols6(savedata.save_cols6 + 1);
		if (ret != 0)
		{
			return -1;
		}

		ret = for_two_cols5();
		if (ret != 0)
		{
			return -1;
		}

		if (templedata.error_number != 0)
		{
			templedata.error_number = 0;
			globalstate.record_valid = false;
			return  -1;
		}

	}
	else if (savedata.save_cols5 != 4)
	{
		int y_start = 0;
		if (savedata.save_cols6 == 5 && savedata.save_cols5 == -2)
		{
			y_start = 0;
		}
		else
		{
			y_start = savedata.save_cols5 + 1;
		}
		globalstate.record_valid = true;
		ret = for_two_cols5(y_start);
		if (ret != 0)
		{
			return -1;
		}

		if (templedata.error_number != 0)
		{
			templedata.error_number = 0;
			globalstate.record_valid = false;
			return  -1;
		}
	}
	return 0;
}


int record_twodata_start()
{
	//开始到达上一个位置
	CString str_pluse;
	int ret = 0;
	globalstate.runing_state = true;
	SocketSendCmd(REPORT_STATE_START, 4);
	SocketSendCmd(U3_DOWNPRESS, 3);
	SocketSendCmd(U1_UPSUCTION, 1);
	SocketSendCmd(U1_OPENLIGHT, 1);
	SocketSendCmd(U1_MATDOWN, 1);
	SocketSendCmd(U1_RESET, 1);
	SocketSendCmd(U2_RESET, 2);
	str_pluse.Format("02%04x%04x", savedata.save_mat_pulse, savedata.save_mat_pulse);
	SocketSendCmd(str_pluse, 1);
	templedata.tem_len = 0;
	
	templedata.tem_len = savedata.first_mat_pulse - savedata.save_mat_pulse;
	templedata.temu1_save_len = savedata.save_mat_pulse;

	str_pluse.Format("02%04x%04x", savedata.save_jacking_pulse, savedata.save_jacking_pulse);
	SocketSendCmd(str_pluse, 2);
	templedata.temu2_save_len = savedata.save_jacking_pulse;

	//移到下一个
	if (savedata.save_cols6 == -2)
	{
		if (savedata.save_cols5 == 4)
		{
			str_pluse.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
			SocketSendCmd(str_pluse, 1);
			templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse;
			str_pluse.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(str_pluse, 3);
			str_pluse.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(str_pluse, 2);
			templedata.temu1_save_len -= savedata.space_cols_pulse;
			templedata.temu2_save_len += int(savedata.u1_u2_scale*savedata.space_cols_pulse);
		}
		else
		{
			str_pluse.Format("03%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
			SocketSendCmd(str_pluse, 1);
			templedata.tem_len += savedata.cols_mat_pulse;
			str_pluse.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(str_pluse, 2);
			templedata.temu1_save_len -= savedata.cols_mat_pulse;
			templedata.temu2_save_len += int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
		}
	}
	else
	{
		if (savedata.save_cols6 == 5)
		{
			str_pluse.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
			SocketSendCmd(str_pluse, 1);
			templedata.tem_len += savedata.space_cols_pulse;
			str_pluse.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(str_pluse, 3);
			str_pluse.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(str_pluse, 2);
			templedata.temu1_save_len -= savedata.space_cols_pulse;
			templedata.temu2_save_len += int(savedata.space_cols_pulse*savedata.u1_u2_scale);
		}
		else
		{
			str_pluse.Format("02%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
			SocketSendCmd(str_pluse, 1);
			templedata.tem_len -= savedata.cols_mat_pulse;
			str_pluse.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(str_pluse, 2);
			templedata.temu1_save_len += savedata.cols_mat_pulse;
			templedata.temu2_save_len -= int(savedata.cols_mat_pulse*savedata.u1_u2_scale);
		}
	}
	Sleep(200);
	if (!sync_judge_adjust)
	{
		SocketSendCmd(REPORT_STATE_CIRCLE, 4);
		AfxMessageBox(_T(" 数据校验不对，重新装填 "));
		CreateThread(NULL, 0, JUST_START_TWO, NULL, 0, NULL);
		return -1;
	}
	else
	{
		//AfxMessageBox(_T(" 数据校验正确，现在开始 "));
		globalstate.record_valid = true;
		ret = judge_twoforce_mat();
		if (globalstate.force_quit)
		{
			globalstate.force_quit = false;
			if (!globalstate.record_valid)
			{
				savedata.b_save_data = 0;
			}
			else
			{
				savedata.b_save_data = 1;
				savedata.save_mat_pulse = templedata.temu1_save_len;
				savedata.save_jacking_pulse = templedata.temu2_save_len;
				savedata.save_surplus_cols = templedata.userd_cols;
				savedata.save_cols6 = templedata.tem_cols;
				savedata.save_cols5 = templedata.tem_rowa;
				savedata.postion_mat = templedata.postion_mat;
			}
			return -1;
		}
		if (ret != 0)
		{
			SocketSendCmd(REPORT_STATE_CIRCLE, 4);
			AfxMessageBox(_T(" 数据校验不对，重新装填开始 "));
			CreateThread(NULL, 0, JUST_START_TWO, NULL, 0, NULL);
			return -1;
		}
		else
		{
			//执行剩下的行数
			ret = bottom_two_test(savedata.valid_mat_cols, savedata.save_surplus_cols + 1);
			if (globalstate.force_quit)
			{
				globalstate.force_quit = false;
				if (!globalstate.record_valid)
				{
					savedata.b_save_data = 0;
				}
				else
				{
					savedata.b_save_data = 1;
					savedata.save_mat_pulse = templedata.temu1_save_len;
					savedata.save_jacking_pulse = templedata.temu2_save_len;
					savedata.save_surplus_cols = templedata.userd_cols;
					savedata.save_cols6 = templedata.tem_cols;
					savedata.save_cols5 = templedata.tem_rowa;
					savedata.postion_mat = templedata.postion_mat;
				}
				return - 1;
			}
			if (ret != 0)
			{
				SocketSendCmd(REPORT_STATE_CIRCLE, 4);
				AfxMessageBox(_T(" 执行处理脚垫发生错误，将重新开始 "));
				CreateThread(NULL, 0, JUST_START_TWO, NULL, 0, NULL);
				return -1;
			}
			//执行完再开始循环检测
			globalstate.record_valid = false;
			CreateThread(NULL, 0, JUST_START_TWO, NULL, 0, NULL);
			return 0;
		}
	}
	return 0;
}

DWORD WINAPI JUST_START_TWO_RECORD(LPVOID  lppargram)
{
	//just_first_jiaodian();
	//just_first_jiaodian();
	record_twodata_start();
	return 0;
}

DWORD WINAPI JUST_START_ONLY(LPVOID  lppargram)
{
	//just_first_jiaodian();
	//just_first_jiaodian();
	bottom_camera_together();
	return 0;
}

DWORD WINAPI JUST_START_OPEN(LPVOID  lppargram)
{
	//just_first_jiaodian();
	return 0;
}

void CNewAutoPatDlg::OnBnClickedTestOne()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!globalstate.socket_connect || !globalstate.open_device_single  || !globalstate.host_connect)
	{
		AfxMessageBox(_T(" 确保网络连接及摄像头开启 "));
		return;
	}
	if (!globalstate.runing_state )
	{
		if (savedata.b_save_data == 1 && templedata.postion_mat==savedata.postion_mat && templedata.postion_mat != 2)
		{
			if (::MessageBox(NULL, "是否从上次开始？", "提示", MB_YESNO) == IDNO)
			{
				CreateThread(NULL, 0, JUST_START, NULL, 0, NULL);
			}
			else
			{
				CreateThread(NULL, 0, RECORD_DATA_START, NULL, 0, NULL);
			}
		}
	   else if(savedata.b_save_data == 1 && templedata.postion_mat == savedata.postion_mat && templedata.postion_mat ==2)
		{
			//这个是贴两个脚垫的程序
			//CreateThread(NULL, 0, JUST_START, NULL, 0, NULL);  JUST_START_TWO_RECORD
			if (::MessageBox(NULL, "是否从上次开始？", "提示", MB_YESNO) == IDNO)
			{
				CreateThread(NULL, 0, JUST_START_TWO, NULL, 0, NULL);
			}
			else
			{
				CreateThread(NULL, 0, JUST_START_TWO_RECORD, NULL, 0, NULL);
			}
		}
	   else if(templedata.postion_mat == 2)
	   {
		   //这个是贴两个脚垫的程序  JUST_START_TWO
		   CreateThread(NULL, 0, JUST_START_TWO, NULL, 0, NULL);
	   }
	   else
	   {
		   CreateThread(NULL, 0, JUST_START, NULL, 0, NULL);
	   }
	}
	
	/*thread t(just_first_jiaodian);
	t.detach();*/
}

void ShowErrorMsg(CString csMessage, int nErrorNum)
{
	CString errorMsg;
	if (nErrorNum == 0)
	{
		errorMsg.Format(_T("%s"), csMessage);
	}
	else
	{
		errorMsg.Format(_T("%s: Error = %x: "), csMessage, nErrorNum);
	}

	switch (nErrorNum)
	{
	case MV_E_HANDLE:           errorMsg += "Error or invalid handle ";                                         break;
	case MV_E_SUPPORT:          errorMsg += "Not supported function ";                                          break;
	case MV_E_BUFOVER:          errorMsg += "Cache is full ";                                                   break;
	case MV_E_CALLORDER:        errorMsg += "Function calling order error ";                                    break;
	case MV_E_PARAMETER:        errorMsg += "Incorrect parameter ";                                             break;
	case MV_E_RESOURCE:         errorMsg += "Applying resource failed ";                                        break;
	case MV_E_NODATA:           errorMsg += "No data ";                                                         break;
	case MV_E_PRECONDITION:     errorMsg += "Precondition error, or running environment changed ";              break;
	case MV_E_VERSION:          errorMsg += "Version mismatches ";                                              break;
	case MV_E_NOENOUGH_BUF:     errorMsg += "Insufficient memory ";                                             break;
	case MV_E_ABNORMAL_IMAGE:   errorMsg += "Abnormal image, maybe incomplete image because of lost packet ";   break;
	case MV_E_UNKNOW:           errorMsg += "Unknown error ";                                                   break;
	case MV_E_GC_GENERIC:       errorMsg += "General error ";                                                   break;
	case MV_E_GC_ACCESS:        errorMsg += "Node accessing condition error ";                                  break;
	case MV_E_ACCESS_DENIED:	errorMsg += "No permission ";                                                   break;
	case MV_E_BUSY:             errorMsg += "Device is busy, or network disconnected ";                         break;
	case MV_E_NETER:            errorMsg += "Network error ";                                                   break;
	}

	//MessageBox(errorMsg, TEXT("PROMPT"), MB_OK | MB_ICONWARNING);
	AfxMessageBox(errorMsg);
}

void CNewAutoPatDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CString cstr;
	auto_slider_bj = m_sliderBJ.GetPos();
	cstr.Format("%d", auto_slider_bj);
	GetDlgItem(IDC_BJ_SHOW)->SetWindowText(cstr);


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

LRESULT CNewAutoPatDlg::OnMyMessage(WPARAM wParam, LPARAM lParam)
{

	CString str = *(CString *)lParam;

	if (m_RECEIVER_DATA.GetLineCount()>20)
	{
		m_RECEDATA_STR.Empty();
		UpdateData(FALSE);
	}


	int nLen = m_RECEIVER_DATA.GetWindowTextLength();
	m_RECEIVER_DATA.SetSel(nLen, nLen);
	m_RECEIVER_DATA.ReplaceSel(str);
	//nLen += str.GetLength();

	m_RECEDATA_STR += str;
	return 0;
}

void CNewAutoPatDlg::OnBnClickedDirectUp()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strmsg;
	strmsg.Format("03%04x%04x", auto_slider_bj, auto_slider_bj);
	SocketSendCmd(strmsg, 3);
}

void CNewAutoPatDlg::OnBnClickedDirectDown()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strmsg;
	strmsg.Format("02%04x%04x", auto_slider_bj, auto_slider_bj);
	SocketSendCmd(strmsg, 3);
}

void CNewAutoPatDlg::OnBnClickedDirecLeft()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strmsg;
	strmsg.Format("03%04x%04x", auto_slider_bj, auto_slider_bj);
	SocketSendCmd(strmsg, 1);
}

void CNewAutoPatDlg::OnBnClickedDirecRight()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strmsg;
	strmsg.Format("02%04x%04x", auto_slider_bj, auto_slider_bj);
	SocketSendCmd(strmsg, 1);
}

void CNewAutoPatDlg::OnBnClickedPadsUp()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strmsg = U1_UPSUCTION;
	SocketSendCmd(strmsg, 1);
}

void CNewAutoPatDlg::OnBnClickedPadsDown()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strmsg = U1_DOWNSUCTION;
	SocketSendCmd(strmsg, 1);
}

void CNewAutoPatDlg::OnBnClickedAirOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	
	SocketSendCmd(U1_OPENSTEAM, 1);
}

void CNewAutoPatDlg::OnBnClickedAirClose()
{
	// TODO: 在此添加控件通知处理程序代码
	SocketSendCmd(U1_CLOSESTEAM, 1);
}

void CNewAutoPatDlg::OnBnClickedMatUp()
{
	// TODO: 在此添加控件通知处理程序代码
	SocketSendCmd(U1_MATUP, 1);
}

void CNewAutoPatDlg::OnBnClickedMatDown()
{
	// TODO: 在此添加控件通知处理程序代码
	SocketSendCmd(U1_MATDOWN, 1);
}

void CNewAutoPatDlg::OnBnClickedBtDataSend()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	SocketSendCmd(m_Data_Send_Str, 1);
}

void CNewAutoPatDlg::OnBnClickedBtDataSend2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	SocketSendCmd(m_Data_Send_Str, 2);
}

void CNewAutoPatDlg::OnBnClickedBtDataSend3()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	SocketSendCmd(m_Data_Send_Str, 3);
}

void CNewAutoPatDlg::OnBnClickedCodeTest()
{
	// TODO: 在此添加控件通知处理程序代码
	//UpdateData(true);
	//CString sendstr = m_Data_Send_Str;
	//SendMessage(WM_MyMessage, 0, LPARAM(&sendstr));
	if (b_test_stop)
	{
		b_test_stop = FALSE;
	}
	else
	{
		b_test_stop = TRUE;
	}
}


void CNewAutoPatDlg::OnBnClickedDataClean()
{
	// TODO: 在此添加控件通知处理程序代码
	m_RECEDATA_STR.Empty();
	UpdateData(FALSE);
}

DWORD WINAPI YA_ZHOUUP(LPVOID  lppargram)
{
	SocketSendCmd(U1_RESET, 1);
	SocketSendCmd(U1_UPSUCTION, 1);
	SocketSendCmd("0230003000", 1);
	SocketSendCmd(U3_UPPRESS, 3);
	return 0;
}

void CNewAutoPatDlg::OnBnClickedU3Press()
{
	// TODO: 在此添加控件通知处理程序代码
	if (globalstate.socket_send_stop)
	{
		globalstate.socket_send_stop = false;
	}
	else
	{
		globalstate.socket_send_stop = true;
	}
}

void CNewAutoPatDlg::OnBnClickedResetU1()
{
	// TODO: 在此添加控件通知处理程序代码
	//SocketSendCmd(U1_RESET, 1);
	if (!globalstate.runing_state)
	{
		CreateThread(NULL, 0, JUST_START_ONLY, NULL, 0, NULL);
	}
	
}

void CNewAutoPatDlg::OnBnClickedBottomReset()
{
	// TODO: 在此添加控件通知处理程序代码
	if (globalstate.opencv_show)
	{
		globalstate.opencv_show = false;
	}
	else
	{
		globalstate.opencv_show = true;
	}
	
}

void CNewAutoPatDlg::OnBnClickedU3ProessDown()
{
	// TODO: 在此添加控件通知处理程序代码
	SocketSendCmd(U3_DOWNPRESS, 3);
	globalstate.material_send = true;
}

void CNewAutoPatDlg::OnBnClickedStopBt()
{
	// TODO: 在此添加控件通知处理程序代码
	//CreateThread(NULL, 0, JUST_START_OPEN, NULL, 0, NULL);
	send_message();
}

void CNewAutoPatDlg::OnBnClickedFindDeviceButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString  strMsg;
	m_ctrCombox.ResetContent();
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	int nRet = HikAPI::EnumDevices(&m_stDevList);
	if (MV_OK != nRet)
	{
		return;
	}

	unsigned int i;
	int nIp1, nIp2, nIp3, nIp4;
	for (i = 0; i < m_stDevList.nDeviceNum; i++)
	{
		MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
		if (NULL == pDeviceInfo)
		{
			continue;
		}
		if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
		{
			nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
			nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
			nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
			nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
			strMsg.Format(_T("[%d]GigE:  %s  %s  (%s) (%d.%d.%d.%d)"), i,
				pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName,
				pDeviceInfo->SpecialInfo.stGigEInfo.chModelName,
				pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber,
				nIp1, nIp2, nIp3, nIp4);
		}
		else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
		{
			strMsg.Format(_T("[%d]UsbV3:  %s %s(%s)"), i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName, 
				pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, 
				pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
		}
		else
		{
			ShowErrorMsg(TEXT("Unknown device enumerated"), 0);;
		}
		m_ctrCombox.AddString(strMsg);
	}

	if (0 == m_stDevList.nDeviceNum)
	{
		ShowErrorMsg(TEXT("No device"), 0);
		return;
	}
	m_ctrCombox.SetCurSel(0);

}


void InitControlBoard()
{
	namedWindow("【控制面板】", 0);
	createTrackbar("二值化阈值：", "【控制面板】", &savedata.opencv_thread, 256, ContrastAndBright);
	createTrackbar("开闭运算：", "【控制面板】", &element_ght, 5, ContrastAndBright);
}

void image_show()
{
	Mat binary, cannyimg, fincutimg;
	Mat src, srcimg;
	Mat element;
	element = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat temple_mat;
	make_image(savedata.opencv_template_circle,temple_mat);
	Point center;
	cv::Mat frame, out;
	int ret;
	int number;
	int length = 0;
	String str;
	if (globalstate.opencv_show)
	{
		InitControlBoard();
	}
	waitKey(100);
	int empty_times = 0;
	float cast = 0.1;
	while (globalstate.image_show_start)
	{
		ret = myhikclass->GetOneFrameTimeout(frame, 200);
		if (ret || frame.empty())
		{
			empty_times++;
			if (empty_times>3)
			{
				AfxMessageBox(_T("get mat failed what should i do "));
				break;
			}			
			continue;
		}
		empty_times = 0;	
		GetBrightness(frame, cast, templedata.dark_quantity);
		
		if (int(templedata.dark_quantity) > 83)
		{
			threshold(frame, srcimg, 228, 255, 1);
		}
		else
		{
			threshold(frame, srcimg, savedata.opencv_thread, 255, 1);
		}
		morphologyEx(srcimg, fincutimg, element_ght, element);
		center = MatchCircleTemplate(fincutimg, temple_mat, savedata.opencv_template_circle);
	
		templedata.tem_center.x = center.x;
		templedata.tem_center.y = center.y;
		circle(frame, Point(center.x, center.y), savedata.opencv_template_circle / 2, Scalar(0, 205, 0), 2, 8, 0);
		str = "(" + to_string(center.x) + ", " + to_string(center.y) +" "+to_string(int(templedata.dark_quantity)) +")";
		putText(frame, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0), 1);
	
		templedata.adjust_x = savedata.center_x - center.x;
		templedata.adjust_y = savedata.center_y - center.y;
		if (globalstate.opencv_show)
		{
			imshow("binary", srcimg);
			imshow("fincutimg", fincutimg);
		}
		cv::imshow("source", frame);

		number = cv::waitKey(10);
		if (number == 's' || number == 'S')
		{
			cv::imwrite("number.jpg", frame);
		}
		else if (number == 'q' || number == 'Q')
		{
			break;
		}
	}

	cv::destroyAllWindows();
	globalstate.image_shoe_stop = false;
}

DWORD WINAPI IMAGE_SHOWT(LPVOID  lppargram)
{
	image_show();
	return 0;
}

void CNewAutoPatDlg::OnBnClickedStartGrabbingButton()
{
	// TODO: 在此添加控件通知处理程序代码
	if (true == globalstate.open_device_single)
	{
		return;
	}
	UpdateData(TRUE);
	int index = m_numDeviceComb;
	if ((index < 0) | (index >= MV_MAX_DEVICE_NUM))
	{
		ShowErrorMsg(TEXT("Please select device"), 0);
		return;
	}

	//创建实例
	if (NULL == m_stDevList.pDeviceInfo[index])
	{
		ShowErrorMsg(TEXT("Device does not exist"), 0);
		return;
	}
	if (NULL != myhikclass)
	{
		return;
	}
	myhikclass = new HikAPI();
	if (NULL == myhikclass)
	{
		return;
	}

	int nRet = myhikclass->Open(m_stDevList.pDeviceInfo[index]);
	if (MV_OK != nRet)
	{
		delete myhikclass;
		myhikclass = NULL;
		return;
	}
	// ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
	if (m_stDevList.pDeviceInfo[index]->nTLayerType == MV_GIGE_DEVICE)
	{
		int nPacketSize = myhikclass->GetOptimalPacketSize();
		if (nPacketSize > 0)
		{
			nRet = myhikclass->SetIntValue("GevSCPSPacketSize", nPacketSize);
			if (nRet != MV_OK)
			{
				ShowErrorMsg(TEXT("Warning: Set Packet Size fail!"), nRet);
			}
		}
		else
		{
			ShowErrorMsg(TEXT("Warning: Get Packet Size fail!"), nPacketSize);
		}
	}
	
	myhikclass->ChangeCameraArea(800, 500, 960, 700);
	nRet = myhikclass->StartGrabbing();
	if (MV_OK != nRet)
	{
		return;
	}
	globalstate.image_show_start = true;
	globalstate .image_shoe_stop = true;
	CreateThread(NULL, 0, IMAGE_SHOWT, NULL, 0, NULL);
    //thread t(image_show);
	//t.detach();

	globalstate.open_device_single = true;

}

void CNewAutoPatDlg::OnBnClickedStopGrabbingButton()
{
	// TODO: 在此添加控件通知处理程序代码
	if (globalstate.open_device_single )
	{
		globalstate.image_show_start = false;
		while (1)
		{
			if (!globalstate.image_shoe_stop)
			{
				break;
			}
			Sleep(50);
		}
		int nRet = MV_OK;
		nRet = myhikclass->StopGrabbing();
		if (MV_OK != nRet)
		{
			return;
		}
		if (myhikclass)
		{
			myhikclass->Close();
			delete myhikclass;
			myhikclass = NULL;
		}
		globalstate.open_device_single = false;
	}
	
}

DWORD WINAPI THREAD_TEST(LPVOID  lppargram)
{
	printf("hello my name is wangxiuwen \n");
	return 0;
}

void CNewAutoPatDlg::OnBnClickedThreadButton()
{
	// TODO: 在此添加控件通知处理程序代码
	//CreateThread(NULL, 0, THREAD_TEST, NULL, 0, NULL);
	if (globalstate.runing_state)
	{
		globalstate.force_quit = true;
	}
}

void CNewAutoPatDlg::OnBnClickedSmallButton()
{
	// TODO: 在此添加控件通知处理程序代码
	//send_message();
	synchro_just();
}

void CNewAutoPatDlg::OnBnClickedSocketConnectButton()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!globalstate.socket_connect)
	{
		CreateThread(NULL, 0, MAIN__THREAD, NULL, 0, NULL); // SOCKET_HOST_MAIN
		//CreateThread(NULL, 0, SOCKET_HOST_MAIN, NULL, 0, NULL);
	}
}
	
void CNewAutoPatDlg::OnBnClickedSocketDisconnectButton()
{
	// TODO: 在此添加控件通知处理程序代码
	if (u1_cpu.socket_cpu != NULL)
	{
		closesocket(u1_cpu.socket_cpu);
		u1_cpu.socket_cpu = NULL;
		u1_cpu.is_connect = FALSE;
		closesocket(u2_cpu.socket_cpu);
		u2_cpu.socket_cpu = NULL;
		u2_cpu.is_connect = FALSE;
		closesocket(u3_cpu.socket_cpu);
		u3_cpu.socket_cpu = NULL;
		u3_cpu.is_connect = FALSE;
		globalstate.socket_connect = false;
		SocketSendCmd(REPORT_STATE_DISCONNECT, 4);
		closesocket(host_ip.socket_cpu);
	}
}


CString CNewAutoPatDlg::InitConfig()
{
	CFileFind config_file_finder;
	CString cs_return = NULL;
	BOOL ifFind = config_file_finder.FindFile(_T(CONFIG_FILE_PATH));
	if (!ifFind)
	{
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("图像中心点x坐标"), _T("496"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("图像中心点y坐标"), _T("339"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U1脉冲与像素系数"), _T("1.02"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U3脉冲与像素系数"), _T("2.78"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U1和U2的脉冲系数"),_T("10.108"),_T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("行之间的脚垫脉冲"), _T("654"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("列之间的脚垫脉冲"), _T("1006"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("间隔脚垫的行脉冲"), _T("318"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("间隔脚垫的列脉冲"), _T("466"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("摄像头起始位置脉冲"), _T("928"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("顶轴起始位置脉冲"), _T("45058"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("摄像头和吸头距离"), _T("1124"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("第一个脚垫的距离"), _T("14840"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("第二个脚垫的距离"), _T("18310"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("脚垫的有效行数"), _T("38"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("贴脚垫的位置"), _T("0"), _T(CONFIG_FILE_PATH));

		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("二值化的值"), _T("214"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("匹配模板的大小"), _T("318"), _T(CONFIG_FILE_PATH));

		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U1的IP"), _T("192.168.201.1"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U2的IP"), _T("192.168.201.2"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U3的IP"), _T("192.168.201.3"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("HOST的IP"), _T("192.168.1.118"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("HOST的端口"), _T("10010"), _T(CONFIG_FILE_PATH));

		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("是否有记录"), _T("0"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("起始脚垫的位置脉冲"), _T("0"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("起始顶轴的位置脉冲"), _T("0"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录剩余脚垫行数"), _T("0"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录是大行的第几个脚垫"), _T("0"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录是小行的第几个脚垫"), _T("0"), _T(CONFIG_FILE_PATH));

		cs_return = "没检测到配置文件，生成默认值 \r\n";
		return cs_return;

	}

	int itmp;
	double dtmp;
	CString cs_tmp;
	bool b_read_failed = false;
	CString get_filestr;
	 
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("图像中心点x坐标"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "图像中心点x坐标参数读取失败 \r\n";
		b_read_failed = true;
	}
	else {
		savedata.center_x = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("图像中心点y坐标"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "图像中心点y坐标参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.center_y = itmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U1脉冲与像素系数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if (  (dtmp- 0) < 1e-6)
	{
		cs_return += "U1脉冲与像素系数参数读取失败 \r\n";
		b_read_failed = true;
	}
	else
	{
		savedata.u1_pulse_pixel = dtmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U3脉冲与像素系数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if ((dtmp - 0) < 1e-6)
	{
		cs_return += "U3脉冲与像素系数参数读取失败 \r\n";
		b_read_failed = true;
	}
	else
	{
		savedata.u3_pulse_pixel = dtmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U1和U2的脉冲系数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if ((dtmp - 0) < 1e-6)
	{
		cs_return += "U1和U2的脉冲系数参数读取失败 \r\n";
		b_read_failed = true;
	}
	else
	{
		savedata.u1_u2_scale = dtmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("行之间的脚垫脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "行之间的脚垫脉冲参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.cols_mat_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("列之间的脚垫脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "列之间的脚垫脉冲参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.row_mat_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("间隔脚垫的行脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "间隔脚垫的行脉冲参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.space_cols_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("间隔脚垫的列脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "间隔脚垫的列脉冲参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.space_rows_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("摄像头起始位置脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "摄像头起始位置脉冲参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.camera_start_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("顶轴起始位置脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "顶轴起始位置脉冲参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.bottom_jacking_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("摄像头和吸头距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "摄像头和吸头距离参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.vacuumpads_camera_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("第一个脚垫的距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "第一个脚垫的距离参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.first_mat_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("第二个脚垫的距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "第二个脚垫的距离参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.second_mat_pulse = itmp;
	}


	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("脚垫的有效行数"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "脚垫的有效行数参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.valid_mat_cols = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("贴脚垫的位置"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "贴脚垫的位置参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.postion_mat = itmp;
	}


	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("二值化的值"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "二值化的值参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.opencv_thread = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("匹配模板的大小"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "匹配模板的大小参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.opencv_template_circle = itmp;
	}


	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U1的IP"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp == "error")
	{
		cs_return += "UI的IP参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		 socketinfo.u1_ip= cs_tmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U2的IP"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp == "error")
	{
		cs_return += "U2的IP参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		socketinfo.u2_ip = cs_tmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U3的IP"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp == "error")
	{
		cs_return += "U3的IP参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		socketinfo.u3_ip = cs_tmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("HOST的IP"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp == "error")
	{
		cs_return += "HOST的IP参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		socketinfo.host_ip = cs_tmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("HOST的端口"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp == "error")
	{
		cs_return += "HOST的端口参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		socketinfo.host_port = cs_tmp;
	}


	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("是否有记录"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "是否有记录参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.b_save_data = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("起始脚垫的位置脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "起始脚垫的位置脉冲参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.save_mat_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("起始顶轴的位置脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "起始顶轴的位置脉冲参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.save_jacking_pulse = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("记录剩余脚垫行数"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "记录剩余脚垫行数参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.save_surplus_cols = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("记录是大行的第几个脚垫"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "记录是大行的第几个脚垫参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.save_cols6 = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("记录是小行的第几个脚垫"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_return += "记录是小行的第几个脚垫参数读取失败\r\n";
		b_read_failed = true;
	}
	else {
		savedata.save_cols5 = itmp;
	}

	if (b_read_failed)
	{
		return  cs_return;
	}
	else
	{
		return "null";
	}
	return "null";

}


CString CNewAutoPatDlg::GetDiffConfig()
{
	bool is_config_change = false;
	CString retuen_cstr = NULL;
	CString str;

	CString cs_tmp;
	int itmp;
	double dtmp;

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("图像中心点x坐标"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.center_x)
	{
		str.Format("图像中心点x坐标从%d改成%d \n", itmp, savedata.center_x);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("图像中心点y坐标"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.center_y)
	{
		str.Format("图像中心点y坐标从%d改成%d \n", itmp, savedata.center_y);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U1脉冲与像素系数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if (dtmp != savedata.u1_pulse_pixel)
	{
		str.Format("U1脉冲与像素系数从%f改成%f \n", dtmp, savedata.u1_pulse_pixel);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U3脉冲与像素系数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if (savedata.u3_pulse_pixel != dtmp)
	{
		str.Format("U3脉冲与像素系数从%f改成%f \n", dtmp, savedata.u3_pulse_pixel);
		retuen_cstr += str;
		is_config_change = true;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U1和U2的脉冲系数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if (dtmp != savedata.u1_u2_scale)
	{	
		str.Format("U1和U2的脉冲系数从%f改成%f \n", dtmp, savedata.u1_u2_scale);
		retuen_cstr += str;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("行之间的脚垫脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.cols_mat_pulse)
	{
		str.Format("行之间的脚垫脉冲从%d改成%d \n", itmp, savedata.cols_mat_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("列之间的脚垫脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.row_mat_pulse)
	{
		str.Format("列之间的脚垫脉冲从%d改成%d \n", itmp, savedata.row_mat_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("间隔脚垫的行脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.space_cols_pulse)
	{
		str.Format("间隔脚垫的行脉冲从%d改成%d \n", itmp, savedata.space_cols_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("间隔脚垫的列脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.space_rows_pulse)
	{
		str.Format("间隔脚垫的列脉冲从%d改成%d \n", itmp, savedata.space_rows_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("摄像头起始位置脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.camera_start_pulse)
	{
		str.Format("摄像头起始位置脉冲从%d改成%d \n", itmp, savedata.camera_start_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("顶轴起始位置脉冲"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.bottom_jacking_pulse)
	{
		str.Format("顶轴起始位置脉冲从%d改成%d \n", itmp, savedata.bottom_jacking_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("摄像头和吸头距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.vacuumpads_camera_pulse)
	{
		str.Format("摄像头和吸头距离从%d改成%d \n", itmp, savedata.vacuumpads_camera_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("第一个脚垫的距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.first_mat_pulse)
	{
		str.Format("第一个脚垫的距离从%d改成%d \n", itmp, savedata.first_mat_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("第二个脚垫的距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.second_mat_pulse)
	{
		str.Format("第二个脚垫的距离从%d改成%d \n", itmp, savedata.second_mat_pulse);
		retuen_cstr += str;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("脚垫的有效行数"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.valid_mat_cols)
	{
		str.Format("脚垫的有效行数从%d改成%d \n", itmp, savedata.valid_mat_cols);
		retuen_cstr += str;
		is_config_change = true;
	}
	

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("贴脚垫的位置"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != templedata.postion_mat)
	{
		str.Format("贴脚垫的位置从%d改成%d \n", itmp, savedata.postion_mat);
		retuen_cstr += str;
		is_config_change = true;
	}


	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("二值化的值"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.opencv_thread)
	{
		str.Format("二值化的值从%d改成%d \n", itmp, savedata.opencv_thread);
		retuen_cstr += str;
		is_config_change = true;
	}
	
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("匹配模板的大小"), -1, _T(CONFIG_FILE_PATH));
	if (itmp != savedata.opencv_template_circle)
	{
		str.Format("匹配模板的大小从%d改成%d \n", itmp, savedata.opencv_template_circle);
		retuen_cstr += str;
		is_config_change = true;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U1的IP"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp != socketinfo.u1_ip)
	{
		str.Format("U1的IP从%s改成%s \n", cs_tmp, socketinfo.u1_ip);
		retuen_cstr += str;
		is_config_change = true;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U2的IP"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp != socketinfo.u2_ip)
	{
		str.Format("U2的IP从%s改成%s \n", cs_tmp, socketinfo.u2_ip);
		retuen_cstr += str;
		is_config_change = true;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("U3的IP"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp != socketinfo.u3_ip)
	{	
		str.Format("U3的IP从%s改成%s \n", cs_tmp, socketinfo.u3_ip);
		retuen_cstr += str;
		is_config_change = true;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("HOST的IP"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp != socketinfo.host_ip)
	{
		str.Format("HOST的IP从%s改成%s \n", cs_tmp, socketinfo.host_ip);
		retuen_cstr += str;
		is_config_change = true;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("HOST的端口"), _T("error"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	if (cs_tmp != socketinfo.host_port)
	{
		str.Format("HOST的端口从%s改成%s \n", cs_tmp, socketinfo.host_port);
		retuen_cstr += str;
		is_config_change = true;
	}

	
	if (is_config_change)
	{
		return  retuen_cstr;
	}
	else
	{
		return "null";
	}
	return "null";
}


void CNewAutoPatDlg::SaveConfig()
{
	CString cstr;
	cstr.Format("%d", savedata.center_x);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("图像中心点x坐标"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.center_y);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("图像中心点y坐标"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%f", savedata.u1_pulse_pixel);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U1脉冲与像素系数"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%f", savedata.u3_pulse_pixel);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U3脉冲与像素系数"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%f", savedata.u1_u2_scale);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U1和U2的脉冲系数"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.cols_mat_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("行之间的脚垫脉冲"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.row_mat_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("列之间的脚垫脉冲"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.space_cols_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("间隔脚垫的行脉冲"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.space_rows_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("间隔脚垫的列脉冲"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.camera_start_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("摄像头起始位置脉冲"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.bottom_jacking_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("顶轴起始位置脉冲"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.vacuumpads_camera_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("摄像头和吸头距离"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.first_mat_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("第一个脚垫的距离"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.second_mat_pulse);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("第二个脚垫的距离"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.valid_mat_cols);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("脚垫的有效行数"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", templedata.postion_mat);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("贴脚垫的位置"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.opencv_thread);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("二值化的值"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", savedata.opencv_template_circle);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("匹配模板的大小"), cstr, _T(CONFIG_FILE_PATH));

	cstr = m_CPU1_IP;
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U1的IP"), cstr, _T(CONFIG_FILE_PATH));

	cstr=m_CPU2__IP;
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U2的IP"), cstr, _T(CONFIG_FILE_PATH));

	cstr=m_CPU3_IP;
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("U3的IP"), cstr, _T(CONFIG_FILE_PATH));

	cstr=m_HomeIp;
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("HOST的IP"), cstr, _T(CONFIG_FILE_PATH));

	cstr=m_host_port;
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("HOST的端口"), cstr, _T(CONFIG_FILE_PATH));

	if (!globalstate.record_valid)
	{
		cstr.Format("%d", 0);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("是否有记录"), cstr, _T(CONFIG_FILE_PATH));
	}
	else
	{
		cstr.Format("%d", 1);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("是否有记录"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.temu1_save_len);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("起始脚垫的位置脉冲"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.temu2_save_len);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("起始顶轴的位置脉冲"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.userd_cols);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录剩余脚垫行数"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.tem_cols);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录是大行的第几个脚垫"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.tem_rowa);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录是小行的第几个脚垫"), cstr, _T(CONFIG_FILE_PATH));
	}

}


void CNewAutoPatDlg::SavePartConfig()
{
	CString cstr;
	if (!globalstate.record_valid)
	{
		cstr.Format("%d", 0);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("是否有记录"), cstr, _T(CONFIG_FILE_PATH));
	}
	else
	{
		cstr.Format("%d", 1);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("是否有记录"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.temu1_save_len);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("起始脚垫的位置脉冲"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.temu2_save_len);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("起始顶轴的位置脉冲"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.userd_cols);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录剩余脚垫行数"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.tem_cols);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录是大行的第几个脚垫"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", templedata.tem_rowa);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录是小行的第几个脚垫"), cstr, _T(CONFIG_FILE_PATH));
	}

}

void CNewAutoPatDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (u1_cpu.socket_cpu != NULL)     //关闭socket
	{
		closesocket(u1_cpu.socket_cpu);
		u1_cpu.socket_cpu = NULL;
		u1_cpu.is_connect = FALSE;
		closesocket(u2_cpu.socket_cpu);
		u2_cpu.socket_cpu = NULL;
		u2_cpu.is_connect = FALSE;
		closesocket(u3_cpu.socket_cpu);
		u3_cpu.socket_cpu = NULL;
		u3_cpu.is_connect = FALSE;
	}

	if (globalstate.open_device_single)
	{
		globalstate.image_show_start = false;
		while (1)
		{
			if (!globalstate.image_shoe_stop)
			{
				break;
			}
			Sleep(50);
		}
		int nRet = MV_OK;
		nRet = myhikclass->StopGrabbing();
		if (MV_OK != nRet)
		{
			return;
		}
		if (myhikclass)
		{
			myhikclass->Close();
			delete myhikclass;
			myhikclass = NULL;
		}
		globalstate.open_device_single = false;
	}


	CString str = GetDiffConfig();
	if ("null" == str)
	{
		if (::MessageBox(NULL, "关闭程序？", "提示", MB_YESNO) == IDNO)
		{
			/*if (true == is_SAVE_config)
			{
			SavePartConfig();
			}		*/
			return;
		}	
		SavePartConfig();	
	}
	else
	{
		int id = ::MessageBox(NULL, str, "保存配置并关闭程序？", MB_YESNOCANCEL);
		switch (id)
		{
		case IDYES:
			SaveConfig();
			break;
		case IDNO:					
			SavePartConfig();		
			break;
		case IDCANCEL:
			return;
		default:
			break;
		}
	}


	CDialogEx::OnClose();
}


void CNewAutoPatDlg::OnSelchangeMatPostion()
{
	// TODO: 在此添加控件通知处理程序代码
	//printf(" 按钮选择%d \n", m_MatPostion.GetCurSel());
	templedata.postion_mat = m_MatPostion.GetCurSel();
}


void CNewAutoPatDlg::OnBnClickedTestDebug()
{
	// TODO: 在此添加控件通知处理程序代码

	printf("复选框选中事件 %d\n", m_Debug_check.GetCheck());
	if ( 1 == m_Debug_check.GetCheck())
	{
		SetWinEnable();
		//globalstate.opencv_show
	}
	else
	{
		SetWinUnEnable();
		globalstate.opencv_show = false;
	}


}


void CNewAutoPatDlg::OnBnClickedChangeData()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	printf("i fell so tired  %s\n", m_datadigital);
	int number;
	float ratio_num;
	switch (m_Dataselect.GetPos())
	{
		case 1:
			number = atoi(m_datadigital);
			savedata.center_x = number;
			break;
		case 2:
			number = atoi(m_datadigital);
			savedata.center_y = number;
			break;
		case 3:
			ratio_num = atof(m_datadigital);
			savedata.u1_pulse_pixel = ratio_num;
			break;
		case 4:
			ratio_num = atof(m_datadigital);
			savedata.u3_pulse_pixel = ratio_num;
			break;
		case 5:
			ratio_num = atof(m_datadigital);
			savedata.u1_u2_scale = ratio_num;
			break;
		case 6:
			number = atoi(m_datadigital);
			savedata.cols_mat_pulse = number;
			break;
		case 7:
			number = atoi(m_datadigital);
			savedata.row_mat_pulse = number;
			break;
		case 8:
			number = atoi(m_datadigital);
			savedata.space_cols_pulse = number;
			break;
		case 9:
			number = atoi(m_datadigital);
			savedata.space_rows_pulse = number;
			break;
		case 10:
			number = atoi(m_datadigital);
			savedata.camera_start_pulse = number;
			break;
		case 11:
			number = atoi(m_datadigital);
			savedata.bottom_jacking_pulse = number;
			break;
		case 12:
			number = atoi(m_datadigital);
			savedata.vacuumpads_camera_pulse = number;
			break;
		case 13:
			number = atoi(m_datadigital);
			savedata.first_mat_pulse = number;
			break;
		case 14:
			number = atoi(m_datadigital);
			savedata.second_mat_pulse = number;
			break;
		case 15:
			number = atoi(m_datadigital);
			savedata.valid_mat_cols = number;
			break;
		case 16:
			//number = atoi(m_datadigital);
			//savedata.postion_mat = number;
			break;
		case 17:
			number = atoi(m_datadigital);
			savedata.opencv_thread = number;
			break;
		case 18:
			number = atoi(m_datadigital);
			savedata.opencv_template_circle = number;
			break;
		default:
			break;
	}
}


void CNewAutoPatDlg::OnNMCustomdrawSelectItem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CString cstr;
	int num = 0;
	num = m_Dataselect.GetPos();
	cstr.Format("%d", num);
	GetDlgItem(IDC_SELECT_NUMBER)->SetWindowText(cstr);
	switch (num)   //IDC_CHANGE_WHAT
	{
	case 1:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("图像中心点x坐标"));
		break;
	case 2:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("图像中心点y坐标"));
		break;
	case 3:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("U1脉冲与像素系数"));
		break;
	case 4:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("U3脉冲与像素系数"));
		break;
	case 5:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("U1和U2的脉冲系数"));
		break;
	case 6:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("行之间的脚垫脉冲"));
		break;
	case 7:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("列之间的脚垫脉冲"));
		break;
	case 8:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("间隔脚垫的行脉冲"));
		break;
	case 9:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("间隔脚垫的列脉冲"));
		break;
	case 10:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("摄像头起始位置脉冲"));
		break;
	case 11:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("顶轴起始位置脉冲"));
		break;
	case 12:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("摄像头和吸头距离"));
		break;
	case 13:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("第一个脚垫的距离"));
		break;
	case 14:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("第二个脚垫的距离"));
		break;
	case 15:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("脚垫的有效行数"));
		break;
	case 16:
		//number = atoi(m_datadigital);
		//savedata.postion_mat = number;
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("暂无"));
		break;
	case 17:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("二值化的值"));
		break;
	case 18:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("匹配模板的大小"));
		break;
	default:
		GetDlgItem(IDC_CHANGE_WHAT)->SetWindowText(_T("暂无"));
		break;
	}

}


void CNewAutoPatDlg::OnBnClickedHostSend()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	SocketSendCmd(m_Data_Send_Str, 4);
}

int for_cols6_debug_run(int cols = 0)
{
	CString strlen;
	for (int ix = cols; ix < 6; ix++)
	{
		//savedata.save_cols6 = ix;
		if (!sync_judge_adjust_debug())
		{
			templedata.error_number++;
			if (templedata.error_number >= 4)
			{
				AfxMessageBox(_T("连续检测错误超过4个，请换脚垫重新开始"));
				return -1;
			}
			if (ix == 5)
			{
				strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
				SocketSendCmd(strlen, 1);
				strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
				SocketSendCmd(strlen, 3);
				strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			}
			else // 不是边缘
			{
				strlen.Format("02%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len -= savedata.cols_mat_pulse;
				strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
			}

			continue;
		}
		//对准之后 下一个
		
		while (b_test_stop)
		{
			Sleep(100);
			if (globalstate.force_quit)
			{
				globalstate.force_quit = false;
				return -1;
			}
		}
		Sleep(200);

		//边缘考虑
		if (ix == 5)
		{
			strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(strlen, 3);
			strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));

		}
		else // 不是边缘
		{
			strlen.Format("02%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
			SocketSendCmd(strlen, 1);
			strlen.Format("03%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
		}
	}
	return 0;
}

int for_cols5_debug_run(int cols = 0)
{
	CString strlen;
	for (int iy = cols; iy < 5; iy++)
	{
		//savedata.save_cols5 = iy;
		if (!sync_judge_adjust_debug())
		{
			templedata.error_number++;
			if (templedata.error_number >= 4)
			{
				AfxMessageBox(_T("连续检测错误4个，请重新换张脚垫开始"));
				templedata.circle_judge_exceed = true;
				return -1;
			}
			if (iy == 4)
			{
				strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len = templedata.tem_len + savedata.space_cols_pulse;
				strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
				SocketSendCmd(strlen, 3);
				strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			}
			else
			{
				strlen.Format("03%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
				SocketSendCmd(strlen, 1);
				templedata.tem_len += savedata.cols_mat_pulse;
				strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
				SocketSendCmd(strlen, 2);
				//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
			}
			continue;
		}
		//对准之后
		
		while (b_test_stop)
		{
			Sleep(100);
			if (globalstate.force_quit)
			{
				globalstate.force_quit = false;
				return -1;
			}
		}
		//b_test_stop = TRUE;*/
		Sleep(200);
		//返回判断边缘
		if (iy == 4)
		{
			strlen.Format("03%04x%04x", savedata.space_cols_pulse, savedata.space_cols_pulse);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", savedata.space_rows_pulse, savedata.space_rows_pulse);
			SocketSendCmd(strlen, 3);
			strlen.Format("02%04x%04x", int(savedata.u1_u2_scale*savedata.space_cols_pulse), int(savedata.u1_u2_scale*savedata.space_cols_pulse));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.u1_u2_scale*savedata.space_cols_pulse));
		}
		else
		{
			strlen.Format("03%04x%04x", savedata.cols_mat_pulse, savedata.cols_mat_pulse);
			SocketSendCmd(strlen, 1);
			strlen.Format("02%04x%04x", int(savedata.cols_mat_pulse*savedata.u1_u2_scale), int(savedata.cols_mat_pulse*savedata.u1_u2_scale));
			SocketSendCmd(strlen, 2);
			//printf("%d \n", int(savedata.cols_mat_pulse*savedata.u1_u2_scale ));
		}
	}
	return 0;
}

int  bottom_test_dubug(int valid_cols, int start_cols = 0)
{
	int ret = 0;
	templedata.error_number = 0;
	templedata.circle_judge_exceed = false;
	for (int i = start_cols; i<valid_cols; i++)
	{
		//savedata.save_surplus_cols = i;
		ret = for_cols6_debug_run();
		if (ret != 0)
		{
			return -1;
		}

		ret = for_cols5_debug_run();
		if (ret != 0)
		{
			return -1;
		}
	}
	//printf("循环结束");
	return 0;
}

void bottom_camera_debug()
{
	int ret = 0;
	CString str_pluse;
	globalstate.runing_state = true;
	SocketSendCmd(U1_UPSUCTION, 1);
	SocketSendCmd(U1_OPENLIGHT, 1);
	SocketSendCmd(U1_MATDOWN, 1);
	SocketSendCmd(U1_RESET, 1);
	SocketSendCmd(U2_RESET, 2);
	str_pluse.Format("02%04x%04x", savedata.first_mat_pulse, savedata.first_mat_pulse);
	SocketSendCmd(str_pluse, 1);
	SocketSendCmd(U3_UPPRESS, 3); //抬起

	globalstate.material_send = false;
	while (!globalstate.material_send)
	{
		Sleep(50);
	}
	globalstate.material_send = false; // 进料确定
	SocketSendCmd(U3_DOWNPRESS, 3);
	Sleep(200);
	SocketSendCmd(U1_RESET, 1);
	str_pluse.Format("02%04x%04x", savedata.camera_start_pulse, savedata.camera_start_pulse);
	SocketSendCmd(str_pluse, 1);


	templedata.dark_quantity = 0.0; //清除残留标志

	Sleep(200);
	while (templedata.dark_quantity<30)
	{
		SocketSendCmd("0202000200", 3);
		Sleep(400);
	}

	str_pluse.Format("02%04x%04x", savedata.bottom_jacking_pulse, savedata.bottom_jacking_pulse);
	SocketSendCmd(str_pluse, 2); //下顶到达摄像头下方

	while (TRUE)
	{
		SocketSendCmd("0200b000b0", 3);
		if (just_circle_stable())
		{
			break;
		}
	} //判断圆心是否稳定
	synchro_just_debug(); //微调
	while (TRUE)
	{
		if (just_circle_stable())
		{
			break;
		}
		SocketSendCmd("0200a000a0", 3);
	} //判断圆心是否稳定两次
	synchro_just_debug();
	str_pluse.Format("02%04x%04x", savedata.row_mat_pulse, savedata.row_mat_pulse);
	SocketSendCmd(str_pluse, 3); //跳到下一个开始  row_mat_pulse
								 //sync_judge_adjust();

	ret = bottom_test_dubug(savedata.valid_mat_cols);
	templedata.tem_len = 0;
	globalstate.runing_state = false;
}


DWORD WINAPI JUST_START_WHATFUCK(LPVOID  lppargram)
{
	//just_first_jiaodian();
	//just_first_jiaodian();
	bottom_camera_debug();
	return 0;
}

void CNewAutoPatDlg::OnBnClickedBottomCamera()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!globalstate.runing_state)
	{
		CreateThread(NULL, 0, JUST_START_WHATFUCK, NULL, 0, NULL);
	}
	
}


void CNewAutoPatDlg::OnBnClickedSetting()
{
	// TODO: 在此添加控件通知处理程序代码
	Csetting cseting;
	cseting.DoModal();
}
