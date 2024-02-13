#include "RecvWin.h"
#include "StdAfx.h"
#include "RichEd20.h"

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

#define WND_CLASS      _T("QOOM_RICHWND")
#define WND_TITLE      _T("QOOM_RICHWND")
#define QOOM_MAINWND   _T("QOOM_MAINWND")

#define MAX_SEND_CHAT 4096

#define WM_MSN_SETMESSAGE			WM_USER + 1001
#define WM_MSN_GETMESSAGE   		WM_USER + 1002
#define WM_MSN_INSERTHWND   		WM_USER + 1003
#define WM_MSN_DELETEHWND   		WM_USER + 1004
#define WM_MSN_GETCHATCNT   		WM_USER + 1005
#define WM_MSN_SETCHATCNT   		WM_USER + 1006

#define WM_MSN_SHOWCHATWINDOW        WM_USER + 4855

//Set chat data to richedit.
typedef struct tagMYREC
{
	TCHAR szText[MAX_SEND_CHAT];
	DWORD dwIndex;
	DWORD dwSize;
} MYREC;

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= 0; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL; //LoadIcon(hInstance, (LPCTSTR)IDI_DSA);
	wcex.hCursor		= NULL; //LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; // (LPCSTR)IDC_DSA;
	wcex.lpszClassName	= WND_CLASS; //szWindowClass;
	wcex.hIconSm		= NULL; //LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   return CreateWindow(WND_CLASS, WND_TITLE, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) 
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
//		case WM_TIMER:
//            ::QueryChatContent(0); //query the first chat only for now
//			break;
		case WM_MSN_GETMESSAGE:// Get the chat content from richedit.
			{
				DWORD nIndex = DWORD(wParam);
				::QueryChatContent(nIndex);
			}
			break;
		case WM_COPYDATA://WM_MSN_SETMESSAGE://:// Set the chat content to richedit.
			{
				PCOPYDATASTRUCT pMyCDS;
				pMyCDS = (PCOPYDATASTRUCT)lParam;
//				MessageBox(NULL, _T("WM_COPYDATA"), _T("WM_COPYDATA"), MB_OK);
				COPYDATASTRUCT *cds=(COPYDATASTRUCT*)(lParam);
//				MessageBox(NULL, (LPCTSTR) ((MYREC *)(pMyCDS->lpData))->szText, _T(""), MB_OK);
				::SetChatContent((LPCTSTR) ((MYREC *)(pMyCDS->lpData))->szText, 
					(DWORD) ((MYREC *)(pMyCDS->lpData))->dwIndex);
			}
			break;
		case WM_MSN_INSERTHWND:// Insert the chat window's handle.
			{
				HWND hChat = HWND(wParam);
				InsertHandle(hChat);
			}
			break;
		case WM_MSN_DELETEHWND:// Remove the chat window's handle.
			{
				DWORD dwIndex = DWORD(wParam);
				CloseChatHandle(dwIndex);
			}
			break;
		case WM_MSN_GETCHATCNT:// Get the chatting number
			{
				DWORD dwCount = GetChatNumber();
				HWND hMainWnd = FindWindow(QOOM_MAINWND, NULL);
				::PostMessage(hMainWnd, WM_MSN_SETCHATCNT, WPARAM(dwCount), 0);
			}
			break;
		case WM_MSN_SHOWCHATWINDOW:// Show/Hide the MSN chatting windows.
			{
				DWORD dwShow = DWORD(wParam);
				BOOL bShow = (dwShow == 1) ? TRUE : FALSE;
				MSNShowChatWindow(bShow);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
