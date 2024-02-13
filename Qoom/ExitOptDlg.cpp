// ExitOptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "qoom.h"
#include "ExitOptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExitOptDlg dialog


CExitOptDlg::CExitOptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExitOptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExitOptDlg)
	m_chkMSN = FALSE;
	m_chkQQ = FALSE;
	//}}AFX_DATA_INIT
}


void CExitOptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExitOptDlg)
	DDX_Check(pDX, IDC_CHECK_MSN, m_chkMSN);
	DDX_Check(pDX, IDC_CHECK_QQ, m_chkQQ);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExitOptDlg, CDialog)
	//{{AFX_MSG_MAP(CExitOptDlg)
	ON_BN_CLICKED(ID_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(ID_SELECT_NONE, OnSelectNone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExitOptDlg message handlers

void CExitOptDlg::OnSelectAll() 
{
	// TODO: Add your control notification handler code here
	m_chkMSN = TRUE;
	m_chkQQ = TRUE;
	UpdateData(FALSE);
}

void CExitOptDlg::OnSelectNone() 
{
	// TODO: Add your control notification handler code here
	m_chkMSN = FALSE;
	m_chkQQ = FALSE;
	UpdateData(FALSE);
}


void CExitOptDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);
	CDialog::OnOK();
}

BOOL CExitOptDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
