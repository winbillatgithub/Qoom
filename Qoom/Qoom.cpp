// Qoom.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Qoom.h"
#include "QoomDlg.h"
#include "../common/globaldef.h"
#include "psapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define EVENT_SINGLE_INSTANCE _T("EVENT_SINGLE_INSTANCE_YCQ_SWB")
/////////////////////////////////////////////////////////////////////////////
// CQoomApp

BEGIN_MESSAGE_MAP(CQoomApp, CWinApp)
	//{{AFX_MSG_MAP(CQoomApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQoomApp construction

CQoomApp::CQoomApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	g_hChatSendEdit = NULL;
	// Currently we assume that all the chat window should be showed at beginning.
	g_bQQShowed = TRUE;
	g_bMSNShowed = TRUE;

	for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
		g_hChatHisEditQQ[i] = NULL;
		g_hTabIndexQQ[i] = DWORD(-1);
		g_bHisChangedQQ[i] = 0;
	}
	for(i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
		g_hChatHisEditMSN[i] = NULL;
		g_hTabIndexMSN[i] = DWORD(-1);
		g_bHisChangedMSN[i] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CQoomApp object

CQoomApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CQoomApp initialization

BOOL CQoomApp::InitInstance()
{
	// Register my own class name
	RegisterOwnClass();

	m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, EVENT_SINGLE_INSTANCE);
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		AfxMessageBox(IDS_ARREADY_RUNNING, MB_OK);
		return FALSE;
	}
retest:
	int nRet = 0;
	IsQQMSNRunning();
	if (m_bFoundQQ == TRUE) {
		nRet = AfxMessageBox(IDS_QQ_RUNNING, MB_ABORTRETRYIGNORE|MB_ICONQUESTION);
		if (nRet == IDABORT) {
			return FALSE;
		} else if (nRet == IDRETRY) {
			Sleep(10);
			goto retest;
		}
	}
retest2:
	if (m_bFoundMSN == TRUE) {
		IsQQMSNRunning();
	}
	if (m_bFoundMSN == TRUE) {
		nRet = AfxMessageBox(IDS_MSN_RUNNING, MB_ABORTRETRYIGNORE|MB_ICONQUESTION);
		if (nRet == IDABORT) {
			return FALSE;
		} else if (nRet == IDRETRY) {
			Sleep(10);
			goto retest2;
		}
	}
	// This function is not required for VC,
	// VB should use this function to use rich edit controls.
	//InitCommonControls(); //To use RichEdit20W

	// You MUST call the following functions in MFC to use rich edit.
	// else you will stop with "cannot find window class".
	AfxInitRichEdit(); 
	
	// For hide task bar.
	CoInitialize(NULL);

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CQoomDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CQoomApp::ExitInstance() 
{
	::CoUninitialize();

	return CWinApp::ExitInstance();
}

void CQoomApp::RegisterOwnClass()
{
	WNDCLASS wc;
	// Get the info for this class.
	// #32770 is the default class name for dialogs boxes.
	::GetClassInfo(AfxGetInstanceHandle(), _T("#32770"), &wc);
	
	// Change the name of the class.
	wc.lpszClassName = QOOM_MAINWND;
	
	// Register this class so that MFC can use it.
	AfxRegisterClass(&wc);  
}

// Whether the qq or msn is running now.
BOOL CQoomApp::IsQQMSNRunning()
{
	m_bFoundQQ = FALSE;
	m_bFoundMSN = FALSE;
	// Get the list of process identifiers.	
	DWORD aProcesses[1024*4], cbNeeded, cProcesses;
	unsigned int i;
	
	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		return FALSE;
	
	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);
	
	// Print the name and process identifier for each process.	
	for ( i = 0; i < cProcesses; i++ ) {
		if (m_bFoundQQ && m_bFoundMSN) {
			break;
		}
		DWORD processID = aProcesses[i];
		TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

		// Get a handle to the process.
		HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,
			FALSE, processID );
		
		// Get the process name.
		if (NULL != hProcess)
		{
			HMODULE hMod;
			DWORD cbNeeded;
			
			if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
				&cbNeeded) )
			{
				GetModuleBaseName( hProcess, hMod, szProcessName, 
					sizeof(szProcessName)/sizeof(TCHAR) );
				if (lstrcmp(szProcessName, g_QQExeName) == 0) {
					m_bFoundQQ = TRUE;
				} else if (lstrcmp(szProcessName, g_MSNExeName) == 0) {
					m_bFoundMSN = TRUE;
				}
			}
		}
		// Close the process handle.		
		CloseHandle( hProcess );
	}

	return TRUE;
}

