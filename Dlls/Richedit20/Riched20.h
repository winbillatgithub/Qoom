
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

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RICHED20_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RICHED20_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef RICHED20_EXPORTS
#define RICHED20_API __declspec(dllexport)
#else
#define RICHED20_API __declspec(dllimport)
#endif

//Note: DWORD(-1) means reserved area!!!
BOOL  InsertHandle(HWND hChat);
BOOL  InsertInterface(DWORD lpInterface);
BOOL  InitializeRecv(BOOL bInitialize);

DWORD GetChatNumber();
BOOL  CloseChatHandle(DWORD dwIndex);
BOOL  MSNShowChatWindow(BOOL bShow);

BOOL  QueryChatContent(DWORD dwIndex);
BOOL  SetChatContent(LPCTSTR pStr, DWORD nIndex);
BOOL  QueryAllRichEdit();

BOOL MSNInnerRichEditSaveText(LPCTSTR szText, DWORD dwSize);
BOOL MSNInnerRichEditSaveText(DWORD dwInterface, DWORD dwIndex, DWORD dwSize);

extern "C" RICHED20_API GUID IID_IRichEditOle;
extern "C" RICHED20_API GUID IID_IRichEditOleCallback;
extern "C" RICHED20_API GUID IID_ITextServices;
extern "C" RICHED20_API GUID IID_ITextHost;
extern "C" RICHED20_API GUID IID_ITextHost2;

#include <unknwn.h>
#include "mytextserv.h"

extern "C" HRESULT WINAPI  CreateTextServices(
	IUnknown *punkOuter, ITextHost *pITextHost, IUnknown **ppUnk);

extern "C" LRESULT WINAPI  REExtendedRegisterClass(HINSTANCE hInstance);
extern "C" LRESULT WINAPI  RichEdit10ANSIWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
extern "C" LRESULT WINAPI  RichEditANSIWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);


//Ordinal ^	Hint	Function	Entry Point	
//   2  (0x0002)	1  (0x0001)	IID_IRichEditOle	0x000203F8	
//   3  (0x0003)	2  (0x0002)	IID_IRichEditOleCallback	0x00020408	
//   4  (0x0004)	0  (0x0000)	CreateTextServices	0x0000DC1E	
//   5  (0x0005)	5  (0x0005)	IID_ITextServices	0x0000C9C8	
//   6  (0x0006)	3  (0x0003)	IID_ITextHost	0x0000E870	
//   7  (0x0007)	4  (0x0004)	IID_ITextHost2	0x0000E880	
//   8  (0x0008)	6  (0x0006)	REExtendedRegisterClass	0x000437FE	
//   9  (0x0009)	7  (0x0007)	RichEdit10ANSIWndProc	0x00031940	
// 10  (0x000A)	8  (0x0008)	RichEditANSIWndProc	0x00013CF8	


