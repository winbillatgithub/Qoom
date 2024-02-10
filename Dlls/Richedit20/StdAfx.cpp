// stdafx.cpp : source file that includes just the standard includes
//	Riched20.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

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

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
void PopMsg(LPCTSTR pszFormat, ...) 
{
//#ifndef _DEBUG
//	return;
//#endif
   va_list argList;
   va_start(argList, pszFormat);

   TCHAR sz[1024];
//#ifdef _UNICODE
//   vswprintf(sz, pszFormat, argList);
//#else
//   vsprintf(sz, pszFormat, argList);
//#endif
   wvsprintf(sz, pszFormat, argList);
   va_end(argList);
   ::MessageBox(NULL, sz, _T("Pop Msg"), MB_OK);
}

void ReportErr(LPCTSTR str)
{
	    LPVOID lpMsgBuf;
        FormatMessage( 
           FORMAT_MESSAGE_ALLOCATE_BUFFER | 
           FORMAT_MESSAGE_FROM_SYSTEM | 
           FORMAT_MESSAGE_IGNORE_INSERTS,
           NULL,
           ::GetLastError(),
           MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // Default language
           (LPTSTR) &lpMsgBuf,
           0,
           NULL 
       );
		::MessageBox( NULL, (LPCTSTR)lpMsgBuf, 
		   str, MB_OK | MB_ICONINFORMATION );
       // Free the buffer.
       LocalFree( lpMsgBuf );
}

void ReportErrEx(LPCTSTR pszFormat, ...) 
{
   va_list argList;
   va_start(argList, pszFormat);

   TCHAR sz[1024];
   wvsprintf(sz, pszFormat, argList);
   va_end(argList);
   
   LPVOID lpMsgBuf;
   FormatMessage( 
         FORMAT_MESSAGE_ALLOCATE_BUFFER | 
         FORMAT_MESSAGE_FROM_SYSTEM | 
         FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL,
         ::GetLastError(),
         MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // Default language
         (LPTSTR) &lpMsgBuf,
         0,
         NULL 
    );
	::MessageBox( NULL, (LPCTSTR)lpMsgBuf, 
		   sz, MB_OK | MB_ICONINFORMATION );
    // Free the buffer.
    LocalFree( lpMsgBuf );
}

void CreateFileName(LPTSTR szNewFilename, LPCTSTR szPrefix, LPCTSTR szSuffix)
{
	SYSTEMTIME tm;
	::GetLocalTime(&tm);
    
#ifndef _UNICODE
    sprintf(szNewFilename, _T("%s%d%02d%02d%02d%02d%02d%s"), 
		szPrefix, tm.wYear-2000, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, szSuffix);
#else
	swprintf(szNewFilename, _T("%s%d%02d%02d%02d%02d%02d%s"), 
		szPrefix, tm.wYear-2000, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, szSuffix);
#endif
	return;
}