//
// Name: FeatureTableDlg3d.h
//
// Copyright (c) 2002-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __FeatureTableDlg3d_H__
#define __FeatureTableDlg3d_H__

#include "vtui/FeatureTableDlg.h"

class BuilderView;

class FeatureTableDlg3d: public FeatureTableDlg
{
public:
	// constructors and destructors
	FeatureTableDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	//void SetLayer(vtLayer *pLayer) { m_pLayer = pLayer; }
	//void SetView(BuilderView *pView) { m_pView = pView; }
	//vtLayer *GetLayer() { return m_pLayer; }

	virtual void OnModified();
	virtual void RefreshViz();

protected:
	//BuilderView *m_pView;
	//vtLayer *m_pLayer;
};

#endif	// __FeatureTableDlg3d_H__

