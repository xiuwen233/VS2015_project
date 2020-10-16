// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3897CF3B_5F98_4DB1_9373_EA4697D284A2__INCLUDED_)
#define AFX_STDAFX_H__3897CF3B_5F98_4DB1_9373_EA4697D284A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>		// MFC socket extensions
#include <afxconv.h>
#include <afxtempl.h>
#include <afxmt.h>

#define WM_THREADSTART WM_USER+100
#define WM_THREADCLOSE WM_USER+101
#define WM_DESTROYDATACONNECTION WM_USER+102

int GetLocalHostIP(CString&);
BOOL WaitWithMessageLoop(HANDLE, int);
CString BrowseForFolder(HWND, LPCTSTR, UINT);
BOOL MakeSureDirectoryPathExists(LPCTSTR);
BOOL FileExists(LPCTSTR, BOOL);
CString GetFileDate(CFileFind&);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__3897CF3B_5F98_4DB1_9373_EA4697D284A2__INCLUDED_)
