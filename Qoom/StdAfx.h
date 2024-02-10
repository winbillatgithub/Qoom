// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3BE71849_83E6_44C8_BD47_FE3598CFF955__INCLUDED_)
#define AFX_STDAFX_H__3BE71849_83E6_44C8_BD47_FE3598CFF955__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include "../Common/GlobalDef.h"

//the edit in the dialog, to keep it simple
extern HWND g_hChatSendEdit; 
extern HWND g_hMainDlg;
extern HWND g_hQQParent;

extern HWND g_hChatHisEditQQ[MAX_QQ_CONCUR_CHAT]; 
extern HWND g_hChatHisEditMSN[MAX_MSN_CONCUR_CHAT]; 

// For the history chat content.
extern DWORD g_dwActiveHis;
extern BOOL  g_bMSNActiveHis;

// For sending message
extern DWORD g_dwActiveSend;
extern BOOL g_bMSNActiveSend;

// For corresponding tabctl and data index.
extern DWORD g_hTabIndexQQ[MAX_QQ_CONCUR_CHAT]; 
extern DWORD g_hTabIndexMSN[MAX_MSN_CONCUR_CHAT]; 

// For multi-refresh
extern DWORD g_bHisChangedQQ[MAX_QQ_CONCUR_CHAT];
extern DWORD g_bHisChangedMSN[MAX_MSN_CONCUR_CHAT];

// Show/Hide window
extern BOOL  g_bMSNShowed;
extern BOOL  g_bQQShowed;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__3BE71849_83E6_44C8_BD47_FE3598CFF955__INCLUDED_)
