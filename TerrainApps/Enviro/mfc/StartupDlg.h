#if !defined(AFX_STARTUPDLG_H__EC885318_989A_11D3_83AB_00A0C9AC6391__INCLUDED_)
#define AFX_STARTUPDLG_H__EC885318_989A_11D3_83AB_00A0C9AC6391__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StartupDlg.h : header file
//

#include "resource.h"
#include "../Options.h"

/////////////////////////////////////////////////////////////////////////////
// CStartupDlg dialog

class CStartupDlg : public CDialog
{
// Construction
public:
	CStartupDlg(CWnd* pParent = NULL);   // standard constructor

	void Read(const char *szFilename);
	void Write();

// Dialog Data
	//{{AFX_DATA(CStartupDlg)
	enum { IDD = IDD_STARTUP };
	CComboBox	m_cbImage;
	CStatic	m_csImage;
	CButton	m_cbTSelect;
	CEdit	m_editTName;
	BOOL	m_bFullscreen;
	BOOL	m_bHtmlpane;
	BOOL	m_bFloatingToolbar;
	int		m_iLaunch;
	CString	m_strTName;
	CString	m_strDataPath;
	CString	m_strImage;
	float	m_fPlantSize;
	BOOL	m_bShadows;
	//}}AFX_DATA
	CString m_strFilename;

	void GetOptionsFrom(EnviroOptions &opt);
	void PutOptionsTo(EnviroOptions &opt);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStartupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateState();

	// Generated message map functions
	//{{AFX_MSG(CStartupDlg)
	afx_msg void OnOpenglInfo();
	afx_msg void OnTSelect();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeLaunch();
	afx_msg void OnEditProp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CStartupDlg g_StartDlg;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STARTUPDLG_H__EC885318_989A_11D3_83AB_00A0C9AC6391__INCLUDED_)
