// QoomDlg.h : header file
//

#if !defined(AFX_QOOMDLG_H__C7AC21D4_A92B_44D5_9FF8_693F50F80BF8__INCLUDED_)
#define AFX_QOOMDLG_H__C7AC21D4_A92B_44D5_9FF8_693F50F80BF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XTabCtrl.h"
/////////////////////////////////////////////////////////////////////////////
// CQoomDlg dialog

class CQoomDlg : public CDialog
{
private:
	// Tooltips
	CToolTipCtrl *m_pTipMSN;
	CToolTipCtrl *m_pTipQQ;
	// Trayicon
	CMenu m_menuTray;
	NOTIFYICONDATA m_TrayData;
	void OnTrayRButtonDown(CPoint pt);
	void OnTrayLButtonDown(CPoint pt);
	void AddTrayIcon();
	unsigned int WM_TASKBARCREATED;
	// closing window
	BOOL m_bFoundQQ;
	BOOL m_bFoundMSN;
	
	// Get msn chatting count.
	int   m_iMsnChatCount;
	// image list
	CImageList imageListQQ;
	CImageList imageListMSN;
	// menu
	BOOL  m_bMsnPopup;
	int   m_closeTabIndex;
	CMenu m_menuClose;
	// functions.
	void  SetFontType(HWND hWnd, BOOL bReadOnly);
	BOOL  CreateSendRichEdit();
	BOOL  CreateHisRichEdit(int nIndex, BOOL bMSN);
	void  SetActiveChat(BOOL bMSNActive, DWORD dwTabIndex);
	void  ShowHisContent(BOOL bMSNActive, DWORD dwIndex);
	BOOL  IsValidIndex(BOOL bMSNActive, DWORD dwIndex);
	DWORD FindDataIndex(BOOL bMSNActive, DWORD dwTabIndex);
	int   FindTabIndex(BOOL bMSNActive, DWORD dwDataIndex);		
	void  AdjustTabIndex(BOOL bMSNActive, DWORD dwDataIndex);
	BOOL  SignChangeFlag(BOOL bMSNActive, DWORD dwTabIndex, BOOL bSet);

	CString TrimChatName(TCHAR* szBuf, BOOL bMsn);
	BOOL SetToolTips(int nTabIndex, const CString& strTip, BOOL bMsn);
	
	BOOL IsQQMSNRunning();
// Construction
public:
	CQoomDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CQoomDlg)
	enum { IDD = IDD_QOOM_DIALOG };
	CStatic	m_btnQQ;
	CStatic	m_btnMsn;
	CXTabCtrl	m_tabQQ;
	CXTabCtrl	m_tabMSN;
	CStatic	m_staticEditSend;
	CStatic	m_staticEdit;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQoomDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	CRichEditCtrl m_editTextSend;
	
	// Generated message map functions
	//{{AFX_MSG(CQoomDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual void OnOK();
	afx_msg void OnBtnRefresh();
	afx_msg void OnGetChatCount(WPARAM wParam, LPARAM lParam); 	
	afx_msg void OnCreateMsnChatWindow(WPARAM wParam, LPARAM lParam); 	
	afx_msg void OnCreateQQChatWindow(WPARAM wParam, LPARAM lParam); 	
	afx_msg void OnDestroyMsnChatWindow(WPARAM wParam, LPARAM lParam); 	
	afx_msg void OnDestroyQQChatWindow(WPARAM wParam, LPARAM lParam); 	
	afx_msg void OnQQTextChanged(WPARAM wParam, LPARAM lParam); 	
	afx_msg void OnSelchangeTabMsn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeTabQq(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnHideMsn();
	afx_msg void OnBtnHideQQ();
	afx_msg void OnClickTabMsn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickTabQq(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickTabMsn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickTabQq(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseChat();
	afx_msg void OnTrayNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnExit();
	afx_msg void OnMenuOpen();
	afx_msg void OnClose();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QOOMDLG_H__C7AC21D4_A92B_44D5_9FF8_693F50F80BF8__INCLUDED_)
