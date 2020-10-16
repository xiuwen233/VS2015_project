// ListenSocket.cpp : implementation file
//

#include "stdafx.h"
#include "iFTP.h"
#include "iFTPDlg.h"
#include "ListenSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListenSocket

CListenSocket::CListenSocket()
:m_pWndServer(NULL)
{
}

CListenSocket::~CListenSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CListenSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CListenSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CListenSocket member functions

void CListenSocket::OnAccept(int nErrorCode) 
{
	CSocket tmpSocket;
	Accept(tmpSocket);
	CConnectThread* pThread = (CConnectThread*)AfxBeginThread(RUNTIME_CLASS(CConnectThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!pThread)
	{
		tmpSocket.Close();
		TRACE(_T("Could not create thread\n"));
		return;
	}
	CIFTPDlg *pWnd = (CIFTPDlg *)m_pWndServer;
	pWnd->m_CriticalSection.Lock();
	pWnd->m_ThreadList.AddTail(pThread);
	pWnd->m_CriticalSection.Unlock();
	pThread->m_pWndServer = m_pWndServer;
	pThread->m_hSocket = tmpSocket.Detach();
	pThread->ResumeThread();
	
	CAsyncSocket::OnAccept(nErrorCode);
}
