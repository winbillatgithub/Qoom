#ifndef MONITORSERVICE_
#define MONITORSERVICE_

#include <tchar.h>


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NIGHTMARE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MONITORSERVICE_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef MONITORSERVICE_EXPORTS
#define MONITORSERVICE_API __declspec(dllexport)
#else
#define MONITORSERVICE_API __declspec(dllimport)
#endif

#include "../../Common/GlobalDef.h"

// External function prototypes
//Hook Part
MONITORSERVICE_API BOOL WINAPI InitMonitor(HWND hCallerHwnd);
MONITORSERVICE_API BOOL WINAPI ExitMonitor();

//Hook into existing Target Window for QQ.
MONITORSERVICE_API DWORD WINAPI QQGetChatNumber();
MONITORSERVICE_API DWORD WINAPI QQGetActiveChat();

MONITORSERVICE_API BOOL  WINAPI QQSetActiveChat(DWORD dwActive);
MONITORSERVICE_API BOOL  WINAPI QQSetActiveChatWindow(HWND dwActiveWnd);

MONITORSERVICE_API  HWND WINAPI QQGetChatWindowHandle(DWORD dwRow, DWORD dwCol);
MONITORSERVICE_API  DWORD WINAPI QQGetChatWindowByHandle(HWND hWnd);
MONITORSERVICE_API  BOOL WINAPI QQGetChatWindowTitle(DWORD dwRow, DWORD dwCol, TCHAR *szBuf);

MONITORSERVICE_API  DWORD WINAPI QQInsertChatWindowHandle(HWND hWnd, DWORD dwRowIndex); //return index, -1 when fail
MONITORSERVICE_API  DWORD WINAPI QQDeleteChatWindowHandle(HWND hWnd);

MONITORSERVICE_API HWND WINAPI  GetQQMainHWND(DWORD dwIndex);

MONITORSERVICE_API DWORD WINAPI QQDeleteProcessId(DWORD dwProcessId);
MONITORSERVICE_API DWORD WINAPI QQSetProcessId(DWORD dwProcessId, DWORD dwIndex);
MONITORSERVICE_API DWORD WINAPI QQInsertProcessId(DWORD dwProcessId);
MONITORSERVICE_API DWORD WINAPI QQGetIndexByProcessId(DWORD dwProcessId);

MONITORSERVICE_API DWORD WINAPI QQDeleteMainWnd(HWND hMainWnd);
MONITORSERVICE_API DWORD WINAPI QQSetMainWnd(HWND hMainWnd, DWORD dwIndex);
MONITORSERVICE_API DWORD WINAPI QQInsertMainWnd(HWND hMainWnd);
MONITORSERVICE_API DWORD WINAPI QQGetIndexByMainWnd(HWND hMainWnd);
//Hook into existing Target Window for MSN.
MONITORSERVICE_API DWORD WINAPI MSNGetChatNumber();
MONITORSERVICE_API DWORD WINAPI MSNGetActiveChat();

MONITORSERVICE_API BOOL  WINAPI MSNSetActiveChat(DWORD dwActive);
MONITORSERVICE_API BOOL  WINAPI MSNSetActiveChatWindow(HWND dwActiveWnd);

MONITORSERVICE_API  HWND WINAPI MSNGetChatWindowHandle(DWORD dwIndex);
MONITORSERVICE_API  DWORD WINAPI MSNGetChatWindowByHandle(HWND hWnd);

MONITORSERVICE_API  DWORD WINAPI MSNInsertChatWindowHandle(HWND hWnd); //return index, -1 when fail
MONITORSERVICE_API  DWORD WINAPI MSNDeleteChatWindowHandle(HWND hWnd);

MONITORSERVICE_API HWND WINAPI  GetMSNMainHWND();
MONITORSERVICE_API BOOL WINAPI MSNGetChatWindowTitle(DWORD dwIndex, TCHAR *szBuf);

BOOL InvokeRichEdit(HWND hChatWnd);
#endif
