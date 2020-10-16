// stdafx.cpp : source file that includes just the standard includes
//	iFTP.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

int GetLocalHostIP(CString &strIPAddress)
{
	PHOSTENT hostinfo;
	char szHostName[256];
	if(gethostname(szHostName, sizeof(szHostName)) ||
		NULL == (hostinfo = gethostbyname(szHostName)))
		return -1;
	strIPAddress = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
	return 0;
}

BOOL WaitWithMessageLoop(HANDLE hEvent, int nTimeout)
{
	while (TRUE)
	{
		DWORD dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, nTimeout, QS_ALLINPUT);
		if (dwRet == WAIT_OBJECT_0)
		{
			TRACE0("WaitWithMessageLoop() event triggered.\n");
			return TRUE;
		}
		else
		{
			if (dwRet == WAIT_OBJECT_0 + 1)
				AfxGetApp()->PumpMessage();
			else
				return dwRet != WAIT_TIMEOUT;
		}
	}
}

CString BrowseForFolder(HWND hWnd, LPCTSTR lpszTitle, UINT nFlags)
{
	CString strResult;
	LPMALLOC pMalloc;
	if(::SHGetMalloc(&pMalloc) != S_OK)
		return strResult;
	TCHAR szBuffer[_MAX_PATH];
	BROWSEINFO bi;
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szBuffer;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = nFlags;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	LPITEMIDLIST pidl;
	if((pidl = ::SHBrowseForFolder(&bi)) != NULL)
	{
		if(::SHGetPathFromIDList(pidl, szBuffer))
			strResult = szBuffer;
		else
			AfxMessageBox(_T("设置失败，不是有效的文件或目录名。"), MB_ICONSTOP|MB_OK);
		pMalloc->Free(pidl);
		pMalloc->Release();
	}
	return strResult;
}

BOOL MakeSureDirectoryPathExists(LPCTSTR lpszDirPath)
{
	CString strDirPath = lpszDirPath;
	int nPos = 0;
	while((nPos = strDirPath.Find(_T('\\'), nPos+1)) != -1)
		CreateDirectory(strDirPath.Left(nPos), NULL);
	return CreateDirectory(strDirPath, NULL);
}

BOOL FileExists(LPCTSTR lpszFileName, BOOL bIsDirCheck)
{
	DWORD dwAttributes = GetFileAttributes(lpszFileName);
	if (dwAttributes == 0xFFFFFFFF)
		return FALSE;
	if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		return bIsDirCheck;
	else
		return !bIsDirCheck;
}

CString GetFileDate(CFileFind &find)
{
	CString strResult;
	CTime time = CTime::GetCurrentTime();
	find.GetLastWriteTime(time);
	CTimeSpan timeSpan = CTime::GetCurrentTime() - time;
	if (timeSpan.GetDays() > 356)
		strResult = time.Format(_T(" %b %d %Y "));
	else
		strResult.Format(_T(" %s %02d:%02d "), time.Format(_T("%b %d")), time.GetHour(), time.GetMinute());
	return strResult;
}
