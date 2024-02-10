// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
/************************************
  REVISION LOG ENTRY
  Revision By: Zhang, Zhefu
  E-mail: codetiger@hotmail.com
  Revised on 10/2/2003 
  Comment: This is program code accompanying "COM Interface Hooking and Its Application"
           written by Zhefu Zhang posted on www.codeguru.com 
           You are free to reuse the code on the base of keeping this comment
		   All Right Reserved by author		   
 ************************************/

#if !defined(AFX_STDAFX_H__D9B0E2AA_3392_4494_99C8_4DA5EC2802F9__INCLUDED_)
#define AFX_STDAFX_H__D9B0E2AA_3392_4494_99C8_4DA5EC2802F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WINVER  0x0500  
#define _WIN32_WINNT  0x0500
#define _WIN32_IE 0x0600

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <tchar.h>

void PopMsg(LPCTSTR pszFormat, ...) ;
void ReportErr(LPCTSTR str);
void ReportErrEx(LPCTSTR pszFormat, ...) ;

//Always pass len>255 char array here, I am lazy to check boundary
void CreateFileName(LPTSTR szNewFilename, LPCTSTR szPrefix, LPCTSTR szSuffix);

//
#define WM_MSN6_QUERY_CHAT_TEXT   WM_USER + 117
//ret = handle 
#define WM_MSN6_QUERY_CHAT_HANDLE WM_USER + 116
//lParam = handle 
#define WM_MSN6_CLOSE_CHAT_HANDLE WM_USER + 115
//lParam = handle
#define WM_MSN6_SET_CHAT_HANDLE   WM_USER + 114

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D9B0E2AA_3392_4494_99C8_4DA5EC2802F9__INCLUDED_)
