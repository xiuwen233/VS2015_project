
// SampleMFCDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CSampleMFCDlg 对话框
class CSampleMFCDlg : public CDialogEx
{
// 构造
public:
	CSampleMFCDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SAMPLEMFC_DIALOG };
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
	CComboBox m_portset;
	CComboBox m_botelvset;
	afx_msg void OnBnClickedConnectPort();
	CButton m_connect_port;
	afx_msg void OnClose();


	int m_nBaud;       //波特率
	int m_nCom;         //串口号
	char m_cParity;    //校验
	int m_nDatabits;    //数据位
	int m_nStopbits;    //停止位
	DWORD m_dwCommEvents; //串口事件？？
	BOOL m_bOpenPort; //串口打开标志

	CEdit m_statue;
	afx_msg void OnBnClickedTest();
	afx_msg LRESULT OnCommunication(WPARAM ch, LPARAM port);
	CEdit m_count;
	CEdit m_recvdata;
	CString m_dataShow;
	CString m_dataport;
	CString m_datasocket;
	afx_msg void OnBnClickedSenddataPort();
	CEdit m_SOCKET;
	CString m_socketip;
	afx_msg void OnBnClickedConnectSocket();
	CEdit m_socketport;
//	CString m_socketportstr;
	int m_socketport_int;
	afx_msg void OnBnClickedSenddataSocket();
	afx_msg void OnBnClickedStopsocket();
	CStatic m_photo_1;

	HICON m_hIconOff; //关闭图标
	HICON m_hIconOPcv; //打开图标
	CStatic m_SERIAPORT;
	afx_msg void OnBnClickedOpencvtest();
	CButton m_opencvstart;
	afx_msg void OnBnClickedControlwid();
	afx_msg void OnBnClickedFilewrite();
	afx_msg void OnBnClickedDoesfile();
	afx_msg void OnBnClickedFileread();
	afx_msg void OnBnClickedBinarywrite();
	afx_msg void OnBnClickedbinaryreadu();
	afx_msg void OnBnClickedvideocapture();
	afx_msg void OnBnClickedstopviseo();
	afx_msg void OnBnClickedGettofirst();
	afx_msg void OnBnClickedStopprocess();
};
