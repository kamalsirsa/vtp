#if !defined(AFX_STATUSDLG_H__4DFA42E4_1FA4_11D3_836D_00A0C9AC6391__INCLUDED_)
#define AFX_STATUSDLG_H__4DFA42E4_1FA4_11D3_836D_00A0C9AC6391__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// StatusDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog

class CStatusDlg : public CDialog
{
// Construction
public:
	CStatusDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStatusDlg)
	enum { IDD = IDD_STATDLG };
	CEdit	m_Edit;
	//}}AFX_DATA

	void AddString(const char *str);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStatusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStatusDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATUSDLG_H__4DFA42E4_1FA4_11D3_836D_00A0C9AC6391__INCLUDED_)
