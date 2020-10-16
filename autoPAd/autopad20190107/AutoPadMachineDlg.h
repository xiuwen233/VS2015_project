
// AutoPadMachineDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CAutoPadMachineDlg 对话框
class CAutoPadMachineDlg : public CDialogEx
{
// 构造
public:
	CAutoPadMachineDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTOPADMACHINE_DIALOG };
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


public:
	int m_nBaud;       //波特率
	int m_nCom;         //串口号
	char m_cParity;    //校验
	int m_nDatabits;    //数据位
	int m_nStopbits;    //停止位
	BOOL m_bOpenPort = false;
	afx_msg void OnBnClickedOpenPort();
	HICON m_hIconRed;    //串口打开时的红灯图标句柄
	HICON m_hIconOff;    //串口关闭时的指示图标句柄
	CButton m_OpenPort;
	CStatic m_Picture;
	CComboBox m_SerialPort;
	CComboBox m_BoteRate;
	afx_msg void OnCbnSelendokCom();
	afx_msg void OnCbnSelendokSpeed();
	afx_msg LRESULT OnCommunication(WPARAM ch, LPARAM port);
	CEdit m_DataShow;
	CString m_DataShowStr;
	afx_msg void OnBnClickedVideoStart();

	CString m_SendDATA;
	afx_msg void OnBnClickedAutosend();
	afx_msg void OnBnClickedStartHh();
	afx_msg void OnBnClickedTiaoshi();
//	CEdit m_Status;
	afx_msg void OnBnClickedChangeMate();
	afx_msg void OnClose();
	afx_msg void OnBnClickedChechover();
	afx_msg void OnBnClickedForcestart();
	afx_msg void OnBnClickedForcestop();
	CStatic m_statictxt;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedMaterialJu();
	CButton m_Material;
};
