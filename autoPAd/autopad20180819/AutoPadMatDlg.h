
// AutoPadMatDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include "afxcmn.h"

// CAutoPadMatDlg 对话框
class CAutoPadMatDlg : public CDialogEx
{
// 构造
public:
	CAutoPadMatDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTOPADMAT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	int Str2Hex(CString str, char *data);
	char HexChar(char c);
	CString InitConfig();
	BOOL SaveConfig();
	int SavePartConfig();
	CString GetDiffConfig();

public:
	CEdit m_DATASHOW;
	CString m_DATASHOWSTR;
	CString m_DATDSEND;
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonOpenport();
	CButton m_PORTOPEN;
	HICON m_hIconRed;    //串口打开时的红灯图标句柄
	HICON m_hIconOff;    //串口关闭时的指示图标句柄

public:
	int m_nBaud;       //波特率
	int m_nCom;         //串口号
	char m_cParity;    //校验
	int m_nDatabits;    //数据位
	int m_nStopbits;    //停止位

	BOOL m_bOpenPort = false;

	afx_msg void OnCbnSelendokComboComselect();
	afx_msg void OnCbnSelendokComboSpeed();
	CComboBox m_SetCom;
	CComboBox m_SetBaud;
	afx_msg LRESULT OnCommunication(WPARAM ch, LPARAM port);
	afx_msg void OnBnClickedVideodetect();
	afx_msg void OnBnClickedVideoSetparam();
	afx_msg void OnBnClickedBtnStop();
	CButton m_btnVideoDetect;
	afx_msg void OnClose();
	CSliderCtrl m_sliderBJ;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedBtnA3();
	afx_msg void OnBnClickedBtnA4();
	afx_msg void OnBnClickedBtnLeft();
	afx_msg void OnBnClickedBtnRight();
	afx_msg void OnBnClickedBtnB2();
	afx_msg void OnBnClickedBtnB3();
	afx_msg void OnBnClickedBtnCup();
	afx_msg void OnBnClickedBtnCdown();
	afx_msg void OnBnClickedBtnAiropen();
	afx_msg void OnBnClickedBtnAirdown();
	afx_msg void OnBnClickedBtnXup();
	afx_msg void OnBnClickedBtnXdown();
	CSliderCtrl m_slider_lenSC;
	CSliderCtrl m_slider_lenY;
	afx_msg void OnBnClickedBtnMatin();
	CComboBox m_MatPostion;
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedBtnSametest();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnCbnSelendokMatPostion();
	CStatic m_PICSET;
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedForcestop();

};
