#if !defined(AFX_CHOOSEDLG_H)
#define AFX_CHOOSEDLG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseDlg dialog

class CChooseDlg : public CDialog
{
// Construction
public:
	CChooseDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseDlg)
	enum { IDD = IDD_TERRAIN };
	CButton	m_cbOK;
	CListBox	m_lbList;
	//}}AFX_DATA
	CString m_strTName;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateEdit();

	// Generated message map functions
	//{{AFX_MSG(CChooseDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeTlist();
	afx_msg void OnDblclkTlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEDLG_H)
