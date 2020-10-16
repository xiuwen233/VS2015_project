#if !defined(AFX_CONTROLSOCKET_H__51DE5049_9A9F_41B0_A132_B2C70C6BEFAC__INCLUDED_)
#define AFX_CONTROLSOCKET_H__51DE5049_9A9F_41B0_A132_B2C70C6BEFAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ControlSocket.h : header file
//

enum
{
	STATUS_IDLE,
	STATUS_LOGIN,
	STATUS_LIST,
	STATUS_UPLOAD,
	STATUS_DOWNLOAD,
};
enum
{
	FTP_DOWNLOAD,
	FTP_UPLOAD,
	FTP_RENAME,
	FTP_DELETE,
	FTP_CREATE_DIR,
	FTP_LIST,
};

/////////////////////////////////////////////////////////////////////////////
// CControlSocket command target

class CControlSocket : public CSocket
{
// Attributes
public:
	CWinThread* m_pThread;
	class CConnectThread* pThread;
	class CIFTPDlg* pWnd;
	CString m_strUserName;
	int m_nStatus;

// Operations
public:
	CControlSocket();
	virtual ~CControlSocket();
	BOOL __cdecl SendResponse(LPCSTR, ...);
	BOOL CreateDataConnection();
	void DestroyDataConnection();
	void ProcessCommand();
	void GetCommandLine();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControlSocket)
	public:
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CControlSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
	CString strReName;
	int m_bPassiveMode;
	int m_nRemotePort;
	CString m_strRemoteHost;
	class CDataSocket *m_pDataSocket;
	CStringList m_strCommands;
	CString m_RxBuffer;
	CString m_strCurrentDir;
	BOOL GetLocalPath(LPCTSTR lpszRelativePath, CString &strLocalPath);
	BOOL GetRelativePath(LPCTSTR lpszLocalPath, CString &strRelativePath);
	BOOL GetDirectoryList(LPCTSTR lpszDirectory, CString &strResult);
	BOOL CheckAccessRights(LPCTSTR lpszDirectory, int nOption);
	int CheckFileName(CString strFilename, int nOption, CString &strResult);
	int CheckDirectory(CString strDirectory, int nOption, CString &strResult);
	void DoChangeDirectory(LPCTSTR lpszDirectory);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTROLSOCKET_H__51DE5049_9A9F_41B0_A132_B2C70C6BEFAC__INCLUDED_)
