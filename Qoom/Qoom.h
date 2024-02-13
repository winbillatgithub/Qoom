// Qoom.h : main header file for the QOOM application
//

#if !defined(AFX_QOOM_H__13A739B9_0F81_46C2_BE39_9FEA01C9B84C__INCLUDED_)
#define AFX_QOOM_H__13A739B9_0F81_46C2_BE39_9FEA01C9B84C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CQoomApp:
// See Qoom.cpp for the implementation of this class
//

class CQoomApp : public CWinApp
{
private:
	BOOL m_bFoundQQ;
	BOOL m_bFoundMSN;
	void RegisterOwnClass();
	BOOL IsQQMSNRunning();
public:
	HANDLE m_hEvent;
	CQoomApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQoomApp)
	public:
	virtual BOOL InitInstance();
	virtual int  ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CQoomApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QOOM_H__13A739B9_0F81_46C2_BE39_9FEA01C9B84C__INCLUDED_)
