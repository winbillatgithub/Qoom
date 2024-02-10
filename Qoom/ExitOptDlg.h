#if !defined(AFX_EXITOPTDLG_H__A50168E7_22B1_4F61_B3B2_51271C53C2BC__INCLUDED_)
#define AFX_EXITOPTDLG_H__A50168E7_22B1_4F61_B3B2_51271C53C2BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExitOptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExitOptDlg dialog

class CExitOptDlg : public CDialog
{
// Construction
public:
	CExitOptDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExitOptDlg)
	enum { IDD = IDD_EXIT_OPTION };
	BOOL	m_chkMSN;
	BOOL	m_chkQQ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExitOptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExitOptDlg)
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXITOPTDLG_H__A50168E7_22B1_4F61_B3B2_51271C53C2BC__INCLUDED_)
