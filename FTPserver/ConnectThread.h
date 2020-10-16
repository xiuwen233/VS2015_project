#if !defined(AFX_CONNECTTHREAD_H__428990E5_27B0_48F0_9155_D91CF3BA0C77__INCLUDED_)
#define AFX_CONNECTTHREAD_H__428990E5_27B0_48F0_9155_D91CF3BA0C77__INCLUDED_

#include "ControlSocket.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectThread.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CConnectThread thread

class CConnectThread : public CWinThread
{
	DECLARE_DYNCREATE(CConnectThread)
protected:
	CConnectThread();           // protected constructor used by dynamic creation

// Attributes
public:
	CWnd* m_pWndServer;
	SOCKET m_hSocket;
	CControlSocket m_ControlSocket;

// Operations
public:
	static VOID CALLBACK TimerProc(HWND, UINT, UINT, DWORD);
	void UpdateTime();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT m_nTimerID;
	CTime m_LastDataTransferTime;
	virtual ~CConnectThread();

	// Generated message map functions
	//{{AFX_MSG(CConnectThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	void OnDestroyDataConnection(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTTHREAD_H__428990E5_27B0_48F0_9155_D91CF3BA0C77__INCLUDED_)
