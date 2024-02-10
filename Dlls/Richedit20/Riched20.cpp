// Riched20.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Riched20.h"
#include "wchar.h"
#include "RecvWin.h"
#include "myRichEditOle.h"
#include "ImgHandler.h"
#include "TxtHandler.h"
#include "comutil.h"

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

#define MSN6_CONN 10 //maximum 10 concurrent chat
#define MSN6_INTERFACE 7 //every chat have 7 windowless richedit
#define MSN_CHATHIS_INDEX  2 // FROM 0
#define MSN_CHATSEND_INDEX 3 // FROM 0

#define MAX_HIS_CHAT 64*1024 // The max chat length 64k

#define g_MMF_NAME  _T("{AF60EB27-C70A-4f36-8D68-6FEA9998C884}") //place holder
#define g_READ_EVENT_MMF_NAME  _T("{85B7FBAC-D46B-40ae-A604-FD28168D3A8B}") //if set client go
#define g_WRITE_EVENT_MMF_NAME  _T("{48208702-377D-4c54-A13F-7D62A16E4714}") //if set Dll go
#define g_RICHEDIT_EVENT_NAME  _T("{48208704-377v-4c54-A13F-7D62516E4714}") //if set Dll go
#define MAX_WAIT 500 

//Every Chat Window Has Up to 7 IWindowlessRichEdit
#pragma data_seg("Shared")

//You are using a IA-32 machine, so DWORD = LPVOID
DWORD g_lpIText[MSN6_CONN * MSN6_INTERFACE] = {NULL};  
// for test
DWORD g_lpITextInterface[100] = {NULL};
int   g_iIndex = 0;
//Chat Window Handle
HWND  g_hMSNChatWnd[MSN6_CONN] = {NULL};
DWORD g_dwActiveIndex = (DWORD)-1;
BOOL  g_bInitialized = FALSE;
HWND  g_hRecvWnd = NULL;		// Server window handle.
DWORD g_dwHisSize[MSN6_CONN] = {(DWORD)-1};	// For refresh improve.
#pragma data_seg()

// Instruct the linker to make the Shared section
// readable, writable, and shared.
#pragma comment(linker, "/section:Shared,rws")


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
//			PopMsg(_T("DLL_PROCESS_ATTACH"));
			if(!g_bInitialized)
			{
				InitializeRecv(TRUE);
			}
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
//			PopMsg(_T("DLL_PROCESS_DETACH"));
			if(g_bInitialized)
			{
				InitializeRecv(FALSE);				
			}
			break;
    }
    return TRUE;
}

DWORD GetChatNumber()
{
	int count = 0;
	for(int kk = 0; kk < MSN6_CONN; kk++)
	{
		if(g_hMSNChatWnd[kk] != NULL && g_hMSNChatWnd[kk] != (HWND)(DWORD)-1)
			count++;
	}
	return count;
}

BOOL  CloseChatHandle(DWORD dwIndex)
{
	g_hMSNChatWnd[dwIndex] = NULL;
    for(int i = 0; i < MSN6_INTERFACE; i++)
	{
		g_lpIText[dwIndex * MSN6_INTERFACE + i] = NULL;
	}
	g_dwHisSize[dwIndex] = (DWORD)-1;

	return TRUE;
}

// for test
BOOL QueryAllRichEdit() {
//	PopMsg(_T("QueryAllRichEdit begin"));

	LRESULT hr;
	TCHAR szFilename[MAX_PATH];
	CreateFileName(szFilename, _T("C:\\log\\"), _T(".txt"));
	
	HANDLE hFile = ::CreateFile(szFilename, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	for(int i = 0; i < g_iIndex; i++) {
		if (::g_lpITextInterface[i] != 0) {
			BSTR bstr;
			TCHAR sz[128] = {'\0'};
			int iLength = 0;
			hr = ((ITextServices*)::g_lpITextInterface[i])->TxGetText(&bstr);
			if (SUCCEEDED(hr)) {
				if (bstr != NULL) { 
					iLength = wsprintf(sz, _T("%d : %s\n"), i, bstr);
					//Process the text you got
					::SysFreeString(bstr);
				} else {
					iLength = wsprintf(sz, _T("%d : %s\n"), i, _T("NULL String!"));
				}
			} else {
				iLength = wsprintf(sz, _T("%d : %s\n"), i, _T("NULL ITextServices!"));				
			}
			iLength = sizeof(sz);
			DWORD dwWritten;
			::WriteFile(hFile, sz, iLength, &dwWritten, NULL);
			if (i == 3)
			{
				BSTR bstrTemp = SysAllocString(_T("message to send."));
				((ITextServices*)::g_lpITextInterface[i])->TxSetText(bstrTemp);
				SysFreeString(bstrTemp);
			}
		}
	}
	::CloseHandle(hFile);
	return TRUE;
}

BOOL InitializeRecv(BOOL bInitialize)
{
	if(bInitialize == ::g_bInitialized) return FALSE;
    if(bInitialize)
	{
		//Create Window
		MyRegisterClass();
		g_hRecvWnd = InitInstance();
        if(!g_hRecvWnd)
		{
			PopMsg(_T("g_hRecvWnd == NULL"));
			return FALSE;
		}
//		PopMsg(_T("InitializeRecv New OK"));
//		::SetTimer(g_hRecvWnd, 101, 30000, NULL); 
	}
	else
	{
//		PopMsg(_T("InitializeRecv Kill"));
		//Destroy Window
		if(!::IsWindow(g_hRecvWnd)) {
			PopMsg(_T("!::IsWindow(g_hRecvWnd)"));
			return FALSE;
		}
		::PostMessage(::g_hRecvWnd, WM_CLOSE, 0, 0);
//		PopMsg(_T("InitializeRecv Kill OK"));
		::g_hRecvWnd = NULL;
	}	
	::g_bInitialized = bInitialize;
	return TRUE;
}

BOOL InsertHandle(HWND hChat)
{
//	PopMsg(_T("InsertHandle()"));
	DWORD dwIndex = -1;
	for(int i = 0; i < MSN6_CONN; i++)
	{
		if (g_hMSNChatWnd[i] == NULL || g_hMSNChatWnd[i] == (HWND)(DWORD)-1)
		{
			g_hMSNChatWnd[i] = hChat;
			dwIndex = i;
			break;
		}
	}
	
	if (dwIndex == -1)
	{
		return FALSE;
	}
	//set interface
	for(i = 0; i < MSN6_INTERFACE; i++)
	{
		if(g_lpIText[dwIndex * MSN6_INTERFACE + i] == NULL)
			g_lpIText[dwIndex * MSN6_INTERFACE + i] = (DWORD)-1;
	}

	g_dwActiveIndex = dwIndex;
	return TRUE;
}

BOOL InsertInterface(DWORD lpInterface)
{
	DWORD dwIndex = ::g_dwActiveIndex;
	if(dwIndex == (DWORD)-1)
		return FALSE;
// for test
//	g_lpITextInterface[g_iIndex] = lpInterface;
//	g_iIndex ++;

	//set interface
	for(int i = 0; i < MSN6_INTERFACE; i++)
	{
		if(g_lpIText[dwIndex * MSN6_INTERFACE + i] == (DWORD)-1 || g_lpIText[dwIndex * MSN6_INTERFACE + i] == (DWORD)NULL)
		{
			g_lpIText[dwIndex * MSN6_INTERFACE + i] = lpInterface;
			return TRUE;
		}
	}
	return FALSE;
}

// Set the chat contents to be sent. according to MSN_CHATSEND_INDEX;
BOOL  SetChatContent(LPCTSTR pStr, DWORD nIndex)
{
//	PopMsg(_T("Chat index=%d, message=%s"), nIndex, pStr);

	if (g_lpIText[nIndex*MSN6_INTERFACE+MSN_CHATSEND_INDEX] == NULL ||
		g_lpIText[nIndex*MSN6_INTERFACE+MSN_CHATSEND_INDEX] == -1)
	{
		PopMsg(_T("It seemed not a valid chat, index = %d"), nIndex);
		return FALSE;
	}

//	PopMsg(_T("SetChatContent step2"));

	((ITextServices*)::g_lpIText[nIndex*MSN6_INTERFACE+MSN_CHATSEND_INDEX])->TxSetText((LPCWSTR)pStr);

	// Click the send button.
//	DWORD dwdata = DWORD(0x00000110);
	DWORD wParam = MAKELONG(272, 0);
	::PostMessage(g_hMSNChatWnd[nIndex], WM_COMMAND, WPARAM(wParam), 0);

	return TRUE;
}

//addr edit, chat edit, send edit
BOOL  QueryChatContent(DWORD dwIndex)
{
	// for test
	//	QueryAllRichEdit();
	//	return TRUE;
	if (dwIndex < 0 || dwIndex > MSN6_CONN) {
		return FALSE;
	}
//	PopMsg(_T("QueryChatContent() begin"));
	if (g_lpIText[dwIndex*MSN6_INTERFACE+MSN_CHATHIS_INDEX] == NULL ||
		g_lpIText[dwIndex*MSN6_INTERFACE+MSN_CHATHIS_INDEX] == -1 ||
		g_hMSNChatWnd[dwIndex] == NULL)
	{
		//PopMsg(_T("It seemed not a valid chat, index = %d"), dwIndex);
		return FALSE;
	}
	
	BSTR bstr;
	TCHAR sz[MAX_HIS_CHAT] = {'\0'}; 
	HRESULT hr = ((ITextServices*)::g_lpIText[dwIndex*MSN6_INTERFACE+MSN_CHATHIS_INDEX])->TxGetText(&bstr);
	if (SUCCEEDED(hr)) {
		if (bstr != NULL) { 
			// Note here wsprintf only support max 1024 bytes.
			// _snwprintf can support more than 1024 bytes.
			_snwprintf(sz, sizeof(sz)/sizeof(TCHAR), _T("%s"), bstr);
			//Process the text you got
			::SysFreeString(bstr);
		}
	} else {
		PopMsg(_T("It is not a valid conntect, index = %d"), dwIndex);
	}
	// Put the chat content to shared memory.
	DWORD dwSize = ::lstrlen(sz)*sizeof(TCHAR);
	// The content changed.
	if (dwSize != g_dwHisSize[dwIndex]) {
		MSNInnerRichEditSaveText((::g_lpIText[dwIndex*MSN6_INTERFACE+MSN_CHATHIS_INDEX]), dwIndex, dwSize);
	}
//	PopMsg(_T("chat content = %s, length = %d"), sz, dwSize);

	return TRUE;
}

BOOL MSNInnerRichEditSaveText(DWORD dwInterface, DWORD dwIndex, DWORD dwSize)
{
	HANDLE hMMF = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
            FALSE, g_MMF_NAME);
    if (hMMF == NULL) 
	{
		::ReportErr(_T("Open MMF Failed in AllocMMF MSNInnerRichEditSaveText"));  return FALSE;
	}

	HANDLE hWriteEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_WRITE_EVENT_MMF_NAME);
	if (hWriteEvent == NULL)
	{
		::CloseHandle(hMMF);
		::ReportErr(_T("Open Write Event Failed in AllocMMF MSNInnerRichEditSaveText"));  return FALSE;
	}

	HANDLE hReadEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_READ_EVENT_MMF_NAME);
	if (hReadEvent == NULL)
	{
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::ReportErr(_T("Open Read Event Failed in AllocMMF MSNInnerRichEditSaveText")); return FALSE;
	}

	DWORD dwRet = ::WaitForSingleObject(hWriteEvent, MAX_WAIT);//INFINITE);
	if (dwRet == WAIT_ABANDONED)
	{
		::ReportErr(_T("Write Wait Error")); 
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		return FALSE;
	}
	else if (dwRet == WAIT_TIMEOUT)
	{
		::ReportErr(_T("Write Wait Time Out")); 
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		return FALSE;
	}
	
	::ResetEvent(hWriteEvent);

	// For the syncronize problem.
	g_dwHisSize[dwIndex] = dwSize;
	
	//head 4 byte to record size
	LPVOID pView = MapViewOfFile(hMMF, 
		FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if (pView == NULL)
	{
        ::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);
		::ReportErr(_T("MSNInnerRichEditSaveText Write MapView Faield")); return FALSE;
	}

	GeneralTxtHandler((ITextServices*)dwInterface, pView, dwIndex);
    
	::UnmapViewOfFile(pView);
	::CloseHandle(hMMF);
	::SetEvent(hReadEvent);
	return TRUE;
}

// Show or hide the chat window
BOOL  MSNShowChatWindow(BOOL bShow)
{
	for(int kk = 0; kk < MSN6_CONN; kk++)
	{
		if(g_hMSNChatWnd[kk] != NULL && g_hMSNChatWnd[kk] != (HWND)(DWORD)-1) {
			if (bShow == TRUE) {
				::ShowWindow(g_hMSNChatWnd[kk], SW_SHOW);
			} else {
				::ShowWindow(g_hMSNChatWnd[kk], SW_HIDE);
			}
		}
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//Fake COM Interface
RICHED20_API GUID IID_IRichEditOle  
 = { 0x00020D00, 0x0, 0x0, { 0xC0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46 } };

RICHED20_API GUID IID_IRichEditOleCallback
 = { 0x00020D03, 0x0, 0x0, { 0xC0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46 } };

RICHED20_API GUID IID_ITextServices
 = { 0x8d33f740, 0xcf58, 0x11ce, {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}};

RICHED20_API GUID IID_ITextHost
 = { 0xc5bdd8d0, 0xd26e, 0x11ce, {0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}};

RICHED20_API GUID IID_ITextHost2
 = { 0xc5bdd8d0, 0xd26e, 0x11ce, {0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}};

typedef HRESULT (__stdcall *lpCreateTextServices)(IUnknown *punkOuter, ITextHost *pITextHost, IUnknown **ppUnk);
typedef LRESULT (__stdcall *lpREExtendedRegisterClass)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef LRESULT (__stdcall *lpRichEdit10ANSIWndProc)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef LRESULT (__stdcall *lpRichEditANSIWndProc)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

#define NEW_DLL_NAME  _T("\\RichEd20.Dll")
//You MUST dynanically load dll
HRESULT WINAPI CreateTextServices(IUnknown *punkOuter, ITextHost *pITextHost, IUnknown **ppUnk)
{
	static int iRichInterface = 0;

	TCHAR szLib[MAX_PATH]; //255 is enough
	DWORD dw = GetSystemDirectory(szLib, MAX_PATH);
	if(dw == 0) return 0;
//	PopMsg(_T("CreateTextServices()       1"));
//	PopMsg(szLib);
	szLib[dw] = TCHAR('\0');
	::lstrcat(szLib, NEW_DLL_NAME);
	HMODULE hLib = LoadLibrary(szLib);
    if(!hLib) return 0;
//	PopMsg(_T("CreateTextServices()       2"));
	
    lpCreateTextServices _CreateTextServices = (HRESULT (__stdcall *) 
		(IUnknown*, ITextHost*, IUnknown**))
		::GetProcAddress(hLib, "CreateTextServices");
	if(!_CreateTextServices) return 0;
//	PopMsg(_T("CreateTextServices()       3"));
	
	HRESULT hr = (_CreateTextServices)(punkOuter, pITextHost, ppUnk);

	HANDLE hEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, g_RICHEDIT_EVENT_NAME);
	if (hEvent == NULL)
	{
		//::ReportErr(_T("OpenEvent g_RICHEDIT_EVENT_NAME = NULL!"));
	}
	else 
	{
		// Signal
		if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
		{
			ITextServices* lpTx;
			((IUnknown*)(*ppUnk))->QueryInterface(IID_ITextServices, (void**)(&lpTx));
			InsertInterface((DWORD)lpTx);
			iRichInterface += 1;
			// Found all the richedit interface.
			if (iRichInterface == MSN6_INTERFACE)
			{
				// Wait for another new chat window.
				iRichInterface = 0;
				// Nonsignal.
				ResetEvent(hEvent);
			}
		}
	}
	
    //::FreeLibrary(hLib);
//	PopMsg(_T("CreateTextServices() End"));
	return hr;
}

//Note:
//Protected Storage System Service Protects "RICHEDIT20.DLL" under %SystemRoot%System32
LRESULT WINAPI REExtendedRegisterClass(HINSTANCE hInstance)
{
	return 0;
}

LRESULT WINAPI RichEdit10ANSIWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

LRESULT WINAPI RichEditANSIWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

