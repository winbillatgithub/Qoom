
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the QQSPOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// QQSPOOK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef QQSPOOK_EXPORTS
#define QQSPOOK_API __declspec(dllexport)
#else
#define QQSPOOK_API __declspec(dllimport)
#endif

#include "../../Common/GlobalDef.h"

// This class is exported from the QQSpook.dll
// External function prototypes

// Hook Part
// The Second Chat Window Will Not Set A New Hook!!!!
QQSPOOK_API BOOL WINAPI InitQQSpook(HWND hChatHwnd, BOOL bMain);

// Release the hook handle.
QQSPOOK_API BOOL WINAPI ExitQQSpook(HWND hChatHwnd, BOOL bMain);

// Get the chat window handle by index.
QQSPOOK_API HWND WINAPI QQpookGetChatWindowHandle(DWORD dwIndex);

//Note: szChatterName must be long enough to hold the chatter name
//it is the name of who are chat with you
QQSPOOK_API BOOL WINAPI QQQueryChatterPersonName(LPCTSTR szChatterName, DWORD dwRow, DWORD dwCol);

// dwChatIndex is the Active Chat Number
// Get the chatting messages from the upper editbox.
QQSPOOK_API BOOL WINAPI QQQueryChatContents(DWORD dwRow, DWORD dwCol);

// Get all the frients' contact information.
QQSPOOK_API BOOL WINAPI QQQueryContactList();

// Send the messages to QQ chatting window.
QQSPOOK_API BOOL WINAPI QQSendChatText(LPCTSTR szText, 
    BOOL bClearPreviosText, BOOL bSendTextImmediately, DWORD dwRow, DWORD dwCol);

// Set the chatter's name
QQSPOOK_API BOOL WINAPI QQSetChatterPersonName(LPCTSTR szChatterName, DWORD dwRow, DWORD dwCol);

// Hide the QQ chat window when needed.
QQSPOOK_API BOOL WINAPI QQShowChatWindow(BOOL bShow);

// Destroy the main QQ window.
QQSPOOK_API BOOL WINAPI QQDestroyMainWindow(HWND hWnd);

// Enum every chat window's richedit.
void QQCheckRichEdit();

// Enum the chat window's richedit.
BOOL QQEnumRichEdit(HWND hChatHwnd, DWORD dwRow, DWORD dwCol);

// Read Contents of RichEdit, and Write To MMF
BOOL QQInnerRichEditSaveText(HWND hRichEditWnd, DWORD dwRow, DWORD dwCol);

// Get Hooked Chat Number
DWORD QQSpookGetChatNumber();

// Insert hChatWnd into g_hChatWnd and return the Array Index
DWORD QQSpookInsertChatHwnd(HWND hChatWnd, DWORD dwRow);

// Query hChatWnd's index in g_hChatWnd
DWORD QQSpookQueryChatHwndIndex(HWND hChatWnd);

// Save the contact list.
BOOL InnerQQMainSaveContactList(HWND hMSNMain);

BOOL ShowQQChatWindow(HWND hChat, BOOL bShowed);