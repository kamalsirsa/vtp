#if !defined(AFX_FENCEDLG_H)
#define AFX_FENCEDLG_H

#include "vtlib/core/Fence3d.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FenceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFenceDlg dialog

class CFenceDlg : public CDialog
{
// Construction
public:
	CFenceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFenceDlg)
	enum { IDD = IDD_FENCEDLG };
	CComboBox	m_cbType;
	int		m_iType;
	float	m_fHeight;
	float	m_fSpacing;
	int		m_iHeight;
	int		m_iSpacing;
	//}}AFX_DATA

	vtLinearParams m_linearparams;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFenceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SlidersToValues();
	void ValuesToSliders();

	// Generated message map functions
	//{{AFX_MSG(CFenceDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeFencetype();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeHeightedit();
	afx_msg void OnChangeSpacingedit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FENCEDLG_H__E7A8189B_F6EC_47B3_945C_8F796B611B81__INCLUDED_)
