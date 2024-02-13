#include "TxtHandler.h"
#include "StdAfx.h"

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

//just write to a disk file for now, MMF is used for later case
// Also we should write the accept richedit index to shared memory
BOOL GeneralTxtHandler(ITextServices* lpTs, LPVOID pView, DWORD dwIndex)
{

	LPBYTE lpByte = (LPBYTE)pView;
	DWORD dwSize, dwUsed;
	DWORD dwFlag = 1;
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
	lpTs->TxSendMessage( 
            EM_STREAMOUT,						// message to send
            (WPARAM) (SF_RTF),					// format options 
            (LPARAM)(EDITSTREAM*)&myStream,     // data (EDITSTREAM *)
			&lr);

    //Write dwUsed back
	// NOT write the dwUsed again, because the EM_STREAMOUT has writed ok.
//	lpByte = (LPBYTE)pView;
//	lpByte += sizeof(DWORD);
//	::CopyMemory(lpByte, &dwUsed, sizeof(DWORD));

	return TRUE;
}

//just write to a disk file for now, MMF is used for later case
BOOL RecordAllMessages(int iIndex, ITextServices* lpTs)
{
	TCHAR sz[1024] = {'\0'};
	BSTR bstr;
	HRESULT hr = lpTs->TxGetText(&bstr);
	if(FAILED(hr)) {
		wsprintf(sz, _T("%d : %s"), iIndex, _T("NULL ITextServices!"));
	} else {
		wsprintf(sz, _T("%d : %s"), iIndex, bstr);
	}
	int iLength = sizeof(sz);
	//Process the text you got
	::SysFreeString(bstr);
	
	
	TCHAR szFilename[MAX_PATH];
	CreateFileName(szFilename, _T("C:\\log\\"), _T(".txt"));
	
	HANDLE hFile = ::CreateFile(szFilename, GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	DWORD dwWritten;
	::WriteFile(hFile, sz, iLength, &dwWritten, NULL);
	::CloseHandle(hFile);
    
	return TRUE;
}

