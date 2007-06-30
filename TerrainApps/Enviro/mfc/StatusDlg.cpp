// StatusDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "vtlib/vtlib.h"
#include "StatusDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog


CStatusDlg::CStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStatusDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CStatusDlg::AddString(const char *str)
{
	CString cstr;

	m_Edit.GetWindowText(cstr);
	cstr += str;
	cstr += "\r\n";
	m_Edit.SetWindowText(cstr);

	RECT Rect;
	GetClientRect(&Rect);
	InvalidateRect(&Rect);
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatusDlg)
	DDX_Control(pDX, IDC_EDIT1, m_Edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStatusDlg, CDialog)
	//{{AFX_MSG_MAP(CStatusDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg message handlers

void CStatusDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}
