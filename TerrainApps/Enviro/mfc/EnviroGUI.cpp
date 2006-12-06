//
// EnviroGUI.cpp
// GUI-specific functionality of the Enviro class
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"

#include "vtlib/vtlib.h"
#include "EnviroGUI.h"
#include "EnviroApp.h"
#include "EnviroFrame.h"

//
// This is a 'singleton', the only instance of the global application object
//
EnviroGUI g_App;

// helper
EnviroFrame *GetFrame()
{
	return (EnviroFrame *) AfxGetMainWnd();
}

EnviroGUI::EnviroGUI()
{
}

EnviroGUI::~EnviroGUI()
{
}

void EnviroGUI::ShowPopupMenu(const IPoint2 &pos)
{
//	GetFrame()->ShowPopupMenu(pos);
}

void EnviroGUI::SetTerrainToGUI(vtTerrain *pTerrain)
{
//	GetFrame()->SetTerrainToGUI(pTerrain);
}

void EnviroGUI::RefreshLayerView()
{
//	LayerDlg *dlg = GetFrame()->m_pLayerDlg;
//	dlg->RefreshTreeContents();
}

void EnviroGUI::ShowLayerView()
{
//	LayerDlg *dlg = GetFrame()->m_pLayerDlg;
//	dlg->Show(true);
}

void EnviroGUI::EarthPosUpdated()
{
	GetFrame()->EarthPosUpdated(m_EarthPos);
}

vtString EnviroGUI::GetPathFromGUI()
{
	return "";
}

vtString EnviroGUI::GetStringFromUser(const vtString &title, const vtString &msg)
{
	return "";
}


