//
// Name:		FeatureTableDlg3d.cpp
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "FeatureTableDlg3d.h"
//include "BuilderView.h"


FeatureTableDlg3d::FeatureTableDlg3d( wxWindow *parent, wxWindowID id,
	const wxString &title, const wxPoint &position, const wxSize& size, long style ) :
	FeatureTableDlg(parent, id, title, position, size, style)
{
	//m_pLayer = NULL;
	//m_pView = NULL;
}

void FeatureTableDlg3d::OnModified()
{
	//m_pLayer->SetModified(true);
}
void FeatureTableDlg3d::RefreshViz()
{
	//m_pView->Refresh();
}

