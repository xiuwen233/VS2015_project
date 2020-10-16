// iFTP.h : main header file for the IFTP application
//

#if !defined(AFX_IFTP_H__CA25D189_9924_412A_BCD1_0E77D9D7C4F8__INCLUDED_)
#define AFX_IFTP_H__CA25D189_9924_412A_BCD1_0E77D9D7C4F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIFTPApp:
// See iFTP.cpp for the implementation of this class
//

class CIFTPApp : public CWinApp
{
public:
	CIFTPApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIFTPApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CIFTPApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IFTP_H__CA25D189_9924_412A_BCD1_0E77D9D7C4F8__INCLUDED_)
