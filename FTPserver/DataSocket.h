#if !defined(AFX_DATASOCKET_H__BD5EC766_1436_476B_B1C3_8ADF82C1D794__INCLUDED_)
#define AFX_DATASOCKET_H__BD5EC766_1436_476B_B1C3_8ADF82C1D794__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataSocket.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CDataSocket command target

class CDataSocket : public CAsyncSocket
{
// Attributes
public:
	BOOL m_bConnected;

// Operations
public:
	CDataSocket(class CControlSocket*);
	virtual ~CDataSocket();
	void RetrieveFile(LPCTSTR lpszFilename);
	void SendFile(LPCTSTR lpszFilename);
	void SendListing(LPCTSTR lpszListing);

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataSocket)
	public:
	virtual void OnAccept(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CDataSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
	int Receive();
	BOOL PrepareSendFile(LPCTSTR lpszFilename);
	BOOL PrepareReceiveFile(LPCTSTR lpszFilename);
	CFile m_File;
	CString m_strListing;
	DWORD m_nBytesTransfered;
	DWORD m_nTotalBytes;
	CControlSocket *m_pControlSocket;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATASOCKET_H__BD5EC766_1436_476B_B1C3_8ADF82C1D794__INCLUDED_)
