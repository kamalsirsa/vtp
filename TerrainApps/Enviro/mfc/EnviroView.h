//
// EnviroView.h : header file
//

#ifndef ENVIROVIEWH
#define ENVIROVIEWH

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "resource.h"

class CSimpleDoc;

/////////////////////////////////////////////////////////////////////////////
// EnviroView view

class EnviroView : public CView
{
protected:
	EnviroView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(EnviroView)

// Attributes
public:
	CSimpleDoc* GetDocument();
	HGLRC m_hGLContext;

	bool m_bTimeOn;
	bool m_bCulleveryframe;

	// navigation
	bool m_bDriveRoads;
	bool m_bTopDown;
	bool m_bMaintainHeight;

//	FILE *fp;
	bool m_bRunning;

// Operations
public:
	void PaintOnIdle();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EnviroView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL CreateViewGLContext(HDC hDC);
	BOOL SetWindowPixelFormat(HDC hDC);
	virtual ~EnviroView();

protected:
	int m_GLPixelIndex;

	// Generated message map functions
protected:
	//{{AFX_MSG(EnviroView)
	afx_msg void OnEnviroTimeon();
	afx_msg void OnUpdateEnviroTimeon(CCmdUI* pCmdUI);
	afx_msg void OnEnviroLodterrain();
	afx_msg void OnUpdateEnviroLodterrain(CCmdUI* pCmdUI);
	afx_msg void OnEnviroCulleveryframe();
	afx_msg void OnUpdateEnviroCulleveryframe(CCmdUI* pCmdUI);
	afx_msg void OnEnviroCullonce();
	afx_msg void OnEnviroIncrease();
	afx_msg void OnEnviroDecrease();
	afx_msg void OnEnviroDriveroads();
	afx_msg void OnUpdateEnviroDriveroads(CCmdUI* pCmdUI);
	afx_msg void OnEnviroShowroads();
	afx_msg void OnUpdateEnviroShowroads(CCmdUI* pCmdUI);
	afx_msg void OnEnviroShowocean();
	afx_msg void OnUpdateEnviroShowocean(CCmdUI* pCmdUI);
	afx_msg void OnEnviroShowsky();
	afx_msg void OnUpdateEnviroShowsky(CCmdUI* pCmdUI);
	afx_msg void OnEnviroShowfog();
	afx_msg void OnUpdateEnviroShowfog(CCmdUI* pCmdUI);
	afx_msg void OnEnviroShowtrees();
	afx_msg void OnUpdateEnviroShowtrees(CCmdUI* pCmdUI);
	afx_msg void OnEnviroTopdownview();
	afx_msg void OnUpdateEnviroTopdownview(CCmdUI* pCmdUI);
	afx_msg void OnShowdetailtexture();
	afx_msg void OnUpdateShowdetailtexture(CCmdUI* pCmdUI);
	afx_msg void OnViewFlyfaster();
	afx_msg void OnViewFlyslower();
	afx_msg void OnViewMaintainheight();
	afx_msg void OnUpdateViewMaintainheight(CCmdUI* pCmdUI);
	afx_msg void OnToolsNavigateOriginalnavengin();
	afx_msg void OnUpdateToolsNavigateOriginalnavengin(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEnviroCullonce(CCmdUI* pCmdUI);
	afx_msg void OnToolsNavigateQuakenavengine();
	afx_msg void OnUpdateToolsNavigateQuakenavengine(CCmdUI* pCmdUI);
	afx_msg void OnToolsNavigateVelocitygravity();
	afx_msg void OnUpdateToolsNavigateVelocitygravity(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFlyfaster(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFlyslower(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in EnviroView.cpp
inline CSimpleDoc* EnviroView::GetDocument()
   { return (CSimpleDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(ENVIROVIEWH)
