// QoomDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Qoom.h"
#include "QoomDlg.h"
#include "../Common/GlobalDef.h"
#include "QoomThread.h"
#include "../dlls/MonitorService/MonitorService.h"
#include "../dlls/QQSpook/QQSpook.h"
#include "../dlls/MSNSpook/MSNSpook.h"
#include "HyperLink.h"
#include "QQParent.h"
#include "shobjidl.h"		// For task bar
#include "ExitOptDlg.h"		// For exit options
#include "psapi.h"			// For enumprocess

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HANDLE hQueryEvent;
CRITICAL_SECTION CriticalSection; 

#define  TIMER_GETMSG_MSN  1
#define  TIMER_GETMSG_QQ   2

namespace
{
	LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
	{
		HKEY hkey;
		LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);
		
		if (retval == ERROR_SUCCESS) 
		{
			long datasize = MAX_PATH;
			TCHAR data[MAX_PATH];
			RegQueryValue(hkey, NULL, data, &datasize);
			lstrcpy(retdata,data);
			RegCloseKey(hkey);
		}
		
		return retval;
	}
}


BOOL ShowInTaskbar(HWND hChat, BOOL bShowed)
{
	if (hChat == NULL) {
		return FALSE;
	}
	//在app的InitInstance中加入::CoInitialize(NULL);
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
	
	//在app的ExitInstance中加入::CoUninitialize();
	if (hResult != NOERROR) {
		PopMsg(_T("Failed to remove task button"));
		return FALSE;
	}
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CHyperLink	m_linkUrl;
	CHyperLink	m_linkMail;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_STATIC_URL, m_linkUrl);
	DDX_Control(pDX, IDC_STATIC_MAIL, m_linkMail);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_linkUrl.SetURL(_T("http://www.antiboss.com/Qoom.html"));
	m_linkUrl.ModifyLinkStyle(0, CHyperLink::StyleUseHover);
	m_linkUrl.SetWindowText(_T("http://www.antiboss.com/Qoom.html"));

	m_linkMail.SetURL(_T("michael_shur@hotmail.com"));
	m_linkMail.ModifyLinkStyle(0, CHyperLink::StyleDownClick);
	m_linkMail.SetWindowText(_T("michael_shur@hotmail.com"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CQoomDlg dialog

CQoomDlg::CQoomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQoomDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQoomDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	// tray icon message
	WM_TASKBARCREATED = 0;
}

void CQoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQoomDlg)
	DDX_Control(pDX, IDC_STATIC_CUR_QQ, m_btnQQ);
	DDX_Control(pDX, IDC_STATIC_CUR_MSN, m_btnMsn);
	DDX_Control(pDX, IDC_TAB_QQ, m_tabQQ);
	DDX_Control(pDX, IDC_TAB_MSN, m_tabMSN);
	DDX_Control(pDX, IDC_STATIC_EDIT_SEND, m_staticEditSend);
	DDX_Control(pDX, IDC_STATIC_EDIT, m_staticEdit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CQoomDlg, CDialog)
	//{{AFX_MSG_MAP(CQoomDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_REFRESH, OnBtnRefresh)
	ON_MESSAGE(WM_MSN_SETCHATCNT, OnGetChatCount) 	
	ON_MESSAGE(WM_MSN_CREATECHAT, OnCreateMsnChatWindow) 	
	ON_MESSAGE(WM_QQ_CREATECHAT, OnCreateQQChatWindow) 	
	ON_MESSAGE(WM_MSN_DESTROYCHAT, OnDestroyMsnChatWindow) 	
	ON_MESSAGE(WM_QQ_DESTROYCHAT, OnDestroyQQChatWindow) 	
	ON_MESSAGE(WM_QQ_TEXT_CHANGE, OnQQTextChanged) 	
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MSN, OnSelchangeTabMsn)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_QQ, OnSelchangeTabQq)
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_BTN_HIDEMSN, OnBtnHideMsn)
	ON_BN_CLICKED(ID_BTN_HIDEQQ, OnBtnHideQQ)
	ON_NOTIFY(NM_CLICK, IDC_TAB_MSN, OnClickTabMsn)
	ON_NOTIFY(NM_CLICK, IDC_TAB_QQ, OnClickTabQq)
	ON_NOTIFY(NM_RCLICK, IDC_TAB_MSN, OnRclickTabMsn)
	ON_NOTIFY(NM_RCLICK, IDC_TAB_QQ, OnRclickTabQq)
	ON_COMMAND(ID_CLOSE_CHAT, OnCloseChat)
	ON_MESSAGE(WM_TRAY_MESSAGE,OnTrayNotify)
	ON_COMMAND(IDM_QOOM_EXIT, OnExit)
	ON_COMMAND(IDM_OPEN_QOOM, OnMenuOpen)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQoomDlg message handlers

BOOL CQoomDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//
	// Initialize the showing text.
	TCHAR szbuf[1024];
	::LoadString(AfxGetInstanceHandle(), IDS_ALERM, szbuf, sizeof(szbuf)/sizeof(TCHAR));
	m_staticEdit.SetWindowText(szbuf);
	// Query event
//	hQueryEvent = ::CreateEvent(NULL, TRUE, FALSE, g_QUERY_EVENT_NAME);	
    // Initialize the critical section one time only.
    InitializeCriticalSection(&CriticalSection);
	
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Load menu
	m_menuTray.LoadMenu(MAKEINTRESOURCE(IDR_MENU_TRAY));   // for tray icon menu.	
	m_menuClose.LoadMenu(MAKEINTRESOURCE(IDR_MENU_CLOSE)); // for close chat window menu.
//	m_popupMenu.CreatePopupMenu();
//	m_popupMenu.AppendMenu(MF_POPUP, (UINT)(m_menuClose.m_hMenu));

	//
	// Associate the tooltip control to the tab control
	m_tabQQ.SetToolTips(m_pTipQQ);
	m_tabMSN.SetToolTips(m_pTipMSN);
	
	// Set the image list
	imageListQQ.Create(IDB_BITMAP_STATE_QQ, 11, 0, RGB(0, 0, 255));
	imageListMSN.Create(IDB_BITMAP_STATE_MSN, 11, 0, RGB(0, 0, 255));

	m_tabQQ.SetImageList(&imageListQQ);
	m_tabMSN.SetImageList(&imageListMSN);
	
	m_tabQQ.SetSelectedColor(RGB(0, 0, 0));
	m_tabMSN.SetSelectedColor(RGB(0, 0, 0));
	m_tabQQ.SetMouseOverColor(RGB(0, 0, 0));
	m_tabMSN.SetMouseOverColor(RGB(0, 0, 0));

	// TODO: Add extra initialization here
	CreateSendRichEdit();
	// QQ
//	CreateHisRichEdit(0, FALSE);
//	CreateHisRichEdit(1, FALSE);
	// MSN
//	CreateHisRichEdit(0, TRUE);
//	CreateHisRichEdit(1, TRUE);
//	g_dwActiveSend = 0;
//	g_bMSNActiveSend = TRUE;
	// Bitmap show hide.
	m_btnQQ.ShowWindow(SW_HIDE);
	m_btnMsn.ShowWindow(SW_HIDE);
	
	::InitThread(NULL);
	::SetTimer(this->GetSafeHwnd(), TIMER_GETMSG_MSN, 1000, NULL);
	//::SetTimer(this->GetSafeHwnd(), TIMER_GETMSG_QQ, 1000, NULL);
	// Show the tray icon
	AddTrayIcon();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CQoomDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CQoomDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CQoomDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

int CQoomDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	g_hMainDlg = this->GetSafeHwnd();
	// For QQ's hide problem. but bugs exist.
	RegisterQQParentClass(AfxGetInstanceHandle());
	g_hQQParent = InitQQParent(AfxGetInstanceHandle());
	// For tooltips
	// Create a tooltip control.  m_ToolTipCtrl is a member variable
	// of type CToolTipCtrl* in CMyPropertySheet class.  It is 
	// initialized to NULL in the constructor, and destroyed in the 
	// destructor of CMyPropertySheet class.
	m_pTipMSN = new CToolTipCtrl;
	m_pTipQQ = new CToolTipCtrl;
	if (!m_pTipMSN->Create(this) ||
		!m_pTipQQ->Create(this))
	{
		TRACE(_T("Unable To create ToolTip\n"));
		return -1;
	}
	//
	// For tray icon message
	WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));

	return 0;
}

BOOL CQoomDlg::CreateSendRichEdit()
{
	RECT rect;
	// 2. Richedit for sending messages.
	// Get the static control's rect.
	m_staticEditSend.GetWindowRect(&rect);
	ScreenToClient(&rect);

	g_hChatSendEdit = ::CreateWindowEx(
		0,								// extended window style
		//_T("RichEdit"),
		_T("RichEdit20W"),
		//(LPCTSTR)RICHEDIT_CLASS ,		// Not properly useful.
		_T(""),							// window name
		WS_CHILDWINDOW | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL | 
		//ES_READONLY |
		ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,        // window style
		rect.left,						// horizontal position of window
		rect.top,						// vertical position of window
		rect.right-rect.left,           // window width
		rect.bottom-rect.top,			// window height
		this->GetSafeHwnd(),			// handle to parent or owner window
		//(HMENU)IDC_RICHEDIT,          // menu handle or child identifier
		NULL,
		AfxGetInstanceHandle()  ,		// handle to application instance
		NULL							// window-creation data
		);
	if (g_hChatSendEdit == NULL) {
		::ReportErr(_T("Failed to create the richedit control!")); 
		return FALSE;
	}
	SetFontType(g_hChatSendEdit, FALSE);
	// Attatch to object.
	m_editTextSend.Attach(g_hChatSendEdit);	//these 2 are the same

	return TRUE;
}

BOOL CQoomDlg::CreateHisRichEdit(int nIndex, BOOL bMSN)
{
	// Invalid index
	if (bMSN == TRUE && IsValidIndex(TRUE, nIndex) == FALSE) {
		return FALSE;
	} else if (bMSN == FALSE && IsValidIndex(FALSE, nIndex) == FALSE) {
		return FALSE;
	}
	// Create the rich edit for displaying the chat contents.
	// 1. Richedit for chat content.
	// Get the static control's rect.
	RECT rect;
	m_staticEdit.GetWindowRect(&rect);
	ScreenToClient(&rect);

	HWND hWndTmp = ::CreateWindowEx(
		0,								// extended window style
		//_T("RichEdit"),
		_T("RichEdit20W"),
		//(LPCTSTR)RICHEDIT_CLASS ,		// Not properly useful.
		_T(""),							// window name
		WS_CHILDWINDOW | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL | 
		//ES_READONLY |
		ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,        // window style
		rect.left,						// horizontal position of window
		rect.top,						// vertical position of window
		rect.right-rect.left,           // window width
		rect.bottom-rect.top,			// window height
		this->GetSafeHwnd(),			// handle to parent or owner window
		//(HMENU)IDC_RICHEDIT,          // menu handle or child identifier
		NULL,
		AfxGetInstanceHandle()  ,		// handle to application instance
		NULL							// window-creation data
		);
	if (hWndTmp == NULL) {
		::ReportErr(_T("Failed to create the richedit control!")); 
		return FALSE;
	}
	CString str;
	if (bMSN == TRUE) {
		g_hChatHisEditMSN[nIndex] = hWndTmp;
		// Also create the tab.
		str.Format(_T("%s%d"), _T(""), nIndex);
		int nCount = m_tabMSN.GetItemCount();
		m_tabMSN.InsertXItem(nCount, str, 0);
		g_hTabIndexMSN[nIndex] = nCount;
	} else {
		int nCount = m_tabQQ.GetItemCount();
		g_hChatHisEditQQ[nCount] = hWndTmp;
		// Also create the tab.
		str.Format(_T("%s[%d-%d]"), _T(""), LOWORD(nIndex), HIWORD(nIndex));
		m_tabQQ.InsertXItem(nCount, str, 0);
		g_hTabIndexQQ[nCount] = nIndex;
	}
	
	// Modify the font type to support the chinese font.
	SetFontType(hWndTmp, TRUE);
	str = _T("Begin talk now...");
	::SetWindowText(hWndTmp, str);

	return TRUE;
}

void CQoomDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	// TODO: Add your message handler code here
	// Kill the timer.
	::KillTimer(this->GetSafeHwnd(), TIMER_GETMSG_MSN);
	::KillTimer(this->GetSafeHwnd(), TIMER_GETMSG_QQ);
	//
	// Release the resource.
	imageListQQ.DeleteImageList();
	imageListMSN.DeleteImageList();
	//
    // Release resources used by the critical section object.
    DeleteCriticalSection(&CriticalSection);
	//
	//
	m_editTextSend.Detach(); //these 2 are the same	
	::ExitThread();
	//
	// Destroy the handle.
	::DestroyWindow(g_hChatSendEdit);
	for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
		if (g_hChatHisEditQQ[i] != NULL) {
			::DestroyWindow(g_hChatHisEditQQ[i]);
		}
	}
	for(i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
		if (g_hChatHisEditMSN[i] != NULL) {
			::DestroyWindow(g_hChatHisEditMSN[i]);
		}
	}
	//
	// Destroy the QQ parent window
	::DestroyWindow(g_hQQParent);
	//
	// Destroy tooltip control
	::DestroyWindow(m_pTipQQ->GetSafeHwnd());
	delete m_pTipQQ;
	m_pTipQQ = NULL;
	::DestroyWindow(m_pTipMSN->GetSafeHwnd());
	delete m_pTipMSN;
	m_pTipMSN = NULL;
	//
	// Destroy tray icon
	Shell_NotifyIcon(NIM_DELETE,&m_TrayData);
	//
	// Destroy the menu.
	m_menuClose.DestroyMenu();
	m_menuTray.DestroyMenu();
}

void CQoomDlg::OnOK() 
{
	// TODO: Add extra validation here
	// ########################################################################
	// Before sending the message, we must make clear that which chat window will
	// receive the message and that window is QQ's or MSN's.
	// ########################################################################
	CString str = "";
	m_editTextSend.GetWindowText(str);
	// We can send zero length string to QQ or MSN.
	if (str.GetLength() < 1) 
	{
		return;
	}
	// How to judge whether QQ or MSN chatting?
	if (g_bMSNActiveSend == FALSE)
	{
		int nActiveChatTrack = g_dwActiveSend;
		if (nActiveChatTrack == -1) 
		{
			PopMsg(_T("no qq active chat"));
			return;
		}
		DWORD row = LOWORD(g_dwActiveSend);
		DWORD col = HIWORD(g_dwActiveSend);
		QQSendChatText(str, TRUE, TRUE, row, col);
	} else {
//		PopMsg(str);

		int nActiveChatTrack = g_dwActiveSend;
		if (nActiveChatTrack == -1) 
		{
			PopMsg(_T("no msn active chat"));
			return;
		}
		MSNSendChatText(str, TRUE, TRUE, nActiveChatTrack);
	}
	m_editTextSend.SetWindowText(_T(""));
	// QQ will close this chat window after send the message.
	// in message mode.
	//CDialog::OnOK();
}

void CQoomDlg::OnBtnRefresh() 
{
	// TODO: Add your control notification handler code here
	// Refresh all
	if (g_bMSNActiveSend == FALSE)
	{
		int nActiveChatTrack = QQGetActiveChat();
		if (nActiveChatTrack == -1) 
		{
			PopMsg(_T("no active chat"));
			return;
		}
		::QQQueryChatContents(0, g_dwActiveSend);
	} else {
		int nActiveChatTrack = MSNGetActiveChat();
		if (nActiveChatTrack == -1) 
		{
			PopMsg(_T("no msn active chat"));
			return;
		}
		MSNQueryChatContents(g_dwActiveSend);
	}
}

void CQoomDlg::OnGetChatCount(WPARAM wParam, LPARAM lParam) 
{ 
	// 
	m_iMsnChatCount	= int(wParam);
}

void CQoomDlg::OnCreateMsnChatWindow(WPARAM wParam, LPARAM lParam)
{
	DWORD dwIndex = (DWORD)wParam;
	if (g_hChatHisEditMSN[dwIndex] == NULL) {
		CreateHisRichEdit(dwIndex, TRUE);
	}
	SetActiveChat(TRUE, dwIndex);
	// Show or hide the window.
	HWND hWnd = MSNGetChatWindowHandle(dwIndex);
	::ShowWindow(hWnd, SW_MINIMIZE);
	if (g_bMSNShowed == FALSE) {
		::ShowWindow(hWnd, SW_HIDE);
	}
}

void CQoomDlg::OnCreateQQChatWindow(WPARAM wParam, LPARAM lParam)
{
	Sleep(500);

	DWORD dwIndex = (DWORD)wParam;
	DWORD dwTabIndex = FindTabIndex(FALSE, dwIndex);
	if (dwTabIndex == (DWORD)-1) {
		CreateHisRichEdit(dwIndex, FALSE);
	}
	dwTabIndex = FindTabIndex(FALSE, dwIndex);
	SetActiveChat(FALSE, dwTabIndex);

	// Show or hide the window.
	HWND hWnd = QQGetChatWindowHandle(LOWORD(dwIndex), HIWORD(dwIndex));
	::ShowWindow(hWnd, SW_MINIMIZE);
	if (g_bQQShowed == FALSE) {
		ShowInTaskbar(hWnd, FALSE);
	}
}

void CQoomDlg::OnDestroyMsnChatWindow(WPARAM wParam, LPARAM lParam)
{
	DWORD dwDataIndex = (DWORD)wParam;
	if (IsValidIndex(TRUE, dwDataIndex) == FALSE) {
		return;
	}
	if (g_hChatHisEditMSN[dwDataIndex] != NULL) {
		::DestroyWindow(g_hChatHisEditMSN[dwDataIndex]);
		g_hChatHisEditMSN[dwDataIndex] = NULL;
	}
	m_tabMSN.DeleteTab(FindTabIndex(TRUE, dwDataIndex));

	// Reduce the tabindex with -1.
	AdjustTabIndex(TRUE, dwDataIndex);

	BOOL bFound = FALSE;
	for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
		if (g_hChatHisEditMSN[i] != NULL) {
			bFound = TRUE;
			m_tabMSN.SetCurSel(0);
			SetActiveChat(TRUE, 0);
			break;
		}
	}
	if (bFound == FALSE) {
		for(i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
			if (g_hChatHisEditQQ[i] != NULL) {
				bFound = TRUE;
				m_tabQQ.SetCurSel(i);
				SetActiveChat(FALSE, i);
				break;
			}
		}
	}
	// Indicate that no QQ and MSN chat window any more.
	if (bFound == FALSE) {
		// Hide the background picture or text box...
		m_staticEdit.ShowWindow(SW_SHOW);		
		// The flag
		m_btnQQ.ShowWindow(SW_HIDE);
		m_btnMsn.ShowWindow(SW_HIDE);
	}
}

void CQoomDlg::OnDestroyQQChatWindow(WPARAM wParam, LPARAM lParam)
{
	DWORD dwDataIndex = (DWORD)wParam;
	if (IsValidIndex(FALSE, dwDataIndex) == FALSE) {
		return;
	}
	DWORD dwTabIndex = FindTabIndex(FALSE, dwDataIndex);
	if (g_hChatHisEditQQ[dwTabIndex] != NULL) {
		::DestroyWindow(g_hChatHisEditQQ[dwTabIndex]);
		g_hChatHisEditQQ[dwTabIndex] = NULL;
	}
	// Reduce the tabindex with -1.
	AdjustTabIndex(FALSE, dwDataIndex);
	// Delete the item, not the same as MSN flow.
	m_tabQQ.DeleteTab(dwTabIndex);
	// Find the new tab index.
	dwTabIndex = FindTabIndex(FALSE, dwDataIndex);

	BOOL bFound = FALSE;
	for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
		if (g_hChatHisEditQQ[i] != NULL) {
			bFound = TRUE;
			m_tabQQ.SetCurSel(0);
			SetActiveChat(FALSE, 0);
			break;
		}
	}
	if (bFound == FALSE) {
		for(i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
			if (g_hChatHisEditMSN[i] != NULL) {
				bFound = TRUE;
				m_tabMSN.SetCurSel(FindTabIndex(TRUE, i));
				SetActiveChat(TRUE, FindTabIndex(TRUE, i));
				break;
			}
		}
	}
	// Indicate that no QQ and MSN chat window any more.
	if (bFound == FALSE) {
		// Hide the background picture or text box...
		m_staticEdit.ShowWindow(SW_SHOW);		
		// The flag
		m_btnQQ.ShowWindow(SW_HIDE);
		m_btnMsn.ShowWindow(SW_HIDE);
	}
}

void CQoomDlg::SetFontType(HWND hWnd, BOOL bReadOnly)
{
	CRichEditCtrl editCtrl;
	editCtrl.Attach(hWnd);
	editCtrl.SetReadOnly(bReadOnly);
	//Set Default Char Format
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_FACE | CFM_COLOR | CFM_CHARSET;
	cf.dwEffects = 0;
	cf.crTextColor = RGB(0,0,255);
	cf.bCharSet = GB2312_CHARSET;
	::lstrcpy((LPTSTR)cf.szFaceName, _T("Arial"));
	editCtrl.SetDefaultCharFormat(cf);

	editCtrl.Detach();

//	::SendMessage(hWnd, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
//	::SendMessage(hWnd, EM_REPLACESEL, 
//		(WPARAM)0,    // undo option
//		(LPARAM)(LPCTSTR)_T("\r\n")     // text string (LPCTSTR)
//		);
//
//	//set bold char format
//	CHARFORMAT cf;
//	cf.cbSize = sizeof(CHARFORMAT);
//	cf.dwMask = CFM_BOLD | CFM_FACE | CFM_COLOR;
//	cf.dwEffects = CFE_BOLD;
//	cf.crTextColor = RGB(0,0,0);
//	::lstrcpy((LPTSTR)cf.szFaceName, _T("Arial"));
//	DWORD dw = ::SendMessage(hWnd, EM_SETCHARFORMAT,
//		SCF_SELECTION,
//		//SCF_ALL,
//		(LPARAM)&cf);
}

void CQoomDlg::SetActiveChat(BOOL bMSNActive, DWORD dwTabIndex)
{
	DWORD dwDataIndex = (DWORD)FindDataIndex(bMSNActive, dwTabIndex);
	if (IsValidIndex(bMSNActive, dwDataIndex) == FALSE) {
		return;
	}
	// Request ownership of the critical section.
	EnterCriticalSection(&CriticalSection); 
	g_dwActiveSend = dwDataIndex;
	g_bMSNActiveSend = bMSNActive;
	if (bMSNActive == TRUE) {
		g_bHisChangedMSN[dwDataIndex] = FALSE;
		m_tabMSN.SetCurSel(dwTabIndex);
		ShowHisContent(TRUE, dwDataIndex);
		// The flag
		m_btnQQ.ShowWindow(SW_HIDE);
		m_btnMsn.ShowWindow(SW_SHOW);
		// Add the tool tips
		TCHAR szBuf[256];
		if (MSNGetChatWindowTitle(dwDataIndex, szBuf)) {
			CString str = szBuf;//TrimChatName(szBuf, TRUE);
			SetToolTips(dwTabIndex, str, TRUE);
		}
	} else {
		g_bHisChangedQQ[dwTabIndex] = FALSE;
		m_tabQQ.SetCurSel(dwTabIndex);
		ShowHisContent(FALSE, dwDataIndex);
		// The flag
		m_btnQQ.ShowWindow(SW_SHOW);
		m_btnMsn.ShowWindow(SW_HIDE);
		// Add the tool tips
		TCHAR szBuf[256];
		if (QQGetChatWindowTitle(LOWORD(dwDataIndex), HIWORD(dwDataIndex), szBuf)) {
			CString str = szBuf;//TrimChatName(szBuf, FALSE);
			SetToolTips(dwTabIndex, str, FALSE);
		}
	}
	// Release ownership of the critical section.
	LeaveCriticalSection(&CriticalSection);
}

void CQoomDlg::ShowHisContent(BOOL bMSNActive, DWORD dwIndex)
{
	// Hide the background picture or text box...
	m_staticEdit.ShowWindow(SW_HIDE);

	if (IsValidIndex(bMSNActive, dwIndex) == FALSE) {
		return;
	}
	if (bMSNActive == TRUE) {
		for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
			if (g_hChatHisEditMSN[i] != NULL) {
				if (i == (int)dwIndex) {
					::ShowWindow(g_hChatHisEditMSN[i], SW_SHOW);
					::UpdateWindow(g_hChatHisEditMSN[i]);
				} else {
					::ShowWindow(g_hChatHisEditMSN[i], SW_HIDE);
				}
			}
		}
		for(i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
			if (g_hChatHisEditQQ[i] != NULL) {
				::ShowWindow(g_hChatHisEditQQ[i], SW_HIDE);
			}
		}
	} else {
		// QQ
		DWORD dwTabIndex = FindTabIndex(FALSE, dwIndex);
		for(DWORD i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
			if (g_hChatHisEditQQ[i] != NULL) {
				// First find the tabindex.
				if (dwTabIndex == i) {
					::ShowWindow(g_hChatHisEditQQ[i], SW_SHOW);
					::UpdateWindow(g_hChatHisEditQQ[i]);
				} else {
					::ShowWindow(g_hChatHisEditQQ[i], SW_HIDE);
				}
			}
		}
		for(i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
			if (g_hChatHisEditMSN[i] != NULL) {
				::ShowWindow(g_hChatHisEditMSN[i], SW_HIDE);
			}
		}
	}
}

void CQoomDlg::OnSelchangeTabMsn(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DWORD dwTabIndex = m_tabMSN.GetCurSel();
	
	if (g_bMSNActiveSend == TRUE) {
		DWORD dwActualTab = FindTabIndex(TRUE, g_dwActiveSend);
		// For refresh.
		if (dwActualTab == dwTabIndex) {
			return;
		}
	}

	SetActiveChat(TRUE, dwTabIndex);
	SignChangeFlag(TRUE, dwTabIndex, FALSE);

	*pResult = 0;
}

void CQoomDlg::OnSelchangeTabQq(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DWORD dwTabIndex = m_tabQQ.GetCurSel();
	if (g_bMSNActiveSend == FALSE) {
		DWORD dwActualTab = FindTabIndex(FALSE, g_dwActiveSend);
		// For refresh.
		if (dwActualTab == dwTabIndex) {
			return;
		}
	}
	SetActiveChat(FALSE, dwTabIndex);
	SignChangeFlag(FALSE, dwTabIndex, FALSE);

	*pResult = 0;
}

BOOL CQoomDlg::IsValidIndex(BOOL bMSNActive, DWORD dwIndex)
{
	if (bMSNActive == TRUE) {
		if (dwIndex < 0 || dwIndex > MAX_MSN_CONCUR_CHAT) {
			return FALSE;
		}
	} else {
		DWORD dwRow = LOWORD(dwIndex);
		DWORD dwCol = HIWORD(dwIndex);
		if (dwRow < 0 || dwRow > MAX_QQ_CONCUR_MAIN ||
			dwCol < 0 || dwCol > MAX_QQ_CONCUR_CHAT) {
			return FALSE;
		}
	}
	return TRUE;
}

DWORD CQoomDlg::FindDataIndex(BOOL bMSNActive, DWORD dwTabIndex) {
	DWORD nIndex = (DWORD)-1;
	if (bMSNActive == TRUE) {
		for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
			if (dwTabIndex == g_hTabIndexMSN[i]) {
				nIndex = i;
				break;
			}
		}
	} else {
		nIndex = g_hTabIndexQQ[dwTabIndex];
	}
	
	return nIndex;
}

int CQoomDlg::FindTabIndex(BOOL bMSNActive, DWORD dwDataIndex)
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

void CQoomDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == TIMER_GETMSG_MSN) {
		for(DWORD i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
			if (g_hChatHisEditMSN[i] != NULL) {
				MSNQueryChatContents(i);
			}
		}
	} 
	// For QQ is not unicode version, hard to judge from the text length changed.
//	else if (nIDEvent == TIMER_GETMSG_QQ) {
//		for(int i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
//			if (g_hChatHisEditQQ[i] != NULL) {
//				DWORD dwDataIndex = FindDataIndex(FALSE, i);
//				DWORD dwRow = LOWORD(dwDataIndex);
//				DWORD dwCol = HIWORD(dwDataIndex);
//				QQQueryChatContents(dwRow, dwCol);
//				// Changed, sign it.
//				if (g_bHisChangedQQ[i] == TRUE) {
//					if (g_bMSNActiveSend == FALSE &&	
//						g_dwActiveSend == dwDataIndex) {
//						// ... do not set the flag at this time.
//					} else {
//						SignChangeFlag(FALSE, i, TRUE);
//					}
//				}
//			}
//		}
//	}
	CDialog::OnTimer(nIDEvent);
}

//BOOL CQoomDlg::BeginLock()
//{
//	HANDLE hEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, g_QUERY_EVENT_NAME);
//
//	if(hEvent == NULL)
//	{
//		//::ReportErr(_T("g_RICHEDIT_EVENT_NAME Event Creation Failed")); 
//		return FALSE;
//	}
//	// Signal
//	if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) {
//	}
//	// Notify the richedit.dll, that a new chat window is created just now.
//	::SetEvent(hEvent);
//}
//
//BOOL CQoomDlg::EndLock()
//{

// dwDataIndex
BOOL CQoomDlg::SignChangeFlag(BOOL bMSNActive, DWORD dwTabIndex, BOOL bSet) 
{
	if (dwTabIndex == (DWORD)-1) {
		return FALSE;
	}
	//
	CXTabCtrl *tab;
	if (bMSNActive == TRUE) {
		tab = &m_tabMSN;
	} else {
		tab = &m_tabQQ;
	}

	tab->SetInfoColorFlag(bSet ? true : false, dwTabIndex, RGB(255, 0, 0));

	return TRUE;
}

void CQoomDlg::AdjustTabIndex(BOOL bMSNActive, DWORD dwDataIndex)
{
	if (bMSNActive == TRUE) {
		g_hTabIndexMSN[dwDataIndex] = (DWORD)-1;
		for(DWORD i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
			if (i > dwDataIndex && g_hTabIndexMSN[i] != (DWORD)-1) {
				g_hTabIndexMSN[i] -= (DWORD)1;
			}
		}
	} else {
		// QQ
		// Also adjust g_hChatHisEditQQ[];
		HWND hTmp[MAX_QQ_CONCUR_CHAT];
		memset(hTmp, 0, MAX_QQ_CONCUR_CHAT*sizeof(HWND));
		
		DWORD dwTabIndex = FindTabIndex(FALSE, dwDataIndex);
		
		g_hChatHisEditQQ[dwTabIndex] = NULL;
		DWORD dwTmpIndex = 0;
		for(DWORD i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
			if (g_hChatHisEditQQ[i] != NULL) {
				hTmp[dwTmpIndex] = g_hChatHisEditQQ[i];
				dwTmpIndex ++;
			}
		}
		::CopyMemory(&g_hChatHisEditQQ, &hTmp, MAX_QQ_CONCUR_CHAT*sizeof(HWND));

		// g_hTabIndexQQ
		DWORD dwTmp[MAX_QQ_CONCUR_CHAT];
		memset(dwTmp, -1, MAX_QQ_CONCUR_CHAT*sizeof(DWORD));

		dwTabIndex = FindTabIndex(FALSE, dwDataIndex);

		g_hTabIndexQQ[dwTabIndex] = (DWORD)-1;
		dwTmpIndex = 0;
		for(i = 0; i < MAX_QQ_CONCUR_CHAT; i++) {
			if (g_hTabIndexQQ[i] != (DWORD)-1) {
				dwTmp[dwTmpIndex] = g_hTabIndexQQ[i];
				dwTmpIndex ++;
			}
		}
		::CopyMemory(&g_hTabIndexQQ, &dwTmp, MAX_QQ_CONCUR_CHAT*sizeof(DWORD));

	}
}

// Show/Hide the chat windows
void CQoomDlg::OnBtnHideMsn() 
{
	// TODO: Add your control notification handler code here
	g_bMSNShowed = !g_bMSNShowed;
	MSNShowChatWindow(g_bMSNShowed);
//	if (g_bMSNShowed) {
//		GetDlgItem(ID_BTN_HIDEMSN)->SetWindowText(_T("Show MSN..."));
//	} else {
//		GetDlgItem(ID_BTN_HIDEMSN)->SetWindowText(_T("Hide MSN..."));
//	}
}

void CQoomDlg::OnBtnHideQQ() 
{
	// TODO: Add your control notification handler code here
	g_bQQShowed = !g_bQQShowed;
	QQShowChatWindow(g_bQQShowed);
//	if (g_bQQShowed) {
//		GetDlgItem(ID_BTN_HIDEQQ)->SetWindowText(_T("Show QQ..."));
//	} else {
//		GetDlgItem(ID_BTN_HIDEQQ)->SetWindowText(_T("Hide QQ..."));
//	}
}

void CQoomDlg::OnClickTabMsn(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	// For refresh.
	m_btnQQ.ShowWindow(SW_HIDE);
	m_btnMsn.ShowWindow(SW_SHOW);

	CPoint pt;
	GetCursorPos(&pt);
	m_tabMSN.ScreenToClient(&pt);
	TCHITTESTINFO testInfo;
	testInfo.pt = pt;
	testInfo.flags = TCHT_ONITEM;
	int nItem = m_tabMSN.HitTest(&testInfo);
	if (nItem >= 0) {
		m_tabMSN.SetCurSel(nItem);
		::SendMessage(m_tabMSN.GetSafeHwnd(), TCM_SETCURSEL, WPARAM(nItem), NULL);
		OnSelchangeTabMsn(pNMHDR, pResult);	
	}

	*pResult = 0;
}

void CQoomDlg::OnClickTabQq(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	// For refresh.
	m_btnQQ.ShowWindow(SW_SHOW);
	m_btnMsn.ShowWindow(SW_HIDE);

	CPoint pt;
	GetCursorPos(&pt);
	m_tabQQ.ScreenToClient(&pt);
	TCHITTESTINFO testInfo;
	testInfo.pt = pt;
	testInfo.flags = TCHT_ONITEM;
	int nItem = m_tabQQ.HitTest(&testInfo);
	if (nItem >= 0) {
		m_tabQQ.SetCurSel(nItem);
		::SendMessage(m_tabQQ.GetSafeHwnd(), TCM_SETCURSEL, WPARAM(nItem), NULL);
		OnSelchangeTabQq(pNMHDR, pResult);	
	}

	*pResult = 0;
}

// text change message
void CQoomDlg::OnQQTextChanged(WPARAM wParam, LPARAM lParam)
{
	DWORD dwTabIndex = (DWORD)wParam;
	BOOL  bQQChanged = BOOL(lParam);
	DWORD dwActualTab = DWORD(-1);

	if (dwTabIndex != (DWORD)-1) {
		if (bQQChanged == TRUE) {
			// QQ changed, current is msn, sign it.
			if (g_bMSNActiveSend == TRUE) {
				SignChangeFlag(FALSE, dwTabIndex, TRUE);
			} else {
				// current is qq, judge whether is current tab.
				dwActualTab = FindTabIndex(FALSE, g_dwActiveSend);
				if (g_bMSNActiveSend == FALSE && dwTabIndex == dwActualTab) {
					// do not need info, because we are using it now.
					;
				} else {
					SignChangeFlag(FALSE, dwTabIndex, TRUE);
				}
			}
		} else {
			if (g_bMSNActiveSend == FALSE) {
				SignChangeFlag(TRUE, dwTabIndex, TRUE);
			} else {
				dwActualTab = FindTabIndex(TRUE, g_dwActiveSend);
				if (g_bMSNActiveSend == TRUE && dwTabIndex == dwActualTab) {
					// do not need info, because we are using it now.
					;
				} else {
					SignChangeFlag(TRUE, dwTabIndex, TRUE);
				}
			}
		}
	}
	// If main dialog is not visable, just active it.
//	if (::IsWindowVisible(this->GetSafeHwnd()) == FALSE) {
//		::SetActiveWindow(this->GetSafeHwnd());
//	}
	if (this->IsIconic())
		this->ShowWindow(SW_RESTORE);
	
//	::FlashWindow(this->GetSafeHwnd(), TRUE); // invert the title bar 

	// フォーカスをメインウィンドウに設定する
	this->SetForegroundWindow();
	
	// ウィンドウにポップアップウィンドウがある場合は、
	// フォーカスをポップアップに設定する
	this->GetLastActivePopup()->SetForegroundWindow();
}

// Right click for closing chat window.
void CQoomDlg::OnRclickTabMsn(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CWaitCursor waitCur;
	m_bMsnPopup = TRUE;
	
	CPoint pt, ptScr;
	GetCursorPos(&pt);
	ptScr = pt;
	m_tabMSN.ScreenToClient(&pt);
	TCHITTESTINFO testInfo;
	testInfo.pt = pt;
	testInfo.flags = TCHT_ONITEM;
	int nItem = m_tabMSN.HitTest(&testInfo);
	if (nItem >= 0) {
		::SendMessage(m_tabMSN.GetSafeHwnd(), TCM_SETCURSEL, WPARAM(nItem), NULL);
		//::SendNotifyMessage(m_tabQQ.GetSafeHwnd(), WM_NOTIFY, WPARAM(TCN_SELCHANGE), LPARAM(pResult));
		OnClickTabMsn(pNMHDR, pResult);
//		m_menuClose.TrackPopupMenu(TPM_RIGHTBUTTON, ptScr.x, ptScr.y, this, NULL);
		m_menuClose.GetSubMenu(0)->TrackPopupMenu(
			TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
			ptScr.x,ptScr.y,this);
	}
	
	*pResult = 0;
	waitCur.Restore();
}

void CQoomDlg::OnRclickTabQq(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CWaitCursor waitCur;
	m_bMsnPopup = FALSE;
	
	CPoint pt, ptScr;
	GetCursorPos(&pt);
	ptScr = pt;
	m_tabQQ.ScreenToClient(&pt);
	TCHITTESTINFO testInfo;
	testInfo.pt = pt;
	testInfo.flags = TCHT_ONITEM;
	int nItem = m_tabQQ.HitTest(&testInfo);
	if (nItem >= 0) {
		::SendMessage(m_tabQQ.GetSafeHwnd(), TCM_SETCURSEL, WPARAM(nItem), NULL);
		//::SendNotifyMessage(m_tabQQ.GetSafeHwnd(), WM_NOTIFY, WPARAM(TCN_SELCHANGE), LPARAM(pResult));
		OnClickTabQq(pNMHDR, pResult);
		m_menuClose.GetSubMenu(0)->TrackPopupMenu(
			TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
			ptScr.x,ptScr.y,this);
	}

	*pResult = 0;
	waitCur.Restore();
}

void CQoomDlg::OnCloseChat() 
{
	// TODO: Add your command handler code here
	CWaitCursor waitCur;
	int nTabIndex = -1;
	DWORD nDataIndex = -1;
	HWND hWnd = NULL;
	if (m_bMsnPopup) {
		nTabIndex = m_tabMSN.GetCurSel();
		nDataIndex = FindDataIndex(TRUE, nTabIndex);
		if (nDataIndex != (DWORD)-1) {
			hWnd = MSNGetChatWindowHandle(nDataIndex);
		}
	} else {
		nTabIndex = m_tabQQ.GetCurSel();
		nDataIndex = FindDataIndex(FALSE, nTabIndex);
		if (nDataIndex != (DWORD)-1) {
			hWnd = QQGetChatWindowHandle(LOWORD(nDataIndex), HIWORD(nDataIndex));
		}
	}
	if (hWnd != NULL && ::IsWindow(hWnd)) {
		// Just close it.
		::PostMessage(hWnd, WM_CLOSE, NULL, NULL);
	} else {
		// Remove the tab only...
		// This case only occured when exception occured.
		if (m_bMsnPopup) {
			OnDestroyMsnChatWindow(nDataIndex, 0);
		} else {
			OnDestroyQQChatWindow(nDataIndex, 0);
		}
	}

	waitCur.Restore();
}

/************************************************************************/
/* For tool tips                                                        */
/************************************************************************/
CString CQoomDlg::TrimChatName(TCHAR* szBuf, BOOL bMsn)
{
	TCHAR szReturn[256];
	memset(szReturn, 0, 256);


	try {
		if (bMsn) {
			// **(****~~~) - ??
//			int nPos = strName.ReverseFind('-');
//			str = strName.Left(nPos -1);
		} else {
			//QQ
//			CString strRight = strName.Right(6);
//			if (strRight == _T("聊天中")) {
//				// 与 michael 聊天中
//				int nLen = strName.GetLength();
//				str = strName.Mid(3, nLen-7);
//			} else if (strRight == _T("送消息")) {
//				// michaeltest - ?送消息
//				int nPos = strName.ReverseFind('-');
//				str = strName.Left(nPos -1);
//			}
			//////////////////////////////////////////////////////////////////////////
			TCHAR szLast[256];
			TCHAR szUnicode[256], szUnicode2[256];
			memset(szLast, 0, 256);
			memset(szUnicode, 0, 256);
			memset(szUnicode2, 0, 256);
			int nSize = lstrlen(szBuf);
			szLast[0] = szBuf[nSize-6];
			szLast[1] = szBuf[nSize-5];
			szLast[2] = szBuf[nSize-4];
			szLast[3] = szBuf[nSize-3];
			szLast[4] = szBuf[nSize-2];
			szLast[5] = szBuf[nSize-1];
			// we want to convert an MBCS string in lpszA
			MultiByteToWideChar(CP_ACP, 0, "聊天中", -1, szUnicode, 64);
			MultiByteToWideChar(CP_ACP, 0, "送消息", -1, szUnicode2, 64);
//			lstrcpy(szUnicode, _T("聊天中"));
//			lstrcpy(szUnicode2, _T("送消息"));
			if (::lstrcmp(szLast, szUnicode) == 0) {
				int nMaxLen = lstrlen(szBuf) - 10;
				szBuf += 5;
				::lstrcpyn(szReturn, szBuf, nMaxLen);
			} else if (::lstrcmp(szLast, szUnicode2) == 0) {
				int nMaxLen = lstrlen(szBuf) - 10;
				::lstrcpyn(szReturn, szBuf, nMaxLen);
			}
		}
	} catch (...) {
		//
	}

	CString str(szReturn);
	return str;
}

BOOL CQoomDlg::SetToolTips(int nTabIndex, const CString& strTip, BOOL bMsn)
{
	// Get the bounding rectangle of each tab in the tab control of the
	// property sheet. Use this rectangle when registering a tool with 
	// the tool tip control.  IDS_FIRST_TOOLTIP is the first ID string 
	// resource that contains the text for the tool.
	
	CRect rect;
	if (bMsn) {
		int count = m_tabMSN.GetItemCount();
		if (nTabIndex < 0 || nTabIndex > count) {
			return FALSE;
		}
		m_tabMSN.GetItemRect(nTabIndex, &rect);
		VERIFY(m_pTipMSN->AddTool(&m_tabMSN, (LPCTSTR)strTip, &rect, nTabIndex+1));
		// Activate the tooltip control.
		m_pTipMSN->Activate(TRUE);
	} else {
		int count = m_tabQQ.GetItemCount();
		if (nTabIndex < 0 || nTabIndex > count) {
			return FALSE;
		}
		m_tabQQ.GetItemRect(nTabIndex, &rect);
		VERIFY(m_pTipQQ->AddTool(&m_tabQQ, (LPCTSTR)strTip, &rect, nTabIndex+1));
		// Activate the tooltip control.
		m_pTipQQ->Activate(TRUE);
	}

	return TRUE;
}

/************************************************************************/
/* FOR TRAY ICON                                                        */
/************************************************************************/
afx_msg void CQoomDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	UINT uID = (UINT) wParam;
	UINT uMsg = (UINT) lParam;

	if (uID != 1)
		return;
	
	if (uMsg == WM_MOUSEMOVE) {
		return;
	}
	CPoint pt;	
	switch (uMsg)
	{
	case WM_LBUTTONDBLCLK:
		OnMenuOpen();
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_CONTEXTMENU:
		GetCursorPos(&pt);
		OnTrayLButtonDown(pt);
		break;
	default:
		break;
	} 
	return; 
}

void CQoomDlg::OnTrayLButtonDown(CPoint pt)
{
	//m_menuTray is the member of CQoomDlg as CMenu m_menuTray;
	m_menuTray.GetSubMenu(0)->TrackPopupMenu(
		TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
		pt.x,pt.y,this);
}

void CQoomDlg::OnTrayRButtonDown(CPoint pt)
{
	OnTrayLButtonDown(pt);
}

void CQoomDlg::AddTrayIcon()
{
	m_TrayData.cbSize = sizeof(NOTIFYICONDATA);
	//Size of this structure, in bytes. 

	m_TrayData.hWnd	= this->m_hWnd;
	//Handle to the window that receives notification 
	//messages associated with an icon in the taskbar 
	//status area. The Shell uses hWnd and uID to 
	//identify which icon to operate on when Shell_NotifyIcon is invoked. 

	m_TrayData.uID = 1;
	//Application-defined identifier of the taskbar icon.
	//The Shell uses hWnd and uID to identify which icon 
	//to operate on when Shell_NotifyIcon is invoked. You
	// can have multiple icons associated with a single 
	//hWnd by assigning each a different uID. 

	m_TrayData.uCallbackMessage	= WM_TRAY_MESSAGE;
	//Application-defined message identifier. The system 
	//uses this identifier to send notifications to the 
	//window identified in hWnd. These notifications are 
	//sent when a mouse event occurs in the bounding 
	//rectangle of the icon, or when the icon is selected 
	//or activated with the keyboard. The wParam parameter 
	//of the message contains the identifier of the taskbar 
	//icon in which the event occurred. The lParam parameter 
	//holds the mouse or keyboard message associated with the
	// event. For example, when the pointer moves over a 
	//taskbar icon, lParam is set to WM_MOUSEMOVE. 

	m_TrayData.hIcon = this->m_hIcon;
	//Handle to the icon to be added, modified, or deleted
	
	lstrcpy(m_TrayData.szTip, _T("Qoom"));
	//Pointer to a null-terminated string with the text 
	//for a standard ToolTip. It can have a maximum of 64 
	//characters including the terminating NULL. 
	
	m_TrayData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	//Flags that indicate which of the other members contain 
	//valid data.  

	BOOL bSuccess = FALSE;

	bSuccess = Shell_NotifyIcon(NIM_ADD, &m_TrayData);

	if(!(bSuccess))
		MessageBox(_T("Unable to Set Tary Icon"));
}

void CQoomDlg::OnExit() 
{
	DestroyWindow();
}

void CQoomDlg::OnMenuOpen() 
{
	::ShowWindow(this->GetSafeHwnd(), SW_RESTORE);
}


BOOL CQoomDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	// Do not reponse the Escape key.
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

LRESULT CQoomDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(message == WM_TASKBARCREATED) // The taskbar rebuild message
	{
        AddTrayIcon();
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}
/************************************************************************/
/* When exit or closing                                                 */
/************************************************************************/
// Minimize the qq main window.
void CQoomDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	::ShowWindow(this->GetSafeHwnd(), SW_MINIMIZE);
	::ShowWindow(this->GetSafeHwnd(), SW_HIDE);
}
// Destroy the mainwindow
void CQoomDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	IsQQMSNRunning();

	CExitOptDlg dlg;
	dlg.m_chkMSN = m_bFoundMSN;
	dlg.m_chkQQ = m_bFoundQQ;

	BOOL bCloseQQ = FALSE;
	BOOL bCloseMSN = FALSE;
	if (dlg.DoModal() == IDOK) {
		bCloseQQ = dlg.m_chkQQ;
		bCloseMSN = dlg.m_chkMSN;
	}
	// wait cursor
	CWaitCursor wait;
	HWND hWnd = NULL;
	if (bCloseQQ == TRUE) {
		for(int i = 0; i < MAX_QQ_CONCUR_MAIN; i++) {
			for(int j = 0; j < MAX_QQ_CONCUR_CHAT; j++) {
				hWnd = QQGetChatWindowHandle(i, j);
				if (hWnd != NULL && ::IsWindow(hWnd)) {
					::PostMessage(hWnd, WM_CLOSE, NULL, NULL);
					Sleep(500);
//					AfxMessageBox(_T("stop"));
				}
			}
		}
		for(i = 0; i < MAX_QQ_CONCUR_MAIN; i++) {
			hWnd = GetQQMainHWND(i);
			if (hWnd != NULL && ::IsWindow(hWnd)) {
				::QQDestroyMainWindow(hWnd);
				Sleep(500);
			}
		}
	}
	if (bCloseMSN == TRUE) {
		for(int i = 0; i < MAX_MSN_CONCUR_CHAT; i++) {
			hWnd = MSNGetChatWindowHandle(i);
			if (hWnd != NULL && ::IsWindow(hWnd)) {
				::PostMessage(hWnd, WM_CLOSE, NULL, NULL);
				Sleep(500);
			}
		}
		hWnd = GetMSNMainHWND();
		if (hWnd != NULL && ::IsWindow(hWnd)) {
			//::PostMessage(hWnd, WM_DESTROY, NULL, NULL);
			MSNDestroyMainWindow(hWnd);
			Sleep(500);
		}
	}
	Sleep(1000);
	wait.Restore();

	CDialog::OnCancel();
}

// Whether the qq or msn is running now.
BOOL CQoomDlg::IsQQMSNRunning()
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
