// iFTPDlg.h : header file
//

#if !defined(AFX_IFTPDLG_H__AAFDE426_F2E2_4624_BEF3_7A38865C65E4__INCLUDED_)
#define AFX_IFTPDLG_H__AAFDE426_F2E2_4624_BEF3_7A38865C65E4__INCLUDED_

#include "ListenSocket.h"	// Added by ClassView
#include "ConnectThread.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CIFTPDlg dialog

class CIFTPDlg : public CDialog
{
// Construction
public:
	CTypedPtrList<CObList, CConnectThread*> m_ThreadList;
	CCriticalSection m_CriticalSection;
	UINT m_nConnectionCount;
	CString m_strHostIP;

private:
	BOOL m_bRunning;
	CListenSocket m_ListenSocket;

public:
	CIFTPDlg(CWnd* pParent = NULL);	// standard constructor
	BOOL CheckMaxUsers();
	BOOL Start();
	void Stop();
	void ItemSwitch(BOOL);

// Dialog Data
	//{{AFX_DATA(CIFTPDlg)
	enum { IDD = IDD_DIALOG1 };
	UINT	m_nMaxUsers;
	UINT	m_nPort;
	CString	m_strName;
	CString	m_strPassword;
	CString	m_strHomeDirectory;
	BOOL	m_bAllowDownload;
	BOOL	m_bAllowUpload;
	BOOL	m_bAllowRename;
	BOOL	m_bAllowDelete;
	BOOL	m_bAllowCreateDirectory;
	CString	m_strState;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIFTPDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CIFTPDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnBrowse();
	afx_msg void OnBtnStart();
	afx_msg void OnBtnStop();
	//}}AFX_MSG
	LRESULT OnThreadClose(WPARAM, LPARAM);
	LRESULT OnThreadStart(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IFTPDLG_H__AAFDE426_F2E2_4624_BEF3_7A38865C65E4__INCLUDED_)
