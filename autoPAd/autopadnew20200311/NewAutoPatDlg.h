
// NewAutoPatDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


#define WM_MyMessage  WM_USER+100


// CNewAutoPatDlg 对话框
class CNewAutoPatDlg : public CDialogEx
{
// 构造
public:
	CNewAutoPatDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NEWAUTOPAT_DIALOG };
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
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
public:
	CString m_CPU1_IP;
	CString m_CPU2__IP;
	CString m_CPU3_IP;
	afx_msg void OnBnClickedIpSure();
	CEdit m_RECEIVER_DATA;
	CString m_RECEDATA_STR;
	afx_msg void OnBnClickedTestOne();
	CSliderCtrl m_sliderBJ;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedDirectUp();
	afx_msg void OnBnClickedDirectDown();
	afx_msg void OnBnClickedDirecLeft();
	afx_msg void OnBnClickedDirecRight();
	afx_msg void OnBnClickedPadsUp();
	afx_msg void OnBnClickedPadsDown();
	afx_msg void OnBnClickedAirOpen();
	afx_msg void OnBnClickedAirClose();
	afx_msg void OnBnClickedMatUp();
	afx_msg void OnBnClickedMatDown();
	afx_msg void OnBnClickedBtDataSend();
	CEdit m_Data_Send;
	afx_msg void OnBnClickedBtDataSend2();
	afx_msg void OnBnClickedBtDataSend3();
	CString m_Data_Send_Str;
	CEdit m_U1_IP_edit;
	CEdit m_U2_IP_edit;
	CEdit m_U3_IP_edit;
	afx_msg void OnBnClickedCodeTest();
	afx_msg void OnBnClickedDataClean();
	afx_msg void OnBnClickedU3Press();
	afx_msg void OnBnClickedResetU1();
	afx_msg void OnBnClickedBottomReset();
	afx_msg void OnBnClickedU3ProessDown();
	afx_msg void OnBnClickedStopBt();
	afx_msg void OnBnClickedFindDeviceButton();
	afx_msg void OnBnClickedStartGrabbingButton();
	afx_msg void OnBnClickedStopGrabbingButton();
	CComboBox m_ctrCombox;
	int m_numDeviceComb;
	afx_msg void OnBnClickedThreadButton();
	afx_msg void OnBnClickedSmallButton();
	afx_msg void OnBnClickedSocketConnectButton();
	afx_msg void OnBnClickedSocketDisconnectButton();


	CString InitConfig();
	CString GetDiffConfig();
	void SaveConfig();
	void SavePartConfig();
	afx_msg void OnClose();
	afx_msg void OnSelchangeMatPostion();
	CComboBox m_MatPostion;
	afx_msg void OnBnClickedTestDebug();
	CButton m_Debug_check;
	afx_msg void OnBnClickedChangeData();
	CEdit m_DataChange;
	CSliderCtrl m_Dataselect;
	afx_msg void OnNMCustomdrawSelectItem(NMHDR *pNMHDR, LRESULT *pResult);

	void SetWinEnable();
	void SetWinUnEnable();
	CString m_datadigital;
	CString m_HomeIp;
	afx_msg void OnBnClickedHostSend();
	CEdit m_HostIP_Edit;
	CEdit m_HostPort;
	CString m_host_port;
	afx_msg void OnBnClickedBottomCamera();
	afx_msg void OnBnClickedSetting();
};
