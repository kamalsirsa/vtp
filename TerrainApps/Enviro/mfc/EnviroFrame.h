/*
 * Name: EnviroFrame
 * Parent: CFrameWnd
 *
 */

#ifndef _ENVIROFRAME_H_INCLUDED_
#define _ENVIROFRAME_H_INCLUDED_

#include "PlantDlg.h"
#include "FenceDlg.h"
#include "../Enviro.h"

class EnviroFrame : public CFrameWnd
{
protected: // create from serialization only
	EnviroFrame();
	DECLARE_DYNCREATE(EnviroFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
	CStatusBar	m_wndStatusBar;
	CToolBar	m_wndToolBar;
	CPlantDlg	m_wndPlants;
	CFenceDlg	m_wndFences;

public:
	void SetMode(MouseMode mode);
	void UpdateStatusBar();
	void ShowPopupMenu(const IPoint2 &pos);
	void SetTerrainToGUI(class vtTerrain *pTerrain);
	void EarthPosUpdated(const DPoint3 &pos);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EnviroFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~EnviroFrame();

protected:  // control bar embedded members

public:

// Generated message map functions
protected:
	//{{AFX_MSG(EnviroFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFlytoSpace();
	afx_msg void OnFlytoTerrain();
	afx_msg void OnUpdateFlytoSpace(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFlytoTerrain(CCmdUI* pCmdUI);
	afx_msg void OnSpaceShowtime();
	afx_msg void OnUpdateSpaceShowtime(CCmdUI* pCmdUI);
	afx_msg void OnEarthFlatunfold();
	afx_msg void OnUpdateEarthFlatunfold(CCmdUI* pCmdUI);
	afx_msg void OnEarthAddpointdata();
	afx_msg void OnToolsTrees();
	afx_msg void OnUpdateToolsTrees(CCmdUI* pCmdUI);
	afx_msg void OnToolsFences();
	afx_msg void OnUpdateToolsFences(CCmdUI* pCmdUI);
	afx_msg void OnToolsNavigate();
	afx_msg void OnUpdateToolsNavigate(CCmdUI* pCmdUI);
	afx_msg void OnToolsMove();
	afx_msg void OnUpdateToolsMove(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEarthAddpointdata(CCmdUI* pCmdUI);
	afx_msg void OnToolsSelect();
	afx_msg void OnUpdateToolsSelect(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif	// _ENVIROFRAME_H_INCLUDED_

