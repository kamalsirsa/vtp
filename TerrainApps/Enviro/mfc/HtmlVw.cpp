// This is part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.
//
// mfcieVw.cpp : implementation of the CMfcieView class
//

#include "StdAfx.h"
#include "vtlib/vtlib.h"
#include "enviroapp.h"
#include "EnviroFrame.h"
#include "HtmlVw.h"

/////////////////////////////////////////////////////////////////////////////
// CMfcieView

IMPLEMENT_DYNCREATE(CMfcieView, CHtmlView)

BEGIN_MESSAGE_MAP(CMfcieView, CHtmlView)
	//{{AFX_MSG_MAP(CMfcieView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMfcieView construction/destruction

CMfcieView::CMfcieView()
{
}

CMfcieView::~CMfcieView()
{
}

BOOL CMfcieView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass = AfxRegisterWndClass(0);
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMfcieView drawing

void CMfcieView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CMfcieView message handlers

void CMfcieView::OnTitleChange(LPCTSTR lpszText)
{
	// this will change the main frame's title bar
	if (m_pDocument != NULL)
		m_pDocument->SetTitle(lpszText);
}

void CMfcieView::OnDocumentComplete(LPCTSTR lpszUrl)
{
	// make sure the main frame has the new URL.  This call also stops the animation
//	((CMainFrame*)GetParentFrame())->SetAddress(lpszUrl);
}

void CMfcieView::OnInitialUpdate()
{
	// Go to the home page initially
	CHtmlView::OnInitialUpdate();
//	GoHome();
	Navigate2(_T("http://vterrain.org/"));

//	GetApp()->m_pHTMLView = this;
}

void CMfcieView::OnBeforeNavigate2(LPCTSTR /*lpszURL*/, DWORD /*nFlags*/,
	LPCTSTR /*lpszTargetFrameName*/, CByteArray& /*baPostedData*/,
	LPCTSTR /*lpszHeaders*/, BOOL* /*pbCancel*/)
{
	// start the animation so that is plays while the new page is being loaded
//	((CMainFrame*)GetParentFrame())->StartAnimation();
}

