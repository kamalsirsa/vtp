//
// PlantDlg.h
//

#if !defined(AFX_PLANTDLG_H__FA34E30B_B2BB_4DE0_90BE_16411383451C__INCLUDED_)
#define AFX_PLANTDLG_H__FA34E30B_B2BB_4DE0_90BE_16411383451C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlantDlg.h : header file
//

#include "resource.h"
#include "../PlantingOptions.h"

class vtSpeciesList3d;

/////////////////////////////////////////////////////////////////////////////
// CPlantDlg dialog

class CPlantDlg : public CDialog
{
// Construction
public:
	CPlantDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlantDlg)
	enum { IDD = IDD_PLANTDLG };
	CComboBox	m_cbSpecies;
	int		m_iSpecies;
	float	m_fHeight;
	int		m_iSize;
	float	m_fSpacing;
	//}}AFX_DATA

	void SetPlantList(vtSpeciesList3d *plants);
	void SetPlantOptions(PlantingOptions &opt);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlantDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	vtSpeciesList3d *m_pPlantList;
	PlantingOptions m_opt;
	void SizerToSize();
	void SizeToSizer();

	// Generated message map functions
	//{{AFX_MSG(CPlantDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSpecies();
	afx_msg void OnChangeSizeedit();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeSpacingedit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLANTDLG_H__FA34E30B_B2BB_4DE0_90BE_16411383451C__INCLUDED_)
