// iFTPDlg.cpp : implementation file
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
// CIFTPDlg dialog

CIFTPDlg::CIFTPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIFTPDlg::IDD, pParent)
{
	GetLocalHostIP(m_strHostIP);
	CString strTempState;
	strTempState.Format(_T("本机IP:%s 当前连接数:0"),m_strHostIP);

	//{{AFX_DATA_INIT(CIFTPDlg)
	m_nPort = 21;
	m_nMaxUsers = 100;
	m_strName = _T("anonymous");
	m_bAllowDownload = TRUE;
	m_bAllowUpload = FALSE;
	m_bAllowDelete = FALSE;
	m_bAllowRename = FALSE;
	m_bAllowCreateDirectory = FALSE;
	m_strState = strTempState;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bRunning = FALSE;
	m_nConnectionCount = 0;
}

void CIFTPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIFTPDlg)
	DDX_Text(pDX, IDC_EDIT1, m_nPort);
	DDV_MinMaxUInt(pDX, m_nPort, 0, 65535);
	DDX_Text(pDX, IDC_EDIT2, m_nMaxUsers);
	DDV_MinMaxUInt(pDX, m_nMaxUsers, 0, 500);
	DDX_Text(pDX, IDC_EDIT4, m_strName);
	DDV_MaxChars(pDX, m_strName, 20);
	DDX_Text(pDX, IDC_EDIT5, m_strPassword);
	DDV_MaxChars(pDX, m_strPassword, 20);
	DDX_Text(pDX, IDC_EDIT3, m_strHomeDirectory);
	DDX_Check(pDX, IDC_CHECK1, m_bAllowDownload);
	DDX_Check(pDX, IDC_CHECK2, m_bAllowUpload);
	DDX_Check(pDX, IDC_CHECK3, m_bAllowRename);
	DDX_Check(pDX, IDC_CHECK4, m_bAllowDelete);
	DDX_Check(pDX, IDC_CHECK5, m_bAllowCreateDirectory);
	DDX_Text(pDX, IDC_TEXT1, m_strState);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIFTPDlg, CDialog)
	//{{AFX_MSG_MAP(CIFTPDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON3, OnBtnBrowse)
	ON_BN_CLICKED(IDC_BUTTON1, OnBtnStart)
	ON_BN_CLICKED(IDC_BUTTON2, OnBtnStop)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_THREADSTART, OnThreadStart)
	ON_MESSAGE(WM_THREADCLOSE, OnThreadClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIFTPDlg message handlers

BOOL CIFTPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIFTPDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CIFTPDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CIFTPDlg::OnBtnBrowse()
{
	UpdateData(TRUE);
	CString strDir = BrowseForFolder(m_hWnd, _T("选择访问目录:"), BIF_RETURNONLYFSDIRS);
	if (!strDir.IsEmpty())
	{
		m_strHomeDirectory = strDir;
		UpdateData(FALSE);
	}	
}

BOOL CIFTPDlg::Start()
{
	if (m_bRunning)
		return FALSE;
	if (m_ListenSocket.Create(m_nPort) && m_ListenSocket.Listen())
	{
		m_ListenSocket.m_pWndServer = this;
		m_bRunning = TRUE;
	}
	return m_bRunning;
}

void CIFTPDlg::Stop()
{
	if (!m_bRunning)
		return;
	m_bRunning = FALSE;
	m_ListenSocket.Close();
	CConnectThread* pThread = NULL;
	do
	{
		m_CriticalSection.Lock();
		POSITION pos = m_ThreadList.GetHeadPosition();
		if (pos)
		{
			pThread = (CConnectThread *)m_ThreadList.GetAt(pos);
			m_CriticalSection.Unlock();
			HANDLE hThread = pThread->m_hThread;
			pThread->SetThreadPriority(THREAD_PRIORITY_HIGHEST);
			pThread->PostThreadMessage(WM_QUIT,0,0);
			if (!WaitWithMessageLoop(hThread, 1000))
			{
				m_CriticalSection.Lock();
				POSITION rmPos = m_ThreadList.Find(pThread);
				if (rmPos)
					m_ThreadList.RemoveAt(rmPos);
				m_CriticalSection.Unlock();
			}
		}
		else
		{
			m_CriticalSection.Unlock();
			pThread = NULL;
		}
	}
	while (pThread);
}

BOOL CIFTPDlg::CheckMaxUsers()
{
	return m_nConnectionCount > m_nMaxUsers;
}

void CIFTPDlg::ItemSwitch(BOOL x)
{
	GetDlgItem(IDC_EDIT1)->EnableWindow(x);
	GetDlgItem(IDC_EDIT2)->EnableWindow(x);
	GetDlgItem(IDC_EDIT4)->EnableWindow(x);
	GetDlgItem(IDC_EDIT5)->EnableWindow(x);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(x);
	GetDlgItem(IDC_EDIT3)->EnableWindow(x);
	GetDlgItem(IDC_CHECK1)->EnableWindow(x);
	GetDlgItem(IDC_CHECK2)->EnableWindow(x);
	GetDlgItem(IDC_CHECK3)->EnableWindow(x);
	GetDlgItem(IDC_CHECK4)->EnableWindow(x);
	GetDlgItem(IDC_CHECK5)->EnableWindow(x);

	GetDlgItem(IDC_BUTTON1)->EnableWindow(x);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(!x);
}

void CIFTPDlg::OnBtnStart()
{
	ItemSwitch(FALSE);
	UpdateData(TRUE);
	if (m_strName.IsEmpty())
	{
		m_strName = _T("anonymous");
		UpdateData(FALSE);
	}
	Start();
}

void CIFTPDlg::OnBtnStop()
{
	ItemSwitch(TRUE);
	Stop();
}

LRESULT CIFTPDlg::OnThreadStart(WPARAM, LPARAM)
{
	m_nConnectionCount++;
	m_strState.Format(_T("本机IP:%s 当前连接数:%d"),m_strHostIP,m_nConnectionCount);
	UpdateData(FALSE);
	return TRUE;
}

LRESULT CIFTPDlg::OnThreadClose(WPARAM, LPARAM)
{
	m_nConnectionCount--;
	m_strState.Format(_T("本机IP:%s 当前连接数:%d"),m_strHostIP,m_nConnectionCount);
	UpdateData(FALSE);
	return TRUE;
}

