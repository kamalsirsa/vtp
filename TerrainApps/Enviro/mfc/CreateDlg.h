#if !defined(AFX_CREATEDLG_H__1C370646_3B7A_11D2_8294_00A0C9820814__INCLUDED_)
#define AFX_CREATEDLG_H__1C370646_3B7A_11D2_8294_00A0C9820814__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// CreateDlg.h : header files
//

#include <afxdtctl.h>
#include "resource.h"
#include "vtlib/core/TParams.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateDlg dialog

class CCreateDlg : public CDialog
{
// Construction
public:
	CCreateDlg(CWnd* pParent = NULL);   // standard constructor
	~CCreateDlg();

// Dialog Data
	//{{AFX_DATA(CCreateDlg)
	enum { IDD = IDD_TPARAMS };
	CString	m_strName;
	CComboBox	m_cbLodMethod;
	CEdit	m_cePreLightFactor;
	CDateTimeCtrl	m_dtTime;
	CComboBox	m_cbBuildingFile;
	CComboBox	m_cbRoadFile;
	CComboBox	m_cbTreeFile;
	CButton	m_TimeOfDay;
	CButton	m_PreLight;
	CButton	m_TriStrips;
	CComboBox	m_cbTextureFileSingle;
	CComboBox	m_cbLODFilename;
	CString	m_strFilename;
	float	m_fVerticalExag;
	BOOL	m_bTimeOn;
	BOOL	m_bSky;
	BOOL	m_bFog;
	int		m_iTexture;
	UINT	m_iTilesize;
	CString	m_strTextureSingle;
	CString	m_strTextureBase;
	CString	m_strTexture4x4;
	BOOL	m_bMipmap;
	BOOL	m_b16bit;
	BOOL	m_bRoads;
	CString	m_strRoadFile;
	BOOL	m_bTexRoads;
	BOOL	m_bPlants;
	CString	m_strTreeFile;
	UINT	m_iVegDistance;
	float	m_fFogDistance;
	BOOL	m_bOceanPlane;
	int		m_iMinHeight;
	CString	m_strBuildingFile;
	BOOL	m_bVehicles;
	float	m_fVehicleSize;
	BOOL	m_bTriStrips;
	BOOL	m_bDetailTexture;
	BOOL	m_bPreLight;
	BOOL	m_bDirt;
	BOOL	m_bHwy;
	BOOL	m_bPaved;
	CTime	m_tTime;
	float	m_fRoadDistance;
	float	m_fRoadHeight;
	float	m_fNavSpeed;
	float	m_fPreLightFactor;
	BOOL	m_bRoadCulture;
	int		m_iLodMethod;
	float	m_fVehicleSpeed;
	BOOL	m_bPreLit;
	//}}AFX_DATA
	int		m_iTerrainSize;		// size of input BT
	int		m_iTerrainDepth;	// in bytes/point
	int		m_iTextureSize;		// size of input BMP

// Methods
	void SetParams(TParams &Params);
	void GetParams(TParams &Params);

	void DetermineTerrainSizeFromBT();
	void DetermineSizeFromBMP();
	void UpdateTiledTextureFilename();
	void UpdateTimeEnable();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	time_t m_iTimeDiff;

	// Generated message map functions
	//{{AFX_MSG(CCreateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeTilesize();
	afx_msg void OnChangeTfilebase();
	afx_msg void OnChangeMem();
	afx_msg void OnEditchangeFilename();
	afx_msg void OnSelchangeFilename();
	afx_msg void OnSelchangeTfilesingle();
	afx_msg void OnPrelight();
	afx_msg void OnSelchangeLodmethod();
	afx_msg void OnEnviroShowtrees();
	afx_msg void OnPrelit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeFilename2();
	afx_msg void OnEnChangeTfilebase2();
	afx_msg void OnEnChangeTname();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREATEDLG_H__1C370646_3B7A_11D2_8294_00A0C9820814__INCLUDED_)
