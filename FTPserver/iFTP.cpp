// iFTP.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "iFTP.h"
#include "iFTPDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIFTPApp

BEGIN_MESSAGE_MAP(CIFTPApp, CWinApp)
	//{{AFX_MSG_MAP(CIFTPApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIFTPApp construction

CIFTPApp::CIFTPApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CIFTPApp object

CIFTPApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CIFTPApp initialization

BOOL CIFTPApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Standard initialization

	CIFTPDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
