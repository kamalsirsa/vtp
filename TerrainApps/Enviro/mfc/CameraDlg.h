//
// Header for the Camera Dialog
//

#if !defined(AFX_CAMERADLG_H__4BE3704D_2A62_11D1_A5E4_00A0C9552733__INCLUDED_)
#define AFX_CAMERADLG_H__4BE3704D_2A62_11D1_A5E4_00A0C9552733__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// CameraDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCameraDlg dialog

class CCameraDlg : public CDialog
{
// Construction
public:
	void UpdateSliders();
	void SliderMoved(CSliderCtrl *pSlider, int nPos);
	void UpdateValues();
	void SetFlightspeed();
	void GetFlightspeed();
	CCameraDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCameraDlg)
	enum { IDD = IDD_CAMDIALOG };
	CSliderCtrl	m_FlightSpeed;
	CSliderCtrl	m_Yon;
	CSliderCtrl	m_ViewAngle;
	CSliderCtrl	m_Hither;
	float	m_fHither;
	float	m_fViewAngle;
	float	m_fYon;
	float	m_fFlightSpeed;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCameraDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
//	void StoreCurrentViewpointTo(int num);
//	void RecallStoredViewpoint(int num);

	// Generated message map functions
	//{{AFX_MSG(CCameraDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeHither();
	afx_msg void OnChangeYon();
	afx_msg void OnChangeVangle();
	afx_msg void OnChangeFspeed();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERADLG_H__4BE3704D_2A62_11D1_A5E4_00A0C9552733__INCLUDED_)
