// ConnectThread.cpp : implementation file
//

#include "stdafx.h"
#include "iFTP.h"
#include "iFTPDlg.h"
#include "ConnectThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConnectThread

IMPLEMENT_DYNCREATE(CConnectThread, CWinThread)

CConnectThread::CConnectThread()
{
	m_nTimerID = 0;
	m_LastDataTransferTime = CTime::GetCurrentTime();
}

CConnectThread::~CConnectThread()
{
}

BOOL CConnectThread::InitInstance()
{
	try
	{
		m_ControlSocket.Attach(m_hSocket);
		m_ControlSocket.m_pThread = this;
		m_ControlSocket.pThread = (CConnectThread*)this;
		m_ControlSocket.pWnd = (CIFTPDlg *)(this->m_pWndServer);
		m_pWndServer->SendMessage(WM_THREADSTART, (WPARAM)this, 0);
		if (((CIFTPDlg *)m_pWndServer)->CheckMaxUsers())
		{
			m_ControlSocket.SendResponse("421 Too many users are connected, please try again later.");
			PostThreadMessage(WM_QUIT,0,0);
		}
		else
		{
			m_ControlSocket.SendResponse("220 Serv-U FTP Server v6.3 for WinSock ready...");
			m_nTimerID = ::SetTimer(NULL, 0, 1000, TimerProc);
		}
	}
	catch (CException* e)
	{
		e->Delete();
	}
	return TRUE;
}

int CConnectThread::ExitInstance()
{
	CIFTPDlg *pWnd = (CIFTPDlg *)m_pWndServer;
	try
	{
		pWnd->m_CriticalSection.Lock();
		POSITION pos = pWnd->m_ThreadList.Find(this);
		if(pos)
			pWnd->m_ThreadList.RemoveAt(pos);
		pWnd->m_CriticalSection.Unlock();
		m_ControlSocket.Close();
		pWnd->SendMessage(WM_THREADCLOSE, (WPARAM)this, 0);
	}
	catch(CException *e)
	{
		pWnd->m_CriticalSection.Unlock();
		e->Delete();
	}
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CConnectThread, CWinThread)
	//{{AFX_MSG_MAP(CConnectThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_THREAD_MESSAGE(WM_DESTROYDATACONNECTION, OnDestroyDataConnection)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectThread message handlers

void CConnectThread::OnDestroyDataConnection(WPARAM wParam, LPARAM lParam)
{
	m_ControlSocket.DestroyDataConnection();
}

VOID CALLBACK CConnectThread::TimerProc(HWND hwnd, UINT uMsg, UINT uIDEvent, DWORD dwTime)
{
	CConnectThread *pThread = (CConnectThread *)AfxGetThread();
	if (uIDEvent == pThread->m_nTimerID)
	{
		CTime time = pThread->m_LastDataTransferTime;
		time += CTimeSpan(0, 0, 5, 0);
		if (time < CTime::GetCurrentTime())
		{
			pThread->m_ControlSocket.SendResponse("426 Connection timed out, aborting transfer.");
			pThread->PostThreadMessage(WM_QUIT,0,0);
		}
	}
}

void CConnectThread::UpdateTime()
{
	m_LastDataTransferTime = CTime::GetCurrentTime();
}
