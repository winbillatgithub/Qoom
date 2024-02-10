// QQSpook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "QQSpook.h"

#include <Richedit.h>
#include <commctrl.h>
#include "shobjidl.h"		// For task bar

// Forward references
LRESULT CALLBACK CallWndProcHook(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
///////////////////////////////////////////////////////////////////////////////

// Instruct the compiler to put the g_hXXXhook data variable in 
// its own data section called Shared. We then instruct the 
// linker that we want to share the data in this section 
// with all instances of this application.
#pragma data_seg("Shared")
//Send Hook Handle
HHOOK g_hQQSpookSendHook[MAX_QQ_CONCUR_MAIN] = {NULL}; 
//Post Hook Handle
HHOOK g_hQQSpookPostHook[MAX_QQ_CONCUR_MAIN] = {NULL};

HWND  g_hQQSpookMainWnd[MAX_QQ_CONCUR_MAIN] = {NULL};
DWORD g_idQQSpookMainThread[MAX_QQ_CONCUR_MAIN] = {0};
HHOOK g_hQQSpookMainSendHook[MAX_QQ_CONCUR_MAIN] = {NULL};
HHOOK g_hQQSpookMainPostHook[MAX_QQ_CONCUR_MAIN] = {NULL};

//g_hQQSpookChatWnd is the same as inside Nightmare
HWND  g_hQQSpookChatWnd[MAX_QQ_CONCUR_MAIN][MAX_QQ_CONCUR_CHAT] = {NULL};      //Chat Window Handle
//Chat Window Thread
DWORD g_idQQSpookChatThread[MAX_QQ_CONCUR_MAIN] = {0};      

//Upper RichEdit Window Handle
HWND  g_hQQSpookUpperRichEdit[MAX_QQ_CONCUR_MAIN][MAX_QQ_CONCUR_CHAT] = {NULL};  
//Lower RichEdit Window Handle 
HWND  g_hQQSpookLowerRichEdit[MAX_QQ_CONCUR_MAIN][MAX_QQ_CONCUR_CHAT] = {NULL};  
//Send Button
HWND  g_hQQSpookSendButton[MAX_QQ_CONCUR_MAIN][MAX_QQ_CONCUR_CHAT] = {NULL};  
//E-mail address EditBox
HWND  g_hQQSpookAddressEdit[MAX_QQ_CONCUR_MAIN][MAX_QQ_CONCUR_CHAT] = {NULL};  

//Temporarily Save Chatter's Name
TCHAR g_szQQChatterName[MAX_QQ_CONCUR_CHAT][MAX_CHATTER_ADDRESS] = {NULL}; 

// If not chatting mode, convert it from message mode to chatting mode.
// Chat mode button
HWND  g_hChatModeButton = NULL;

TCHAR g_szQQSpookSendText[4096] = {NULL};

// For refresh improve.
DWORD g_dwQQHisSize[MAX_QQ_CONCUR_MAIN][MAX_QQ_CONCUR_CHAT] = {(DWORD)-1};	

//----------------------------------------------  
#pragma data_seg()

// Instruct the linker to make the Shared section
// readable, writable, and shared.
#pragma comment(linker, "/section:Shared,rws")

// Nonshared variables
HINSTANCE g_hinstDll = NULL;
//Rich Edit Stream Out
EDITSTREAM myStream = {
	0,			// dwCookie -- app specific
	0,			// dwError
	NULL		// Callback
};

DWORD CALLBACK writeFunc(
						 DWORD_PTR dwCookie, // application-defined value
						 LPBYTE pbBuff,      // data buffer
						 LONG cb,            // number of bytes to read or write
						 LONG *pcb           // number of bytes transferred
						 )
{
	LPBYTE lpMem = (LPBYTE)(dwCookie);
    LPBYTE lpByte = lpMem;
	
	DWORD dwSize, dwUsed, dwIndex, dwFlag;
	::CopyMemory(&dwSize, lpByte, sizeof(DWORD));
	lpByte += sizeof(DWORD);
	::CopyMemory(&dwUsed, lpByte, sizeof(DWORD));
    lpByte += sizeof(DWORD);
	::CopyMemory(&dwIndex, lpByte, sizeof(DWORD));
    lpByte += sizeof(DWORD);
	::CopyMemory(&dwFlag, lpByte, sizeof(DWORD));
    lpByte += sizeof(DWORD);
	
	lpByte += dwUsed;
	::memcpy(lpByte, pbBuff, cb);
	dwUsed += cb;
	
	lpByte = (LPBYTE)lpMem;
	lpByte += sizeof(DWORD);
	::CopyMemory(lpByte, &dwUsed, sizeof(DWORD));
	
	*pcb = cb;
	return 0;
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL, _T("QQSpook attatched"), _T(""), MB_OK);
			CoInitialize(NULL);
			g_hinstDll = (HINSTANCE)hModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			::CoUninitialize();
			break;
    }
    return TRUE;
}

//Note: it may be the QQ main window
BOOL WINAPI InitQQSpook(HWND hChatHwnd, BOOL bMain)
{
	//QQ main window may be created before OR after chat window!!!!!
//    if (::IsQQMain(hChatHwnd))
//		bMain = TRUE;

	DWORD td = ::GetWindowThreadProcessId(hChatHwnd, NULL);

	if (bMain)
	{
		int nInsert = 0;		// Insert position.
		BOOL bFound = FALSE;	// Flag of search.
		//PopMsg(_T("init InitQQSpook main 1"));
		for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
		{
			if (g_idQQSpookMainThread[i] == td) //re-enter
			{
				//PopMsg(_T("g_idQQSpookMainThread == td"));
				g_hQQSpookMainWnd[i] = hChatHwnd;
				return TRUE;
			}
			else if (g_idQQSpookMainThread[i] == 0 && bFound == FALSE)
			{
				bFound = TRUE;
				nInsert = i;
			}
		}
		// It means more than 10 QQ process has been hooked.
		// No more spaces.
		if (bFound == FALSE)
		{
			PopMsg(_T("more than 10 QQ process has been hooked."));
			return FALSE;
		}
		//PopMsg(_T("init InitQQSpook main 2"));
//		if (g_idQQSpookMainThread == 0)
		{
			//check if chat is being hooked or not, if hooked, re-use chat hook
            for(i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
			{
				if (g_idQQSpookChatThread[i] == td) //no need to hook
				{
					g_hQQSpookMainWnd[i] = hChatHwnd;
					g_hQQSpookMainSendHook[i] = g_hQQSpookSendHook[i];
                    g_hQQSpookMainPostHook[i] = g_hQQSpookPostHook[i];
				    g_idQQSpookMainThread[i] = g_idQQSpookChatThread[i];
				    return TRUE;
				}
				
			}

			//PopMsg(_T("init InitQQSpook main 3"));
			g_idQQSpookMainThread[nInsert] = td;
			g_hQQSpookMainWnd[nInsert] = hChatHwnd;
			//hook the first hook and return
            g_hQQSpookMainSendHook[nInsert] = SetWindowsHookEx(WH_CALLWNDPROC,
                       (HOOKPROC) CallWndProcHook,
					   g_hinstDll, 
		               g_idQQSpookMainThread[nInsert]);
        	if (g_hQQSpookMainSendHook[nInsert] == NULL) 
			{
				// Make sure that a hook has been installed.
                ::ReportErr(_T("in InitQQSpook -- SetMainSendHook[%d] return NULL"), nInsert);
		        BOOL b = UnhookWindowsHookEx(g_hQQSpookMainSendHook[nInsert]);
		        if (!b)
				{
					::ReportErr(_T("in InitQQSpook -- SetMainSendHook[%d] Fail->UnsetHook Fail"), nInsert);
				}
		        ::g_hQQSpookMainWnd[nInsert] = NULL;
                g_hQQSpookMainSendHook[nInsert] = NULL;
		        g_idQQSpookMainThread[nInsert] = 0;
        		return FALSE;
			}	
	
			g_hQQSpookMainPostHook[nInsert] = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc,
					       g_hinstDll, 
		                   g_idQQSpookMainThread[nInsert]);
	        if (g_hQQSpookMainPostHook[nInsert] == NULL)
			{
				// Make sure that a hook has been installed.
                ::ReportErr(_T("in InitQQSpook -- SetMainPostHook[%d] return NULL"), nInsert);

                g_hQQSpookMainWnd[nInsert] = NULL;
                g_hQQSpookMainSendHook[nInsert] = NULL;
		        g_hQQSpookMainPostHook[nInsert] = NULL;
		        g_idQQSpookMainThread[nInsert]  = 0;
		       return FALSE;
			}
//			PopMsg(_T("init InitQQSpook main OK"));
			return TRUE; //
		}
//		else //shared section data error
//		{
//			PopMsg(_T("init InitQQSpook main err"));
//			g_hQQSpookMainWnd = NULL;
//            g_hQQSpookMainSendHook = NULL;
//		    g_hQQSpookMainPostHook = NULL;
//		    g_idQQSpookMainThread  = 0;
//		    return FALSE;
//		}
		return TRUE;
	}
	
	BOOL bFound = FALSE;
	int nInsertRow = 0;
	//PopMsg(_T("init InitQQSpook main 1"));
	for(int iRow = 0; iRow < MAX_QQ_CONCUR_MAIN; iRow++)
	{
		if (g_idQQSpookMainThread[iRow] == td) //no need to hook
		{
			//PopMsg(_T("init InitQQSpook main already"));
			//Set Hook To Previous Value
			DWORD dwCol = QQSpookInsertChatHwnd(hChatHwnd, iRow);
			if (dwCol == (UINT)-1) return FALSE;
			g_hQQSpookSendHook[iRow] = g_hQQSpookMainSendHook[iRow];
			g_hQQSpookPostHook[iRow] = g_hQQSpookMainPostHook[iRow];
			g_idQQSpookChatThread[iRow] = g_idQQSpookMainThread[iRow];

			//Note, it may fail
			QQEnumRichEdit(hChatHwnd, iRow, dwCol);
			return TRUE;
		}
		else if (g_idQQSpookMainThread[iRow] == 0 && bFound == FALSE)
		{
			bFound = TRUE;
			nInsertRow = iRow;
		}
	}
	if (bFound == FALSE)
	{
		PopMsg(_T("more than 10 QQ process has been hooked."));
		return FALSE;
	}
	PopMsg(_T("InitQQSpook can not access here."));
	//Check if this thread has been hooked, 
	//it should be yes unless MS QQ team create chat window on multithread in QQ Message 17.0
	for(iRow = 0; iRow < MAX_QQ_CONCUR_MAIN; iRow++)
	{
		if (g_idQQSpookChatThread[iRow] == td) //no need to hook
		{
			//Set Hook To Previous Value
			DWORD dwCol = QQSpookInsertChatHwnd(hChatHwnd, iRow);
			if (dwCol == (UINT)-1) return FALSE;
            g_hQQSpookSendHook[iRow] = g_hQQSpookSendHook[iRow];
            g_hQQSpookPostHook[iRow] = g_hQQSpookPostHook[iRow];
            g_idQQSpookChatThread[iRow] = g_idQQSpookChatThread[iRow];
			
			//Note, it may fail
			QQEnumRichEdit(hChatHwnd, iRow, dwCol);
			return TRUE;
		}		
	}
	
	PopMsg(_T("init InitQQSpook main 2"));
	//First Window On This Thread, Hook It
    DWORD dwCol= QQSpookInsertChatHwnd(hChatHwnd, nInsertRow);
    if (dwCol == (UINT)-1) return FALSE;
    g_idQQSpookChatThread[nInsertRow] = ::GetWindowThreadProcessId(hChatHwnd, NULL);
    
	// Install the hook on the specified thread
    g_hQQSpookSendHook[nInsertRow] = SetWindowsHookEx(WH_CALLWNDPROC,
                       (HOOKPROC) CallWndProcHook,
					   g_hinstDll, 
		               g_idQQSpookChatThread[nInsertRow]);

	if (g_hQQSpookSendHook[nInsertRow] == NULL) 
	{
		// Make sure that a hook has been installed.
        ::ReportErr(_T("in InitQQSpook -- SetSendHook[%d] return NULL"), nInsertRow);
		BOOL b = UnhookWindowsHookEx(g_hQQSpookSendHook[nInsertRow]);
		if (!b)
		{
			::ReportErr(_T("in InitQQSpook -- SetSendHook[%d] Fail->UnsetHook Fail"), nInsertRow);
		}
		::g_hQQSpookChatWnd[nInsertRow][dwCol] = NULL;
        g_hQQSpookSendHook[nInsertRow] = NULL;
		g_idQQSpookChatThread[nInsertRow] = 0;
		
		return FALSE;
	}	
	
	PopMsg(_T("init InitQQSpook main 3"));
	//Note, it may fail
	QQEnumRichEdit(hChatHwnd, nInsertRow, dwCol);
	
	g_hQQSpookPostHook[nInsertRow] = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc,
					       g_hinstDll, 
		                   g_idQQSpookChatThread[nInsertRow]);
	if (g_hQQSpookPostHook[nInsertRow] == NULL) 
	{
		// Make sure that a hook has been installed.
        ::ReportErr(_T("in InitQQSpook -- SetPostHook[%d] return NULL"), nInsertRow);

        g_hQQSpookChatWnd[nInsertRow][dwCol] = NULL;
        g_hQQSpookSendHook[nInsertRow] = NULL;
		g_hQQSpookPostHook[nInsertRow] = NULL;
		g_idQQSpookChatThread[nInsertRow] = 0;
		QQEnumRichEdit(NULL, nInsertRow, dwCol);
		return FALSE;
	}
    return TRUE;
}

BOOL WINAPI ExitQQSpook(HWND hChatHwnd, BOOL bMain)
{
	if (hChatHwnd == NULL) {
		return FALSE;
	}
//	BOOL bMain = FALSE;
	//QQ main window may be created before OR after chat window!!!!!
//    if (::IsQQMain(hChatHwnd))
//		bMain = TRUE;

	//PopMsg(_T("exit QQ"));
	if (bMain)
	{
		//PopMsg(_T("exit QQ main window 1"));
		BOOL bFound = FALSE;
		DWORD dwRow = 0;
		DWORD tid = ::GetWindowThreadProcessId(hChatHwnd, NULL);
		for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
		{
			if (g_idQQSpookMainThread[i] == tid) //error
			{
				bFound = TRUE;
				dwRow = i;
				break;
			}
		}
		if (bFound == FALSE)
		{
			PopMsg(_T("tid=%d-g_idQQSpookMainThread=%d, exit QQ main window, "), tid, g_idQQSpookMainThread[0]);
//			g_hQQSpookMainWnd = NULL;
//			g_hQQSpookMainSendHook = NULL;
//			g_hQQSpookMainPostHook = NULL;
//			g_idQQSpookMainThread  = 0;
			return FALSE;
		}
		else
		{
			DWORD dwThreadNum = 0;
			DWORD dwRow2 = (UINT)-1;
	        for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
			{
				if (g_idQQSpookChatThread[i] == tid) 
				{
					dwThreadNum++;
					dwRow2 = i;
				}
			}
			if (dwRow != dwRow2)
			{
//				PopMsg(_T("dwRow != dwRow2"));
			}
//			if (g_idQQSpookMainThread == tid) 
//				dwThreadNum++;

			if (dwThreadNum > 1)
			{
//				g_hQQSpookMainWnd = NULL;
//              g_hQQSpookMainSendHook = NULL;
//		        g_hQQSpookMainPostHook = NULL;
//		        g_idQQSpookMainThread  = 0;
//				PopMsg(_T("dwThreadNum[%d] > 1 "), dwThreadNum);
//		        return TRUE;
			}
			//PopMsg(_T("exit QQ main window 2 dwThreadNum %d"), dwThreadNum);
			//unhook completely
			BOOL b =  UnhookWindowsHookEx(g_hQQSpookMainSendHook[dwRow]);
            if (!b) 
			{
//				::ReportErr(_T("in ExitQQSpook -- UnSetMainSendHook[%d] return NULL"), dwRow);
		        if (g_hQQSpookMainPostHook[dwRow])
			        UnhookWindowsHookEx(g_hQQSpookMainPostHook[dwRow]);

		        g_hQQSpookMainWnd[dwRow] = NULL;
				for(i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
				{
					QQEnumRichEdit(NULL, dwRow, i);
				}
				
                g_hQQSpookMainSendHook[dwRow] = NULL;
				g_hQQSpookSendHook[dwRow] = NULL;

		        g_hQQSpookMainPostHook[dwRow] = NULL;
				g_hQQSpookPostHook[dwRow] = NULL;

		        g_idQQSpookMainThread[dwRow] = 0;
				g_idQQSpookChatThread[dwRow] = 0;
		        return TRUE;
			}

	        if (g_hQQSpookMainPostHook[dwRow])
		        UnhookWindowsHookEx(g_hQQSpookMainPostHook[dwRow]);

			g_hQQSpookMainWnd[dwRow] = NULL;
			for(i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
			{
				QQEnumRichEdit(NULL, dwRow, i);
			}
			
			g_hQQSpookMainSendHook[dwRow] = NULL;
			g_hQQSpookSendHook[dwRow] = NULL;
			
			g_hQQSpookMainPostHook[dwRow] = NULL;
			g_hQQSpookPostHook[dwRow] = NULL;
			
			g_idQQSpookMainThread[dwRow] = 0;
			g_idQQSpookChatThread[dwRow] = 0;
//			PopMsg(_T("exit QQ OK"));
			return TRUE;
		}
		return TRUE;
	}

	//PopMsg(_T("exit QQ chat???"));
	DWORD dwIndex = QQSpookQueryChatHwndIndex(hChatHwnd);
	if (dwIndex == (UINT)-1) return FALSE;
	WORD dwRow = LOWORD(dwIndex);
	WORD dwCol = HIWORD(dwIndex);

//	DWORD tid = ::g_idQQSpookChatThread[dwRow];
//	DWORD dwThreadNum = 0;
//	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
//	{
//		if (g_idQQSpookChatThread[i] == tid) dwThreadNum ++;
//	}
//	if (g_idQQSpookMainThread == tid) dwThreadNum++;

	//If the thread is used by other chat or main window, do not unhook it
//	if (dwThreadNum > 1)
//	{
//		g_hQQSpookChatWnd[dwIndex] = NULL;
//		g_hQQSpookSendHook[dwIndex] = NULL;
//		g_hQQSpookPostHook[dwIndex] = NULL;
//		g_idQQSpookChatThread[dwIndex] = 0;
		//erase richedit array
//		QQEnumRichEdit(NULL, dwRow, dwCol);
//		PopMsg(_T("dwThreadNum[%d] > 1 "), dwThreadNum);
//		return TRUE;
//	}
	// Reset the refresh flag
	g_dwQQHisSize[dwRow][dwCol] = (DWORD)-1;

    g_hQQSpookSendHook[dwRow] = NULL;
	g_hQQSpookPostHook[dwRow] = NULL;
	g_idQQSpookChatThread[dwRow] = 0;
	QQEnumRichEdit(NULL, dwRow, dwCol);
/*
	//the last chat running on the thread ready to quit
	BOOL b =  UnhookWindowsHookEx(g_hQQSpookSendHook[dwRow]);
    if (!b) 
	{
        ::ReportErr(_T("in ExitQQSpook -- UnSetSendHook[%d] return NULL"), dwRow);
		if (g_hQQSpookPostHook[dwRow])
			UnhookWindowsHookEx(g_hQQSpookPostHook[dwRow]);

        g_hQQSpookSendHook[dwRow] = NULL;
		g_hQQSpookPostHook[dwRow] = NULL;
		g_idQQSpookChatThread[dwRow] = 0;
		QQEnumRichEdit(NULL, dwRow, dwCol);
		return FALSE;  
	}

	if (g_hQQSpookPostHook[dwRow])
		UnhookWindowsHookEx(g_hQQSpookPostHook[dwRow]);

    g_hQQSpookSendHook[dwRow] = NULL;
	g_hQQSpookPostHook[dwRow] = NULL;
	g_idQQSpookChatThread[dwRow] = 0;
	QQEnumRichEdit(NULL, dwRow, dwCol);	
*/
	return TRUE;
}

DWORD QQSpookGetChatNumber()
{
	DWORD dwRet = 0;
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hQQSpookChatWnd[j][i]) dwRet++;
		}
	}
	return dwRet;
}

// return the column index.
DWORD QQSpookInsertChatHwnd(HWND hChatWnd, DWORD dwRow)
{
	if (dwRow < 0 || dwRow >= MAX_QQ_CONCUR_MAIN)
	{
		return (UINT)-1;
	}
	for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
	{
		if (g_hQQSpookChatWnd[dwRow][i] == NULL)
		{
//			::ReportErr(_T("QQSpookInsertChatHwnd=%X row=%d col=%d"), (DWORD)hChatWnd, dwRow, i);
			g_hQQSpookChatWnd[dwRow][i] = hChatWnd;
			return i;
		}
	}
	return (UINT)-1;
}

DWORD QQSpookQueryChatHwndIndex(HWND hChatWnd)
{
	if (hChatWnd == NULL) return (UINT)-1;
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hQQSpookChatWnd[j][i] == hChatWnd) 
				return DWORD(MAKELONG(j, i));
		}
	}
	return (UINT)-1;
}


BOOL CALLBACK EnumChildProc(
  HWND hWnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
	DWORD dwIndex = (DWORD)lParam;
	WORD wRow = LOWORD(dwIndex);
	WORD wCol = HIWORD(dwIndex);
	//I only consider RichEdit20W, not RichEdit20A since we are on Win2k+
	TCHAR szClassName[64] = {0};
	int nRet = GetClassName(hWnd, szClassName, 64);
    if (nRet == 0) return TRUE;

	// Cut the length to szClassName[8] = 0;
	// The send message window is the RichEdit that parent window's
	// Class is "AfxWnd42" in QQ 2005.
	if (::lstrcmpi(szClassName, _T("RichEdit")) == 0)
	{
		//Got It
		HWND hParent = GetParent(hWnd);
		if (hParent != NULL)
		{
			// It's parent window's class name is "AfxWnd42".
			TCHAR szParentClassName[64] = {0};
			nRet = GetClassName(hParent, szParentClassName, 64);
			if (nRet == 0) return TRUE;
			if (::lstrcmpi(szParentClassName, _T("AfxWnd42")) == 0)
			{
				if (g_hQQSpookLowerRichEdit[wRow][wCol] == NULL) 
				{
					g_hQQSpookLowerRichEdit[wRow][wCol] = hWnd;
					return TRUE;
				}
			}
		}
	}
	// The history message window's class is "RichEdit20A"
	// There is only one window's class named "RichEdit20A".
	if (::lstrcmpi(szClassName, _T("RichEdit20A")) == 0)
	{
		//Got It
		if (g_hQQSpookUpperRichEdit[wRow][wCol] == NULL) //this should be enumed first
		{
			g_hQQSpookUpperRichEdit[wRow][wCol] = hWnd;
			return TRUE;
		}
	}

//	if (::lstrcmp(szClassName, _T("Edit")) == 0)
//	{
//		//In Messenger 4.6, 4.7 Chatter Name Edit is the First Edit Child of the Chat Window
//		//In Messenger 5.0 Chatter Edit is the Second Edit Child of the chat Window
//		//the first Edit is the GrandChild of the Chat Window
//		if (::GetParent(::GetParent(hWnd)) != NULL) return TRUE;
//		if (g_hQQSpookAddressEdit[wRow][wCol] == NULL) 
//		{
//			g_hQQSpookAddressEdit[wRow][wCol] = hWnd;
//			return TRUE;
//		}
//	}

    if (::lstrcmp(szClassName, _T("Button")) == 0)
	{
		TCHAR szTitle[64] = {0};
		nRet = GetWindowText(hWnd, szTitle, 64);
		if (nRet > 2) 
		{
			TCHAR szUnicode[64] = {0};
			// we want to convert an MBCS string in lpszA
			MultiByteToWideChar(CP_ACP, 0, "?ëó(&S)", -1, szUnicode, 64);
			if (::lstrcmp(szTitle, szUnicode) == 0)
			{
				//PopMsg(_T("Got send button"));
				if (g_hQQSpookSendButton[wRow][wCol] == NULL) 
				{
					g_hQQSpookSendButton[wRow][wCol] = hWnd;
					return TRUE;
				}
			}
			// Because of QQ is not unicode version.
			// we MUST compare with unicode.
			memset(szUnicode, 0, 64);
			// we want to convert an MBCS string in lpszA
			MultiByteToWideChar(CP_ACP, 0, "„÷ìVñÕéÆ(&T)", -1, szUnicode, 64);
			
			if (::lstrcmp(szTitle, szUnicode) == 0)
			{
				g_hChatModeButton = hWnd;
				return TRUE;
			}
		}
	}

	return TRUE;
}

void QQCheckRichEdit()
{
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hQQSpookChatWnd[j][i] && g_hQQSpookUpperRichEdit[j][i] == NULL)
			{
				QQEnumRichEdit(g_hQQSpookChatWnd[j][i], j, i);
			}
		}
	}
}

BOOL QQEnumRichEdit(HWND hChatHwnd, DWORD dwRow, DWORD dwCol)
{
//	PopMsg(_T("QQEnumRichEdit 1"));
	if (hChatHwnd == NULL)
	{
		g_hQQSpookUpperRichEdit[dwRow][dwCol] = NULL;    //Upper RichEdit Window Handle
        g_hQQSpookLowerRichEdit[dwRow][dwCol] = NULL;    //Lower RichEdit Window Handle
        g_hQQSpookSendButton[dwRow][dwCol] = NULL;       //Send Button
        g_hQQSpookChatWnd[dwRow][dwCol] = NULL;          //Parent Window of Upper Rich Edit 
        g_hQQSpookAddressEdit[dwRow][dwCol] = NULL;      //E-mail address
		return TRUE;
	}
	g_hChatModeButton = NULL;
	
	g_hQQSpookUpperRichEdit[dwRow][dwCol] = NULL;
	g_hQQSpookLowerRichEdit[dwRow][dwCol] = NULL;
	//Enum all the sibling
    BOOL bRet = EnumChildWindows(
        hChatHwnd,       // handle to parent window
        EnumChildProc,       // callback function
        (LPARAM)MAKELONG((WORD)dwRow, (WORD)dwCol)  // application-defined value
    );
	if (g_hQQSpookUpperRichEdit[dwRow][dwCol] == NULL)
	{
		//PopMsg(_T("QQEnumRichEdit failed to find his edit"));
	}
	if (g_hQQSpookLowerRichEdit[dwRow][dwCol] == NULL)
	{
		//PopMsg(_T("QQEnumRichEdit failed to find message edit"));
	}
	if (g_hQQSpookSendButton[dwRow][dwCol] == NULL)
	{
		//PopMsg(_T("QQEnumRichEdit failed to find send button"));
	}
	if (g_hChatModeButton != NULL)
	{
		::SendMessage(g_hChatModeButton, BM_CLICK, 0, 0);
		MessageBox(NULL, _T("Converted to message mode!"), _T("Warning"), MB_OK);
	}
	if (g_hQQSpookUpperRichEdit[dwRow][dwCol] != NULL &&
		g_hQQSpookLowerRichEdit[dwRow][dwCol] != NULL &&
		g_hQQSpookSendButton[dwRow][dwCol] != NULL)
	{
		//PopMsg(_T("QQEnumRichEdit[%d][%d] %X -- %X"), dwRow, dwCol, g_hQQSpookUpperRichEdit[dwRow][dwCol], g_hQQSpookLowerRichEdit[dwRow][dwCol]);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
//PostMessage Hook Proc
LRESULT WINAPI GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	MSG* msg = (MSG*)lParam;
	HWND hWnd = msg->hwnd;

	// Uncomment the line below to invoke the debugger 
    // on the process that just got the injected DLL.
    // ForceDebugBreak();
	if (msg->message == WM_QQSPOOK_DESTROYMAIN) {
		HWND hWnd = (HWND)msg->wParam;
		::PostQuitMessage(0);
    }
	else if (msg->message == WM_QQSPOOK_QUERYTEXT) //wParam : index , lParam : chat Handle
	{
		DWORD dwIndex = msg->wParam;
		DWORD dwRow = LOWORD(dwIndex);
		DWORD dwCol = HIWORD(dwIndex);
//		PopMsg(_T("Querytext hwnd=%x g_hQQSpookChatWnd[dwRow][dwCol] = %x row=%d col=%d"), hWnd, g_hQQSpookChatWnd[dwRow][dwCol], dwRow, dwCol);
		if (hWnd == g_hQQSpookChatWnd[dwRow][dwCol] && g_hQQSpookChatWnd[dwRow][dwCol] != NULL)
		{
			if (g_hQQSpookUpperRichEdit[dwRow][dwCol] == NULL) {
				::QQEnumRichEdit(g_hQQSpookChatWnd[dwRow][dwCol], dwRow, dwCol);
			}
			//PopMsg(_T("WM_QQSPOOK_QUERYTEXT"));
			QQInnerRichEditSaveText(g_hQQSpookUpperRichEdit[dwRow][dwCol], dwRow, dwCol);
		}
	}
	else if (msg->message == WM_QQSPOOK_QUERYCONTACTLIST)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
		{
			if (hWnd == ::g_hQQSpookMainWnd[i]) //it should be, but take care all time
			{
				::InnerQQMainSaveContactList(g_hQQSpookMainWnd[i]);
				break;
			}
		}
	}
	//WM_QQSPOOK_SENDTEXT -- wParam : ClearPreviosText , lParam : Send At Once
	else if (msg->message == WM_QQSPOOK_SENDTEXT) //Send Text To Lower RichEdit
	{
		DWORD dwIndex = msg->wParam;
		DWORD j = LOWORD(dwIndex);
		DWORD i = HIWORD(dwIndex);
		//PopMsg(_T("WM_QQSPOOK_SENDTEXT"));
		if (hWnd == g_hQQSpookChatWnd[j][i] && g_hQQSpookChatWnd[j][i] != NULL)
		{
			//PopMsg(_T("send message111111111"));
			if (g_hQQSpookLowerRichEdit[j][i] == NULL)
				::QQEnumRichEdit(g_hQQSpookChatWnd[j][i], j, i);
	 
			if (g_hQQSpookLowerRichEdit[j][i] == NULL || !::IsWindow(g_hQQSpookLowerRichEdit[j][i]))
			{
				//still failed? well, forget it then
				PopMsg(_T("Can not found the richedit."));
			}
			else
			{
				//PopMsg(_T("send message2222"));
				BOOL bClearPreviosText = TRUE;

				if (bClearPreviosText)
				{
					::SendMessage(g_hQQSpookLowerRichEdit[j][i], EM_SETSEL, (WPARAM)0, (LPARAM)-1);
					::SendMessage(g_hQQSpookLowerRichEdit[j][i], EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)NULL);
				}
        		SETTEXTEX st;
				st.flags = ST_DEFAULT; 
#ifndef _UNICODE
				st.codepage = CP_ACP;
#else
				st.codepage = 1200; //unicode
#endif
				//PopMsg(g_szQQSpookSendText);
				int len = ::lstrlen(g_szQQSpookSendText);
				TCHAR* szLocal = new TCHAR[len + 1];
				::lstrcpy(szLocal, g_szQQSpookSendText);
				szLocal[len] = 0;
				//PopMsg(_T("Sending --< %s"), szLocal);
				//DWORD dw = SendMessage( g_hQQSpookUpperRichEdit, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)szLocal);
				DWORD dw = ::SendMessage(g_hQQSpookLowerRichEdit[j][i], EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)szLocal);
				delete szLocal;
				//PopMsg(_T(" EM_SETTEXTEX %d"), dw);
				if (g_hQQSpookSendButton[j][i])
				{
					// Why the chat window will be destroyed???
					// Resolved:Set the window style to visable, but NOT update the window.
					// Then Minimize the chat window, every think will be ok...
					long lStyle = ::GetWindowLong(g_hQQSpookChatWnd[j][i], GWL_STYLE);
					lStyle |= WS_VISIBLE;
//					::SetWindowLong(g_hQQSpookChatWnd[j][i], GWL_STYLE, lStyle);
					
					::SendMessage(g_hQQSpookSendButton[j][i], BM_CLICK,
						 0, 
						 0);
//					::ShowWindow(g_hQQSpookChatWnd[j][i], SW_SHOW);
//					::SendMessage(::g_hQQSpookChatWnd[j][i], WM_COMMAND,
//						(WPARAM)MAKELONG((WORD)btnID, BN_CLICKED), 
//						(LPARAM)g_hQQSpookSendButton[j][i]);
					return 0;
				} else {
					PopMsg(_T("Failed to send QQ message!"));
				}
			}//if found upper richedit
		}//found the chat
	}

	//find the hook index
	hWnd = msg->hwnd;
	DWORD dwIndex = -1;
	DWORD tid = GetWindowThreadProcessId(hWnd, NULL);
	//g_hQQSpookChatWnd's parent is the Desktop Window
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (tid == ::g_idQQSpookChatThread[i]) 
		{
			dwIndex = i;	
			break;
		}
	}
	if (dwIndex != MAX_QQ_CONCUR_MAIN) //got
	{
		if (nCode < 0) 
		{
			// just pass it on 
	        return CallNextHookEx (g_hQQSpookPostHook[dwIndex], nCode, wParam, lParam) ;
		}  
	    return CallNextHookEx (g_hQQSpookPostHook[dwIndex], nCode, wParam, lParam) ;
	}
	else
	{
		for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
		{
			if (tid == ::g_idQQSpookMainThread[j]) 
			{
				return CallNextHookEx (::g_hQQSpookMainPostHook[j],
					nCode, wParam, lParam) ;
			}
		}
		return 0;
	}
}

//SendMessage Hook Proc
LRESULT CALLBACK CallWndProcHook(
  int nCode,      // hook code
  WPARAM wParam,  // If sent by the current thread, it is nonzero; otherwise, it is zero. 
  LPARAM lParam   // message data
)
{
	//::PopMsg(_T("CallWndProcHook"));
	CWPSTRUCT* pCwp = (CWPSTRUCT*)lParam;
    HWND hWnd = pCwp->hwnd;
	DWORD dwParam = (DWORD)pCwp->wParam;
			
	//WM_CLOSE is sent first, then WM_DESTROY, WM_NCDESTROY, and return, at last CLOSE return
	if (pCwp->message == WM_CLOSE) {
		//PopMsg(_T("CallWndProcHook::message == WM_CLOSE"));
		for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
		{
			for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
			{
				if (g_hQQSpookChatWnd[j][i] == pCwp->hwnd && g_hQQSpookChatWnd[j][i] != NULL)
				{
					::QQEnumRichEdit(g_hQQSpookChatWnd[j][i], j, i);
					DWORD dwRet = ::SendMessage(g_hQQSpookAddressEdit[j][i], WM_GETTEXT, 
						MAX_CHATTER_ADDRESS, (LPARAM)(LPCTSTR)g_szQQChatterName[i]);
					g_szQQChatterName[i][dwRet] = TCHAR('\0');
					QQInnerRichEditSaveText(g_hQQSpookUpperRichEdit[j][i], j, i);
				}
			}
			if (pCwp->hwnd == ::g_hQQSpookMainWnd[j])
			{
				::InnerQQMainSaveContactList(g_hQQSpookMainWnd[j]);
			}
		}
	} else if (pCwp->message == WM_VSCROLL && dwParam == SB_BOTTOM) {
		// This message indicate that QQ chat history window's text change.
//		PopMsg(_T("pCwp->message == WM_VSCROLL && dwParam == SB_BOTTOM"));
//		PopMsg(_T("pCwp->hwnd = %d"), pCwp->hwnd);
		for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
		{
			for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
			{ 
//				if (g_hQQSpookUpperRichEdit[j][i] != NULL) {
//					PopMsg(_T("%d-%d:%d"), j, i, g_hQQSpookUpperRichEdit[j][i]);
//				}
				if (g_hQQSpookChatWnd[j][i] != NULL && g_hQQSpookUpperRichEdit[j][i] == NULL) {
					QQEnumRichEdit(g_hQQSpookChatWnd[j][i], j, i);
				}
				if (g_hQQSpookUpperRichEdit[j][i] == pCwp->hwnd && 
					g_hQQSpookUpperRichEdit[j][i] != NULL)
				{
//					PopMsg(_T("row=%d,col=%d Changed"), j, i);
					QQQueryChatContents(j, i);
					break;
				}
			}
		}
	}

	//find the hook index
	hWnd = pCwp->hwnd;
	DWORD dwIndex = -1;
	DWORD tid = GetWindowThreadProcessId(hWnd, NULL);
	//g_hQQSpookChatWnd's parent is the Desktop Window
	for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
	{
		if (tid == ::g_idQQSpookChatThread[i])
		{
			dwIndex = i;
			break;
		}
	}
	if (dwIndex != MAX_QQ_CONCUR_MAIN) //got
	{
		if (nCode < 0) 
		{
			// just pass it on 
	        return CallNextHookEx (g_hQQSpookSendHook[dwIndex], nCode, wParam, lParam) ;
		}  
	    return CallNextHookEx (g_hQQSpookSendHook[dwIndex], nCode, wParam, lParam) ;
	}
	else
	{
		for(i = 0; i < MAX_QQ_CONCUR_MAIN; i++)
		{
			if (tid == ::g_idQQSpookMainThread[i]) 
			{
				return CallNextHookEx (::g_hQQSpookMainSendHook[i],
					nCode, wParam, lParam) ;
			}
		}
		return 0;
	}
}

BOOL WINAPI QQSetRichEditReadOnly(BOOL bReadOnly, DWORD dwRow, DWORD dwCol)
{
	if (::g_hQQSpookChatWnd[dwRow][dwCol] == NULL) return FALSE;
	::QQEnumRichEdit(g_hQQSpookChatWnd[dwRow][dwCol], dwRow, dwCol);
	if (g_hQQSpookUpperRichEdit[dwRow][dwCol] == NULL) return FALSE;
	::SendMessage(g_hQQSpookUpperRichEdit[dwRow][dwCol], EM_SETREADONLY,bReadOnly,0);
	//PostMessage(g_hQQSpookUpperRichEdit, WM_APP, 0,0);
	return TRUE;
}

BOOL QQInnerRichEditSaveText(HWND hRichEditWnd, DWORD dwRow, DWORD dwCol)
{
	HANDLE hMMF = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
            FALSE, g_MMF_NAME);
    if (hMMF == NULL) 
	{
		::ReportErr(_T("Open MMF Failed in AllocMMF QQInnerRichEditSaveText"));  return FALSE;
	}

	HANDLE hWriteEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_WRITE_EVENT_MMF_NAME);
	if (hWriteEvent == NULL)
	{
		::CloseHandle(hMMF);
		::ReportErr(_T("Open Write Event Failed in AllocMMF QQInnerRichEditSaveText"));  return FALSE;
	}

	HANDLE hReadEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_READ_EVENT_MMF_NAME);
	if (hReadEvent == NULL)
	{
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::ReportErr(_T("Open Read Event Failed in AllocMMF QQInnerRichEditSaveText")); return FALSE;
	}

	DWORD dwRet = ::WaitForSingleObject(hWriteEvent, MAX_WAIT);//INFINITE);
	if (dwRet == WAIT_ABANDONED)
	{
		::ReportErr(_T("QQInnerRichEditSaveText Write Wait Error")); 
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		return FALSE;
	}
	else if (dwRet == WAIT_TIMEOUT)
	{
		//::ReportErr(_T("Write Wait Time Out")); 
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		return FALSE;
	}
	
	::ResetEvent(hWriteEvent);

	//head 4 byte to record size
	LPVOID pView = MapViewOfFile(hMMF, 
		FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (pView == NULL)
	{
        ::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		::ReportErr(_T("QQInnerRichEditSaveText Write MapView Faield")); return FALSE;
	}

	LPBYTE lpByte = (LPBYTE)pView;
	DWORD dwSize, dwUsed;
	DWORD dwFlag = 0;
	DWORD dwIndex = (DWORD)MAKELONG(dwRow, dwCol);
	::CopyMemory(&dwSize, lpByte, sizeof(DWORD));
	lpByte += sizeof(DWORD);
	::CopyMemory(&dwUsed, lpByte, sizeof(DWORD));
    lpByte += sizeof(DWORD);
	::CopyMemory(lpByte, &dwIndex, sizeof(DWORD));
    lpByte += sizeof(DWORD);
	::CopyMemory(lpByte, &dwFlag, sizeof(DWORD));
    lpByte += sizeof(DWORD);
	
	// Read now.
	myStream.dwCookie = (DWORD_PTR)pView;
	myStream.dwError = 0;
	myStream.pfnCallback = writeFunc;
	
	LRESULT lr = 0;
	::SendMessage(
		hRichEditWnd,
		EM_STREAMOUT,						// message to send
		(WPARAM) (SF_RTF),					// format options 
		(LPARAM)(EDITSTREAM*)&myStream     // data (EDITSTREAM *)
		);
    
	::UnmapViewOfFile(pView);
	::CloseHandle(hMMF);
	::SetEvent(hReadEvent);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI QQSendChatText(LPCTSTR szText, 
	BOOL bClearPreviosText, BOOL bSendTextImmediately, DWORD dwRow, DWORD dwCol)
{
	if (::g_hQQSpookChatWnd[dwRow][dwCol] == NULL) return FALSE;
	//in case, relocate the richedit ctrl 
	//PopMsg(_T("QQEnumRichEdit handle:%X index:%d"), g_hQQSpookChatWnd[dwChatIndex], dwChatIndex);
	::QQEnumRichEdit(g_hQQSpookChatWnd[dwRow][dwCol], dwRow, dwCol);
	if (::g_hQQSpookUpperRichEdit[dwRow][dwCol] == NULL) return FALSE;
    //Copy text to shared section, it is a MUST! it moves the text to the target process -- QQ
    ::lstrcpy(g_szQQSpookSendText, szText);
	//PopMsg(g_szQQSpookSendText);
//	WriteLogInt(dwChatIndex, 10);
//	WriteLogInt((DWORD)g_hQQSpookChatWnd[dwChatIndex], 16);
	//Use Post is better
	::PostMessage(g_hQQSpookChatWnd[dwRow][dwCol], WM_QQSPOOK_SENDTEXT,
		WPARAM(MAKELONG(dwRow, dwCol)),
        0);

	return TRUE;
}

//LRESULT CALLBACK WindowProc(
//  HWND hwnd,       // handle to window
//  UINT uMsg,       // WM_COMMAND
//  WPARAM wParam,   // identifier of button, BN_CLICKED
//  LPARAM lParam    // handle to button (HWND)
//);
//Parameters
//wParam 
//The low-order word contains the button's control identifier. 
//The high-order word specifies the notification message. 
//
//lParam 
//Handle to the button. 

//Note: szChatterName must be long enough to hold the chatter name
BOOL WINAPI QQQueryChatterPersonName(LPCTSTR szChatterName, DWORD dwRow, DWORD dwCol)
{
	BOOL bSuccess = FALSE;

	if (::g_hQQSpookChatWnd[dwRow][dwCol] == NULL) return FALSE;
	::QQEnumRichEdit(g_hQQSpookChatWnd[dwRow][dwCol], dwRow, dwCol);
	if (::g_hQQSpookAddressEdit[dwRow][dwCol] == NULL) return FALSE;

	DWORD dwRet = ::SendMessage(g_hQQSpookAddressEdit[dwRow][dwCol], WM_GETTEXT, 
			MAX_CHATTER_ADDRESS, (LPARAM)(LPCTSTR)g_szQQChatterName[dwRow]);
    g_szQQChatterName[dwRow][dwRet] = TCHAR('\0');
	__try
	{
		::lstrcpy((LPTSTR)szChatterName, g_szQQChatterName[dwRow]);
		bSuccess = TRUE;
	}
	__finally
	{
		if (!bSuccess) return FALSE;
		else return TRUE;
	}
}

BOOL WINAPI QQSetChatterPersonName(LPCTSTR szChatterName, DWORD dwRow, DWORD dwCol)
{
	if (::g_hQQSpookChatWnd[dwRow][dwCol] == NULL) return FALSE;
	::QQEnumRichEdit(g_hQQSpookChatWnd[dwRow][dwCol], dwRow, dwCol);
	if (::g_hQQSpookAddressEdit[dwRow][dwCol] == NULL) return FALSE;

	int len = ::lstrlen(szChatterName);
    TCHAR* szLocal = new TCHAR[len + 1];
	::lstrcpy(szLocal, szChatterName);

	::SendMessage(g_hQQSpookAddressEdit[dwRow][dwCol], WM_SETTEXT, 0, (LPARAM)szLocal);
	
	delete szLocal;
	return TRUE;
}

BOOL WINAPI QQQueryChatContents(DWORD dwRow, DWORD dwCol)
{
	//PopMsg(_T("QueryChatContents1"));
/*
	if (::g_hQQSpookChatWnd[dwRow][dwCol] == NULL) return FALSE;
	if (g_hQQSpookUpperRichEdit[dwRow][dwCol] == NULL) {
		::QQEnumRichEdit(g_hQQSpookChatWnd[dwRow][dwCol], dwRow, dwCol);
	}
	if (g_hQQSpookUpperRichEdit[dwRow][dwCol] == NULL) return FALSE;
	// Here judge whether the text has changed
	// if no change, just return.
    GETTEXTLENGTHEX gtlex;
	gtlex.flags = GTL_DEFAULT;
	// QQ is not unicode.
	gtlex.codepage = CP_ACP;

	UpdateWindow(g_hQQSpookUpperRichEdit[dwRow][dwCol]);
	DWORD dwSize = SendMessage( 
        g_hQQSpookUpperRichEdit[dwRow][dwCol],		// handle to destination window 
        EM_GETTEXTLENGTHEX,						// message to send
        (WPARAM)&gtlex,							// text length (GETTEXTLENGTHEX *)
        (LPARAM)0								// not used; must be zero
		);
	// May be return the error code.
	if (dwSize == E_INVALIDARG ||
		dwSize == E_OUTOFMEMORY ||
		dwSize == E_UNEXPECTED ||
		dwSize == E_NOTIMPL ||
		dwSize == E_NOINTERFACE ||
		dwSize == E_POINTER ||
		dwSize == E_HANDLE ||
		dwSize == E_ACCESSDENIED) {
//		return FALSE;
	}
*/
	// the dwSize always changed with 2 values, such as 96 or 192 etc, because
	// the QQ richedit is ANSI version, not unicode version.
	// The MSDN said:
	// This behavior can occur when an application uses both ANSI functions and common dialogs, 
	// which use Unicode. It can also occur when an application uses the ANSI version of 
	// GetWindowTextLength with a window whose window procedure is Unicode, 
	// or the Unicode version of GetWindowTextLength with a window whose window procedure is ANSI.
	// The content changed.
	// So we use > to avoid this problem.
	// If dwSize is too big, it's bad length.
//	if (g_dwQQHisSize[dwRow][dwCol] == (DWORD)-1 ||
//		(dwSize > g_dwQQHisSize[dwRow][dwCol] && dwSize < MAX_HIS_CHAT)) {
//		g_dwQQHisSize[dwRow][dwCol] = dwSize;
		//PopMsg(_T("QueryChatContents2"));
		//You can not do this, for it is in your app process now
		//QQInnerRichEditSaveText(g_hQQSpookUpperRichEdit);
	::PostMessage(g_hQQSpookChatWnd[dwRow][dwCol], 
		WM_QQSPOOK_QUERYTEXT, 
		WPARAM(MAKELONG(dwRow, dwCol)),
		0);
//	}

	return TRUE;
}

BOOL WINAPI QQQueryContactList(DWORD dwRow)
{
	//PopMsg(_T("QueryContactList1"));
	if (g_hQQSpookMainWnd[dwRow] == NULL || !::IsWindow(g_hQQSpookMainWnd[dwRow]) || 
		!IsQQMain(g_hQQSpookMainWnd[dwRow]))
		return FALSE;
    //PopMsg(_T("QueryContactList2"));
    ::PostMessage(g_hQQSpookMainWnd[dwRow], WM_QQSPOOK_QUERYCONTACTLIST, 0,0);
	return TRUE;
}

BOOL CALLBACK CatchContactListView(
  HWND hWnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
	TCHAR szClassName[64];
	int nRet = GetClassName(hWnd, szClassName, 64);
    if (nRet == 0) return TRUE;
	
    BOOL bRet;
	bRet;
	if (::lstrcmp(szClassName, _T("SysListView32")) == 0)
	{
		::CopyMemory((LPVOID)lParam, &hWnd, sizeof(HWND));
		return FALSE;
	}
	return TRUE;
}

//get its child window of ListViewCtrl, crack it
BOOL InnerQQMainSaveContactList(HWND hQQMain)
{
	//it usually to be PluginHostClass\QQMSBLGeneric\SysListView32
	//Since I have no knowledge (or no time to try all version of QQ)
	//I enum all children and get the SysListView32
 	HWND hListView = NULL;
    EnumChildWindows(hQQMain, CatchContactListView, (LPARAM)&hListView);
    if (hListView == NULL) return FALSE;
    
	HANDLE hMMF = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
            FALSE, g_MMF_NAME);
    if (hMMF == NULL) 
	{
		::ReportErr(_T("Open MMF Failed in AllocMMF InnerMainSaveContactList"));  return FALSE;
	}

	HANDLE hWriteEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_WRITE_EVENT_MMF_NAME);
	if (hWriteEvent == NULL)
	{
		::CloseHandle(hMMF);
		::ReportErr(_T("Open Write Event Failed in AllocMMF InnerMainSaveContactList"));  return FALSE;
	}

	HANDLE hReadEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_READ_EVENT_MMF_NAME);
	if (hReadEvent == NULL)
	{
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::ReportErr(_T("Open Read Event Failed in AllocMMF InnerMainSaveContactList")); return FALSE;
	}

	DWORD dwRet = ::WaitForSingleObject(hWriteEvent, MAX_WAIT);//INFINITE);
	if (dwRet == WAIT_ABANDONED)
	{
		::ReportErr(_T("QQInnerRichEditSaveText Write Wait Error")); 
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		return FALSE;
	}
	else if (dwRet == WAIT_TIMEOUT)
	{
		//::ReportErr(_T("Write Wait Time Out")); 
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		return FALSE;
	}
	
	::ResetEvent(hWriteEvent);

	//heading 4 byte total size
	//heading 4 byte used size
	DWORD pos = 0;
	//head 4 byte to record size
	LPVOID pView = MapViewOfFile(hMMF, 
		FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (pView == NULL)
	{
        ::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		::ReportErr(_T("QQInnerRichEditSaveText Write MapView Faield")); return FALSE;
	}

	LPBYTE lpByte = (LPBYTE)pView;
	DWORD dwSize, dwUsed;
	::CopyMemory(&dwSize, lpByte, sizeof(DWORD));
	lpByte += sizeof(DWORD);
	::CopyMemory(&dwUsed, lpByte, sizeof(DWORD));
    lpByte += sizeof(DWORD);

	LPVOID lpMem = (LPVOID)lpByte; //Actual Data Head
    lpByte = (LPBYTE)lpMem;
	lpByte += dwUsed;

	HWND hListCtrl = hListView;
	int nMaxItems = ListView_GetItemCount(hListCtrl);
	//since the headCtrl have no use here, forget it
    

	int columnCount = 0;
    //Get HeadCtrl Text
	for(;;)
	{
		TCHAR szName[2 * MAX_PATH];
		LVCOLUMN lv;
		lv.mask = LVCF_TEXT;
		lv.cchTextMax = 2 * MAX_PATH;
        lv.pszText = szName;
		if (!ListView_GetColumn(hListCtrl,columnCount,&lv)) break;
		columnCount++;
		int len = ::lstrlen(szName);
        szName[len] = TCHAR('\t');
		
		lpByte = (LPBYTE)lpMem;
	    lpByte += dwUsed;
        ::CopyMemory(lpByte, szName, (len+1)*sizeof(TCHAR));
        dwUsed += (len+1)*sizeof(TCHAR);
	}

	//write \r\n
	lpByte = (LPBYTE)lpMem;
	lpByte += dwUsed;
	::CopyMemory(lpByte, _T("\r\n"), 2 * sizeof(TCHAR));
    dwUsed += 2*sizeof(TCHAR);

    for (int nItem = 0; nItem < nMaxItems; nItem++)
    {		
        TCHAR szName[MAX_PATH];
        ListView_GetItemText(hListCtrl, nItem, 0, szName, sizeof(szName)/sizeof(szName[0]));
        int len = ::lstrlen(szName);
	  
		szName[len] = TCHAR('\t');
		lpByte = (LPBYTE)lpMem;
	    lpByte += dwUsed;
	    ::CopyMemory(lpByte, szName, (len+1)*sizeof(TCHAR));
        dwUsed += (len+1)*sizeof(TCHAR);
	      
	    // then write the subItem text
	    if (columnCount > 1)
		{
			for(int m = 1; m < columnCount; m++)
			{
				TCHAR szSubName[2 * MAX_PATH];
			    ListView_GetItemText(hListCtrl, nItem, m, szSubName,
				    sizeof(szSubName)/sizeof(szSubName[0]));
			  
				len = lstrlen(szSubName);
	  		    szSubName[len] = TCHAR('\t');
				lpByte = (LPBYTE)lpMem;
	            lpByte += dwUsed;
		        ::CopyMemory(lpByte, szSubName, (len+1)*sizeof(TCHAR));
                dwUsed += (len+1)*sizeof(TCHAR);
			}
		}
		//write \r\n
		lpByte = (LPBYTE)lpMem;
	    lpByte += dwUsed;
	    ::CopyMemory(lpByte, _T("\r\n"), 2 * sizeof(TCHAR));
        dwUsed += 2*sizeof(TCHAR);
	}
   
	//append system time, local time is more meaningful, isn't it? man
	SYSTEMTIME tm;
	::GetLocalTime(&tm);
    
	TCHAR str[128];

    _stprintf(str, _T("\n%4d/%2d/%2d %2d:%2d:%2d\n"), 
		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

	lpByte = (LPBYTE)lpMem;
	lpByte += dwUsed;
    int len = ::lstrlen(str);
    ::CopyMemory(lpByte, str, len*sizeof(TCHAR));
	dwUsed += len*sizeof(TCHAR);
    
    //Write dwUsed back
	lpByte = (LPBYTE)pView;
	lpByte += sizeof(DWORD);
	::CopyMemory(lpByte, &dwUsed, sizeof(DWORD));
    
	::UnmapViewOfFile(pView);
	::CloseHandle(hMMF);
	::SetEvent(hReadEvent);
	return TRUE;
}


HWND WINAPI QQSpookGetChatWindowHandle(DWORD dwRow, DWORD dwCol)
{
	if (dwRow < 0 || dwRow >= MAX_QQ_CONCUR_MAIN ||
		dwCol < 0 || dwCol >= MAX_QQ_CONCUR_CHAT)
	{
		return ::g_hQQSpookChatWnd[dwCol][dwCol];
	}
	return NULL;
}

BOOL WINAPI QQShowChatWindow(BOOL bShow)
{
	BOOL bRet = FALSE;
	for(int j = 0; j < MAX_QQ_CONCUR_MAIN; j++)
	{
		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++)
		{
			if (g_hQQSpookChatWnd[j][i] != NULL) {
				bRet = TRUE;
				if (bShow == TRUE) {
					ShowQQChatWindow(g_hQQSpookChatWnd[j][i], TRUE);
				} else {
					ShowQQChatWindow(g_hQQSpookChatWnd[j][i], FALSE);
				}
			}
		}
	}
	return bRet;
}

//BOOL ShowQQChatWindow(BOOL bShowed, HWND hChat)
//{
//	if (bShowed) {
//		long lStyle = ::GetWindowLong(hChat, GWL_STYLE);
//		lStyle &= ~(WS_CHILD);
//		lStyle |= WS_VISIBLE;
//		::SetWindowLong(hChat, GWL_STYLE, lStyle);
//		::SetParent(hChat, NULL);
//		::ShowWindow(hChat, SW_SHOW);
//	} else {
//		// Get the parent tool window to hide the qq chat window.
//		HWND hParent = ::FindWindow(QQ_PARENT_WND_CLASS, NULL);
//		if (hParent == NULL) {
//			return FALSE;
//		}
//		long lStyle = ::GetWindowLong(hChat, GWL_STYLE);
//		lStyle |= WS_VISIBLE;	// If no visible, the chat window will auto-destroy!!!
//		lStyle &= ~(WS_POPUP);
//		::SetWindowLong(hChat, GWL_STYLE, lStyle);
//		// Remove WS_EX_APPWINDOW style
//		lStyle = ::GetWindowLong(hChat, GWL_EXSTYLE);
//		lStyle &= ~(WS_EX_APPWINDOW);
//		::SetWindowLong(hChat, GWL_EXSTYLE, lStyle);
//		
//		::SetParent(hChat, hParent);
//		::ShowWindow(hChat, SW_SHOW);
//	}
//	return TRUE;
//}

BOOL ShowQQChatWindow(HWND hChat, BOOL bShowed)
{
	if (hChat == NULL) {
		return FALSE;
	}
	//ç›appìIInitInstanceíÜâ¡ì¸::CoInitialize(NULL);
	HRESULT hr;
	ITaskbarList *pTaskbarList;
	// for test
	//	::ShowWindow(hChat, SW_HIDE);
	long lStyle = ::GetWindowLong(hChat, GWL_STYLE);
	if (!(lStyle & WS_CAPTION)) {
		lStyle |= WS_CAPTION;
		::SetWindowLong(hChat, GWL_STYLE, lStyle);
	}
	lStyle = ::GetWindowLong(hChat, GWL_EXSTYLE);
	if (!(lStyle & WS_EX_APPWINDOW)) {
		lStyle |= (WS_EX_APPWINDOW);
		::SetWindowLong(hChat, GWL_EXSTYLE, lStyle);
	}
	// For test.
	//	::ShowWindow(hChat, SW_SHOW);
	::UpdateWindow(hChat);

	hr=CoCreateInstance(CLSID_TaskbarList,NULL,CLSCTX_INPROC_SERVER,
		IID_ITaskbarList,(void**)&pTaskbarList);
	
	pTaskbarList->HrInit();
	
	HRESULT hResult = NOERROR;
	if(bShowed){
		hResult = pTaskbarList->AddTab(hChat);
		::ShowWindow(hChat, SW_RESTORE);
	}
	else{
		hResult = pTaskbarList->DeleteTab(hChat);
		::ShowWindow(hChat, SW_MINIMIZE);
	}
	
	pTaskbarList->Release();
	
	//ç›appìIExitInstanceíÜâ¡ì¸::CoUninitialize();
	if (hResult != NOERROR) {
		PopMsg(_T("Failed to remove task button"));
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI QQDestroyMainWindow(HWND hWnd)
{
	// Post WM_DESTROY message
	::PostMessage(hWnd, WM_QQSPOOK_DESTROYMAIN, WPARAM(hWnd), NULL);
	return TRUE;
}

