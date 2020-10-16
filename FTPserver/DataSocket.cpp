// DataSocket.cpp : implementation file
//

#include "stdafx.h"
#include "iFTP.h"
#include "DataSocket.h"
#include "ControlSocket.h"
#include "ConnectThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataSocket

CDataSocket::CDataSocket(CControlSocket *pSocket)
{
	m_pControlSocket = pSocket;
	m_bConnected = FALSE;
}

CDataSocket::~CDataSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CDataSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CDataSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CDataSocket member functions

void CDataSocket::SendListing(LPCTSTR lpszListing)
{
	m_strListing = lpszListing;
	OnSend(0);
}

void CDataSocket::SendFile(LPCTSTR lpszFilename)
{
	if (!PrepareSendFile(lpszFilename))
	{
		m_pControlSocket->m_nStatus = STATUS_IDLE;
		m_pControlSocket->SendResponse("426 Connection closed; transfer aborted.");
		AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
		return;
	}
	OnSend(0);
}

void CDataSocket::RetrieveFile(LPCTSTR lpszFilename)
{
	if (!PrepareReceiveFile(lpszFilename))
	{
		m_pControlSocket->m_nStatus = STATUS_IDLE;
		m_pControlSocket->SendResponse("450 Can't access file.");
		AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
		return;
	}
}

BOOL CDataSocket::PrepareReceiveFile(LPCTSTR lpszFilename)
{
	if (m_File.m_hFile != CFile::hFileNull)
		m_File.Close();
	if (!m_File.Open(lpszFilename, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite))
		return FALSE;
	m_nBytesTransfered = 0;
	m_nTotalBytes = 0;
	return TRUE;
}

BOOL CDataSocket::PrepareSendFile(LPCTSTR lpszFilename)
{
	if (m_File.m_hFile != CFile::hFileNull)
		m_File.Close();
	if (!m_File.Open(lpszFilename, CFile::modeRead | CFile::shareDenyNone | CFile::typeBinary))
		return FALSE;
	m_nBytesTransfered = 0;
	m_nTotalBytes = m_File.GetLength();
	return TRUE;
}

void CDataSocket::OnAccept(int nErrorCode) 
{
	CAsyncSocket tmpSocket;
	Accept(tmpSocket);
	SOCKET socket = tmpSocket.Detach();
	Close();
	Attach(socket);
	m_bConnected = TRUE;
	CAsyncSocket::OnAccept(nErrorCode);
}

void CDataSocket::OnClose(int nErrorCode) 
{
	if (m_pControlSocket)
	{
		ShutDown(1);
		if (m_pControlSocket->m_nStatus == STATUS_UPLOAD)
		{
			while(Receive() != 0)
			{
				// receive data from buffer
			}
		}
		else
		{
			m_pControlSocket->SendResponse("426 Connection closed; transfer aborted.");
			AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
		}
	}
	m_pControlSocket->m_nStatus = STATUS_IDLE;	
	m_bConnected = FALSE;
	CAsyncSocket::OnClose(nErrorCode);
}

void CDataSocket::OnConnect(int nErrorCode) 
{
	m_bConnected = TRUE;
	if (nErrorCode)
	{
		m_pControlSocket->m_nStatus = STATUS_IDLE;
		m_pControlSocket->SendResponse("425 Can't open data connection.");
		AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
	}
	CAsyncSocket::OnConnect(nErrorCode);
}

void CDataSocket::OnReceive(int nErrorCode) 
{
	CAsyncSocket::OnReceive(nErrorCode);
	Receive();
}

#define PACKET_SIZE 4096
void CDataSocket::OnSend(int nErrorCode) 
{
	CAsyncSocket::OnSend(nErrorCode);
	switch(m_pControlSocket->m_nStatus)
	{
		case STATUS_LIST:
		{
			while (!m_strListing.IsEmpty())
			{
				USES_CONVERSION;
				LPCSTR lpszDataBlock = T2A(m_strListing.Left(PACKET_SIZE));
				if (Send(lpszDataBlock, strlen(lpszDataBlock)) == SOCKET_ERROR)
				{
					if (GetLastError() == WSAEWOULDBLOCK)
						break;
					else
					{
						Close();
						// m_strListing.Empty();
						m_pControlSocket->m_nStatus = STATUS_IDLE;
						m_pControlSocket->SendResponse("426 Connection closed; transfer aborted.");
						AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
					}
				}
				else
					m_strListing = m_strListing.Mid(PACKET_SIZE);
			}
			if (m_strListing.IsEmpty())
			{
				Close();
				// m_strListing.Empty();
				m_pControlSocket->m_nStatus = STATUS_IDLE;
				m_pControlSocket->SendResponse("226 Transfer complete");
				AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
			}
			break;
		}
		case STATUS_DOWNLOAD:
		{
			while (m_nBytesTransfered < m_nTotalBytes)
			{
				byte data[PACKET_SIZE];
				m_File.Seek(m_nBytesTransfered, CFile::begin);
				DWORD dwRead = m_File.Read(data, PACKET_SIZE);
				int dwBytes;
				if ((dwBytes = Send(data, dwRead)) == SOCKET_ERROR)
				{
					if (GetLastError() == WSAEWOULDBLOCK)
						break;
					else
					{
						m_File.Close();
						Close();
						// m_nTotalBytes = 0;
						// m_nBytesTransfered = 0;
						m_pControlSocket->m_nStatus = STATUS_IDLE;
						m_pControlSocket->SendResponse("426 Connection closed; transfer aborted.");
						AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
					}
				}
				else
				{
					m_nBytesTransfered += dwBytes;
					((CConnectThread *)AfxGetThread())->UpdateTime();
				}
			}
			if (m_nBytesTransfered == m_nTotalBytes)
			{
				m_File.Close();
				Close();
				// m_nTotalBytes = 0;
				// m_nBytesTransfered = 0;
				m_pControlSocket->m_nStatus = STATUS_IDLE;
				m_pControlSocket->SendResponse("226 Transfer complete");
				AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
			}
			break;
		}
	}
}

int CDataSocket::Receive()
{
	int nRead = 0;
	if (m_pControlSocket->m_nStatus == STATUS_UPLOAD)
	{
		if (m_File.m_hFile == CFile::hFileNull)
			return 0;
		byte data[PACKET_SIZE];
		nRead = CAsyncSocket::Receive(data, PACKET_SIZE);
		switch(nRead)
		{
		case 0:
			{
				m_File.Close();
				Close();
				m_pControlSocket->SendResponse("226 Transfer complete");
				AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
				break;
			}
		case SOCKET_ERROR:
			{
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					m_File.Close();
					Close();
					m_pControlSocket->SendResponse("426 Connection closed; transfer aborted.");
					AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
				}
				break;
			}
		default:
			{
				((CConnectThread *)AfxGetThread())->UpdateTime();
				TRY
				{
					m_File.Write(data, nRead);
				}
				CATCH_ALL(e)
				{
					m_File.Close();
					Close();
					m_pControlSocket->SendResponse("450 Can't access file.");
					AfxGetThread()->PostThreadMessage(WM_DESTROYDATACONNECTION, 0, 0);
					return 0;
				}
				END_CATCH_ALL;
			}
		}
	}
	return nRead;
}
