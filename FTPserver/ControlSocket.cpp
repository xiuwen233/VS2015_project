// ControlSocket.cpp : implementation file
//

#include "stdafx.h"
#include "iFTP.h"
#include "iFTPDlg.h"
#include "ControlSocket.h"
#include "ConnectThread.h"
#include "DataSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CControlSocket

CControlSocket::CControlSocket()
{
	m_nStatus = STATUS_LOGIN;
	m_pDataSocket = NULL;
	m_strRemoteHost.Empty();
	m_nRemotePort = -1;
	m_bPassiveMode = FALSE;
}

CControlSocket::~CControlSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CControlSocket, CSocket)
	//{{AFX_MSG_MAP(CControlSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CControlSocket member functions

BOOL __cdecl CControlSocket::SendResponse(LPCSTR pstrFormat, ...)
{
	va_list argList;
	int nMaxLen = sizeof(LPCSTR);
	va_start(argList, pstrFormat);
	for (LPCSTR lpsz = pstrFormat; *lpsz != '\0'; lpsz += sizeof(CHAR))
	{
		if (*lpsz != '%' || *(lpsz += sizeof(CHAR)) == '%')
		{
			nMaxLen += sizeof(CHAR);
			continue;
		}
		switch (*lpsz)
		{
			case 's':
			{
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg)
					nMaxLen += strlen(pstrNextArg);
				break;
			}
			case 'd':
				va_arg(argList, int);
				nMaxLen += 32;
				break;
			default:
				ASSERT(FALSE);
		}
	}
	LPSTR lpszTargets = new CHAR[nMaxLen * sizeof(CHAR)];
{
	LPSTR pData = lpszTargets;
	va_start(argList, pstrFormat);
	for (LPCSTR lpsz = pstrFormat; *lpsz != '\0'; lpsz += sizeof(CHAR))
	{
		if (*lpsz != '%' || *(lpsz += sizeof(CHAR)) == '%')
		{
			*pData = *lpsz;
			pData += sizeof(CHAR);
			continue;
		}
		switch (*lpsz)
		{
			case 's':
			{
				LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
				if (pstrNextArg)
				{
					strcpy(pData, pstrNextArg);
					pData += strlen(pstrNextArg);
				}
				break;
			}
			case 'd':
			{
				unsigned nValue = va_arg(argList, int);
				unsigned nLength = 1;
				unsigned nWidth = 10;
				while(nWidth <= nValue)
				{
					++nLength;
					nWidth *= 10;
				}
				LPSTR lpszTemp = (pData += nLength * sizeof(CHAR));
				while(nLength--)
				{
					pData -= sizeof(CHAR);
					*pData = '0' + (CHAR)(nValue % 10);
					nValue /= 10;
				}
				pData = lpszTemp;
				break;
			}
			default:
				ASSERT(FALSE);
		}
	}
	strcpy(pData, "\r\n");
}
	int nBytes = CSocket::Send(lpszTargets, strlen(lpszTargets));
	delete[] lpszTargets;
	if (nBytes == SOCKET_ERROR)
	{
		Close();
		m_pThread->PostThreadMessage(WM_QUIT, 1, 0);
		return FALSE;
	}
	((CConnectThread *)AfxGetThread())->UpdateTime();
	return TRUE;
}

void CControlSocket::DestroyDataConnection()
{
	if (!m_pDataSocket)
		return;
	delete m_pDataSocket;
	m_pDataSocket = NULL;
	m_strRemoteHost.Empty();
	m_nRemotePort = -1;
	m_bPassiveMode = FALSE;
}

void CControlSocket::OnClose(int nErrorCode) 
{
	Close();
	m_pThread->PostThreadMessage(WM_QUIT, 0, 0);
	
	CSocket::OnClose(nErrorCode);
}

#define BUFFERSIZE 4096
void CControlSocket::OnReceive(int nErrorCode) 
{
	CHAR buff[BUFFERSIZE+1];
	int nRead = Receive(buff, BUFFERSIZE);
	if(!nRead)
		Close();
	else if (nRead != SOCKET_ERROR)
	{		
		buff[nRead] = 0;
		m_RxBuffer += CString(buff);
		GetCommandLine();
	}
	
	CSocket::OnReceive(nErrorCode);
}

void CControlSocket::GetCommandLine()
{
	CString strTemp;
	int nIndex;
	while(!m_RxBuffer.IsEmpty())
	{
		nIndex = m_RxBuffer.Find(_T("\r\n"));
		if (nIndex != -1)
		{
			strTemp = m_RxBuffer.Left(nIndex);
			m_RxBuffer = m_RxBuffer.Mid(nIndex + 2);
			if (!strTemp.IsEmpty())
			{
				m_strCommands.AddTail(strTemp);
				ProcessCommand();
			}
		}
		else
			break;
	}
}

void CControlSocket::ProcessCommand()
{
	CString strCommand, strArguments;
	CString strBuff = m_strCommands.RemoveHead();
	int nIndex = strBuff.Find(_T(' '));
	if (nIndex == -1)
		strCommand = strBuff;
	else
	{
		strCommand = strBuff.Left(nIndex);
		strArguments = strBuff.Mid(nIndex+1);
	}
	strCommand.MakeUpper();
	USES_CONVERSION;
	if (strCommand.Right(4) == "ABOR")
		strCommand = "ABOR";
	if ((m_nStatus == STATUS_LOGIN) && (strCommand != "USER" && strCommand != "PASS"))
		SendResponse("530 Please login with USER and PASS.");
	else if (strCommand == "USER")
	{
		m_nStatus = STATUS_LOGIN;
		m_strUserName = strArguments;
		CString strPeerAddress;
		UINT nPeerPort;
		GetPeerName(strPeerAddress, nPeerPort);
		SendResponse("331 Please specify the password.");
	}
	else if (strCommand == "PASS")
	{
		if (m_strUserName.IsEmpty())
			SendResponse("503 Login with USER first.");
		else
		{
			if (!m_strUserName.CompareNoCase(pWnd->m_strName))
			{
				if (!strArguments.Compare(pWnd->m_strPassword) || pWnd->m_strPassword.IsEmpty() || !m_strUserName.CompareNoCase(_T("anonymous")))
				{
					m_strCurrentDir = _T("/");
					m_nStatus = STATUS_IDLE;
					SendResponse("230 Login successful.");
					return;
				}
			}
			SendResponse("530 Not logged in, user or password incorrect!");
		}
	}		
	else if ((strCommand == "QUIT") || (strCommand == "BYE"))
	{
		CConnectThread *pThread = (CConnectThread *)m_pThread;
		SendResponse("221 Goodbye!");
		Close();
		m_pThread->PostThreadMessage(WM_QUIT, 1, 0);
	}
	else if (strCommand == "TYPE")
		SendResponse("200 Type set to %s", T2A(strArguments));
	else if (strCommand == "PWD")
		SendResponse("257 \"%s\" is current directory.", T2A(m_strCurrentDir));
	else if (strCommand == "CDUP")
		DoChangeDirectory(_T(".."));
	else if (strCommand == "CWD")
		DoChangeDirectory(strArguments);
	else if (strCommand == "PORT")
	{
		CString strSub;
		int nCount=0;
		while (AfxExtractSubString(strSub, strArguments, nCount++, _T(',')))
		{
			switch(nCount)
			{
				case 1:	// a1
					m_strRemoteHost = strSub;
					m_strRemoteHost += _T(".");
					break;
				case 2:	// a2
					m_strRemoteHost += strSub;
					m_strRemoteHost += _T(".");
					break;
				case 3:	// a3
					m_strRemoteHost += strSub;
					m_strRemoteHost += _T(".");
					break;
				case 4:	// a4
					m_strRemoteHost += strSub;
					break;
				case 5:	// p1
					m_nRemotePort = 256*atoi(T2A(strSub));
					break;
				case 6:	// p2
					m_nRemotePort += atoi(T2A(strSub));
					break;
			}
		}
		SendResponse("200 PORT Command successful.");
	}
	else if (strCommand == "PASV")
	{	
		DestroyDataConnection();
		m_pDataSocket = new CDataSocket(this);
		if (!m_pDataSocket->Create())
		{
			DestroyDataConnection();	
			SendResponse("421 Failed to create socket.");
			return;
		}
		m_pDataSocket->Listen();
		m_pDataSocket->AsyncSelect();
		CString strIPAddress, strTmp;
		UINT nPort;
		GetSockName(strIPAddress, nPort);
		m_pDataSocket->GetSockName(strTmp, nPort);
		strIPAddress.Replace(_T('.'), _T(','));
		SendResponse("227 Entering Passive Mode (%s,%d,%d).", T2A(strIPAddress), nPort/256, nPort%256);
		m_bPassiveMode = TRUE;
	} 
	else if (strCommand == "LIST")
	{
		CString strListing;
		if (!GetDirectoryList(strArguments, strListing))// something went wrong
			return;
		SendResponse("150 Opening ASCII mode data connection for directory list."); 
		if (CreateDataConnection())
		{
			if (strListing.IsEmpty())
			{
				DestroyDataConnection();
				SendResponse("226 Transfer complete."); 
				m_nStatus = STATUS_IDLE;
				return;
			}
		}
		else
		{
			DestroyDataConnection();
			return;
		}
		m_nStatus = STATUS_LIST;
		m_pDataSocket->SendListing(strListing);
	}
	else if (strCommand == "RETR")
	{
		CString strResult;
		int nResult = CheckFileName(strArguments, FTP_DOWNLOAD, strResult);
		m_nStatus = STATUS_DOWNLOAD;
		switch(nResult)
		{
			case ERROR_ACCESS_DENIED:
				SendResponse("550 Permission denied.");
				break;
			case ERROR_FILE_NOT_FOUND:
				SendResponse("550 File not found.");
				break;
			default:
				SendResponse("150 Opening BINARY mode data connection for file transfer.");
				if (CreateDataConnection())
					m_pDataSocket->SendFile(strResult);
				else
				{
					m_nStatus = STATUS_IDLE;
					DestroyDataConnection();
				}
		}
	}
	else if (strCommand == "STOR")
	{
		CString strResult;
		int nResult = CheckFileName(strArguments, FTP_UPLOAD, strResult);
		m_nStatus = STATUS_UPLOAD;
		switch(nResult)
		{
			case ERROR_ACCESS_DENIED:
				SendResponse("550 Permission denied.");
				break;
			case ERROR_FILE_NOT_FOUND:
				SendResponse("550 Filename invalid.");
				break;
			default:
				SendResponse("150 Opening BINARY mode data connection for file transfer.");
				if (CreateDataConnection())
					m_pDataSocket->RetrieveFile(strResult);
				else
				{
					m_nStatus = STATUS_IDLE;
					DestroyDataConnection();
				}
		}
	}
	else if (strCommand == "SIZE")
	{
		CString strResult;
		int nResult = CheckFileName(strArguments, FTP_DOWNLOAD, strResult);
		switch(nResult)
		{
			case ERROR_ACCESS_DENIED:
				SendResponse("550 Permission denied.");
				break;
			case ERROR_FILE_NOT_FOUND:
				SendResponse("550 File not found.");
				break;
			default:
			{
				CFileStatus status;
				CFile::GetStatus(strResult, status);
				SendResponse("213 %d", status.m_size);
				break;
			}
		}
	}
	else if (strCommand == "DELE")
	{
		CString strResult;
		int nResult = CheckFileName(strArguments, FTP_DELETE, strResult);
		switch(nResult)
		{
			case ERROR_ACCESS_DENIED:
				SendResponse("550 Permission denied.");
				break;
			case ERROR_FILE_NOT_FOUND:
				SendResponse("550 File not found.");
				break;
			default:
				CString strRelativePath;
				GetRelativePath(strResult, strRelativePath);
				if (!DeleteFile(strResult))
					SendResponse("450 Internal error deleting the file: \"%s\".", T2A(strRelativePath));
				else
					SendResponse("250 File \"%s\" was deleted successfully.", T2A(strRelativePath));
		}
	}
	else if (strCommand == "RNFR")
	{
		int nResult = 0; 
		nResult = CheckFileName(strArguments, FTP_RENAME, strReName);
		if (nResult != ERROR_SUCCESS)
		{
			nResult = CheckDirectory(strArguments,FTP_RENAME,strReName);
			if (nResult != ERROR_SUCCESS)
			{
				switch(nResult)
				{
				case ERROR_ACCESS_DENIED:
					SendResponse("550 Permission denied.");
					break;
				case ERROR_PATH_NOT_FOUND:
					SendResponse("550 Directory not found.");
					break;
				case ERROR_FILE_NOT_FOUND:
					SendResponse("550 File not found.");
					break;
				}
			}
			else
				SendResponse("350 Directory exists, ready for destination name.");
		}
		else
			SendResponse("350 File exists, ready for destination name.");
	}
	else if (strCommand == "RNTO")
	{
		CString strResult;
		CheckFileName(strArguments, FTP_RENAME, strResult);
		if (!MoveFile(strReName,strResult))
			SendResponse("450 Internal error renamed the file.");
		else
			SendResponse("250 renamed successfully.");
	}
	else if (strCommand == "RMD")
	{
		CString strResult;
		int nResult = CheckDirectory(strArguments, FTP_DELETE, strResult);
		switch(nResult)
		{
			case ERROR_ACCESS_DENIED:
				SendResponse("550 Permission denied.");
				break;
			case ERROR_PATH_NOT_FOUND:
				SendResponse("550 Directory not found.");
				break;
			default:
				if (!RemoveDirectory(strResult))
				{
					if (GetLastError() == ERROR_DIR_NOT_EMPTY)
						SendResponse("550 Directory not empty.");
					else
						SendResponse("450 Internal error deleting the directory.");
				}
				else
					SendResponse("250 Directory deleted successfully.");
		}
	}
	else if (strCommand == "MKD")
	{		
		CString strResult;
		int nResult = CheckDirectory(strArguments, FTP_CREATE_DIR, strResult);
		switch(nResult)
		{
			case ERROR_SUCCESS:
				SendResponse("550 Directory already exists.");
				break;
			case ERROR_ACCESS_DENIED:
				SendResponse("550 Can't create directory. Permission denied.");
				break;
			default:
				if (!MakeSureDirectoryPathExists(strResult))
					SendResponse("450 Internal error creating the directory.");
				else
					SendResponse("250 Directory created successfully.");
		}
	}
	else if (strCommand == "ABOR")
	{
		if (m_pDataSocket)
		{
			if (m_nStatus != STATUS_IDLE)
				SendResponse("426 Data connection closed.");
			m_pThread->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
		}
		SendResponse("226 ABOR command successful.");
	} 
	else if (strCommand == "SYST")
		SendResponse("215 UNIX Type: L8");
	else if (strCommand == "NOOP")
		SendResponse("200 OK");
	else
		SendResponse("502 Command not implemented.");
}

void CControlSocket::DoChangeDirectory(LPCTSTR lpszDirectory)
{
	CString strLocalPath;
	USES_CONVERSION;
	int nResult = CheckDirectory(lpszDirectory, FTP_LIST, strLocalPath);
	switch(nResult)
	{
	case ERROR_ACCESS_DENIED:
		SendResponse("550 \"%s\": Permission denied.", T2A(lpszDirectory));
		break;
	case ERROR_PATH_NOT_FOUND:
		SendResponse("550 \"%s\": Directory not found.", T2A(lpszDirectory));
		break;
	default:
		CString strRelativePath;
		if (GetRelativePath(strLocalPath, strRelativePath))
			m_strCurrentDir = strRelativePath;
		SendResponse("250 \"%s\" is current directory.", T2A(m_strCurrentDir));
	}
}

int CControlSocket::CheckDirectory(CString strDirectory, int nOption, CString &strResult)
{
	strDirectory.Replace(_T('\\'),_T('/'));
	while(strDirectory.Replace(_T("//"),_T("/")));
	strDirectory.TrimRight(_T('/'));
	if (strDirectory.IsEmpty())
	{
		if (nOption == FTP_LIST)
			strDirectory = _T("/");
		else
			return ERROR_PATH_NOT_FOUND;
	}
	else
	{
		if (strDirectory.Left(1) != _T("/"))
		{
			if (m_strCurrentDir.Right(1) != _T('/'))
				strDirectory = m_strCurrentDir + _T("/") + strDirectory;
			else
				strDirectory = m_strCurrentDir + strDirectory;
		}
	}
	BOOL bPathExists = GetLocalPath(strDirectory, strResult);
	if (!bPathExists && (nOption != FTP_CREATE_DIR))
		return ERROR_PATH_NOT_FOUND;
	if (!CheckAccessRights(strResult, nOption))
		return ERROR_ACCESS_DENIED;
	if (!bPathExists)
		return ERROR_PATH_NOT_FOUND;
	return ERROR_SUCCESS;
}

int CControlSocket::CheckFileName(CString strFilename, int nOption, CString &strResult)
{
	strFilename.Replace(_T('\\'), _T('/'));
	while(strFilename.Replace(_T("//"), _T("/")));
	strFilename.TrimRight(_T('/'));
	if (strFilename.IsEmpty())
		return ERROR_FILE_NOT_FOUND;
	CString strDirectory = m_strCurrentDir;
	int nPos = strFilename.ReverseFind(_T('/'));
	if (nPos != -1)
	{
		strDirectory = strFilename.Left(nPos);
		if (strDirectory.IsEmpty())
			strDirectory = _T("/");
		strFilename = strFilename.Mid(nPos+1);
	}
	CString strLocalPath;
	if (!GetLocalPath(strDirectory, strLocalPath))
		return ERROR_FILE_NOT_FOUND;
	strResult = strLocalPath + _T("\\") + strFilename;
	BOOL bFileExist;
	bFileExist = FileExists(strResult, FALSE);
	if ((nOption != FTP_UPLOAD) && !bFileExist)
		return ERROR_FILE_NOT_FOUND;
	if ((nOption == FTP_UPLOAD) && bFileExist && (!pWnd->m_bAllowDelete))
		return ERROR_ACCESS_DENIED;
	if (!CheckAccessRights(strLocalPath, nOption))
		return ERROR_ACCESS_DENIED;
	return ERROR_SUCCESS;
}

BOOL CControlSocket::CheckAccessRights(LPCTSTR lpszDirectory, int nOption)
{
	CString strCheckDir = lpszDirectory;
	while(!strCheckDir.IsEmpty())
	{
		CString strPath1 = strCheckDir;
		strPath1.TrimRight(_T('\\'));
		CString strPath2 = pWnd->m_strHomeDirectory;
		strPath2.TrimRight(_T('\\'));
		if (strPath1.CompareNoCase(strPath2) == 0)
		{
			if (((!pWnd->m_bAllowDownload) && (nOption == FTP_DOWNLOAD)) ||
				((!pWnd->m_bAllowUpload) && (nOption == FTP_UPLOAD)) ||
				((!pWnd->m_bAllowRename) && (nOption == FTP_RENAME)) ||
				((!pWnd->m_bAllowDelete) && (nOption == FTP_DELETE)) ||
				((!pWnd->m_bAllowCreateDirectory) && (nOption == FTP_CREATE_DIR)) ||
				((!pWnd->m_bAllowDownload && !pWnd->m_bAllowUpload) && (nOption == FTP_LIST)))
				return FALSE;
			return TRUE;
		}
		int nPos = strCheckDir.ReverseFind(_T('\\'));
		if (nPos != -1)
			strCheckDir = strCheckDir.Left(nPos);
		else
			strCheckDir.Empty();
	} 
	return FALSE;
}

BOOL CControlSocket::GetDirectoryList(LPCTSTR lpszDirectory, CString &strResult)
{
	CString strDirectory = lpszDirectory;
	strResult.Empty();
	strDirectory.TrimRight(_T('*'));
	if (strDirectory.IsEmpty())
		strDirectory = m_strCurrentDir;
	CString strLocalPath;
	USES_CONVERSION;
	int nResult = CheckDirectory(strDirectory, FTP_LIST, strLocalPath);
	switch(nResult)
	{
	case ERROR_ACCESS_DENIED:
		SendResponse("550 \"%s\": Permission denied.", T2A(lpszDirectory));
		return FALSE;
	case ERROR_PATH_NOT_FOUND:
		SendResponse("550 \"%s\": Directory not found.", T2A(lpszDirectory));
		return FALSE;
	}
	CFileFind find;
	BOOL bFound = FALSE;
	if ((GetFileAttributes(strLocalPath) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		CString strPath = strLocalPath;
		if (strPath.Right(1)==_T("\\"))
			bFound = find.FindFile(strLocalPath + _T("*.*"));
		else
			bFound = find.FindFile(strLocalPath + _T("\\*.*"));
	}
	else
		bFound = find.FindFile(strLocalPath);
	while (bFound)
	{
		bFound = find.FindNextFile();
		if (find.IsDots())
			continue;
		if (find.IsDirectory())
			strResult += _T("drwx------");
		else
			strResult += _T("-rwx------");
		strResult += _T(" 1 user group ");
		CString strLength;
		strLength.Format(_T("%12d"), find.GetLength());
		strResult += strLength;
		strResult += GetFileDate(find);
		strResult += find.GetFileName();
		strResult += _T("\r\n");
	}
	return TRUE;
}

BOOL CControlSocket::CreateDataConnection()
{
	if (!m_bPassiveMode)
	{
		if (m_strRemoteHost.IsEmpty() || m_nRemotePort == -1)
		{
			SendResponse("425 Can't open data connection.");
			return FALSE;
		}
		m_pDataSocket = new CDataSocket(this);
		if (m_pDataSocket->Create())
		{
			m_pDataSocket->AsyncSelect();
			if (m_pDataSocket->Connect(m_strRemoteHost, m_nRemotePort) == 0)
			{
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					SendResponse("425 Can't open data connection.");
					return FALSE;
				}
			}
		}
		else
		{
			SendResponse("421 Failed to create data connection socket.");
			return FALSE;
		}
	}
	// wait for 10s
	DWORD dwTickCount = GetTickCount() + 10000;
	while (!m_pDataSocket->m_bConnected)
	{
		MSG msg;
		while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (dwTickCount < GetTickCount())
		{
			SendResponse("421 Failed to create data connection socket.");
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CControlSocket::GetRelativePath(LPCTSTR lpszLocalPath, CString &strRelativePath)
{
	CString strOffset = pWnd->m_strHomeDirectory;
	CString strLocalPath = lpszLocalPath;
	strOffset.MakeLower();
	strLocalPath.MakeLower();
	if (strOffset.Right(1) != _T("\\"))
		strOffset += _T("\\");
	if (strLocalPath.Right(1) != _T("\\"))
		strLocalPath += _T("\\");
	if (strOffset == strLocalPath)
		strRelativePath = _T("/");
	else
	{
		strRelativePath = strLocalPath;
		strRelativePath.Replace(strOffset, _T("/"));
		strRelativePath.Replace(_T('\\'), _T('/'));
		strRelativePath.TrimRight(_T('/'));
	}
	return TRUE;
}

BOOL CControlSocket::GetLocalPath(LPCTSTR lpszRelativePath, CString &strLocalPath)
{
	CString strRelativePath = lpszRelativePath;
	strRelativePath.Replace(_T('\\'), _T('/'));
	while(strRelativePath.Replace(_T("//"), _T("/")));
	CStringList partList;
	CString strSub;
	int nCount=0;
	BOOL bPathExists = TRUE;
	while(AfxExtractSubString(strSub, strRelativePath, nCount++, _T('/')))
	{
		if (strSub != _T(".."))
		{
			strSub.TrimLeft(_T('.'));
			strSub.TrimRight(_T('.'));
		}
		if (!strSub.IsEmpty())
			partList.AddTail(strSub);
	}
	CString strHomeDir;
	if (lpszRelativePath[0] == _T('/'))
		strHomeDir = pWnd->m_strHomeDirectory;
	else
		GetLocalPath(m_strCurrentDir, strHomeDir);
	while(!partList.IsEmpty())
	{
		CString strPart = partList.GetHead();
		partList.RemoveHead();
		CString strCheckDir;
		if (strPart == _T(".."))
		{
			int nPos = strHomeDir.ReverseFind(_T('\\'));
			if (nPos != -1)
				strCheckDir = strHomeDir.Left(nPos);
		}
		else
			strCheckDir = strHomeDir + _T("\\") + strPart;
		if (FileExists(strCheckDir, TRUE))
			strHomeDir = strCheckDir;
		else
		{
			if (FileExists(strCheckDir, FALSE))
				strHomeDir = strCheckDir;
			else
			{
				strHomeDir = strCheckDir;
				bPathExists = FALSE;
			}
		}
	}
	strLocalPath = strHomeDir;
	if (strLocalPath.Right(1) == _T(":"))
		strLocalPath += _T("\\");
	return bPathExists;
}
