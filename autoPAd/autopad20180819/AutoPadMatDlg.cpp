
// AutoPadMatDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AutoPadMat.h"
#include "AutoPadMatDlg.h"
#include "afxdialogex.h"
#include"SerialPort.h"

#include <fstream>
#include"Command.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CONFIG_FILE_PATH "cfg/config.ini"
#define CONFIG_APP_NAME  "umAppConfig"

using namespace cv;
using namespace std;

//log文件
std::ofstream ofs_com_log;
std::ofstream ofs_send_log;
std::ofstream ofs_point_log;

bool g_bopenport;
CSerialPort serialport;//串口对象
CString cstr_com_recv_string;//接收到的字符串
CStatic  BTN_videoDETECT; //检测按钮控制

bool is_cmd_running = true;//机器暂停控制位
bool is_send_cmd = true;//串口命令允许控制位
bool is_com_recv_data = false;//接收标志位（用于控制等待下一条指令的执行）
bool is_save_video = false;//保存视频标志位
bool is_detect_running = false;//脚垫圆心检测线程控制位
bool is_draw_circle_img = true;//输出图像绘制圆参数控制位
bool is_show_back_img = false;
bool is_waiyforC3_start = false;//判断C3是否完成
bool is_SAVE_config = false;//判断是否储存


int bool_restart = 0;
bool g_brestart = false;
bool g_brecord = false;
bool g_bput = false; 
bool g_bC3get = false;
bool g_bpressure = false;
bool g_bmateriel = true;
bool g_StartNext = false;//用来判断下一张脚垫的执行程序
bool g_ForceNext = false; //强行下一张脚垫

int cols_6;
int cols_5;
int len_firstmat = 1104; //第一个脚垫距离
int len_secondmat = 4256;   //第二个脚垫距离
int glive_postion = 0;   //实时检测的吸嘴位置
int glive_bottom_postion = 0; //实时记录下顶的位置
int len_absorb_camera = 214;//摄像头和吸嘴之间的距离
int len_reset_bottom = 291;//复位下顶的距离
int len_absorb_mat = 21;//吸头和脚垫之间在竖直方向的偏差
int num_availablemat_clos = 38; //有效吸取的脚垫数目

int len_newline_pressbearing = 125;//换行压轴移动到行间
int len_newline_bottom = 84; //换行下顶移动到下一个
int len_newline_camera = 82; //换行吸头移动到下一个
int len_nextmat_camera = 163; //不换行吸头移到下一个脚垫
int len_nextmat_bottom = 167; //不换行下顶移到下一个脚垫
float param_pixel_pulse_x = 0.45; //x方向上像素与脉冲的参数
float param_pixel_pulse_y = 1.5; //y方向上像素与脉冲的参数

int param_mat_postion = 1; // 贴脚垫的位置
int i_reset_move_up_len = 13072;//复位后摄像头移动到的位置
int i_speed_up_down_len = 125;//电机加减速补偿


int record_availablemat_clos = 0; //记录当前脚垫的位置
int record_livetime_camera_postion = 0;
int record_livetime_bottom_postion = 0;
int record_clos_6 = 0;
int record_clos_5 = 0;

Point point_test[5];//用于检测图像是否需要改变阈值
 //有效ROI区域
int i_rect_img_x = 212;
int i_rect_img_y = 88;
int i_rect_img_w = 154;
int i_rect_img_h = 210;

//中心点
int g_center_x = i_rect_img_w / 2;
int g_center_y = i_rect_img_h / 2;
Point center(g_center_x, g_center_x);
int i_sleep_ms = 280;//检测延时（电机运动完成后，摄像获取图像，检测到圆心的处理总耗时）

Mat img_capture;//摄像头输入图像
Mat img_mask;//匹配模版图像
Mat img_mask_1;
Mat img_mask_2;

int i_img_mask_d = 117;//模板匹配圆直径
int i_erase_max = 700;//抹去连续区域像素点数（去噪稳定）
int i_erase_min = 0;
int i_bin_thresh = 6;//二值化阈值
int i_match_type = 4;//模板匹配方法
int i_match_line = 1; //模板的外圈厚度

float f_dark_quantity;//图像亮度

int i_slider_num = 1;//手动微调步进

//HEHE


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


char CAutoPadMatDlg::HexChar(char c)
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
int CAutoPadMatDlg::Str2Hex(CString str, char* data)
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

//保存一条log
void SaveReceiveLog(char * str)
{

	char tmpBUfReceive[256];
	SYSTEMTIME sys;
	GetLocalTime(&sys);

	sprintf(tmpBUfReceive, "%4d/%02d/%02d %02d:%02d:%02d.%03d RC %s\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, str);
	ofs_com_log << tmpBUfReceive;
}

void SaveSendLog(char * str)
{
	char tmpBufSend[256];
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(tmpBufSend, "%4d/%02d/%02d %02d:%02d:%02d.%03d SD %s\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, str);
	ofs_send_log << tmpBufSend;
}


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
	SaveSendLog(str);
	//i_tx_count++;

	return 0;
}


//发送一条指令，并检查返回是否为当前指令
bool SendCmd(char * str)
{
	while (!is_send_cmd)
	{
		char ch[128];
		sprintf(ch, "上个指令未完成，操作太快:%s\r\n", str);
		CString cstr(ch);
		AfxMessageBox(cstr);
		is_send_cmd = true;
	}

	while (!is_cmd_running)
	{
		Sleep(1);
	}

	is_send_cmd = false;

	ForceSendCmd(str);

	//等待串口返回后，说明此指令生命周期结束
	char c3[3] = { str[2], str[3], ' ' };
	while (1)
	{
		Sleep(5);
		if (is_com_recv_data && cstr_com_recv_string == c3)
		{
			is_com_recv_data = false;
			is_send_cmd = true;
			break;
		}
	}

	//i_tx_count++;

	return 0;
}

//愚蠢的判断字符串里是否有EB或者BE 这里只有两种情况不管其他的
int JustCommod(int plus)
{
	int quotient = 0;
	quotient = plus / 255;
	int remainder = 0;
	remainder = plus % 255;
	if (quotient >0)
	{
		if (quotient == 0xEB || quotient == 0xBE)
		{
			quotient = quotient - 1;
		}	
	}

	if (remainder == 0xEB || remainder == 0xBE)
	{
		remainder = remainder - 1;
	}

	return quotient * 255 + remainder;
}

//发送命令线程
DWORD WINAPI SendCmdThread(LPVOID pParam)
{
	char * str = (char*)pParam;
	//SendCmd(str);
	ForceSendCmd(str);
	return 0;
}

// opencv程序块

//一些必要的初始化
bool SysInit()
{
	CreateDirectory("log", NULL);

	char chats[256];
	char chatrecv[256];
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(chats, "log/%4d_%02d_%02d_%02d_%02d_%02d_Receive.txt", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
	sprintf(chatrecv, "log/%4d_%02d_%02d_%02d_%02d_%02d_Send.txt", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

	ofs_com_log.open(chats, std::ofstream::app);
	ofs_send_log.open(chatrecv, std::ofstream::app);
	ofs_point_log.open("log/log_dx_dy.txt", std::ofstream::app);

	//初始化模板匹配模板图像
	Mat mask = Mat(Size(i_img_mask_d + 2, i_img_mask_d + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(i_img_mask_d / 2 + 1, i_img_mask_d / 2 + 1), i_img_mask_d / 2, Scalar(255, 255, 255), 1);
	img_mask = mask.clone();

	mask = Mat(Size(i_img_mask_d + 3, i_img_mask_d + 3), CV_8UC1, Scalar::all(0));
	circle(mask, Point(i_img_mask_d / 2 + 2, i_img_mask_d / 2 + 2), i_img_mask_d / 2 + 1, Scalar(255, 255, 255), 1);
	img_mask_1 = mask.clone();

	mask = Mat(Size(i_img_mask_d + 1, i_img_mask_d + 1), CV_8UC1, Scalar::all(0));
	circle(mask, Point(i_img_mask_d / 2, i_img_mask_d / 2), i_img_mask_d / 2 - 1, Scalar(255, 255, 255), 1);
	img_mask_2 = mask.clone();

	return 0;
}

//opencv控制面板响应函数
static void ContrastAndBright(int, void *)
{
	Mat m = Mat(800, 600, 0);
	imshow("【控制面板】", m);
	int line_type = i_match_line;
	if (line_type == 0)
	{
		line_type = 1;
	}
	Mat mask = Mat(Size(i_img_mask_d + 2, i_img_mask_d + 2), CV_8UC1, Scalar::all(0));
	circle(mask, Point(i_img_mask_d / 2 + 1, i_img_mask_d / 2 + 1), i_img_mask_d / 2, Scalar(255, 255, 255), line_type);
	img_mask = mask.clone();

	mask = Mat(Size(i_img_mask_d + 3, i_img_mask_d + 3), CV_8UC1, Scalar::all(0));
	circle(mask, Point(i_img_mask_d / 2 + 2, i_img_mask_d / 2 + 2), i_img_mask_d / 2 + 1, Scalar(255, 255, 255), line_type);
	img_mask_1 = mask.clone();

	mask = Mat(Size(i_img_mask_d + 1, i_img_mask_d + 1), CV_8UC1, Scalar::all(0));
	circle(mask, Point(i_img_mask_d / 2, i_img_mask_d / 2), i_img_mask_d / 2 - 1, Scalar(255, 255, 255), line_type);
	img_mask_2 = mask.clone();
}

//创建opencv控制面板
void InitControlBoard()
{
	namedWindow("【控制面板】", 0);
	createTrackbar("模板半径：", "【控制面板】", &i_img_mask_d, 180, ContrastAndBright);
	createTrackbar("擦除max：", "【控制面板】", &i_erase_max, 3000, ContrastAndBright);
	createTrackbar("擦除min：", "【控制面板】", &i_erase_min, 500, ContrastAndBright);
	createTrackbar("阈值：", "【控制面板】", &i_bin_thresh, 200, ContrastAndBright);
	createTrackbar("模板匹配：", "【控制面板】", &i_match_type, 5, ContrastAndBright);
	createTrackbar("roi_x：", "【控制面板】", &i_rect_img_x, 500, ContrastAndBright);
	createTrackbar("roi_y：", "【控制面板】", &i_rect_img_y, 500, ContrastAndBright);
	createTrackbar("roi_w：", "【控制面板】", &i_rect_img_w, 500, ContrastAndBright);
	createTrackbar("roi_h：", "【控制面板】", &i_rect_img_h, 500, ContrastAndBright);
	createTrackbar("延时：", "【控制面板】", &i_sleep_ms, 400, ContrastAndBright);
	createTrackbar("外圈大小：", "【控制面板】", &i_match_line, 6, ContrastAndBright);
}

//检测圆心是否是同一个
bool Detected_circleStable(Point * center_point)
{
	if (abs(center_point[0].x - center_point[1].x)<2 && abs(center_point[0].x - center_point[2].x)<2
		&& abs(center_point[0].y - center_point[1].y)<2 && abs(center_point[0].y - center_point[2].y)<2)
	{
		return false;
	}
	else {
		return true;
	}
}

//模版匹配
Point MatchCircleTemplate(Mat bitwise_out, Mat templ)
{
	Mat g_resultImage;
	Mat srcImage;
	bitwise_out.copyTo(srcImage);
	int resultImage_cols = bitwise_out.cols - templ.cols + 1;
	int resultImage_rows = bitwise_out.rows - templ.rows + 1;
	g_resultImage.create(resultImage_cols, resultImage_rows, CV_32FC1);

	//进行匹配
	matchTemplate(bitwise_out, templ, g_resultImage, i_match_type);

	//标准化
	normalize(g_resultImage, g_resultImage, 0, 2, NORM_MINMAX, -1, Mat());
	double minValue, maxValue;
	Point minLocation, maxLocation, matchLocation;


	//定位最匹配的位置
	minMaxLoc(g_resultImage, &minValue, &maxValue, &minLocation, &maxLocation);

	if (i_match_type == TM_SQDIFF || i_match_type == CV_TM_SQDIFF_NORMED)
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}

	int x = matchLocation.x + i_img_mask_d / 2;
	int y = matchLocation.y + i_img_mask_d / 2;

	Point p_re;
	p_re.x = matchLocation.x + i_img_mask_d / 2;
	p_re.y = matchLocation.y + i_img_mask_d / 2;

	return p_re;

}

//区域去除，CheckMode: 0代表去除黑区域，1代表去除白区域; NeihborMode：0代表4邻域，1代表8邻域;  
void RemoveRegion(Mat& Src, Mat& Dst, int AreaLimitMin, int AreaLimitMax, int CheckMode, int NeihborMode)
{
	int i_remove_count = 0;//记录除去的个数  
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
				//开始该点处的检查
				vector<Point2i> GrowBuffer;//堆栈，用于存储生长点  
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
				if (GrowBuffer.size()<AreaLimitMax && GrowBuffer.size() > AreaLimitMin)
				{
					CheckResult = 2; //判断结果（是否超出限定的大小），1为未超出，2为超出 
				}
				else { CheckResult = 1;   i_remove_count++; }
				for (int z = 0; z<GrowBuffer.size(); z++)//更新Label记录  
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

//圆心检测线程
DWORD WINAPI DetectCircleThread(LPVOID pParam)
{
	//BTN_videoDETECT.SetWindowText("停止检测");

	theApp.GetMainWnd()->GetDlgItem(IDC_VIDEODETECT)->SetWindowText("停止检测");
	bool is_change_run_cmd = false;

	VideoCapture capture(1);

	Mat show_image;

	capture >> show_image;

	if (!show_image.data)
	{
		capture.open(0);
		capture >> show_image;

		if (!show_image.data)
		{
			is_cmd_running = false;
			is_save_video = false;
			is_detect_running = false;
			destroyAllWindows();
			//BTN_videoDETECT.SetWindowText("开始检测");
			theApp.GetMainWnd()->GetDlgItem(IDC_VIDEODETECT)->SetWindowText("开始检测");
			theApp.GetMainWnd()->GetDlgItem(IDC_BTN_STOP)->SetWindowText("点击继续");
			AfxMessageBox("摄像头打开失败，请检查摄像头后重新运行检测！");
			//MessageBox(NULL, "摄像头打开失败，请检查摄像头后重新运行检测！", "提示", MB_OK);
			return -1;
		}
	}

	VideoWriter videoWriter;

	char chats[256];
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(chats, "log/%4d_%02d_%02d_%02d_%02d_%02d_%03d.avi", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
	const string outFlie = chats;

	if (is_save_video)
	{
		//打开视频文件，准备写入  
		if (videoWriter.open(outFlie, -1, 20.0, Size(i_rect_img_w * 2, i_rect_img_h), true) == false)
		{
			MessageBox(NULL, "videoWriter open 失败！", "提示", MB_OK);
			is_save_video = false;
		}
	}

	//读取直方图
	/*MatND dst_hist;
	cv_yml_fs["dst_hist"] >> dst_hist;*/

	/*if (!cv_yml_fs.isOpened()) {
		is_cmd_running = false;
		is_save_video = false;
		is_detect_running = false;
		destroyAllWindows();
		m_btnVideoDetect.SetWindowText("开始检测");
		MessageBox(NULL, "归一化直方图文件读取失败，停止检测！", "警告", MB_OK);
		return -1;
	}*/

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
			is_cmd_running = false;
			is_save_video = false;
			is_detect_running = false;
			destroyAllWindows();
			//BTN_videoDETECT.SetWindowText("开始检测");
			theApp.GetMainWnd()->GetDlgItem(IDC_VIDEODETECT)->SetWindowText("开始检测");
			theApp.GetMainWnd()->GetDlgItem(IDC_BTN_STOP)->SetWindowText("点击继续");
			//MessageBox(NULL, "摄像头异常断开导致暂停！！！！\r\n请重新确认摄像头模块功能正常后，点击开始检测。", "毁灭性警告！", MB_OK);
			AfxMessageBox("摄像头打开失败，请检查摄像头后重新运行检测！");
			break;
		}

		//截取矩形区域
		show_image = show_image(Rect(i_rect_img_x, i_rect_img_y, i_rect_img_w, i_rect_img_h));
		Mat color_img = show_image.clone();
		img_capture = show_image.clone();

		//计算亮度
		float cast;
		GetBrightness(show_image, cast, f_dark_quantity);
		String strs = "ca:" + to_string(cast);
		putText(show_image, strs, Point(0, 10), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);
		strs = "da:" + to_string(f_dark_quantity);
		putText(show_image, strs, Point(0, 23), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);

		//显示时间
		char date_str[256];
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		sprintf(date_str, "%4d%02d%02d %02d:%02d:%02d.%03d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		putText(show_image, date_str, Point(0, show_image.rows), FONT_HERSHEY_PLAIN, 0.7, Scalar(0, 255, 255), 1);


		line(show_image, Point(0, show_image.rows / 2), Point(show_image.cols, show_image.rows / 2), Scalar(0, 0, 255));
		line(show_image, Point(show_image.cols / 2, 0), Point(show_image.cols / 2, show_image.rows), Scalar(0, 0, 255));

		Mat gray_img;
		cvtColor(color_img, gray_img, CV_BGR2GRAY);


		//直方图归一化
		//Mat m_specify_in = gray_img.clone();
		//OneChannelHistSpecify(gray_img, dst_hist, gray_img);
		//Mat m_specify_out = gray_img.clone();

		int b_thresh = i_bin_thresh;

		Mat m_threshold;
		/*Mat canny;
		Canny(gray_img, canny,150,150);
		imshow("canny",canny);*/
		threshold(gray_img, m_threshold, b_thresh, 255, 1);
		RemoveRegion(m_threshold, m_threshold, i_erase_min, i_erase_max, 1, 1);

		Point p1, p2, p3;

		p1 = MatchCircleTemplate(m_threshold, img_mask);
		p2 = MatchCircleTemplate(m_threshold, img_mask_1);
		p3 = MatchCircleTemplate(m_threshold, img_mask_2);

		center.x = (p1.x + p2.x + p3.x) / 3;
		center.y = (p1.y + p2.y + p3.y) / 3;

		Mat detect_out_img = Mat(gray_img.size(), CV_8UC3, Scalar::all(0));

		if (is_draw_circle_img)
		{
			//圆中心
			circle(show_image, center, 3, Scalar(0, 255, 255), -1, 8, 0);

			String str = "[" + to_string(center.x) + "," + to_string(center.y) + "]";
			putText(show_image, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);

			strs = "dx:" + to_string(center.x - g_center_x);
			putText(show_image, strs, Point(0, 36), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);

			strs = "dy:" + to_string(center.y - g_center_y);
			putText(show_image, strs, Point(0, 49), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 255), 1);


			//圆轮廓  
			int circle_line = i_match_line;
			if (circle_line == 0)
			{
				circle_line = 1;
			}
			circle(detect_out_img, Point(center.x, center.y + 1), i_img_mask_d / 2, Scalar(0, 255, 255), i_match_line, 8, 0);
			str = "[" + to_string(center.x) + "," + to_string(center.y) + "]";
			putText(detect_out_img, str, center, FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);

			t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();

			putText(detect_out_img, to_string(t * 1000), Point(10, 10), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255), 1);

		}

		if (is_show_back_img)
		{
			//imshow("m_specify_in", m_specify_in);
			//imshow("m_specify_out", m_specify_out);
			imshow("m_threshold", m_threshold);
		}

		//新建一个要合并的图像
		Mat img_merge;
		Size size(i_rect_img_w * 2, i_rect_img_h);
		img_merge.create(size, CV_MAKETYPE(show_image.depth(), 3));
		img_merge = Scalar::all(0);
		Mat outImg_left, outImg_right;
		//在新建合并图像中设置感兴趣区域
		outImg_left = img_merge(Rect(0, 0, i_rect_img_w, i_rect_img_h));
		outImg_right = img_merge(Rect(i_rect_img_w, 0, i_rect_img_w, i_rect_img_h));
		//outImg_right2 = img_merge(Rect(i_rect_img_w, 0, i_rect_img_w, i_rect_img_h));
		//将待拷贝图像拷贝到感性趣区域中
		show_image.copyTo(outImg_left);
		detect_out_img.copyTo(outImg_right);

		imshow("image1", img_merge);


		if (is_save_video)
		{
			videoWriter.write(img_merge);
		}

		char c_key = waitKey(1);

		if (c_key == 's')
		{
			destroyAllWindows();
			is_show_back_img = !is_show_back_img;
		}

	}

	videoWriter.release();
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


// CAutoPadMatDlg 对话框



CAutoPadMatDlg::CAutoPadMatDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_AUTOPADMAT_DIALOG, pParent)
	, m_DATASHOWSTR(_T(""))
	, m_DATDSEND(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoPadMatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RECIVE, m_DATASHOW);
	DDX_Text(pDX, IDC_EDIT_RECIVE, m_DATASHOWSTR);
	DDX_Text(pDX, IDC_EDIT_SEND, m_DATDSEND);
	DDX_Control(pDX, IDC_BUTTON_OPENPORT, m_PORTOPEN);
	DDX_Control(pDX, IDC_COMBO_COMSELECT, m_SetCom);
	DDX_Control(pDX, IDC_COMBO_SPEED, m_SetBaud);
	DDX_Control(pDX, IDC_VIDEODETECT, m_btnVideoDetect);
	DDX_Control(pDX, IDC_SLIDER3, m_sliderBJ);
	DDX_Control(pDX, IDC_SLIDER_LENVS, m_slider_lenSC);
	DDX_Control(pDX, IDC_SLIDER2, m_slider_lenY);
	DDX_Control(pDX, IDC_MAT_POSTION, m_MatPostion);
	DDX_Control(pDX, IDC_STATIC_PIC, m_PICSET);
}

BEGIN_MESSAGE_MAP(CAutoPadMatDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CAutoPadMatDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_OPENPORT, &CAutoPadMatDlg::OnBnClickedButtonOpenport)
	ON_CBN_SELENDOK(IDC_COMBO_COMSELECT, &CAutoPadMatDlg::OnCbnSelendokComboComselect)
	ON_CBN_SELENDOK(IDC_COMBO_SPEED, &CAutoPadMatDlg::OnCbnSelendokComboSpeed)
	ON_MESSAGE(WM_COMM_RXCHAR, OnCommunication)
	ON_BN_CLICKED(IDC_VIDEODETECT, &CAutoPadMatDlg::OnBnClickedVideodetect)
	ON_BN_CLICKED(IDC_VIDEO_SETPARAM, &CAutoPadMatDlg::OnBnClickedVideoSetparam)
	ON_BN_CLICKED(IDC_BTN_STOP, &CAutoPadMatDlg::OnBnClickedBtnStop)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BTN_A3, &CAutoPadMatDlg::OnBnClickedBtnA3)
	ON_BN_CLICKED(IDC_BTN_A4, &CAutoPadMatDlg::OnBnClickedBtnA4)
	ON_BN_CLICKED(IDC_BTN_LEFT, &CAutoPadMatDlg::OnBnClickedBtnLeft)
	ON_BN_CLICKED(IDC_BTN_RIGHT, &CAutoPadMatDlg::OnBnClickedBtnRight)
	ON_BN_CLICKED(IDC_BTN_B2, &CAutoPadMatDlg::OnBnClickedBtnB2)
	ON_BN_CLICKED(IDC_BTN_B3, &CAutoPadMatDlg::OnBnClickedBtnB3)
	ON_BN_CLICKED(IDC_BTN_CUP, &CAutoPadMatDlg::OnBnClickedBtnCup)
	ON_BN_CLICKED(IDC_BTN_CDOWN, &CAutoPadMatDlg::OnBnClickedBtnCdown)
	ON_BN_CLICKED(IDC_BTN_AIROPEN, &CAutoPadMatDlg::OnBnClickedBtnAiropen)
	ON_BN_CLICKED(IDC_BTN_AIRDOWN, &CAutoPadMatDlg::OnBnClickedBtnAirdown)
	ON_BN_CLICKED(IDC_BTN_XUP, &CAutoPadMatDlg::OnBnClickedBtnXup)
	ON_BN_CLICKED(IDC_BTN_XDOWN, &CAutoPadMatDlg::OnBnClickedBtnXdown)
	ON_BN_CLICKED(IDC_BTN_MATIN, &CAutoPadMatDlg::OnBnClickedBtnMatin)
	ON_BN_CLICKED(IDC_BTN_RESET, &CAutoPadMatDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDC_BTN_SAMETEST, &CAutoPadMatDlg::OnBnClickedBtnSametest)
	ON_BN_CLICKED(IDC_BTN_START, &CAutoPadMatDlg::OnBnClickedBtnStart)
	ON_CBN_SELENDOK(IDC_MAT_POSTION, &CAutoPadMatDlg::OnCbnSelendokMatPostion)
	ON_BN_CLICKED(IDC_BUTTON6, &CAutoPadMatDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_FORCESTOP, &CAutoPadMatDlg::OnBnClickedForcestop)
	ON_CBN_SELCHANGE(IDC_MAT_POSTION, &CAutoPadMatDlg::OnCbnSelchangeMatPostion)
END_MESSAGE_MAP()


//初始化机器配置
CString CAutoPadMatDlg::InitConfig()
{
	CFileFind config_file_finder;

	BOOL ifFind = config_file_finder.FindFile(_T(CONFIG_FILE_PATH));
	if (!ifFind)
	{
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("串口端口"), _T("4"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("串口波特率"), _T("9600"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("图像中心点x坐标"), _T("77"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("图像中心点y坐标"), _T("105"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("贴脚垫的位置"), _T("1"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("第二个脚垫脉冲距离"), _T("4256"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("第一个脚垫脉冲距离"), _T("1104"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("吸嘴到摄像头距离"), _T("214"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("吸取补偿偏移量"), _T("21"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("脚垫排数"), _T("38"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("模板匹配圆直径"), _T("117"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("匹配圆直径粗细"), _T("1"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("二值化阈值"), _T("6"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("模板匹配方法"), _T("4"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("ROI区域X"), _T("212"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("ROI区域Y"), _T("88"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("ROI区域W"), _T("154"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("ROI区域H"), _T("210"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("检测延时"), _T("280"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("X轴像素和脉冲参数"), _T("0.45"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("Y轴像素和脉冲参数"), _T("1.5"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("复位后摄像头移动距离"), _T("13072"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("复位后下顶移动距离"), _T("291"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("保存视频"), _T("0"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("非换行下顶移动"), _T("167"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("非换行吸头移动"), _T("163"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("换行物料移动"), _T("125"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("换行下顶移动"), _T("84"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("换行吸头移动"), _T("82"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("电机加减速补偿"), _T("125"), _T(CONFIG_FILE_PATH));
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("重启开机是否有数据记录"), _T("0"), _T(CONFIG_FILE_PATH));
	}

	bool is_fale_read_pram = false;
	int itmp;
	double dtmp;
	CString cs_tmp;
	CString cs_return;

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("串口端口"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "串口端口参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		m_nCom = 4;
	}
	else {
		m_nCom = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("串口波特率"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "串口波特率参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		m_nBaud = 9600;
	}
	else {
		m_nBaud = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("图像中心点x坐标"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "图像中心点x坐标参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		g_center_x = 77;
	}
	else {
		g_center_x = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("图像中心点y坐标"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "图像中心点y坐标参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		g_center_y = 105;
	}
	else {
		g_center_y = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("贴脚垫的位置"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "贴脚垫的位置 参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		param_mat_postion = 1;
	}
	else {
		param_mat_postion = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("第二个脚垫脉冲距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "第二个脚垫脉冲距离 参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_secondmat = 4256;
	}
	else {
		len_secondmat = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("第一个脚垫脉冲距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "第一个脚垫脉冲距离 参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_firstmat = 1104;
	}
	else {
		len_firstmat = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("吸嘴到摄像头距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "吸嘴到摄像头距离参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_absorb_camera = 214;
	}
	else {
		len_absorb_camera = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("吸取补偿偏移量"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "吸取补偿偏移量参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_absorb_mat = 21;
	}
	else {
		len_absorb_mat = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("脚垫排数"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "脚垫排数参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		num_availablemat_clos = 38;
	}
	else {
		num_availablemat_clos = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("模板匹配圆直径"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "模板匹配圆直径参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_img_mask_d = 117;
	}
	else {
		i_img_mask_d = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("匹配圆直径粗细"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "匹配圆直径粗细参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_match_line = 1;
	}
	else {
		i_match_line = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("二值化阈值"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "二值化阈值参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_bin_thresh = 6;
	}
	else {
		i_bin_thresh = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("模板匹配方法"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "模板匹配方法参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_match_type = 4;
	}
	else {
		i_match_type = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("ROI区域X"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "ROI区域X参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_rect_img_x = 212;
	}
	else {
		i_rect_img_x = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("ROI区域Y"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "ROI区域Y参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_rect_img_y = 88;
	}
	else {
		i_rect_img_y = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("ROI区域W"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "ROI区域W参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_rect_img_w = 154;
	}
	else {
		i_rect_img_w = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("ROI区域H"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "ROI区域H参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_rect_img_h = 210;
	}
	else {
		i_rect_img_h = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("检测延时"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "检测延时参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_sleep_ms = 280;
	}
	else {
		i_sleep_ms = itmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("X轴像素和脉冲参数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if (dtmp == 0.0)
	{
		cs_tmp = "X轴像素和脉冲参数！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		param_pixel_pulse_x = 0.45;
	}
	else {
		param_pixel_pulse_x = dtmp;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("Y轴像素和脉冲参数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if (dtmp == 0.0)
	{
		cs_tmp = "Y轴像素和脉冲参数参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		param_pixel_pulse_y = 1.5;
	}
	else {
		param_pixel_pulse_y = dtmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("复位后摄像头移动距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "复位后摄像头移动距离参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_reset_move_up_len = 13072;
	}
	else {
		i_reset_move_up_len = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("复位后下顶移动距离"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "复位后下顶移动距离参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_reset_bottom = 291;
	}
	else {
		len_reset_bottom = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("保存视频"), -1, _T(CONFIG_FILE_PATH));
	is_save_video = itmp == 1 ? true : false;
	if (itmp == -1)
	{
		cs_tmp = "保存视频参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("非换行下顶移动"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "非换行下顶移动参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_nextmat_bottom = 167;
	}
	else {
		len_nextmat_bottom = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("非换行吸头移动"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "非换行吸头移动参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_nextmat_camera = 163;
	}
	else {
		len_nextmat_camera = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("换行物料移动"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "换行物料移动参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_newline_pressbearing = 125;
	}
	else {
		len_newline_pressbearing = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("换行下顶移动"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "换行下顶移动参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_newline_bottom = 84;
	}
	else {
		len_newline_bottom = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("换行吸头移动"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "换行吸头移动参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		len_newline_camera = 82;
	}
	else {
		len_newline_camera = itmp;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("电机加减速补偿"), -1, _T(CONFIG_FILE_PATH));
	i_speed_up_down_len = itmp == -1 ? 125 : itmp;
	if (itmp == -1)
	{
		cs_tmp = "电机加减速补偿参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		i_speed_up_down_len = 125;
	}
	else {
		i_speed_up_down_len = itmp;
	}

	//cs_tmp.ReleaseBuffer();

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("重启开机是否有数据记录"), -1, _T(CONFIG_FILE_PATH));
	if (itmp == -1)
	{
		cs_tmp = "重启开机是否有数据记录参数读取失败！\r\n";
		cs_return += cs_tmp;
		is_fale_read_pram = true;
		bool_restart = 0;
	}
	else {
		bool_restart = itmp;
	}

	//用于读取配置记录的数据
	if (bool_restart == 1)
	{
		g_brestart = true;
		itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("记录运行完的脚垫行数"), -1, _T(CONFIG_FILE_PATH));
		if (itmp == -1)
		{
			cs_tmp = "记录运行完的脚垫行数参数读取失败！\r\n";
			cs_return += cs_tmp;
			is_fale_read_pram = true;
			g_brestart = false;
		}
		else {
			record_availablemat_clos = itmp;
		}

		itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("记录参数clos_6"), -1, _T(CONFIG_FILE_PATH));
		if (itmp == -1)
		{
			cs_tmp = "记录参数clos_6参数读取失败！\r\n";
			cs_return += cs_tmp;
			is_fale_read_pram = true;
			g_brestart = false;
		}
		else {
			record_clos_6 = itmp;
		}

		itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("记录参数clos_5"), -1, _T(CONFIG_FILE_PATH));
		if (itmp == -1)
		{
			cs_tmp = "记录参数clos_5参数读取失败！\r\n";
			cs_return += cs_tmp;
			is_fale_read_pram = true;
			g_brestart = false;
		}
		else {
			record_clos_5 = itmp;
		}

		itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("记录吸头位置"), -1, _T(CONFIG_FILE_PATH));
		if (itmp == -1)
		{
			cs_tmp = "记录吸头位置参数读取失败！\r\n";
			cs_return += cs_tmp;
			is_fale_read_pram = true;
			g_brestart = false;
		}
		else {
			record_livetime_camera_postion = itmp;
		}

		itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("记录下定位置"), -1, _T(CONFIG_FILE_PATH));
		if (itmp == -1)
		{
			cs_tmp = "记录下定位置参数读取失败！\r\n";
			cs_return += cs_tmp;
			is_fale_read_pram = true;
			g_brestart = false;
		}
		else {
			record_livetime_bottom_postion = itmp;
		}
	}

	if (!ifFind)
	{
		cs_return = "当前目录不存在config.ini配置文件，系统新建并保存默认参数的配置文件！\r\n";
		return cs_return;
	}
	else if (is_fale_read_pram) {
		return cs_return;
	}
	else {
		return "null";
	}
}

//保存机器部分配置文件
int CAutoPadMatDlg::SavePartConfig()
{ 
	CString cstr;
	if (g_brecord == false)
	{
		bool_restart = 0;
		cstr.Format("%d", bool_restart);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("重启开机是否有数据记录"), cstr, _T(CONFIG_FILE_PATH));
	}
	else {
		bool_restart = 1;
		cstr.Format("%d", bool_restart);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("重启开机是否有数据记录"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_availablemat_clos);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录运行完的脚垫行数"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_clos_6);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录参数clos_6"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_clos_5);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录参数clos_5"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_livetime_camera_postion);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录吸头位置"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_livetime_bottom_postion);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录下定位置"), cstr, _T(CONFIG_FILE_PATH));

	}
	return 0;
}
//保存机器配置文件
BOOL CAutoPadMatDlg::SaveConfig()
{
	CString cstr;

	cstr.Format("%d", m_nCom);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("串口端口"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", m_nBaud);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("串口波特率"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", g_center_x);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("图像中心点x坐标"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", g_center_y);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("图像中心点y坐标"), cstr, _T(CONFIG_FILE_PATH));
 
	cstr.Format("%d", param_mat_postion);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("贴脚垫的位置"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_secondmat);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("第二个脚垫脉冲距离"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_firstmat);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("第一个脚垫脉冲距离"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_absorb_camera);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("吸嘴到摄像头距离"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_absorb_mat);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("吸取补偿偏移量"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", num_availablemat_clos);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("脚垫排数"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_img_mask_d);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("模板匹配圆直径"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_match_line);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("匹配圆直径粗细"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_bin_thresh);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("二值化阈值"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_match_type);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("模板匹配方法"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_rect_img_x);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("ROI区域X"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_rect_img_y);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("ROI区域Y"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_rect_img_w);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("ROI区域W"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_rect_img_h);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("ROI区域H"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_sleep_ms);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("检测延时"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%f", param_pixel_pulse_x);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("X轴像素和脉冲参数"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%f", param_pixel_pulse_y);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("Y轴像素和脉冲参数"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_reset_move_up_len);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("复位后摄像头移动距离"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_reset_bottom);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("复位后下顶移动距离"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", is_save_video);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("保存视频"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_nextmat_bottom);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("非换行下顶移动"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_nextmat_camera);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("非换行吸头移动"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_newline_pressbearing);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("换行物料移动"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_newline_bottom);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("换行下顶移动"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", len_newline_camera);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("换行吸头移动"), cstr, _T(CONFIG_FILE_PATH));

	cstr.Format("%d", i_speed_up_down_len);
	::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("电机加减速补偿"), cstr, _T(CONFIG_FILE_PATH));

	if (g_brecord == false)
	{
		bool_restart = 0;
		cstr.Format("%d", bool_restart);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("重启开机是否有数据记录"), cstr, _T(CONFIG_FILE_PATH));
	}
	else {
		bool_restart = 1;
		cstr.Format("%d", bool_restart);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("重启开机是否有数据记录"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_availablemat_clos);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录运行完的脚垫行数"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_clos_6);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录参数clos_6"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_clos_5);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录参数clos_5"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_livetime_camera_postion);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录吸头位置"), cstr, _T(CONFIG_FILE_PATH));

		cstr.Format("%d", record_livetime_bottom_postion);
		::WritePrivateProfileString(_T(CONFIG_APP_NAME), _T("记录下定位置"), cstr, _T(CONFIG_FILE_PATH));

	}

	return 0;
}

//比较当前运行的参数与配置文件的值
CString CAutoPadMatDlg::GetDiffConfig()
{
	bool is_config_change = false;
	CString retuen_cstr;

	CString cs_tmp;
	int itmp;
	float dtmp;

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("串口端口"), -1, _T(CONFIG_FILE_PATH));
	if (m_nCom != itmp)
	{
		cs_tmp.Format("串口:%d已更改为:%d。\r\n", itmp, m_nCom);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("串口波特率"), -1, _T(CONFIG_FILE_PATH));
	if (m_nBaud != itmp)
	{
		cs_tmp.Format("波特率:%d已更改为:%d。\r\n", itmp, m_nBaud);
		retuen_cstr += cs_tmp;
		is_config_change = true;

	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("图像中心点x坐标"), -1, _T(CONFIG_FILE_PATH));
	if (g_center_x != itmp)
	{
		cs_tmp.Format("图像中心点x坐标:%d已更改为:%d。\r\n", itmp, g_center_x);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("图像中心点y坐标"), -1, _T(CONFIG_FILE_PATH));
	if (g_center_y != itmp)
	{
		cs_tmp.Format("图像中心点y坐标:%d已更改为:%d。\r\n", itmp, g_center_y);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}


	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("贴脚垫的位置"), -1, _T(CONFIG_FILE_PATH));
	if (param_mat_postion != itmp)
	{
		cs_tmp.Format("贴脚垫的位置:%d已更改为:%d。\r\n", itmp, param_mat_postion);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}
	//::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("盒子两孔间距"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	//dtmp = atof(cs_tmp);
	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("第二个脚垫脉冲距离"), -1, _T(CONFIG_FILE_PATH));
	if (len_secondmat != itmp)
	{
		cs_tmp.Format("第二个脚垫的间距:%d已更改为:%d。\r\n", itmp, len_secondmat);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("第一个脚垫脉冲距离"), -1, _T(CONFIG_FILE_PATH));
	if (len_firstmat != itmp)
	{
		cs_tmp.Format("第一个脚垫脉冲距离:%d已更改为:%d。\r\n", itmp, len_firstmat);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("吸嘴到摄像头距离"), -1, _T(CONFIG_FILE_PATH));
	if (len_absorb_camera != itmp)
	{
		cs_tmp.Format("吸嘴到摄像头距离:%d已更改为:%d。\r\n", itmp, len_absorb_camera);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("吸取补偿偏移量"), -1, _T(CONFIG_FILE_PATH));
	if (len_absorb_mat != itmp)
	{
		cs_tmp.Format("吸取补偿偏移量:%d已更改为:%d。\r\n", itmp, len_absorb_mat);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("脚垫排数"), -1, _T(CONFIG_FILE_PATH));
	if (num_availablemat_clos != itmp)
	{
		cs_tmp.Format("脚垫排数:%d已更改为:%d。\r\n", itmp, num_availablemat_clos);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("模板匹配圆直径"), -1, _T(CONFIG_FILE_PATH));
	if (i_img_mask_d != itmp)
	{
		cs_tmp.Format("模板匹配圆直径:%d已更改为:%d。\r\n", itmp, i_img_mask_d);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("匹配圆直径粗细"), -1, _T(CONFIG_FILE_PATH));
	if (i_match_line != itmp)
	{
		cs_tmp.Format("模板匹配圆直径:%d已更改为:%d。\r\n", itmp, i_match_line);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("二值化阈值"), -1, _T(CONFIG_FILE_PATH));
	if (i_bin_thresh != itmp)
	{
		cs_tmp.Format("二值化阈值:%d已更改为:%d。\r\n", itmp, i_bin_thresh);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("模板匹配方法"), -1, _T(CONFIG_FILE_PATH));
	if (i_match_type != itmp)
	{
		cs_tmp.Format("模板匹配方法:%d已更改为:%d。\r\n", itmp, i_match_type);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("ROI区域X"), -1, _T(CONFIG_FILE_PATH));
	if (i_rect_img_x != itmp)
	{
		cs_tmp.Format("ROI区域X:%d已更改为:%d。\r\n", itmp, i_rect_img_x);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("ROI区域Y"), -1, _T(CONFIG_FILE_PATH));
	if (i_rect_img_y != itmp)
	{
		cs_tmp.Format("ROI区域Y:%d已更改为:%d。\r\n", itmp, i_rect_img_y);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("ROI区域W"), -1, _T(CONFIG_FILE_PATH));
	if (i_rect_img_w != itmp)
	{
		cs_tmp.Format("ROI区域W:%d已更改为:%d。\r\n", itmp, i_rect_img_w);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("ROI区域H"), -1, _T(CONFIG_FILE_PATH));
	if (i_rect_img_h != itmp)
	{
		cs_tmp.Format("ROI区域H:%d已更改为:%d。\r\n", itmp, i_rect_img_h);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("检测延时"), -1, _T(CONFIG_FILE_PATH));
	if (i_sleep_ms != itmp)
	{
		cs_tmp.Format("检测延时:%d已更改为:%d。\r\n", itmp, i_sleep_ms);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("X轴像素和脉冲参数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if (param_pixel_pulse_x != dtmp)
	{
		cs_tmp.Format("X轴像素和脉冲参数:%f已更改为:%f。\r\n", dtmp, param_pixel_pulse_x);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	::GetPrivateProfileString(_T(CONFIG_APP_NAME), _T("Y轴像素和脉冲参数"), _T("0.0"), cs_tmp.GetBuffer(MAX_PATH), MAX_PATH, _T(CONFIG_FILE_PATH));
	dtmp = atof(cs_tmp);
	if (param_pixel_pulse_y != dtmp)
	{
		cs_tmp.Format("Y轴像素和脉冲参数:%f已更改为:%f。\r\n", dtmp, param_pixel_pulse_y);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("复位后摄像头移动距离"), -1, _T(CONFIG_FILE_PATH));
	if (i_reset_move_up_len != itmp)
	{
		cs_tmp.Format("复位后摄像头移动距离:%d已更改为:%d。\r\n", itmp, i_reset_move_up_len);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("复位后下顶移动距离"), -1, _T(CONFIG_FILE_PATH));
	if (len_reset_bottom != itmp)
	{
		cs_tmp.Format("复位后下顶移动距离:%d已更改为:%d。\r\n", itmp, len_reset_bottom);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("保存视频"), -1, _T(CONFIG_FILE_PATH));
	if ((int)is_save_video != itmp)
	{
		cs_tmp.Format("保存视频:%d已更改为:%d。\r\n", itmp, is_save_video);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}



	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("非换行下顶移动"), -1, _T(CONFIG_FILE_PATH));
	if ((int)len_nextmat_bottom != itmp)
	{
		cs_tmp.Format("非换行下顶移动:%d已更改为:%d。\r\n", itmp, len_nextmat_bottom);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("非换行吸头移动"), -1, _T(CONFIG_FILE_PATH));
	if (len_nextmat_camera != itmp)
	{
		cs_tmp.Format("非换行吸头移动:%d已更改为:%d。\r\n", itmp, len_nextmat_camera);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("换行物料移动"), -1, _T(CONFIG_FILE_PATH));
	if (len_newline_pressbearing != itmp)
	{
		cs_tmp.Format("换行物料移动:%d已更改为:%d。\r\n", itmp, len_newline_pressbearing);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("换行下顶移动"), -1, _T(CONFIG_FILE_PATH));
	if (len_newline_bottom != itmp)
	{
		cs_tmp.Format("换行下顶移动:%d已更改为:%d。\r\n", itmp, len_newline_bottom);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("换行吸头移动"), -1, _T(CONFIG_FILE_PATH));
	if (len_newline_camera != itmp)
	{
		cs_tmp.Format("换行吸头移动:%d已更改为:%d。\r\n", itmp, len_newline_camera);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	itmp = ::GetPrivateProfileInt(_T(CONFIG_APP_NAME), _T("电机加减速补偿"), -1, _T(CONFIG_FILE_PATH));
	if (i_speed_up_down_len != itmp)
	{
		cs_tmp.Format("电机加减速补偿:%d已更改为:%d。\r\n", itmp, i_speed_up_down_len);
		retuen_cstr += cs_tmp;
		is_config_change = true;
	}

	if (is_config_change)
	{
		return retuen_cstr;
	}
	else {
		return "null";
	}
}

//设置波特率的函数
int BaudSet(int m )
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

// CAutoPadMatDlg 消息处理程序
BOOL CAutoPadMatDlg::OnInitDialog()
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

	
	//串口
	CString cstr = InitConfig();
	if (cstr != "null")
	{
		cstr += "\r\n警告：由于配置文件原因，目前参数不适合机器运行！";
		AfxMessageBox(cstr);
	}
	g_center_x = i_rect_img_w / 2;
	g_center_y = i_rect_img_h / 2;
	SysInit();

	m_cParity = 'N';
	m_nDatabits = 8;
	m_nStopbits = 1;

	m_SetCom.SetCurSel(m_nCom -1);
	m_SetBaud.SetCurSel(BaudSet(m_nBaud));
	m_MatPostion.SetCurSel(param_mat_postion -1);

	m_hIconRed = AfxGetApp()->LoadIcon(IDI_ICON_ON);
	m_hIconOff = AfxGetApp()->LoadIcon(IDI_ICON_OFF);
	m_PICSET.SetIcon(m_hIconOff);

	//步进滑动条
	m_sliderBJ.SetRange(1, 255);
	m_sliderBJ.SetTicFreq(1);
	m_sliderBJ.SetPos(i_slider_num);

	//吸嘴微调X滑动条
	m_slider_lenSC.SetRange(200, 400);
	m_slider_lenSC.SetTicFreq(1);
	m_slider_lenSC.SetPos(len_absorb_camera);
	cstr.Format("%d", len_absorb_camera);
	GetDlgItem(IDC_STATIC_CS)->SetWindowText(cstr);

	//吸嘴Y滑动条
	m_slider_lenY.SetRange(-30, 30);
	m_slider_lenY.SetTicFreq(1);
	m_slider_lenY.SetPos(len_absorb_mat);
	cstr.Format("%d", len_absorb_mat);
	GetDlgItem(IDC_STATIC_Y)->SetWindowText(cstr);
	m_DATDSEND = "";
	m_bOpenPort = false;
	ShowWindow(SW_SHOW);
	is_detect_running = true;
	CreateThread(NULL, 0, DetectCircleThread, NULL, 0, NULL);

	//打开串口
	OnBnClickedButtonOpenport();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CAutoPadMatDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAutoPadMatDlg::OnPaint()
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
HCURSOR CAutoPadMatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);

//这是接受数据的接口
LRESULT CAutoPadMatDlg::OnCommunication(WPARAM ch, LPARAM port)
{
	if (ch == 196)//判断进料
	{
		g_bput = true;
	}

	if (ch == 195) //c3 
	{
		g_bC3get = true;
	}

	if (ch == 80) // 判断是否有料吸起
	{
		g_bpressure = true;
	}
	if (ch == 5)
	{
		g_bpressure = false;
	}

	if (port <= 0 || port > 11)
		return -1;

	//i_rx_count++;   //接收的字节计数
	//CString strTemp;
	//strTemp.Format("%ld", i_rx_count);
	//strTemp = "RX:" + strTemp;
	//m_ctrlRXCOUNT.SetWindowText(strTemp);  //显示接收计数

	//if (m_bStopDispRXData)   //如果选择了“停止显示”接收数据，则返回
	//	return -1;          //注意，这种情况下，计数仍在继续，只是不显示
	//						//若设置了“自动清空”，则达到8行后，自动清空接收编辑框中显示的数据
	if (m_DATASHOW.GetLineCount() >= 10)
	{
		m_DATASHOWSTR.Empty();
		UpdateData(FALSE);
	}
	//如果没有“自动清空”，数据行达到400后，也自动清空
	//因为数据过多，影响接收速度，显示是最费CPU时间的操作
	if (m_DATASHOW.GetLineCount()>400)
	{
		m_DATASHOWSTR.Empty();
		UpdateData(FALSE);
	}

	//如果选择了"十六进制显示"，则显示十六进制值
	CString str;
	
	str.Format("%02X ", ch);

		is_com_recv_data = true;
		if (ch == 80 || ch == 235 || ch == 196 || ch == 195 || ch == 193 ||ch == 197)
		{
			//cstr_com_recv_string = str;
		}
		else
		{
			cstr_com_recv_string = str;
		}

		if (ch != 171)
		{
			SaveReceiveLog(str.GetBuffer());
			str.ReleaseBuffer();
		}
	

	//以下是将接收的字符加在字符串的最后，这里费时很多
	//但考虑到数据需要保存成文件，所以没有用List Control
	int nLen = m_DATASHOW.GetWindowTextLength();
	m_DATASHOW.SetSel(nLen, nLen);
	m_DATASHOW.ReplaceSel(str);
	nLen += str.GetLength();

	m_DATASHOWSTR += str;
	return 0;
}

//手动发送
void CAutoPadMatDlg::OnBnClickedButtonSend()
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
		int len = Str2ToHex(m_DATDSEND, data); // _t_Str2Hex  Str2Hex
		serialport.WriteToPort(data, len);
		//i_tx_count += (long)((m_strSendData.GetLength() + 1) / 3);
		//SaveSendLog(m_strSendData.GetBuffer());
		m_DATDSEND.ReleaseBuffer();
	

	/*	CString strTemp;
		strTemp.Format("TX:%d", i_tx_count);
		cst_ctrl_i_tx_count.SetWindowText(strTemp);*/
	}
}

//打开串口
void CAutoPadMatDlg::OnBnClickedButtonOpenport()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bOpenPort)  //关闭串口
	{
		m_PORTOPEN.SetWindowText("打开串口");
		serialport.ClosePort();//关闭串口
		//m_ctrlPortStatus.SetWindowText("状态：串口已关闭");
		m_PICSET.SetIcon(m_hIconOff);
		m_bOpenPort = false;
	}
	else  //打开串口
	{
		CString strStatus;
		if (serialport.InitPort(this, m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits, EV_RXFLAG | EV_RXCHAR, 512))
		{
			serialport.StartMonitoring();
			m_PICSET.SetIcon(m_hIconRed);
			strStatus.Format("状态：COM%d 打开，%d,%c,%d,%d", m_nCom, m_nBaud, m_cParity, m_nDatabits, m_nStopbits);
			//"当前状态：串口打开，无奇偶校验，8数据位，1停止位");
		}
		else
		{
			AfxMessageBox("没有发现此串口或被占用");
			m_PICSET.SetIcon(m_hIconOff);
			return;
		}
		m_PORTOPEN.SetWindowText("关闭串口");
		//m_ctrlPortStatus.SetWindowText(strStatus);
		m_bOpenPort = true;
	}
}

//com 下拉
void CAutoPadMatDlg::OnCbnSelendokComboComselect()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nCom = m_SetCom.GetCurSel() + 1;
}

//波特率下拉
void CAutoPadMatDlg::OnCbnSelendokComboSpeed()
{
	// TODO: 在此添加控件通知处理程序代码
	int i = m_SetBaud.GetCurSel();
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

//开启圆心检测
void CAutoPadMatDlg::OnBnClickedVideodetect()
{
	// TODO: 在此添加控件通知处理程序代码
	if (is_detect_running)
	{
		is_detect_running = false;
		destroyAllWindows();
		GetDlgItem(IDC_VIDEODETECT)->SetWindowText("开始检测");
	}
	else {
		is_detect_running = true;
		CreateThread(NULL, 0, DetectCircleThread, NULL, 0, NULL);
		GetDlgItem(IDC_VIDEODETECT)->SetWindowText("停止检测");
	}
}

 //控制台
void CAutoPadMatDlg::OnBnClickedVideoSetparam()
{
	// TODO: 在此添加控件通知处理程序代码
	InitControlBoard();
}

//暂停按钮
void CAutoPadMatDlg::OnBnClickedBtnStop()
{

	if (is_cmd_running)
	{
		is_cmd_running = false;
		GetDlgItem(IDC_BTN_STOP)->SetWindowText("点击继续");
	}
	else {
		is_cmd_running = true;
		GetDlgItem(IDC_BTN_STOP)->SetWindowText("点击暂停");
	}
}


void CAutoPadMatDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString cstrtmp = GetDiffConfig();
	if (cstrtmp == "null")
	{
		if (::MessageBox(NULL, "关闭程序？", "提示", MB_YESNO) == IDNO)
		{ 
			/*if (true == is_SAVE_config)
			{
				SavePartConfig();
			}		*/
			return;
		}
		if (true == is_SAVE_config)
		{
	     	SavePartConfig();
		}		
	}
	else {
		int id = ::MessageBox(NULL, cstrtmp, "保存配置并关闭程序？", MB_YESNOCANCEL);
		switch (id)
		{
		case IDYES:
			SaveConfig();
			break;
		case IDNO:
			if (true == is_SAVE_config)
			{
				SavePartConfig();
			}
			break;
		case IDCANCEL:
			return;
		default:
			break;
		}
	}

	
	//is_show_y_position = false;
	is_detect_running = false;
	destroyAllWindows();

	ofs_com_log.close();
	ofs_send_log.close();
	ofs_point_log.close();
	CDialogEx::OnClose();
}


void CAutoPadMatDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CString cstr;

	//步径
	i_slider_num = m_sliderBJ.GetPos();
	cstr.Format("%d", i_slider_num);
	GetDlgItem(IDC_STATIC_BUJI)->SetWindowText(cstr);

	//吸头和摄像头
	len_absorb_camera = m_slider_lenSC.GetPos();
	cstr.Format("%d", len_absorb_camera);
	GetDlgItem(IDC_STATIC_CS)->SetWindowText(cstr);

	//吸头和脚垫
	len_absorb_mat = m_slider_lenY.GetPos();
	cstr.Format("%d", len_absorb_mat);
	GetDlgItem(IDC_STATIC_Y)->SetWindowText(cstr);

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CAutoPadMatDlg::OnBnClickedBtnA3()
{
	// TODO: 在此添加控件通知处理程序代码
		char str[20];
		sprintf(str, "EBA301%02XBE", i_slider_num);
		CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnA4()
{
	// TODO: 在此添加控件通知处理程序代码
	char str[20];
	sprintf(str, "EBA401%02XBE", i_slider_num);
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnLeft()
{
	// TODO: 在此添加控件通知处理程序代码
	char str[20];
	sprintf(str, "EBB601%02XBE", i_slider_num);
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnRight()
{
	// TODO: 在此添加控件通知处理程序代码
	char str[20];
	sprintf(str, "EBB501%02XBE", i_slider_num);
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnB2()
{
	// TODO: 在此添加控件通知处理程序代码
	char str[20];
	sprintf(str, "EBB301%02XBE", i_slider_num);
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnB3()
{
	// TODO: 在此添加控件通知处理程序代码
	char str[20];
	sprintf(str, "EBB201%02XBE", i_slider_num);
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnCup()
{
	// TODO: 在此添加控件通知处理程序代码
	char * str = CAMERA_DOWN;
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnCdown()
{
	// TODO: 在此添加控件通知处理程序代码
	char * str = CAMERA_UP;
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnAiropen()
{
	// TODO: 在此添加控件通知处理程序代码
	char * str = AIR_OPEN;
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnAirdown()
{
	// TODO: 在此添加控件通知处理程序代码
	char * str = AIR_CLOSE;
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnXup()
{
	// TODO: 在此添加控件通知处理程序代码
	char * str = MAT_UP;
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


void CAutoPadMatDlg::OnBnClickedBtnXdown()
{
	// TODO: 在此添加控件通知处理程序代码
	char * str = MAT_DOWN;
	CreateThread(NULL, 0, SendCmdThread, str, 0, NULL);
}


bool VideoAdjustRoughly()
{
	Sleep(i_sleep_ms);

	int err_x = center.x - g_center_x;
	int err_y = center.y - g_center_y;

	float fx = (float)err_x / param_pixel_pulse_x;//脚垫方向 f_step_pram_x
	float fy = (float)err_y / param_pixel_pulse_y;//吸头方向 f_step_pram_y

												  //四舍五入到实际步进距离
	int err_trans_x = round(fx);
	int err_trans_y = round(fy);

	char str[32];
	char str_d[32];//下顶微调

	if (err_trans_x < 0)
	{
		sprintf(str, "EBB501%02XBE", -err_trans_x);
		SendCmd(str);
	}
	else if (err_trans_x > 0)
	{
		sprintf(str, "EBB601%02XBE", err_trans_x);
		SendCmd(str);
	}

	if (err_trans_y < 0)
	{
		sprintf(str, "EBA401%02XBE", -err_trans_y);
		sprintf(str_d, "EBB201%02XBE", -err_trans_y);
		SendCmd(str);
		SendCmd(str_d);
	}
	else if (err_trans_y > 0)
	{
		sprintf(str, "EBA301%02XBE", err_trans_y);
		sprintf(str_d, "EBB301%02XBE", err_trans_y);
		SendCmd(str);
		SendCmd(str_d);
	}
	glive_postion = glive_postion + err_trans_y * 4;
	glive_bottom_postion = glive_bottom_postion - err_trans_y;
	return true;
}

bool VideoAdjustment()
{
	//bool is_save_img = false;
	int err_x = 0;
	int err_y = 0;
	char str[32];
	char str_d[32];   //下顶微调

	for (int times = 0, cout_t = 0; times<10; times++)
	{
		Sleep(i_sleep_ms);
		err_x = center.x - g_center_x;
		err_y = center.y - g_center_y;
		point_test[0] = center;
		/*char log_str[256];
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		sprintf(log_str, "%4d/%02d/%02d %02d:%02d:%02d.%03d err_x:%02d, err_y:%02d\n", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, err_x, err_y);
		ofs_point_log << log_str;*/

		char str[32];
		char str_d[32];   //下顶微调

		if (abs(err_x) > 10 || abs(err_y) > 10)
		{
			Sleep(i_sleep_ms);
			err_x = center.x - g_center_x;
			err_y = center.y - g_center_y;
			point_test[1] = center;
			if (abs(err_x) > 10 || abs(err_y) > 10)
			{
				Sleep(i_sleep_ms);
				err_x = center.x - g_center_x;
				err_y = center.y - g_center_y;
				point_test[2] = center;

				if (Detected_circleStable(point_test) != false)
				{
					//AfxMessageBox(_T("错误，第二次调的太大了！"));
					Sleep(300);
					cout_t++;
					if (cout_t == 2)
					{
						return false;
					}
					continue;
				}

			}
		}

		float fx = (float)err_x / param_pixel_pulse_x;//脚垫方向f_step_pram_x
		float fy = (float)err_y / param_pixel_pulse_y;//吸头方向

													  //四舍五入到实际步进距离
		int err_trans_x = round(fx);
		int err_trans_y = round(fy);
		if (err_trans_x < 0)
		{
			sprintf(str, "EBB501%02XBE", -err_trans_x);
			SendCmd(str);
		}
		else if (err_trans_x > 0)
		{
			sprintf(str, "EBB601%02XBE", err_trans_x);
			SendCmd(str);
		}

		if (err_trans_y < 0)
		{
			sprintf(str, "EBA401%02XBE", -err_trans_y);
			sprintf(str_d, "EBB201%02XBE", -err_trans_y);
			SendCmd(str);
			SendCmd(str_d);
		}
		else if (err_trans_y > 0)
		{
			sprintf(str, "EBA301%02XBE", err_trans_y);
			sprintf(str_d, "EBB301%02XBE", err_trans_y);
			SendCmd(str);
			SendCmd(str_d);
		}
		glive_postion = glive_postion + err_trans_y * 4;
		glive_bottom_postion = glive_bottom_postion - err_trans_y;


		//微调两次
		Sleep(i_sleep_ms/2);
		err_x = center.x - g_center_x;
		err_y = center.y - g_center_y;
		if (abs(err_x)>2 )
		{
			fx = (float)err_x / param_pixel_pulse_x; 
			err_trans_x = round(fx);
			if (err_trans_x < 0)
			{
				sprintf(str, "EBB501%02XBE", -err_trans_x);
				SendCmd(str);
			}
			else if (err_trans_x > 0)
			{
				sprintf(str, "EBB601%02XBE", err_trans_x);
				SendCmd(str);
			}
		}
		if (abs(err_y)>2)
		{
			fy = (float)err_y / param_pixel_pulse_y;
			err_trans_y = round(fy);
			if (err_trans_y < 0)
			{
				sprintf(str, "EBA401%02XBE", -err_trans_y);
				sprintf(str_d, "EBB201%02XBE", -err_trans_y);
				SendCmd(str);
				SendCmd(str_d);
			}
			else if (err_trans_y > 0)
			{
				sprintf(str, "EBA301%02XBE", err_trans_y);
				sprintf(str_d, "EBB301%02XBE", err_trans_y);
				SendCmd(str);
				SendCmd(str_d);
			}
			glive_postion = glive_postion + err_trans_y * 4;
			glive_bottom_postion = glive_bottom_postion - err_trans_y;
		}

		break;

	}
	return true;
}

//开启一个线程等待C3的回应
DWORD WINAPI WaitForC3(LPVOID pParam)
{
	while (1)
	{
		if (g_bC3get == true)
		{
			g_bC3get = false;
			//ForceSendCmd(BOX_STATUS_FREE);
		}
		Sleep(20);
	}
	return 0;
}

//一个综合的程序用来跑完所有的
DWORD WINAPI IJUSTWANTTODIE(LPVOID pParam)
{  
	//g_ForceNext = false;
	char cmdstr[30];
	char str_cmd[40];
	char str_dis[32];
	g_center_x = i_rect_img_w / 2;
	g_center_y = i_rect_img_h / 2;

	glive_postion = i_reset_move_up_len;
	//判断是用来贴哪一个脚垫的

	if (param_mat_postion == 1)
	{
		glive_postion = glive_postion - len_firstmat;
	}
	else {
		glive_postion = glive_postion - len_secondmat;
	}

	glive_bottom_postion = len_reset_bottom;
	//等待进料
	while (true)
	{  
		g_ForceNext = false;
		SendCmd(CAMERA_RESET);
		SendCmd(PROP_RESET);
		if (g_StartNext == false)
		{
			SendCmd(BOX_RESET);
			SendCmd(BOX_STATUS_BUSY);
		}
		
		SendCmd(PRESS_UPORDOWN);
		Sleep(2000);
		SendCmd(LIGHT_RED);

		while (g_bmateriel)
		{
			Sleep(20);
		}
		g_bmateriel = true;
		SendCmd(LIGHT_GREEN);

		SendCmd(PRESS_UPORDOWN);
		Sleep(3000);
		// 进料检测
		sprintf(str_cmd, "EBA102%04XBE", i_reset_move_up_len);
		SendCmd(str_cmd);

		//摄像头移动到固定位置
		int time = len_reset_bottom / 255;
		for (int bottom_times = 0; bottom_times < time; bottom_times++)
		{
			SendCmd("EBB201FFBE");
		}
		sprintf(cmdstr, "EBB201%02XBE", len_reset_bottom % 255);
		SendCmd(cmdstr);

		Sleep(100);
		while (f_dark_quantity > -30)
		{
			SendCmd("EBB50120BE");
			Sleep(50);
		}
		SendCmd("EBB50160BE");
		SendCmd(LIGHT_GREEN);
		//寻找圆心
		//while (true)
		//{
		//	//计算n帧坐标方差，判断检测结果是否稳定
		//	int  frame_n = 8;
		//	int sum_x = 0;
		//	int sum_x_2 = 0;
		//	int sum_y = 0;
		//	int sum_y_2 = 0;
		//	int average_x = 0;
		//	int average_y = 0;
		//	int variance_x = 0;
		//	int variance_y = 0;
		//	Sleep(i_sleep_ms / 2);
		//	for (int i = 0; i < frame_n; i++)
		//	{
		//		Sleep(i_sleep_ms / 2);
		//		sum_x += (center.x - g_center_x);
		//		sum_x_2 += (center.x - g_center_x) *(center.x - g_center_x);
		//		sum_y += (center.y - g_center_y);
		//		sum_y_2 += (center.y - g_center_y) *(center.y - g_center_y);
		//		average_x = sum_x / (i + 1);
		//		average_y = sum_y / (i + 1);
		//		variance_x = sum_x_2 / (i + 1) - (sum_x / (i + 1)) * (sum_x / (i + 1));
		//		variance_y = sum_y_2 / (i + 1) - (sum_y / (i + 1)) * (sum_y / (i + 1));
		//		if (variance_x > 10 || variance_y > 50)
		//		{
		//			SendCmd("EBB50ABE");
		//			break;
		//		}
		//	}
		//	average_x = sum_x / frame_n;
		//	average_y = sum_y / frame_n;
		//	variance_x = sum_x_2 / frame_n - (sum_x / frame_n) * (sum_x / frame_n);
		//	variance_y = sum_y_2 / frame_n - (sum_y / frame_n) * (sum_y / frame_n);
		//	if (variance_x < 10 && variance_y<50 && center.x > g_center_x - 5 && center.x<g_center_x + 5 && center.y > g_center_y - 35 && center.y < g_center_y + 35)
		//	{
		//		SendCmd("EBB5FABE");
		//		break;
		//	}
		//	else {
		//		SendCmd("EBB508BE");
		//	}
		//}

		Point isSame;
		Point temp;
		//判断圆心是否稳定
		while (1)
		{
			int cout = 0;
			//强行下一张
			if (g_ForceNext == true)
			{
				break;
			}
			while (true)
			{ 
				//强行下一张
				if (g_ForceNext == true)
				{
					break;
				}
				isSame = center;
				Sleep(i_sleep_ms / 2);
				temp = center;
				if (abs(isSame.x - temp.x) > 5 || abs(isSame.y - temp.y) > 5)
				{
					SendCmd("EBB50113BE");
					cout = 0;
					continue;
				}
				cout++;
				isSame = temp;
				if (cout > 8)
				{
					break;
				}
			}
			//判断圆心在哪里
			if (abs(center.x - g_center_x) > 20)
			{
				SendCmd("EBB50113BE");
				continue;
			}
			Sleep(20);
			//强行下一张
			if (g_ForceNext == true)
			{
				break;
			}

			for (int times = 0; times < 2; times++)
			{
				VideoAdjustRoughly();
				Sleep(20);
			}

			if (abs(center.x - g_center_x)>4 || abs(center.y - g_center_y)>4)
			{
				SendCmd("EBB5011DBE");
				continue;
			}

			Sleep(20);
			for (int w = 0; w < 3; w++)
			{
				Sleep(i_sleep_ms / 2);
				point_test[w] = center;
			}

			if (Detected_circleStable(point_test) == true )
			{
				SendCmd("EBB50113BE");
				continue;
			}
			/*if (abs(center.x - g_center_x) > 3 || abs(center.y - g_center_y)>3)
			{
			SendCmd("EBB510BE");
			continue;
			}*/
			SendCmd("EBB501F0BE");
			break;
		}

		//强行下一张 继续循环测试
		if (g_ForceNext == true)
		{
			continue;
		}
		Sleep(20);
		//开始运行吸取脚垫
		if (g_StartNext == false)
		{
			ForceSendCmd(BOX_STATUS_FREE);
		}
	   
		Sleep(20);
		//吸取脚垫的循环
		for (int i = 0; i < num_availablemat_clos; i++)
		{  
			//强行下一张 继续循环测试
			if (g_ForceNext == true)
			{
				break;
			}
			record_availablemat_clos = i;
			Sleep(50);
			for (cols_6 = 0; cols_6 < 6; cols_6++)
			{
				//强行下一张 继续循环测试
				if (g_ForceNext == true)
				{
					break;
				}
				VideoAdjustment();
				record_livetime_bottom_postion = glive_bottom_postion;
				record_livetime_camera_postion = glive_postion;
				record_clos_6 = cols_6;
				g_brecord = true;
				Sleep(100);
				//吸头对准
				sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);
				SendCmd(str_cmd);
				glive_postion = glive_postion + len_absorb_camera * 4; //距离实时改变

			   // 加上摄像头和脚垫之间的微调
				if (len_absorb_mat > 0)
				{
					sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
					SendCmd(str_cmd);
				}
				else if (len_absorb_mat < 0)
				{
					sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
					SendCmd(str_cmd);
				}

				SendCmd(AIR_OPEN); //吸气
				SendCmd(CAMERA_UP);
				SendCmd(MAT_UP); //下顶顶起
				Sleep(20);
				SendCmd(MAT_DOWN);//下顶落下
				Sleep(20);
				SendCmd(CAMERA_DOWN);
				Sleep(20);
				//检测脚垫是否正确吸取
				SendCmd(AIR_PRESSURE_CHECK);
				Sleep(100);
				if (g_bpressure == false)   //如果不是做异常处理
				{
					Sleep(200);
					SendCmd(CAMERA_UP);
					SendCmd(MAT_UP); //下顶顶起
					Sleep(50);
					SendCmd(MAT_DOWN);//下顶落下
					Sleep(20);
					SendCmd(CAMERA_DOWN);
					Sleep(20);
					SendCmd(AIR_PRESSURE_CHECK);
					Sleep(100);
					if (g_bpressure == false)
					{
						SendCmd(LIGHT_RED);
						SendCmd(AIR_CLOSE);
						sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
						SendCmd(str_cmd);
						glive_postion = glive_postion - len_absorb_camera * 4;
						AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
						Sleep(200);
						SendCmd(LIGHT_GREEN);
						Sleep(200);
					}
				}

				//在让脚垫回去
				if (len_absorb_mat > 0)
				{
					sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
					SendCmd(str_cmd);
				}
				else if (len_absorb_mat < 0)
				{
					sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
					SendCmd(str_cmd);
				}

				if (g_bpressure == false)
				{
					if (cols_6 == 5)  //如果在边界出错
					{
						sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing); //物料前进
						SendCmd(str_cmd);
						sprintf(str_cmd, "EBA301%02XBE", len_newline_camera); //摄像头到下一个
						SendCmd(str_cmd);
						glive_postion = glive_postion + len_newline_camera * 4;
						record_livetime_camera_postion = glive_postion;
						glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
						record_livetime_bottom_postion = glive_bottom_postion;
						record_clos_6 = cols_6 + 1;
						sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
						SendCmd(str_dis);
						continue;
					}
					else {
						sprintf(str_cmd, "EBA401%02XBE", len_nextmat_camera); //摄像头到下一个
						SendCmd(str_cmd);
						glive_postion = glive_postion - len_nextmat_camera * 4;
						record_livetime_camera_postion = glive_postion;
						glive_bottom_postion = glive_bottom_postion + len_nextmat_bottom;
						record_livetime_bottom_postion = glive_bottom_postion;
						record_clos_6 = cols_6 + 1;
						sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
						SendCmd(str_dis);
						continue;
					}
				}

				//运动到盒子	
				sprintf(str_cmd, "EBA202%04XBE", glive_postion);
				SendCmd(str_cmd);

				while (!g_bput)
				{
				Sleep(20);
				}
				g_bput = false;

				SendCmd(CAMERA_UP);
				SendCmd(AIR_CLOSE);//放气
				Sleep(300);//为了压一下脚垫
				SendCmd(CAMERA_DOWN);
				Sleep(20);
				ForceSendCmd(BOX_OUT);
				Sleep(20);
				//SendCmd(BOX_RESET);
				//SendCmd(BOX_STATUS_FREE);
				//返回下一个位置
				if (cols_6 == 5)  //到达边界就要处理异常
				{
					sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
					SendCmd(str_cmd);

					/*	sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
					SendCmd(str);*/

					glive_postion = glive_postion - len_absorb_camera * 4 + len_newline_camera * 4;
					sprintf(str_cmd, "EBA102%04XBE", glive_postion);
					SendCmd(str_cmd);
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_6 = cols_6 + 1;
					sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
					SendCmd(str_dis);
				}
				else {
					//sprintf(str, "EBA401%02XBE", i_sourse_next_len);
					//SendCmd(str);
					//g_longlen = g_longlen - i_sourse_next_len - distance_vs;
					glive_postion = glive_postion - len_absorb_camera * 4 - len_nextmat_camera * 4;
					sprintf(str_cmd, "EBA102%04XBE", glive_postion);
					SendCmd(str_cmd);
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion + len_nextmat_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_6 = cols_6 + 1;
					sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
					SendCmd(str_dis);
				}
			}
			//强行下一张 继续循环测试
			if (g_ForceNext == true)
			{
				break;
			}
			record_clos_6 = cols_6;
			//第二排5个
			for (cols_5 = 0; cols_5 < 5; cols_5++)
			{
				//强行下一张 继续循环测试
				if (g_ForceNext == true)
				{
					break;
				}
				VideoAdjustment();
				record_livetime_camera_postion = glive_postion;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_5 = cols_5;
				Sleep(100);
				//吸头对准
				sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);
				SendCmd(str_cmd);
				glive_postion = glive_postion + len_absorb_camera * 4;
				// 加上摄像头和脚垫之间的微调
				if (len_absorb_mat > 0)
				{
					sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
					SendCmd(str_cmd);
				}
				else if (len_absorb_mat < 0)
				{
					sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
					SendCmd(str_cmd);
				}

				SendCmd(AIR_OPEN); //吸气
				SendCmd(CAMERA_UP);
				SendCmd(MAT_UP); //下顶顶起
				Sleep(50);
				SendCmd(MAT_DOWN);//下顶落下
				Sleep(20);
				SendCmd(CAMERA_DOWN);
				Sleep(20);
				//检测脚垫是否正确吸取
				SendCmd(AIR_PRESSURE_CHECK);
				Sleep(100);
				if (g_bpressure == false)   //如果不是做异常处理
				{
					Sleep(200);
					SendCmd(CAMERA_UP);
					SendCmd(MAT_UP); //下顶顶起
					Sleep(50);
					SendCmd(MAT_DOWN);//下顶落下
					Sleep(20);
					SendCmd(CAMERA_DOWN);
					SendCmd(AIR_PRESSURE_CHECK);
					Sleep(100);
					if (g_bpressure == false)
					{
						SendCmd(LIGHT_RED);
						SendCmd(AIR_CLOSE);
						sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
						SendCmd(str_cmd);
						glive_postion = glive_postion - len_absorb_camera * 4;
						AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
						Sleep(200);
						SendCmd(LIGHT_GREEN);
						Sleep(200);
					}
				}

				//在让脚垫回去
				if (len_absorb_mat > 0)
				{
					sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
					SendCmd(str_cmd);
				}
				else if (len_absorb_mat < 0)
				{
					sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
					SendCmd(str_cmd);
				}

				//没吸起来就到下一个
				if (g_bpressure == false)
				{
					if (cols_5 == 4)
					{
						sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
						SendCmd(str_cmd);

						sprintf(str_cmd, "EBA301%02XBE", len_newline_camera);
						SendCmd(str_cmd);
						glive_postion = glive_postion + len_newline_camera * 4;
						record_livetime_camera_postion = glive_postion;
						glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
						record_livetime_bottom_postion = glive_bottom_postion;
						record_clos_5 = cols_5 + 1;
						sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
						SendCmd(str_dis);
						continue;
					}
					else {
						sprintf(str_cmd, "EBA301%02XBE", len_nextmat_camera);
						SendCmd(str_cmd);
						glive_postion = glive_postion + len_nextmat_camera * 4;
						record_livetime_camera_postion = glive_postion;
						glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
						record_livetime_bottom_postion = glive_bottom_postion;
						record_clos_5 = cols_5 + 1;
						sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
						SendCmd(str_dis);
						continue;
					}
				}

				//运动到盒子
				sprintf(str_cmd, "EBA202%04XBE", glive_postion);
				SendCmd(str_cmd);

				while (!g_bput)
				{
				Sleep(20);
				}
				g_bput = false;

				SendCmd(CAMERA_UP);
				SendCmd(AIR_CLOSE); //放气
				Sleep(300); //为了压一下脚垫
				SendCmd(CAMERA_DOWN);
				Sleep(20);
				ForceSendCmd(BOX_OUT);
				Sleep(20);
				//SendCmd(BOX_RESET);
				/*SendCmd(BOX_OUT);
				SendCmd(BOX_STATUS_FREE);*/
				//运动到下一个脚垫
				if (cols_5 == 4)
				{
					sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
					SendCmd(str_cmd);

					//sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
					//SendCmd(str);

					glive_postion = glive_postion - len_absorb_camera * 4 + len_newline_camera * 4;
					sprintf(str_cmd, "EBA102%04XBE", glive_postion);
					SendCmd(str_cmd);
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_5 = cols_5 + 1;
					sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
					SendCmd(str_dis);

				}
				else {
					//sprintf(str, "EBA301%02XBE", i_sourse_next_len);
					//SendCmd(str);

					glive_postion = glive_postion - len_absorb_camera * 4 + len_nextmat_camera * 4;
					sprintf(str_cmd, "EBA102%04XBE", glive_postion);
					SendCmd(str_cmd);
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_5 = cols_5 + 1;
					sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
					SendCmd(str_dis);

				}

			}
			//强行下一张 继续循环测试
			if (g_ForceNext == true)
			{
				break;
			}
			record_clos_5 = cols_5;
		}
		
		//SendCmd(BOX_STATUS_BUSY);
		g_StartNext = true;
		//运行结束进入下一个循环
	}
	return 0;
}

//----------------------------一个用来记录位置并且执行的位置
DWORD WINAPI RunComputer(LPVOID pParam)
{  

	char str_cmd[40];
	char str_dis[32];
	SendCmd(CAMERA_RESET);
	SendCmd(PROP_RESET);
	SendCmd(BOX_RESET);
	//先移动到上次的位置 记得加上脚垫距离
	if (param_mat_postion == 1)
	{
		glive_postion = record_livetime_camera_postion + len_firstmat;
	}
	else {
		glive_postion = record_livetime_camera_postion + len_secondmat;
	}

	sprintf(str_cmd, "EBA102%04XBE", glive_postion); //摄像头移动到固定位置
	SendCmd(str_cmd);

	if (param_mat_postion == 1)
	{
		glive_postion = glive_postion - len_firstmat;
	}
	else {
		glive_postion = glive_postion - len_secondmat;
	}

	glive_bottom_postion = record_livetime_bottom_postion;
	int time = record_livetime_bottom_postion / 255;
	for (int bottom_times = 0; bottom_times < time; bottom_times++)
	{
		SendCmd("EBB201FFBE");
	}
	sprintf(str_dis, "EBB201%02XBE", record_livetime_bottom_postion % 255); //摄像头移动到固定位置
	SendCmd(str_dis);

	//再移动到下一个脚垫出
	cols_6 = record_clos_6;
	cols_5 = record_clos_5;
	if (cols_6 != 6)
	{

		if (cols_6 == 5)  //如果在边界出错
		{
			sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing); //物料前进
			SendCmd(str_cmd);
			sprintf(str_cmd, "EBA301%02XBE", len_newline_camera); //摄像头到下一个
			SendCmd(str_cmd);
			g_brecord = true;
			glive_postion = glive_postion + len_newline_camera * 4;
			record_livetime_camera_postion = glive_postion;
			glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_6 = cols_6 + 1;
			sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
			SendCmd(str_dis);
		}
		else {
			sprintf(str_cmd, "EBA401%02XBE", len_nextmat_camera); //摄像头到下一个
			SendCmd(str_cmd);
			g_brecord = true;
			glive_postion = glive_postion - len_nextmat_camera * 4;
			record_livetime_camera_postion = glive_postion;
			glive_bottom_postion = glive_bottom_postion + len_nextmat_bottom;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_6 = cols_6 + 1;
			sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
			SendCmd(str_dis);
		}
		cols_6++;
		record_clos_6 = cols_6;

	}
	else
	{
		if (cols_5 == 4)
		{
			sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
			SendCmd(str_cmd);
			sprintf(str_cmd, "EBA301%02XBE", len_newline_camera);
			SendCmd(str_cmd);
			g_brecord = true;
			glive_postion = glive_postion + len_newline_camera * 4;
			record_livetime_camera_postion = glive_postion;
			glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_5 = cols_5 + 1;
			sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
			SendCmd(str_dis);
		}
		else {
			sprintf(str_cmd, "EBA301%02XBE", len_nextmat_camera);
			SendCmd(str_cmd);
			g_brecord = true;
			glive_postion = glive_postion + len_nextmat_camera * 4;
			record_livetime_camera_postion = glive_postion;
			glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_5 = cols_5 + 1;
			sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
			SendCmd(str_dis);
		}
		cols_5++;
		record_clos_5 = cols_5;
	}

	//判断是否有脚垫
	Sleep(200);
	VideoAdjustment();
	record_livetime_bottom_postion = glive_bottom_postion;
	record_livetime_camera_postion = glive_postion;
	Sleep(200);
	if (abs(g_center_x - center.x)<5 && abs(g_center_y - center.y)<5)
	{
		Sleep(200);
	}
	else {
		AfxMessageBox(_T("错误,没有找到脚垫，请重新放入一张新脚垫！"));
		Sleep(200);
		//开启重新检测的线程
		CreateThread(NULL, 0, IJUSTWANTTODIE, NULL, 0, NULL);
		return 0;
	}


	//----------------------
	//------------开始吸取
	ForceSendCmd(BOX_STATUS_FREE);
	if (cols_6 != 6)
	{
		for (; cols_6 < 6; cols_6++)
		{
			VideoAdjustment();
			record_livetime_bottom_postion = glive_bottom_postion;
			record_livetime_camera_postion = glive_postion;
			record_clos_6 = cols_6;
			Sleep(100);
			//吸头对准
			sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);
			SendCmd(str_cmd);
			glive_postion = glive_postion + len_absorb_camera * 4; //距离实时改变

		   // 加上摄像头和脚垫之间的微调
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			SendCmd(AIR_OPEN); //吸气
			SendCmd(CAMERA_UP);
			SendCmd(MAT_UP); //下顶顶起
			Sleep(50);
			SendCmd(MAT_DOWN);//下顶落下
			Sleep(20);
			SendCmd(CAMERA_DOWN);

			//检测脚垫是否正确吸取
			SendCmd(AIR_PRESSURE_CHECK);
			Sleep(100);
			if (g_bpressure == false)   //如果不是做异常处理
			{
			  Sleep(200);
			  SendCmd(CAMERA_UP);
			  SendCmd(MAT_UP); //下顶顶起
			  Sleep(50);
			  SendCmd(MAT_DOWN);//下顶落下
			  Sleep(20);
			  SendCmd(CAMERA_DOWN);
			  SendCmd(AIR_PRESSURE_CHECK);
			  Sleep(100);
			  if (g_bpressure == false)
			  {  
				  SendCmd(LIGHT_RED);
				  SendCmd(AIR_CLOSE);
				  sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
				  SendCmd(str_cmd);
				  glive_postion = glive_postion - len_absorb_camera * 4;
				  AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
				  Sleep(200);
				  SendCmd(LIGHT_GREEN);
				  Sleep(200);
			   }
			 }

			//在让脚垫回去
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			if (g_bpressure == false)
			{
				if (cols_6 == 5)  //如果在边界出错
				{
					sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing); //物料前进
					SendCmd(str_cmd);
					sprintf(str_cmd, "EBA301%02XBE", len_newline_camera); //摄像头到下一个
					SendCmd(str_cmd);
					glive_postion = glive_postion  + len_newline_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
					record_livetime_bottom_postion =  glive_bottom_postion;
					record_clos_6 = cols_6 +1;
					sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
					SendCmd(str_dis);
					continue;
				}
				else {
					sprintf(str_cmd, "EBA401%02XBE", len_nextmat_camera); //摄像头到下一个
					SendCmd(str_cmd);
					glive_postion = glive_postion  - len_nextmat_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion +len_nextmat_bottom;
					record_livetime_bottom_postion =  glive_bottom_postion;
					record_clos_6 = cols_6 +1;
					sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
					SendCmd(str_dis);
					continue;
				}	
			}

			//运动到盒子	
			sprintf(str_cmd, "EBA202%04XBE", glive_postion);
			SendCmd(str_cmd);

			while (!g_bput)
			{
				Sleep(20);
			}
			g_bput = false;
			SendCmd(CAMERA_UP);
			SendCmd(AIR_CLOSE);//放气
			SendCmd(CAMERA_DOWN);
			Sleep(300);
			ForceSendCmd(BOX_OUT);
			Sleep(20);
			/*SendCmd(BOX_RESET);
			SendCmd(BOX_STATUS_FREE);*/
			//返回下一个位置
			if (cols_6 == 5)  //到达边界就要处理异常
			{
				sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
				SendCmd(str_cmd);

				/*	sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
				SendCmd(str);*/

				glive_postion = glive_postion - len_absorb_camera * 4 + len_newline_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_6 = cols_6 + 1;
				sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
				SendCmd(str_dis);

			}
			else {
				//sprintf(str, "EBA401%02XBE", i_sourse_next_len);
				//SendCmd(str);
				//g_longlen = g_longlen - i_sourse_next_len - distance_vs;
				glive_postion = glive_postion - len_absorb_camera * 4 - len_nextmat_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion + len_nextmat_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_6 = cols_6 + 1;
				sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
				SendCmd(str_dis);
			}
		}
		record_clos_6 = cols_6;

		//第二排5个
		for (cols_5 = 0; cols_5 < 5; cols_5++)
		{

			VideoAdjustment();
			record_livetime_camera_postion = glive_postion;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_5 = cols_5;
			Sleep(100);
			//吸头对准
			sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);
			SendCmd(str_cmd);
			glive_postion = glive_postion + len_absorb_camera * 4;
			// 加上摄像头和脚垫之间的微调
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			SendCmd(AIR_OPEN); //吸气
			SendCmd(CAMERA_UP);
			SendCmd(MAT_UP); //下顶顶起
			Sleep(50);
			SendCmd(MAT_DOWN);//下顶落下
			Sleep(20);
			SendCmd(CAMERA_DOWN);

			//检测脚垫是否正确吸取
			SendCmd(AIR_PRESSURE_CHECK);
			Sleep(100);
			if (g_bpressure == false)   //如果不是做异常处理
			{
				Sleep(200);
				SendCmd(CAMERA_UP);
				SendCmd(MAT_UP); //下顶顶起
				Sleep(50);
				SendCmd(MAT_DOWN);//下顶落下
				Sleep(20);
				SendCmd(CAMERA_DOWN);
				SendCmd(AIR_PRESSURE_CHECK);
				Sleep(100);
				if (g_bpressure == false)
				{
					SendCmd(LIGHT_RED);
					SendCmd(AIR_CLOSE);
					sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
					SendCmd(str_cmd);
					glive_postion = glive_postion - len_absorb_camera * 4;
					AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
					Sleep(200);
					SendCmd(LIGHT_GREEN);
					Sleep(200);
				}
			}

			//在让脚垫回去
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			//没吸起来就到下一个
			if (g_bpressure == false)
			{
					if (cols_5 == 4)
					{
					sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
					SendCmd(str_cmd);

					sprintf(str_cmd, "EBA301%02XBE", len_newline_camera);
					SendCmd(str_cmd);
					glive_postion = glive_postion  + len_newline_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_5 = cols_5+1;
					sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
					SendCmd(str_dis);
					continue;
					}
					else {
					sprintf(str_cmd, "EBA301%02XBE", len_nextmat_camera);
					SendCmd(str_cmd);
					glive_postion = glive_postion  + len_nextmat_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_5 = cols_5+1;
					sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
					SendCmd(str_dis);
					continue;
					}
			}

			//运动到盒子
			sprintf(str_cmd, "EBA202%04XBE", glive_postion);
			SendCmd(str_cmd);

			while (!g_bput)
			{
				Sleep(20);
			}
			g_bput = false;

			SendCmd(CAMERA_UP);
			SendCmd(AIR_CLOSE); //放气
			SendCmd(CAMERA_DOWN);
			Sleep(300);
			ForceSendCmd(BOX_OUT);
			Sleep(20);
			/*SendCmd(BOX_RESET);
			SendCmd(BOX_STATUS_FREE);*/
			//运动到下一个脚垫
			if (cols_5 == 4)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
				SendCmd(str_cmd);

				//sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
				//SendCmd(str);

				glive_postion = glive_postion - len_absorb_camera * 4 + len_newline_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_5 = cols_5 + 1;
				sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
				SendCmd(str_dis);

			}
			else {
				//sprintf(str, "EBA301%02XBE", i_sourse_next_len);
				//SendCmd(str);
				glive_postion = glive_postion - len_absorb_camera * 4 + len_nextmat_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_5 = cols_5 + 1;
				sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
				SendCmd(str_dis);
			}
		}
		record_clos_5 = cols_5;
	}
	else {
		for (; cols_5 < 5; cols_5++)
		{
			VideoAdjustment();
			record_livetime_camera_postion = glive_postion;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_5 = cols_5;
			Sleep(100);
			//吸头对准
			sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);
			SendCmd(str_cmd);
			glive_postion = glive_postion + len_absorb_camera * 4;
			// 加上摄像头和脚垫之间的微调
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			SendCmd(AIR_OPEN); //吸气
			SendCmd(CAMERA_UP);
			SendCmd(MAT_UP); //下顶顶起
			Sleep(50);
			SendCmd(MAT_DOWN);//下顶落下
			Sleep(20);
			SendCmd(CAMERA_DOWN);

			//检测脚垫是否正确吸取
			SendCmd(AIR_PRESSURE_CHECK);
			Sleep(100);
			if (g_bpressure == false)   //如果不是做异常处理
			{
				Sleep(200);
				SendCmd(CAMERA_UP);
				SendCmd(MAT_UP); //下顶顶起
				Sleep(50);
				SendCmd(MAT_DOWN);//下顶落下
				Sleep(20);
				SendCmd(CAMERA_DOWN);
				SendCmd(AIR_PRESSURE_CHECK);
				Sleep(100);
				if (g_bpressure == false)
				{
					SendCmd(LIGHT_RED);
					SendCmd(AIR_CLOSE);
					sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
					SendCmd(str_cmd);
					glive_postion = glive_postion - len_absorb_camera * 4;
					AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
					Sleep(200);
					SendCmd(LIGHT_GREEN);
					Sleep(200);
				}
			}

			//在让脚垫回去
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			//没吸起来就到下一个
			if (g_bpressure == false)
			{
					if (cols_5 == 4)
					{
					sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
					SendCmd(str_cmd);

					sprintf(str_cmd, "EBA301%02XBE", len_newline_camera);
					SendCmd(str_cmd);
					glive_postion = glive_postion  + len_newline_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_5 = cols_5 +1;
					sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
					SendCmd(str_dis);

					continue;
					}
					else {
					sprintf(str_cmd, "EBA301%02XBE", len_nextmat_camera);
					SendCmd(str_cmd);
					glive_postion = glive_postion  + len_nextmat_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_5 = cols_5 +1;
					sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
					SendCmd(str_dis);
					continue;
					}
			}

			//运动到盒子
			sprintf(str_cmd, "EBA202%04XBE", glive_postion);
			SendCmd(str_cmd);

			while (!g_bput)
			{
				Sleep(20);
			}
			g_bput = false;

			SendCmd(CAMERA_UP);
			SendCmd(AIR_CLOSE); //放气
			SendCmd(CAMERA_DOWN);
			Sleep(300);
			ForceSendCmd(BOX_OUT);
			Sleep(20);
		/*	SendCmd(BOX_RESET);
			SendCmd(BOX_STATUS_FREE);*/
			//运动到下一个脚垫
			if (cols_5 == 4)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
				SendCmd(str_cmd);

				//sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
				//SendCmd(str);

				glive_postion = glive_postion - len_absorb_camera * 4 + len_newline_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_5 = cols_5 + 1;
				sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
				SendCmd(str_dis);

			}
			else {
				//sprintf(str, "EBA301%02XBE", i_sourse_next_len);
				//SendCmd(str);
				glive_postion = glive_postion - len_absorb_camera * 4 + len_nextmat_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_5 = cols_5 + 1;
				sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
				SendCmd(str_dis);
			}
		}

		record_clos_5 = cols_5;
	}

	//吸取剩下的脚垫
	for (int i = record_availablemat_clos + 1; i < num_availablemat_clos; i++)
	{  
		//强行下一张 继续循环测试
		if (g_ForceNext == true)
		{
			break;
		}
		record_availablemat_clos = i;
		Sleep(50);
		for (cols_6 = 0; cols_6 < 6; cols_6++)
		{ 
			//强行下一张 继续循环测试
			if (g_ForceNext == true)
			{
				break;
			}
			VideoAdjustment();
			record_livetime_bottom_postion = glive_bottom_postion;
			record_livetime_camera_postion = glive_postion;
			record_clos_6 = cols_6;
			Sleep(100);
			//吸头对准
			sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);
			SendCmd(str_cmd);
			glive_postion = glive_postion + len_absorb_camera * 4; //距离实时改变

																   // 加上摄像头和脚垫之间的微调
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			SendCmd(AIR_OPEN); //吸气
			SendCmd(CAMERA_UP);
			SendCmd(MAT_UP); //下顶顶起
			Sleep(50);
			SendCmd(MAT_DOWN);//下顶落下
			Sleep(20);
			SendCmd(CAMERA_DOWN);

			//检测脚垫是否正确吸取
			SendCmd(AIR_PRESSURE_CHECK);
			Sleep(100);
			if (g_bpressure == false)   //如果不是做异常处理
			{
			  Sleep(200);
			  SendCmd(CAMERA_UP);
			  SendCmd(MAT_UP); //下顶顶起
			  Sleep(50);
			  SendCmd(MAT_DOWN);//下顶落下
			  Sleep(20);
			  SendCmd(CAMERA_DOWN);
			  SendCmd(AIR_PRESSURE_CHECK);
			  Sleep(100);
			  if (g_bpressure == false)
			  {  
				  SendCmd(LIGHT_RED);
				  SendCmd(AIR_CLOSE);
				  sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
				  SendCmd(str_cmd);
				  glive_postion = glive_postion - len_absorb_camera * 4;
				  AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
				  Sleep(200);
				  SendCmd(LIGHT_GREEN);
				  Sleep(200);
			   }
			 }

			//在让脚垫回去
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			if (g_bpressure == false)
			{
				if (cols_6 == 5)  //如果在边界出错
				{
					sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing); //物料前进
					SendCmd(str_cmd);
					sprintf(str_cmd, "EBA301%02XBE", len_newline_camera); //摄像头到下一个
					SendCmd(str_cmd);
					glive_postion = glive_postion  + len_newline_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
					record_livetime_bottom_postion =  glive_bottom_postion;
					record_clos_6 = cols_6+1;
					sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
					SendCmd(str_dis);
					continue;
				}
				else {
					sprintf(str_cmd, "EBA401%02XBE", len_nextmat_camera); //摄像头到下一个
					SendCmd(str_cmd);
					glive_postion = glive_postion  - len_nextmat_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion +len_nextmat_bottom;
					record_livetime_bottom_postion =  glive_bottom_postion;
					record_clos_6 = cols_6+1;
					sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
					SendCmd(str_dis);
					continue;
				}		
			}

			//运动到盒子	
			sprintf(str_cmd, "EBA202%04XBE", glive_postion);
			SendCmd(str_cmd);

			while (!g_bput)
			{
				Sleep(20);
			}
			g_bput = false;
			SendCmd(CAMERA_UP);
			SendCmd(AIR_CLOSE);//放气
			SendCmd(CAMERA_DOWN);
			Sleep(300);
			ForceSendCmd(BOX_OUT);
			Sleep(20);
			/*SendCmd(BOX_RESET);
			SendCmd(BOX_STATUS_FREE);*/
			//返回下一个位置
			if (cols_6 == 5)  //到达边界就要处理异常
			{
				sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
				SendCmd(str_cmd);

				/*	sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
				SendCmd(str);*/

				glive_postion = glive_postion - len_absorb_camera * 4 + len_newline_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_6 = cols_6 + 1;
				sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
				SendCmd(str_dis);

			}
			else {
				//sprintf(str, "EBA401%02XBE", i_sourse_next_len);
				//SendCmd(str);
				//g_longlen = g_longlen - i_sourse_next_len - distance_vs;
				glive_postion = glive_postion - len_absorb_camera * 4 - len_nextmat_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion + len_nextmat_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_6 = cols_6 + 1;
				sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
				SendCmd(str_dis);
			}
		}
		//强行下一张 继续循环测试
		if (g_ForceNext == true)
		{
			break;
		}
		record_clos_6 = cols_6;
		//第二排5个
		for (cols_5 = 0; cols_5 < 5; cols_5++)
		{ 
			//强行下一张 继续循环测试
			if (g_ForceNext == true)
			{
				break;
			}
			VideoAdjustment();
			record_livetime_camera_postion = glive_postion;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_5 = cols_5;
			Sleep(100);
			//吸头对准
			sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);
			SendCmd(str_cmd);
			glive_postion = glive_postion + len_absorb_camera * 4;
			// 加上摄像头和脚垫之间的微调
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			SendCmd(AIR_OPEN); //吸气
			SendCmd(CAMERA_UP);
			SendCmd(MAT_UP); //下顶顶起
			Sleep(50);
			SendCmd(MAT_DOWN);//下顶落下
			Sleep(20);
			SendCmd(CAMERA_DOWN);

			//检测脚垫是否正确吸取
			SendCmd(AIR_PRESSURE_CHECK);
			Sleep(100);
			if (g_bpressure == false)   //如果不是做异常处理
			{
				Sleep(200);
				SendCmd(CAMERA_UP);
				SendCmd(MAT_UP); //下顶顶起
				Sleep(50);
				SendCmd(MAT_DOWN);//下顶落下
				Sleep(20);
				SendCmd(CAMERA_DOWN);
				SendCmd(AIR_PRESSURE_CHECK);
				Sleep(100);
				if (g_bpressure == false)
				{
					SendCmd(LIGHT_RED);
					SendCmd(AIR_CLOSE);
					sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
					SendCmd(str_cmd);
					glive_postion = glive_postion - len_absorb_camera * 4;
					AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
					Sleep(200);
					SendCmd(LIGHT_GREEN);
					Sleep(200);
				}
			}

			//在让脚垫回去
			if (len_absorb_mat > 0)
			{
				sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
				SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
				sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
				SendCmd(str_cmd);
			}

			//没吸起来就到下一个
			if (g_bpressure == false)
			{
					if (cols_5 == 4)
					{
					sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
					SendCmd(str_cmd);

					sprintf(str_cmd, "EBA301%02XBE", len_newline_camera);
					SendCmd(str_cmd);
					glive_postion = glive_postion  + len_newline_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_5 = cols_5 + 1;
					sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
					SendCmd(str_dis);

					continue;
					}
					else {
					sprintf(str_cmd, "EBA301%02XBE", len_nextmat_camera);
					SendCmd(str_cmd);
					glive_postion = glive_postion  + len_nextmat_camera * 4;
					record_livetime_camera_postion = glive_postion;
					glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
					record_livetime_bottom_postion = glive_bottom_postion;
					record_clos_5 = cols_5 + 1;
					sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
					SendCmd(str_dis);
					continue;
					}
			}

			//运动到盒子
			sprintf(str_cmd, "EBA202%04XBE", glive_postion);
			SendCmd(str_cmd);

			while (!g_bput)
			{
				Sleep(20);
			}
			g_bput = false;

			SendCmd(CAMERA_UP);
			SendCmd(AIR_CLOSE); //放气
			SendCmd(CAMERA_DOWN);
			Sleep(300);
			ForceSendCmd(BOX_OUT);
			Sleep(20);
			/*SendCmd(BOX_RESET);
			SendCmd(BOX_STATUS_FREE);*/
			//运动到下一个脚垫
			if (cols_5 == 4)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
				SendCmd(str_cmd);

				//sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
				//SendCmd(str);

				glive_postion = glive_postion - len_absorb_camera * 4 + len_newline_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_5 = cols_5 + 1;
				sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
				SendCmd(str_dis);

			}
			else {
				//sprintf(str, "EBA301%02XBE", i_sourse_next_len);
				//SendCmd(str);

				glive_postion = glive_postion - len_absorb_camera * 4 + len_nextmat_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);
				SendCmd(str_cmd);
				record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_5 = cols_5 + 1;
				sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
				SendCmd(str_dis);
			}
		}
		//强行下一张 继续循环测试
		if (g_ForceNext == true)
		{
			break;
		}
		record_clos_5 = cols_5;
	}

	g_StartNext = true;
	Sleep(1000);
	CreateThread(NULL, 0, IJUSTWANTTODIE, NULL, 0, NULL);
	return 0;
}

void CAutoPadMatDlg::OnBnClickedBtnMatin()
{
	// TODO: 在此添加控件通知处理程序代码
	g_bmateriel = false;
}

//复位
DWORD WINAPI MACHINERESET(LPVOID pParam)
{
	char cstr_cmd[50];
	SendCmd(CAMERA_RESET);
	sprintf(cstr_cmd, "EBA102%04XBE", i_reset_move_up_len);
	SendCmd(cstr_cmd);
	SendCmd(PROP_RESET);
	int tm = 0;
	tm = len_reset_bottom / 255;
	for (int m = 0; m < tm; m++)
	{
		SendCmd("EBB201FFBE");
	}
	sprintf(cstr_cmd, "EBB201%02XBE", len_reset_bottom % 255);
	SendCmd(cstr_cmd);
	return 0;
}

//用来判断脚垫的起始位置
DWORD  WINAPI STARTdector(LPVOID pParam)
{
	char cmdstr[30];
	char str_cmd[40];
	char str_dis[32];
	SendCmd(CAMERA_RESET);
	SendCmd(PROP_RESET);
	SendCmd(BOX_RESET);
	SendCmd(BOX_STATUS_BUSY);
	SendCmd(PRESS_UPORDOWN);
	Sleep(2000);
	SendCmd(LIGHT_RED);

	while (g_bmateriel)
	{
		Sleep(20);
	}
	g_bmateriel = true;
	SendCmd(LIGHT_GREEN);

	SendCmd(PRESS_UPORDOWN);
	Sleep(3000);
	// 进料检测
	sprintf(str_cmd, "EBA102%04XBE", i_reset_move_up_len);
	SendCmd(str_cmd);

	//摄像头移动到固定位置
	int time = len_reset_bottom / 255;
	for (int bottom_times = 0; bottom_times < time; bottom_times++)
	{
		SendCmd("EBB201FFBE");
	}
	sprintf(cmdstr, "EBB201%02XBE", len_reset_bottom % 255);
	SendCmd(cmdstr);

	Sleep(100);
	while (f_dark_quantity > -30)
	{
		SendCmd("EBB50120BE");
		Sleep(50);
	}
	SendCmd("EBB50160BE");
	SendCmd(LIGHT_GREEN);

	Point isSame;
	Point temp;
	//判断圆心是否稳定
	while (1)
	{
		int cout = 0;
		while (true)
		{
			isSame = center;
			Sleep(i_sleep_ms / 2);
			temp = center;
			if (abs(isSame.x - temp.x) > 5 || abs(isSame.y - temp.y) > 5)
			{
				SendCmd("EBB50113BE");
				cout = 0;
				continue;
			}
			cout++;
			isSame = temp;
			if (cout > 8)
			{
				break;
			}
		}
		//判断圆心在哪里
		if (abs(center.x - g_center_x) > 20)
		{
			SendCmd("EBB50113BE");
			continue;
		}
		Sleep(20);
		for (int times = 0; times < 2; times++)
		{
			VideoAdjustRoughly();
			Sleep(20);
		}

		if (abs(center.x - g_center_x)>4 || abs(center.y - g_center_y)>4)
		{
			SendCmd("EBB5011DBE");
			continue;
		}

		Sleep(20);
		for (int w = 0; w < 3; w++)
		{
			Sleep(i_sleep_ms / 2);
			point_test[w] = center;
		}

		if (Detected_circleStable(point_test) == true)
		{
			SendCmd("EBB50113BE");
			continue;
		}
		/*if (abs(center.x - g_center_x) > 3 || abs(center.y - g_center_y)>3)
		{
		SendCmd("EBB510BE");
		continue;
		}*/
		SendCmd("EBB501F0BE");
		break;
	}
	return 0;
}

DWORD  WINAPI  upANDDOWN(LPVOID pParam)
{
	while (1)
	{
		SendCmd(CAMERA_UP);
		SendCmd(CAMERA_DOWN);
	}
	return 0;
}

//对齐
DWORD  WINAPI  FORMTTEST(LPVOID pParam)
{
	char str_cmd[40];
	char str_dis[32];

	VideoAdjustRoughly();
	for (int i = 0; i < num_availablemat_clos; i++)  //num_availablemat_clos
	{
		record_availablemat_clos = i;
		Sleep(50);
		for (cols_6 = 0; cols_6 < 6; cols_6++)
		{

			VideoAdjustment();
			/*record_livetime_bottom_postion = glive_bottom_postion;
			record_livetime_camera_postion = glive_postion;
			record_clos_6 = cols_6;*/
			//g_brecord = true;
			//Sleep(100);
			//吸头对准
			/*sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);*/
			// SendCmd(str_cmd);
			//glive_postion = glive_postion + len_absorb_camera * 4; //距离实时改变

			// 加上摄像头和脚垫之间的微调
			/*if (len_absorb_mat > 0)
			{
			sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
			SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
			sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
			SendCmd(str_cmd);
			}*/

			//SendCmd(AIR_OPEN); //吸气
			//SendCmd(CAMERA_UP);
			//SendCmd(MAT_UP); //下顶顶起
			//Sleep(50);
			//SendCmd(MAT_DOWN);//下顶落下
			//Sleep(20);
			//SendCmd(CAMERA_DOWN);

			//检测脚垫是否正确吸取
			//SendCmd(AIR_PRESSURE_CHECK);
			//Sleep(100);
			//if (g_bpressure == false)   //如果不是做异常处理
			//{
			//  Sleep(200);
			//  SendCmd(CAMERA_UP);
			//  //SendCmd(MAT_UP); //下顶顶起
			//  //Sleep(50);
			//  //SendCmd(MAT_DOWN);//下顶落下
			//  //Sleep(20);
			//  SendCmd(CAMERA_DOWN);
			//  SendCmd(AIR_PRESSURE_CHECK);
			//  Sleep(100);
			//  if (g_bpressure == false)
			//  {  
			//	  SendCmd(LIGHT_RED);
			//	  SendCmd(AIR_CLOSE);
			//	  sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
			//	  SendCmd(str_cmd);
			//	  glive_postion = glive_postion - len_absorb_camera * 4;
			//	  AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
			//	  Sleep(200);
			//	  SendCmd(LIGHT_GREEN);
			//	  Sleep(200);
			//  }
			//   }

			//在让脚垫回去
			/*	if (len_absorb_mat > 0)
			{
			sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
			SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
			sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
			SendCmd(str_cmd);
			}*/

			//if (g_bpressure == false)
			//{
			//	if (cols_6 == 5)  //如果在边界出错
			//	{
			//		sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing); //物料前进
			//		SendCmd(str_cmd);
			//		sprintf(str_cmd, "EBA301%02XBE", len_newline_camera); //摄像头到下一个
			//		SendCmd(str_cmd);
			//		glive_postion = glive_postion  + len_newline_camera * 4;
			//    record_livetime_camera_postion = glive_postion;
			//     glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
			//     record_livetime_bottom_postion =  glive_bottom_postion;
			//     record_clos_6 = cols_6 +1;
			//		sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
			//		SendCmd(str_dis);
			//		continue;
			//	}
			//	else {
			//		sprintf(str_cmd, "EBA401%02XBE", len_nextmat_camera); //摄像头到下一个
			//		SendCmd(str_cmd);
			//		glive_postion = glive_postion  - len_nextmat_camera * 4;
			//     record_livetime_camera_postion = glive_postion;
			//     glive_bottom_postion = glive_bottom_postion +len_nextmat_bottom;
			//     record_livetime_bottom_postion =  glive_bottom_postion;
			//     record_clos_6 = cols_6 + 1;
			//		sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
			//		SendCmd(str_dis);
			//		continue;
			//	}
			//	
			//}


			//运动到盒子	
			//sprintf(str_cmd, "EBA202%04XBE", glive_postion);
			//SendCmd(str_cmd);

			//while (!g_bput)
			//{
			//	Sleep(20);
			//}
			//g_bput = false;
			//SendCmd(CAMERA_UP);
			//SendCmd(AIR_CLOSE);//放气
			//SendCmd(CAMERA_DOWN);
			//Sleep(20);
			//ForceSendCmd(BOX_OUT);
			//Sleep(20);
			//SendCmd(BOX_RESET);
			//SendCmd(BOX_STATUS_FREE);
			//返回下一个位置
			if (cols_6 == 5)  //到达边界就要处理异常
			{
				sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
				SendCmd(str_cmd);

				/*	sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
				SendCmd(str);*/

				sprintf(str_cmd, "EBA301%02XBE", len_newline_camera);
				SendCmd(str_cmd);

				sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
				SendCmd(str_dis);


			}
			else {
				//sprintf(str, "EBA401%02XBE", i_sourse_next_len);
				//SendCmd(str);
				//g_longlen = g_longlen - i_sourse_next_len - distance_vs;
				//glive_postion = glive_postion - len_absorb_camera * 4 - len_nextmat_camera * 4;
				sprintf(str_cmd, "EBA401%02XBE", len_nextmat_camera);
				SendCmd(str_cmd);

				sprintf(str_dis, "EBB201%02XBE", len_nextmat_bottom);
				SendCmd(str_dis);
			}
		}
		//record_clos_6 = cols_6;
		//第二排5个
		for (cols_5 = 0; cols_5 < 5; cols_5++)
		{

			VideoAdjustment();
			/*record_livetime_camera_postion = glive_postion;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_5 = cols_5;*/
			Sleep(100);
			//吸头对准
			/*	sprintf(str_cmd, "EBA301%02XBE", len_absorb_camera);
			SendCmd(str_cmd);
			glive_postion = glive_postion + len_absorb_camera * 4;*/
			// 加上摄像头和脚垫之间的微调
			/*if (len_absorb_mat > 0)
			{
			sprintf(str_cmd, "EBB501%02XBE", len_absorb_mat);
			SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
			sprintf(str_cmd, "EBB601%02XBE", -len_absorb_mat);
			SendCmd(str_cmd);
			}*/

			//SendCmd(AIR_OPEN); //吸气
			//SendCmd(CAMERA_UP);
			//SendCmd(MAT_UP); //下顶顶起
			//Sleep(50);
			//SendCmd(MAT_DOWN);//下顶落下
			//Sleep(20);
			//SendCmd(CAMERA_DOWN);

			//检测脚垫是否正确吸取
			//SendCmd(AIR_PRESSURE_CHECK);
			//Sleep(100);
			//if (g_bpressure == false)   //如果不是做异常处理
			//{
			//	Sleep(200);
			//	SendCmd(CAMERA_UP);
			//	//SendCmd(MAT_UP); //下顶顶起
			//	//Sleep(50);
			//	//SendCmd(MAT_DOWN);//下顶落下
			//	//Sleep(20);
			//	SendCmd(CAMERA_DOWN);
			//	SendCmd(AIR_PRESSURE_CHECK);
			//	Sleep(100);
			//	if (g_bpressure == false)
			//	{
			//		SendCmd(LIGHT_RED);
			//		SendCmd(AIR_CLOSE);
			//		sprintf(str_cmd, "EBA401%02XBE", len_absorb_camera); //出错了先回到原点
			//		SendCmd(str_cmd);
			//		glive_postion = glive_postion - len_absorb_camera * 4;
			//		AfxMessageBox(_T("没有正确吸取，请移除没有吸取的脚垫，再点击确定"));
			//		Sleep(200);
			//		SendCmd(LIGHT_GREEN);
			//		Sleep(200);
			//	}
			//}

			//在让脚垫回去
			/*if (len_absorb_mat > 0)
			{
			sprintf(str_cmd, "EBB601%02XBE", len_absorb_mat);
			SendCmd(str_cmd);
			}
			else if (len_absorb_mat < 0)
			{
			sprintf(str_cmd, "EBB501%02XBE", -len_absorb_mat);
			SendCmd(str_cmd);
			}*/

			//没吸起来就到下一个
			/*		if (g_bpressure == false)
			{
			if (cols_5 == 4)
			{
			sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
			SendCmd(str_cmd);

			sprintf(str_cmd, "EBA301%02XBE", len_newline_camera);
			SendCmd(str_cmd);
			glive_postion = glive_postion  + len_newline_camera * 4;
			record_livetime_camera_postion = glive_postion;
			glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_5 = cols_5 +1;
			sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
			SendCmd(str_dis);
			continue;
			}
			else {
			sprintf(str_cmd, "EBA301%02XBE", len_nextmat_camera);
			SendCmd(str_cmd);
			glive_postion = glive_postion  + len_nextmat_camera * 4;
			record_livetime_camera_postion = glive_postion;
			glive_bottom_postion = glive_bottom_postion - len_nextmat_bottom;
			record_livetime_bottom_postion = glive_bottom_postion;
			record_clos_5 = cols_5 +1;
			sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
			SendCmd(str_dis);
			continue;
			}
			}*/

			//运动到盒子
			//sprintf(str_cmd, "EBA202%04XBE", glive_postion);
			//SendCmd(str_cmd);

			//while (!g_bput)
			//{
			//	Sleep(20);
			//}
			//g_bput = false;

			//SendCmd(CAMERA_UP);
			//SendCmd(AIR_CLOSE); //放气
			//SendCmd(CAMERA_DOWN);
			//Sleep(20);
			//ForceSendCmd(BOX_OUT);
			//Sleep(20);
			//SendCmd(BOX_RESET);
			/*SendCmd(BOX_OUT);
			SendCmd(BOX_STATUS_FREE);*/
			//运动到下一个脚垫
			if (cols_5 == 4)
			{
				sprintf(str_cmd, "EBB501%02XBE", len_newline_pressbearing);
				SendCmd(str_cmd);

				//sprintf(str, "EBA301%02XBE", i_sourse_next_t_len);
				//SendCmd(str);

				/*glive_postion = glive_postion - len_absorb_camera * 4 + len_newline_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);*/
				sprintf(str_cmd, "EBA301%02XBE", len_newline_camera);
				SendCmd(str_cmd);
				/*	record_livetime_camera_postion = glive_postion;
				glive_bottom_postion = glive_bottom_postion - len_newline_bottom;
				record_livetime_bottom_postion = glive_bottom_postion;
				record_clos_5 = cols_5 + 1;*/
				sprintf(str_dis, "EBB301%02XBE", len_newline_bottom);
				SendCmd(str_dis);

			}
			else {
				//sprintf(str, "EBA301%02XBE", i_sourse_next_len);
				//SendCmd(str);

				/*glive_postion = glive_postion - len_absorb_camera * 4 + len_nextmat_camera * 4;
				sprintf(str_cmd, "EBA102%04XBE", glive_postion);*/
				sprintf(str_cmd, "EBA301%02XBE", len_nextmat_camera);
				SendCmd(str_cmd);

				sprintf(str_dis, "EBB301%02XBE", len_nextmat_bottom);
				SendCmd(str_dis);

			}

		}
		//	record_clos_5 = cols_5;
	}
	return 0;
}

void CAutoPadMatDlg::OnBnClickedBtnReset()
{
	// TODO: 在此添加控件通知处理程序代码
	HANDLE hadle1;
	hadle1 = CreateThread(NULL, 0, MACHINERESET, NULL, 0, NULL);
	CloseHandle(hadle1);
}


void CAutoPadMatDlg::OnBnClickedBtnSametest()
{
	// TODO: 在此添加控件通知处理程序代码
	HANDLE hadle2;
	hadle2 = CreateThread(NULL, 0, FORMTTEST, NULL, 0, NULL);
	CloseHandle(hadle2);
}


void CAutoPadMatDlg::OnBnClickedBtnStart()
{
	// TODO: 在此添加控件通知处理程序代码

	is_SAVE_config = true;
	g_StartNext = false;
	if (g_brestart == true)
	{
		if (::MessageBox(NULL, "检测到上次未用完脚垫 是否继续（请确定未动过脚垫）？", "提示", MB_YESNO) == IDNO)
		{

			CreateThread(NULL, 0, IJUSTWANTTODIE, NULL, 0, NULL);
			GetDlgItem(IDC_BTN_START)->EnableWindow(false);
			return;
		}
		else {
			CreateThread(NULL, 0, RunComputer, NULL, 0, NULL);
			GetDlgItem(IDC_BTN_START)->EnableWindow(false);
			return;
		}
	}

	CreateThread(NULL, 0, IJUSTWANTTODIE, NULL, 0, NULL);  //  HopeForME HopeForTest  PlantLineTest  IJUSTWANTTODIE
	GetDlgItem(IDC_BTN_START)->EnableWindow(false);
}


void CAutoPadMatDlg::OnCbnSelendokMatPostion()
{
	// TODO: 在此添加控件通知处理程序代码
	param_mat_postion = m_MatPostion.GetCurSel() + 1;
}


//
DWORD WINAPI VideoJUST(LPVOID pParam)
{
	VideoAdjustRoughly();
	return 0;
}

DWORD WINAPI VideoJUS4545T(LPVOID pParam)
{ 
	SendCmd("EBA13330BE");
	while (1)
	{
		SendCmd("EBA22240BE");
		SendCmd("EBA12240BE");
	}
	return 0;

}

void CAutoPadMatDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	CreateThread(NULL, 0, STARTdector, NULL, 0, NULL); // upANDDOWN VideoJUST  STARTdector
} 


void CAutoPadMatDlg::OnBnClickedForcestop()
{
	// TODO: 在此添加控件通知处理程序代码
	g_ForceNext = true;
}



