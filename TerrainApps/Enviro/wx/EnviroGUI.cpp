//
// EnviroGUI.cpp
// GUI-specific functionality of the Enviro class
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "EnviroGUI.h"
#include "app.h"
#include "frame.h"
#include "LayerDlg.h"

DECLARE_APP(vtApp);


//
// This is a 'singleton', the only instance of the global application object
//
EnviroGUI g_App;

EnviroGUI::EnviroGUI()
{
}

EnviroGUI::~EnviroGUI()
{
}

void EnviroGUI::ShowPopupMenu(const IPoint2 &pos)
{
	vtFrame *pFrame = (vtFrame *) (wxGetApp().GetTopWindow());
	pFrame->ShowPopupMenu(pos);
}

void EnviroGUI::SetTerrainToGUI(vtTerrain *pTerrain)
{
	vtFrame *pFrame = (vtFrame *) (wxGetApp().GetTopWindow());
	pFrame->SetTerrainToGUI(pTerrain);
}

void EnviroGUI::RefreshLayerView()
{
	vtFrame *pFrame = (vtFrame *) (wxGetApp().GetTopWindow());
	LayerDlg *dlg = pFrame->m_pLayerDlg;
	dlg->RefreshTreeContents();
}

void EnviroGUI::ShowLayerView()
{
	vtFrame *pFrame = (vtFrame *) (wxGetApp().GetTopWindow());
	LayerDlg *dlg = pFrame->m_pLayerDlg;
	dlg->Show(true);
}

