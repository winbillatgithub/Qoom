// MSNSpook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "MSNSpook.h"
#include <Richedit.h>
#include <commctrl.h>
#include "windows.h"
#include "winuser.h"
#include "winable.h"
#include "../../Common/GlobalDef.h"
//#include "../Richedit20/riched20.h"

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
HHOOK g_hMSNSpookSendHook[MAX_MSN_CONCUR_CHAT] = 
{NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL}; 
//Post Hook Handle
HHOOK g_hMSNSpookPostHook[MAX_MSN_CONCUR_CHAT] = 
{NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL};

HWND  g_hMSNSpookMainWnd = NULL;
DWORD g_idMSNSpookMainThread = 0;
HHOOK g_hMSNSpookMainSendHook = NULL;
HHOOK g_hMSNSpookMainPostHook = NULL;

//g_hMSNSpookChatWnd is the same as inside Nightmare
HWND  g_hMSNSpookChatWnd[MAX_MSN_CONCUR_CHAT] = {NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL};      //Chat Window Handle
//Chat Window Thread
DWORD g_idMSNSpookChatThread[MAX_MSN_CONCUR_CHAT] = {0, 0, 0, 0, 0, 
0, 0, 0, 0, 0};      

//Upper RichEdit Window Handle
HWND  g_hMSNSpookUpperRichEdit[MAX_MSN_CONCUR_CHAT] = {NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL};  
//Lower RichEdit Window Handle 
HWND  g_hMSNSpookLowerRichEdit[MAX_MSN_CONCUR_CHAT] = {NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL};  
//Send Button
HWND  g_hMSNSpookSendButton[MAX_MSN_CONCUR_CHAT] = {NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL};  
//E-mail address EditBox
HWND  g_hMSNSpookAddressEdit[MAX_MSN_CONCUR_CHAT] = {NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL};  

//Temporarily Save Chatter's Name
TCHAR g_szChatterName[MAX_MSN_CONCUR_CHAT][MAX_CHATTER_ADDRESS] = {NULL, NULL, NULL, NULL, NULL, 
NULL, NULL, NULL, NULL, NULL}; 

TCHAR g_szMSNSpookSendText[4096] = {NULL};
//----------------------------------------------  
#pragma data_seg()

// Instruct the linker to make the Shared section
// readable, writable, and shared.
#pragma comment(linker, "/section:Shared,rws")

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
			g_hinstDll = (HINSTANCE)hModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}



//Note: it may be the MSN main window
BOOL WINAPI InitMSNSpook(HWND hChatHwnd)
{
	BOOL bMain = FALSE;
	//MSN main window may be created before OR after chat window!!!!!
    if(::IsMSNMain(hChatHwnd))
		bMain = TRUE;
	
	DWORD td = ::GetWindowThreadProcessId(hChatHwnd, NULL);

	if(bMain)
	{
		//PopMsg(_T("init InitMSNSpook main 1"));
		if(g_idMSNSpookMainThread == td) //re-enter
		{
            g_hMSNSpookMainWnd = hChatHwnd;
			return TRUE;
		}
		//PopMsg(_T("init InitMSNSpook main 2"));
		if(g_idMSNSpookMainThread == 0)
		{
			//check if chat is being hooked or not, if hooked, re-use chat hook
            for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
			{
				if(g_idMSNSpookChatThread[i] == td) //no need to hook
				{
					g_hMSNSpookMainWnd = hChatHwnd;
					g_hMSNSpookMainSendHook = g_hMSNSpookSendHook[i];
                    g_hMSNSpookMainPostHook = g_hMSNSpookPostHook[i];
				    g_idMSNSpookMainThread = g_idMSNSpookChatThread[i];
				    return TRUE;
				}		
			}

			//PopMsg(_T("init InitMSNSpook main 3"));
			g_idMSNSpookMainThread = td;
			g_hMSNSpookMainWnd = hChatHwnd;
			//hook the first hook and return
            g_hMSNSpookMainSendHook = SetWindowsHookEx(WH_CALLWNDPROC,
                       (HOOKPROC) CallWndProcHook,
					   g_hinstDll, 
		               g_idMSNSpookMainThread);
        	if(g_hMSNSpookMainSendHook == NULL) 
			{
				// Make sure that a hook has been installed.
                ::ReportErr(_T("in InitMSNSpook -- SetMainSendHook return NULL"));
		        BOOL b = UnhookWindowsHookEx(g_hMSNSpookMainSendHook);
		        if(!b)
				{
					::ReportErr(_T("in InitMSNSpook -- SetMainSendHook Fail->UnsetHook Fail"));
				}
		        ::g_hMSNSpookMainWnd = NULL;
                g_hMSNSpookMainSendHook = NULL;
		        g_idMSNSpookMainThread = 0;
        		return FALSE;
			}	
	
			g_hMSNSpookMainPostHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc,
					       g_hinstDll, 
		                   g_idMSNSpookMainThread);
	        if(g_hMSNSpookMainPostHook == NULL) 
			{
				// Make sure that a hook has been installed.
                ::ReportErr(_T("in InitMSNSpook -- SetMainPostHook return NULL"));

                g_hMSNSpookMainWnd = NULL;
                g_hMSNSpookMainSendHook = NULL;
		        g_hMSNSpookMainPostHook = NULL;
		        g_idMSNSpookMainThread  = 0;
		       return FALSE;
			}
			//PopMsg(_T("init InitMSNSpook main 4"));
			return TRUE; //
		}
		else //shared section data error
		{
			PopMsg(_T("init InitMSNSpook main err"));
			g_hMSNSpookMainWnd = NULL;
            g_hMSNSpookMainSendHook = NULL;
		    g_hMSNSpookMainPostHook = NULL;
		    g_idMSNSpookMainThread  = 0;
		    return FALSE;
		}
		return TRUE;
	}

	if(g_idMSNSpookMainThread == td) //no need to hook
	{
		//Set Hook To Previous Value
		DWORD dwIndex = MSNSpookInsertChatHwnd(hChatHwnd);
		if(dwIndex == (UINT)-1) return FALSE;
        g_hMSNSpookSendHook[dwIndex] = g_hMSNSpookMainSendHook;
        g_hMSNSpookPostHook[dwIndex] = g_hMSNSpookMainPostHook;
        g_idMSNSpookChatThread[dwIndex] = g_idMSNSpookMainThread;
			
		//Note, it may fail
		MSNEnumRichEdit(hChatHwnd, dwIndex);
		return TRUE;
	}		

	//Check if this thread has been hooked, 
	//it should be yes unless MS MSN team create chat window on multithread in MSN Message 17.0
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if(g_idMSNSpookChatThread[i] == td) //no need to hook
		{
			//Set Hook To Previous Value
			DWORD dwIndex = MSNSpookInsertChatHwnd(hChatHwnd);
			if(dwIndex == (UINT)-1) return FALSE;
            g_hMSNSpookSendHook[dwIndex] = g_hMSNSpookSendHook[i];
            g_hMSNSpookPostHook[dwIndex] = g_hMSNSpookPostHook[i];
            g_idMSNSpookChatThread[dwIndex] = g_idMSNSpookChatThread[i];
			
			//Note, it may fail
			MSNEnumRichEdit(hChatHwnd, dwIndex);
			return TRUE;
		}		
	}
	
	//First Window On This Thread, Hook It
    DWORD dwIndex = MSNSpookInsertChatHwnd(hChatHwnd);
    if(dwIndex == (UINT)-1) return FALSE;
    g_idMSNSpookChatThread[dwIndex] = ::GetWindowThreadProcessId(hChatHwnd, NULL);
    
	// Install the hook on the specified thread
    g_hMSNSpookSendHook[dwIndex] = SetWindowsHookEx(WH_CALLWNDPROC,
                       (HOOKPROC) CallWndProcHook,
					   g_hinstDll, 
		               g_idMSNSpookChatThread[dwIndex]);

	if(g_hMSNSpookSendHook[dwIndex] == NULL) 
	{
		// Make sure that a hook has been installed.
        ::ReportErr(_T("in InitMSNSpook -- SetSendHook return NULL"));
		BOOL b = UnhookWindowsHookEx(g_hMSNSpookSendHook[dwIndex]);
		if(!b)
		{
			::ReportErr(_T("in InitMSNSpook -- SetSendHook Fail->UnsetHook Fail"));
		}
		::g_hMSNSpookChatWnd[dwIndex] = NULL;
        g_hMSNSpookSendHook[dwIndex] = NULL;
		g_idMSNSpookChatThread[dwIndex] = 0;

		return FALSE;
	}	
	
	//Note, it may fail
	MSNEnumRichEdit(hChatHwnd, dwIndex);
	
	g_hMSNSpookPostHook[dwIndex] = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc,
					       g_hinstDll, 
		                   g_idMSNSpookChatThread[dwIndex]);
	if(g_hMSNSpookPostHook[dwIndex] == NULL) 
	{
		// Make sure that a hook has been installed.
        ::ReportErr(_T("in InitMSNSpook -- SetPostHook return NULL"));

        g_hMSNSpookChatWnd[dwIndex] = NULL;
        g_hMSNSpookSendHook[dwIndex] = NULL;
		g_hMSNSpookPostHook[dwIndex] = NULL;
		g_idMSNSpookChatThread[dwIndex] = 0;
		MSNEnumRichEdit(NULL, dwIndex);
		return FALSE;
	}
    return TRUE;
}

BOOL WINAPI ExitMSNSpook(HWND hChatHwnd)
{
	if (hChatHwnd == NULL) {
		return FALSE;
	}
	BOOL bMain = FALSE;
	//MSN main window may be created before OR after chat window!!!!!
    if(::IsMSNMain(hChatHwnd))
		bMain = TRUE;

	//PopMsg(_T("exit MSN"));
	if(bMain)
	{
		//PopMsg(_T("exit MSN 1"));
		DWORD tid = ::GetWindowThreadProcessId(hChatHwnd, NULL);
		if(g_idMSNSpookMainThread != tid) //error
		{
			PopMsg(_T("%d-%d, ExitMSNMain"), tid, g_idMSNSpookMainThread);
			g_hMSNSpookMainWnd = NULL;
            g_hMSNSpookMainSendHook = NULL;
		    g_hMSNSpookMainPostHook = NULL;
		    g_idMSNSpookMainThread  = 0;
		    return FALSE;
		}
		else
		{
			DWORD dwThreadNum = 0;
	        for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
			{
				if(g_idMSNSpookChatThread[i] == tid) dwThreadNum++;
			}
			if(g_idMSNSpookMainThread == tid) dwThreadNum++;

			if(dwThreadNum > 1)
			{
				g_hMSNSpookMainWnd = NULL;
                g_hMSNSpookMainSendHook = NULL;
		        g_hMSNSpookMainPostHook = NULL;
		        g_idMSNSpookMainThread  = 0;
		        return TRUE;
			}
			//PopMsg(_T("exit MSN 2 dwThreadNum %d"), dwThreadNum);
			//unhook completely
			BOOL b =  UnhookWindowsHookEx(g_hMSNSpookMainSendHook);
            if(!b) 
			{
				::ReportErr(_T("in ExitMSNSpook -- UnSetMainSendHook return NULL"));
		        if(g_hMSNSpookMainPostHook)
			        UnhookWindowsHookEx(g_hMSNSpookMainPostHook);

		        g_hMSNSpookMainWnd = NULL;
                g_hMSNSpookMainSendHook = NULL;
		        g_hMSNSpookMainPostHook = NULL;
		        g_idMSNSpookMainThread  = 0;
		        return TRUE;
			}

	        if(g_hMSNSpookMainPostHook)
		        UnhookWindowsHookEx(g_hMSNSpookMainPostHook);

            g_hMSNSpookMainWnd = NULL;
            g_hMSNSpookMainSendHook = NULL;
		    g_hMSNSpookMainPostHook = NULL;
		    g_idMSNSpookMainThread  = 0;
		    return TRUE;
		}
		//PopMsg(_T("exit MSN ???"));
		return TRUE;
	}

	//PopMsg(_T("exit MSN chat???"));
	DWORD dwIndex = MSNSpookQueryChatHwndIndex(hChatHwnd);
	if(dwIndex == (UINT)-1) return FALSE;

	DWORD tid = ::g_idMSNSpookChatThread[dwIndex];
	DWORD dwThreadNum = 0;
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if(g_idMSNSpookChatThread[i] == tid) dwThreadNum++;
	}
	if(g_idMSNSpookMainThread == tid) dwThreadNum++;

	//If the thread is used by other chat or main window, do not unhook it
	if(dwThreadNum > 1)
	{
        g_hMSNSpookChatWnd[dwIndex] = NULL;
        g_hMSNSpookSendHook[dwIndex] = NULL;
		g_hMSNSpookPostHook[dwIndex] = NULL;
		g_idMSNSpookChatThread[dwIndex] = 0;
		//erase richedit array
		MSNEnumRichEdit(NULL, dwIndex);
		return TRUE;
	}

	//the last chat running on the thread ready to quit
	BOOL b =  UnhookWindowsHookEx(g_hMSNSpookSendHook[dwIndex]);
    if(!b) 
	{
        ::ReportErr(_T("in ExitMSNSpook -- UnSetSendHook return NULL"));
		if(g_hMSNSpookPostHook[dwIndex])
			UnhookWindowsHookEx(g_hMSNSpookPostHook[dwIndex]);

		g_hMSNSpookChatWnd[dwIndex] = NULL;
        g_hMSNSpookSendHook[dwIndex] = NULL;
		g_hMSNSpookPostHook[dwIndex] = NULL;
		g_idMSNSpookChatThread[dwIndex] = 0;
		MSNEnumRichEdit(NULL, dwIndex);
		return FALSE;  
	}

	if(g_hMSNSpookPostHook[dwIndex])
		UnhookWindowsHookEx(g_hMSNSpookPostHook[dwIndex]);

	g_hMSNSpookChatWnd[dwIndex] = NULL;
    g_hMSNSpookSendHook[dwIndex] = NULL;
	g_hMSNSpookPostHook[dwIndex] = NULL;
	g_idMSNSpookChatThread[dwIndex] = 0;
	MSNEnumRichEdit(NULL, dwIndex);	
	return TRUE;
}

DWORD MSNSpookGetChatNumber()
{
	DWORD dwRet = 0;
	HWND hRichWnd = FindWindow(RICH_WND_CLASS, NULL);
	::PostMessage(hRichWnd, WM_MSN_GETCHATCNT, 0, 0);

	return dwRet;
}

DWORD MSNSpookInsertChatHwnd(HWND hChatWnd)
{
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if(g_hMSNSpookChatWnd[i] == NULL)
		{
            g_hMSNSpookChatWnd[i] = hChatWnd;
			return i;
		}
	}
	return (UINT)-1;
}

DWORD MSNSpookQueryChatHwndIndex(HWND hChatWnd)
{
	if(hChatWnd == NULL) return (UINT)-1;
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if(g_hMSNSpookChatWnd[i] == hChatWnd) return i;
	}
	return (UINT)-1;
}


BOOL CALLBACK EnumChildProc(
  HWND hWnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
	DWORD dwIndex = (DWORD)lParam;
	//I only consider RichEdit20W, not RichEdit20A since we are on Win2k+
	TCHAR szClassName[64];
	int nRet = GetClassName(hWnd, szClassName, 64);
    if(nRet == 0) return TRUE;
	szClassName[8] = 0;

	//Cut the length to szClassName[8] = 0;
	BOOL bRet;
	bRet;
	if(::lstrcmp(szClassName, _T("RichEdit")) == 0)
	{
		//Got It
		if(g_hMSNSpookUpperRichEdit[dwIndex] == NULL) //this should be enumed first
		{
			g_hMSNSpookUpperRichEdit[dwIndex] = hWnd;
			return TRUE;
		}
		if(g_hMSNSpookLowerRichEdit[dwIndex] == NULL) 
		{
			g_hMSNSpookLowerRichEdit[dwIndex] = hWnd;
			return TRUE;
		}
	}

	if(::lstrcmp(szClassName, _T("Edit")) == 0)
	{
		//In Messenger 4.6, 4.7 Chatter Name Edit is the First Edit Child of the Chat Window
		//In Messenger 5.0 Chatter Edit is the Second Edit Child of the chat Window
		//the first Edit is the GrandChild of the Chat Window
		if(::GetParent(::GetParent(hWnd)) != NULL) return TRUE;
		if(g_hMSNSpookAddressEdit[dwIndex] == NULL) 
		{
			g_hMSNSpookAddressEdit[dwIndex] = hWnd;
			return TRUE;
		}
	}

    if(::lstrcmp(szClassName, _T("Button")) == 0)
	{
		if(g_hMSNSpookSendButton[dwIndex] == NULL) 
		{
			g_hMSNSpookSendButton[dwIndex] = hWnd;
			return TRUE;
		}
	}

	return TRUE;
}

void MSNCheckRichEdit()
{
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if(g_hMSNSpookChatWnd[i] && g_hMSNSpookUpperRichEdit[i] == NULL)
		{
			MSNEnumRichEdit(g_hMSNSpookChatWnd[i], i);
		}
	}
}

BOOL MSNEnumRichEdit(HWND hChatHwnd, DWORD dwChatIndex)
{
	if(hChatHwnd == NULL)
	{
		g_hMSNSpookUpperRichEdit[dwChatIndex] = NULL;   //Upper RichEdit Window Handle
        g_hMSNSpookLowerRichEdit[dwChatIndex] = NULL;   //Lower RichEdit Window Handle
        g_hMSNSpookSendButton[dwChatIndex] = NULL;      //Send Button
        g_hMSNSpookChatWnd[dwChatIndex] = NULL;      //Parent Window of Upper Rich Edit 
        g_hMSNSpookAddressEdit[dwChatIndex] = NULL;     //E-mail address
		return TRUE;
	}

	g_hMSNSpookUpperRichEdit[dwChatIndex] = NULL;
	g_hMSNSpookLowerRichEdit[dwChatIndex] = NULL; 
	//Enum all the sibling
    BOOL bRet = EnumChildWindows(
        hChatHwnd,       // handle to parent window
        EnumChildProc,       // callback function
        (LPARAM)dwChatIndex  // application-defined value
    );
	//Check the Correctness of g_hMSNSpookUpperRichEdit & g_hMSNSpookUpperRichEdit
    if(g_hMSNSpookUpperRichEdit[dwChatIndex] && g_hMSNSpookLowerRichEdit[dwChatIndex])
	{
		RECT rectUp, rectLow;
		if( ::GetWindowRect(g_hMSNSpookUpperRichEdit[dwChatIndex], &rectUp) &&
			::GetWindowRect(g_hMSNSpookLowerRichEdit[dwChatIndex], &rectLow))
		{
			if(rectUp.bottom > rectLow.bottom)
			{
				HWND hWnd = g_hMSNSpookUpperRichEdit[dwChatIndex];
                g_hMSNSpookLowerRichEdit[dwChatIndex] = g_hMSNSpookUpperRichEdit[dwChatIndex];
                g_hMSNSpookUpperRichEdit[dwChatIndex] = hWnd;
			}
		}
	}
	//PopMsg(_T("MSNEnumRichEdit %X -- %X"), hRichEditHwnd, g_hMSNSpookUpperRichEdit);
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
	if (msg->message == WM_MSNSPOOK_DESTROYMAIN) {
		HWND hWnd = (HWND)msg->wParam;
		::PostQuitMessage(0);
    }
    else if(msg->message == WM_MSNSPOOK_QUERYTEXT) //wParam : index , lParam : chat Handle
	{
		for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
		{
			if(hWnd == g_hMSNSpookChatWnd[i] && g_hMSNSpookChatWnd[i] != NULL)
			{
				if(g_hMSNSpookUpperRichEdit[i] == NULL)
					::MSNEnumRichEdit(g_hMSNSpookChatWnd[i], i);
                //if you need RTF, do it here				
//#ifdef I_NEED_RTF
				//MSNInnerRichEditSaveRTF(g_hMSNSpookUpperRichEdit[i], i);
//#else
		        MSNInnerRichEditSaveText(g_hMSNSpookUpperRichEdit[i], i);
//#endif		        
			}
		}
	}

	if(msg->message == WM_MSNSPOOK_QUERYCONTACTLIST)
	{
		if(hWnd == ::g_hMSNSpookMainWnd) //it should be, but take care all time
		{
			::InnerMSNMainSaveContactList(g_hMSNSpookMainWnd);
		}
	}
	

	//WM_MSNSPOOK_SENDTEXT -- wParam : ClearPreviosText , lParam : Send At Once
	if(msg->message == WM_MSNSPOOK_SENDTEXT) //Send Text To Lower RichEdit
	{
		for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
		{
			if(hWnd == g_hMSNSpookChatWnd[i] && g_hMSNSpookChatWnd[i] != NULL)
			{
				if(g_hMSNSpookLowerRichEdit[i] == NULL)
					::MSNEnumRichEdit(g_hMSNSpookChatWnd[i], i);
		 
//		        if(g_hMSNSpookLowerRichEdit[i] == NULL || !::IsWindow(g_hMSNSpookLowerRichEdit[i]))
				{
					//still failed? well, forget it then
				}
//		        else
				{
					BOOL bClearPreviosText = msg->wParam == WPARAM_CLEAR_PREVIOUS_TEXT ? TRUE : FALSE;

			        if(bClearPreviosText)
					{
						::SendMessage(g_hMSNSpookLowerRichEdit[i], EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		                ::SendMessage(g_hMSNSpookLowerRichEdit[i], EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)NULL);
					}
        	        SETTEXTEX st;
	                st.flags = ST_DEFAULT; 
#ifndef _UNICODE
                    st.codepage = CP_ACP;
#else
	                st.codepage = 1200; //unicode
#endif
                    int len = ::lstrlen(g_szMSNSpookSendText);
	                TCHAR* szLocal = new TCHAR[len + 1];
	                ::lstrcpy(szLocal, g_szMSNSpookSendText);
	                //PopMsg(_T("Sending --< %s"), szLocal);
	                //DWORD dw = SendMessage( g_hMSNSpookUpperRichEdit, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)szLocal);
	                DWORD dw = ::SendMessage(g_hMSNSpookLowerRichEdit[i], EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)szLocal);
	                delete szLocal;
	                //PopMsg(_T(" EM_SETTEXTEX %d"), dw);
                    BOOL bSendTextImmediately = msg->lParam == LPARAM_SEND_TEXT_INSTANTLY ? TRUE : FALSE;
	                if(bSendTextImmediately)
					{
						DWORD wParam = MAKELONG(272, 0); // 0x00000110
						if (FALSE == ::PostMessage(::g_hMSNSpookChatWnd[0], WM_COMMAND,
							WPARAM(wParam), 
							0))
						{
							ReportErr(_T("Failed to send message to MSN chat window!"));
						}
					}
				}//if found upper richedit
			}//found the chat
		}//end of for
	}

	//find the hook index
	hWnd = msg->hwnd;
	DWORD dwIndex = -1;
	DWORD tid = GetWindowThreadProcessId(hWnd, NULL);
	//g_hMSNSpookChatWnd's parent is the Desktop Window
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if(tid == ::g_idMSNSpookChatThread[i]) 
		{
			dwIndex = i;	
			break;
		}
	}
	if(dwIndex != MAX_MSN_CONCUR_CHAT) //got
	{
		if (nCode < 0) 
		{
			// just pass it on 
	        return CallNextHookEx (g_hMSNSpookPostHook[dwIndex], nCode, wParam, lParam) ;
		}  
	    return CallNextHookEx (g_hMSNSpookPostHook[dwIndex], nCode, wParam, lParam) ;
	}
	else
	{
        if(tid == ::g_idMSNSpookMainThread) 
		{
			return CallNextHookEx (::g_hMSNSpookMainPostHook,
				nCode, wParam, lParam) ;
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
			
	//WM_CLOSE is sent first, then WM_DESTROY, WM_NCDESTROY, and return, at last CLOSE return
	//(pCwp->message == WM_DESTROY && g_hMSNSpookChatWnd == pCwp->hwnd) 
	if(pCwp->message == WM_CLOSE)
	{
		for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
		{
			if(g_hMSNSpookChatWnd[i] == pCwp->hwnd && g_hMSNSpookChatWnd[i] != NULL)
			{
				::MSNEnumRichEdit(g_hMSNSpookChatWnd[i], i);
				DWORD dwRet = ::SendMessage(g_hMSNSpookAddressEdit[i], WM_GETTEXT, 
			          MAX_CHATTER_ADDRESS, (LPARAM)(LPCTSTR)g_szChatterName[i]);
                g_szChatterName[i][dwRet] = TCHAR('\0');
//#ifdef I_NEED_RTF
//		        MSNInnerRichEditSaveRTF(g_hMSNSpookUpperRichEdit[i], 1);
//#else
				MSNInnerRichEditSaveText(g_hMSNSpookUpperRichEdit[i], i);
//#endif
			}
		}
		if(pCwp->hwnd == ::g_hMSNSpookMainWnd)
		{
			::InnerMSNMainSaveContactList(g_hMSNSpookMainWnd);
		}
	}

	//find the hook index
	hWnd = pCwp->hwnd;
	DWORD dwIndex = -1;
	DWORD tid = GetWindowThreadProcessId(hWnd, NULL);
	//g_hMSNSpookChatWnd's parent is the Desktop Window
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++)
	{
		if(tid == ::g_idMSNSpookChatThread[i]) 
		{
			dwIndex = i;	
			break;
		}
	}
	if(dwIndex != MAX_MSN_CONCUR_CHAT) //got
	{
		if (nCode < 0) 
		{
			// just pass it on 
	        return CallNextHookEx (g_hMSNSpookSendHook[dwIndex], nCode, wParam, lParam) ;
		}  
	    return CallNextHookEx (g_hMSNSpookSendHook[dwIndex], nCode, wParam, lParam) ;
	}
	else
	{
        if(tid == ::g_idMSNSpookMainThread) 
		{
			return CallNextHookEx (::g_hMSNSpookMainSendHook,
				nCode, wParam, lParam) ;
		}
		return 0;
	}
}

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
	//stream into the MMF
	LPBYTE lpMem = (LPBYTE)(dwCookie);
    LPBYTE lpByte = lpMem;

	DWORD dwSize, dwUsed;
	::CopyMemory(&dwSize, lpByte, sizeof(DWORD));
	lpByte += sizeof(DWORD);
	::CopyMemory(&dwUsed, lpByte, sizeof(DWORD));
    lpByte += sizeof(DWORD);

	lpByte += dwUsed;
	::CopyMemory(lpByte, pbBuff, cb);
	dwUsed += cb;

	lpByte = (LPBYTE)lpMem;
	lpByte += sizeof(DWORD);
	::CopyMemory(lpByte, &dwUsed, sizeof(DWORD));

	*pcb = cb;
	return 0;
}

BOOL WINAPI MSNSetRichEditReadOnly(BOOL bReadOnly, DWORD dwChatIndex)
{
	if(::g_hMSNSpookChatWnd[dwChatIndex] == NULL) return FALSE;
	::MSNEnumRichEdit(g_hMSNSpookChatWnd[dwChatIndex], dwChatIndex);
	if(g_hMSNSpookUpperRichEdit[dwChatIndex] == NULL) return FALSE;
	::SendMessage(g_hMSNSpookUpperRichEdit[dwChatIndex], EM_SETREADONLY,bReadOnly,0);
	//PostMessage(g_hMSNSpookUpperRichEdit, WM_APP, 0,0);
	return TRUE;
}

BOOL MSNInnerRichEditSaveText(HWND hRichEditWnd, DWORD dwChatIndex)
{
	HANDLE hMMF = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
            FALSE, g_MMF_NAME);
    if (hMMF == NULL) 
	{
		::ReportErr(_T("Open MMF Failed in AllocMMF InnerRichEditSaveText"));  return FALSE;
	}

	HANDLE hWriteEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_WRITE_EVENT_MMF_NAME);
	if(hWriteEvent == NULL)
	{
		::CloseHandle(hMMF);
		::ReportErr(_T("Open Write Event Failed in AllocMMF InnerRichEditSaveText"));  return FALSE;
	}

	HANDLE hReadEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_READ_EVENT_MMF_NAME);
	if(hReadEvent == NULL)
	{
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::ReportErr(_T("Open Read Event Failed in AllocMMF InnerRichEditSaveText")); return FALSE;
	}

	DWORD dwRet = ::WaitForSingleObject(hWriteEvent, MAX_WAIT);//INFINITE);
	if(dwRet == WAIT_ABANDONED)
	{
		::ReportErr(_T("InnerRichEditSaveText Write Wait Error")); 
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		return FALSE;
	}
	else if(dwRet == WAIT_TIMEOUT)
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
	if(pView == NULL)
	{
        ::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		::ReportErr(_T("InnerRichEditSaveText Write MapView Faield")); return FALSE;
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

    GETTEXTLENGTHEX gtlex;
	gtlex.flags = GTL_DEFAULT;
#ifdef _UNICODE
    gtlex.codepage = 1200;
#else 
	gtlex.codepage = CP_ACP;
#endif

	UINT chNum = SendMessage( 
        (HWND) hRichEditWnd,            // handle to destination window 
        EM_GETTEXTLENGTHEX,     // message to send
        (WPARAM)&gtlex,        // text length (GETTEXTLENGTHEX *)
        (LPARAM)0         // not used; must be zero
    );
	//PopMsg(_T("need %d"), chNum);
	LPTSTR sz = new TCHAR[chNum + 128]; //Note: You need more space

	GETTEXTEX gt;
	gt.cb = sizeof(TCHAR) * (chNum + 128);
	gt.flags = GT_USECRLF;
#ifdef _UNICODE
    gt.codepage = 1200;
#else 
	gt.codepage = CP_ACP;
#endif
	gt.lpDefaultChar = NULL;
	gt.lpUsedDefChar = NULL;

	DWORD dwGot = SendMessage( 
        (HWND) hRichEditWnd,     // handle to destination window 
        EM_GETTEXTEX,            // message to send
        (WPARAM)&gt,             // text information (GETTEXTEX *)
        (LPARAM)sz               // output buffer (LPCTSTR)
    );
	//PopMsg(_T("got %d"), dwGot);
	//PopMsg(_T("got %s"), sz);
	DWORD dwDisp = dwGot*sizeof(TCHAR);
	lpByte = (LPBYTE)lpMem;
	lpByte += dwUsed;
	::CopyMemory(lpByte, sz, dwDisp);
	dwUsed += dwDisp;
	delete sz;

	//new line
    lpByte = (LPBYTE)lpMem;
	lpByte += dwUsed;
	::CopyMemory(lpByte, _T("\r\n"), 2 * sizeof(TCHAR));
	dwUsed += 2 * sizeof(TCHAR);

	DWORD dwRetLen = ::SendMessage(g_hMSNSpookAddressEdit[dwChatIndex], WM_GETTEXT, 
	        MAX_CHATTER_ADDRESS, (LPARAM)(LPCTSTR)g_szChatterName[dwChatIndex]);
    g_szChatterName[dwChatIndex][dwRetLen] = TCHAR('\0');

	//append chatter name
	if(::lstrlen(g_szChatterName[dwChatIndex]) > 0)
	{			
		lpByte = (LPBYTE)lpMem;
	    lpByte += dwUsed;
		dwDisp = ::lstrlen(g_szChatterName[dwChatIndex])*sizeof(TCHAR);
		::CopyMemory(lpByte, g_szChatterName[dwChatIndex], dwDisp);
		dwUsed += dwDisp;
	}
		
	//append system time, local time is more meaningful, isn't it? man
	SYSTEMTIME tm;
	::GetLocalTime(&tm);
    
	TCHAR str[128];

    _stprintf(str, _T("\n%4d/%2d/%2d %2d:%2d:%2d\n"), 
		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

	lpByte = (LPBYTE)lpMem;
	lpByte += dwUsed;
    dwDisp = ::lstrlen(str)*sizeof(TCHAR);
    ::CopyMemory(lpByte, str, dwDisp);
	dwUsed += dwDisp;
    
    //Write dwUsed back
	lpByte = (LPBYTE)pView;
	lpByte += sizeof(DWORD);
	::CopyMemory(lpByte, &dwUsed, sizeof(DWORD));
    
	::UnmapViewOfFile(pView);
	::CloseHandle(hMMF);
	::SetEvent(hReadEvent);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI MSNSendChatText(LPCTSTR szText, 
	BOOL bClearPreviosText, BOOL bSendTextImmediately, DWORD dwChatIndex)
{
//	PopMsg(szText);

	MYREC MyRec;
	lstrcpy(MyRec.szText, szText);
	MyRec.dwSize = lstrlen(szText);
	MyRec.dwIndex = dwChatIndex;
	COPYDATASTRUCT cds;
	cds.cbData = sizeof( MyRec );	 // size of data
	cds.dwData = 0;
	cds.lpData = &MyRec;

	HWND hMainWnd = FindWindow(QOOM_MAINWND, NULL);
	HWND hRichWnd = FindWindow(RICH_WND_CLASS, NULL);

	::SendMessage(hRichWnd, WM_COPYDATA, WPARAM(hMainWnd), (LPARAM)&cds);

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
BOOL WINAPI MSNQueryChatterPersonName(LPCTSTR szChatterName, DWORD dwChatIndex)
{
	BOOL bSuccess = FALSE;

	if(::g_hMSNSpookChatWnd[dwChatIndex] == NULL) return FALSE;
	::MSNEnumRichEdit(g_hMSNSpookChatWnd[dwChatIndex], dwChatIndex);
	if(::g_hMSNSpookAddressEdit[dwChatIndex] == NULL) return FALSE;

	DWORD dwRet = ::SendMessage(g_hMSNSpookAddressEdit[dwChatIndex], WM_GETTEXT, 
			MAX_CHATTER_ADDRESS, (LPARAM)(LPCTSTR)g_szChatterName[dwChatIndex]);
    g_szChatterName[dwChatIndex][dwRet] = TCHAR('\0');
	__try
	{
		::lstrcpy((LPTSTR)szChatterName, g_szChatterName[dwChatIndex]);
		bSuccess = TRUE;
	}
	__finally
	{
		if(!bSuccess) return FALSE;
		else return TRUE;
	}
}

BOOL WINAPI SetChatterPersonName(LPCTSTR szChatterName, DWORD dwChatIndex)
{
	if(::g_hMSNSpookChatWnd[dwChatIndex] == NULL) return FALSE;
	::MSNEnumRichEdit(g_hMSNSpookChatWnd[dwChatIndex], dwChatIndex);
	if(::g_hMSNSpookAddressEdit[dwChatIndex] == NULL) return FALSE;

	int len = ::lstrlen(szChatterName);
    TCHAR* szLocal = new TCHAR[len + 1];
	::lstrcpy(szLocal, szChatterName);

	::SendMessage(g_hMSNSpookAddressEdit[dwChatIndex], WM_SETTEXT, 0, (LPARAM)szLocal);
	
	delete szLocal;
	return TRUE;
}

BOOL WINAPI MSNQueryChatContents(DWORD dwChatIndex)
{
	//PopMsg(_T("QueryChatContents1"));
//	if(::g_hMSNSpookChatWnd[dwChatIndex] == NULL) return FALSE;
//	::MSNEnumRichEdit(g_hMSNSpookChatWnd[dwChatIndex], dwChatIndex);
//	if(g_hMSNSpookUpperRichEdit[dwChatIndex] == NULL) return FALSE;
	
	//PopMsg(_T("QueryChatContents2"));
	HWND hRichWnd = FindWindow(RICH_WND_CLASS, NULL);
	::PostMessage(hRichWnd, WM_MSN_GETMESSAGE, WPARAM(dwChatIndex), 0);
				
	return TRUE;
}

BOOL WINAPI MSNQueryContactList()
{
	//PopMsg(_T("QueryContactList1"));
	if(g_hMSNSpookMainWnd == NULL || !::IsWindow(g_hMSNSpookMainWnd) || 
		!IsMSNMain(g_hMSNSpookMainWnd))
		return FALSE;
    //PopMsg(_T("QueryContactList2"));
    ::PostMessage(g_hMSNSpookMainWnd, WM_MSNSPOOK_QUERYCONTACTLIST, 0,0);
	return TRUE;
}

BOOL CALLBACK CatchContactListView(
  HWND hWnd,      // handle to child window
  LPARAM lParam   // application-defined value
)
{
	TCHAR szClassName[64];
	int nRet = GetClassName(hWnd, szClassName, 64);
    if(nRet == 0) return TRUE;
	
    BOOL bRet;
	bRet;
	if(::lstrcmp(szClassName, _T("SysListView32")) == 0)
	{
		::CopyMemory((LPVOID)lParam, &hWnd, sizeof(HWND));
		return FALSE;
	}
	return TRUE;
}

//get its child window of ListViewCtrl, crack it
BOOL InnerMSNMainSaveContactList(HWND hMSNMain)
{
	//it usually to be PluginHostClass\MSNMSBLGeneric\SysListView32
	//Since I have no knowledge (or no time to try all version of MSN)
	//I enum all children and get the SysListView32
 	HWND hListView = NULL;
    EnumChildWindows(hMSNMain, CatchContactListView, (LPARAM)&hListView);
    if(hListView == NULL) return FALSE;
    
	HANDLE hMMF = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
            FALSE, g_MMF_NAME);
    if (hMMF == NULL) 
	{
		::ReportErr(_T("Open MMF Failed in AllocMMF InnerMainSaveContactList"));  return FALSE;
	}

	HANDLE hWriteEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_WRITE_EVENT_MMF_NAME);
	if(hWriteEvent == NULL)
	{
		::CloseHandle(hMMF);
		::ReportErr(_T("Open Write Event Failed in AllocMMF InnerMainSaveContactList"));  return FALSE;
	}

	HANDLE hReadEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_READ_EVENT_MMF_NAME);
	if(hReadEvent == NULL)
	{
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::ReportErr(_T("Open Read Event Failed in AllocMMF InnerMainSaveContactList")); return FALSE;
	}

	DWORD dwRet = ::WaitForSingleObject(hWriteEvent, MAX_WAIT);//INFINITE);
	if(dwRet == WAIT_ABANDONED)
	{
		::ReportErr(_T("InnerRichEditSaveText Write Wait Error")); 
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		return FALSE;
	}
	else if(dwRet == WAIT_TIMEOUT)
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
	if(pView == NULL)
	{
        ::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		::ReportErr(_T("InnerRichEditSaveText Write MapView Faield")); return FALSE;
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
		if(!ListView_GetColumn(hListCtrl,columnCount,&lv)) break;
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
	    if(columnCount > 1)
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


HWND WINAPI MSNSpookGetChatWindowHandle(DWORD dwIndex)
{
	if(dwIndex >= 0 || dwIndex < MAX_MSN_CONCUR_CHAT)
		return ::g_hMSNSpookChatWnd[dwIndex];
	return NULL;
}

BOOL WINAPI MSNShowChatWindow(BOOL bShow)
{
	HWND hRichWnd = FindWindow(RICH_WND_CLASS, NULL);
	// As some chat window may be showed, but others maybe hide
	// So hide->show or show->hide.
	DWORD dwHide = (bShow == TRUE) ? 0 : 1;
	::PostMessage(hRichWnd, WM_MSN_SHOWCHATWINDOW, WPARAM(dwHide), 0);
	DWORD dwShow = (bShow == TRUE) ? 1 : 0;
	::PostMessage(hRichWnd, WM_MSN_SHOWCHATWINDOW, WPARAM(dwShow), 0);
	return TRUE;
}

BOOL WINAPI MSNDestroyMainWindow(HWND hWnd)
{
	// Post WM_DESTROY message
	::PostMessage(hWnd, WM_MSNSPOOK_DESTROYMAIN, WPARAM(hWnd), NULL);
	return TRUE;
}
