//
// Name:     app.cpp
// Purpose:  The application class for a wxWindows application.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "../TerrainSceneWP.h"
#include "../Enviro.h"		// for g_App
#include "../Options.h"

#include "app.h"
#include "frame.h"
#include "StartupDlg.h"

static void Args(int argc, char **argv)
{
   return;
}

IMPLEMENT_APP(vtApp)

//
// Initialize the app object
//
bool vtApp::OnInit(void)
{
	Args(argc, argv);

	g_Options.Read("Enviro.ini");
	g_App.LoadTerrainDescriptions();

	//
	// Create and show the Startup Dialog
	//
	StartupDlg StartDlg(NULL, -1, "Enviro Startup", wxDefaultPosition);
	StartDlg.GetOptionsFrom(g_Options);
	StartDlg.CenterOnParent();
	int result = StartDlg.ShowModal();
	if (result == wxID_CANCEL)
		return FALSE;

	StartDlg.PutOptionsTo(g_Options);
	g_Options.Write();

	//
	// Create the main frame window
	//
	wxString title = "VTP Enviro ";
#if VTLIB_DSM
	title += "DSM";
#elif VTLIB_OSG
	title += "OSG";
#elif VTLIB_SGL
	title += "SGL";
#elif VTLIB_SSG
	title += "SSG";
#endif
	vtFrame *frame = new vtFrame(NULL, title,
		wxPoint(50, 50), wxSize(800, 600));

	vtGetScene()->Init();
	g_App.StartControlEngine(NULL);

	if (g_Options.m_bFullscreen)
		frame->SetFullScreen(true);

	return TRUE;
}

//
// ask the user to choose from a list of known terrain
//
bool AskForTerrainName(wxWindow *pParent, wxString &str)
{
    vtTerrain *pTerr;

	// count them
	int num = 0;
    for (pTerr = GetTerrainScene().m_pFirstTerrain; pTerr; pTerr=pTerr->GetNext())
		num++;

	// get their names
	wxString *choices = new wxString[num];
	num = 0;
	int first_idx = 0;
	for (pTerr = GetTerrainScene().m_pFirstTerrain; pTerr; pTerr=pTerr->GetNext())
	{
		choices[num] = (const char *)(pTerr->GetName());
		if (str == choices[num]) first_idx = num;
		num++;
	}

	if (!num)
	{
		wxMessageBox("No terrains found (datapath/Terrains/*.ini)");
		return false;
	}

	wxSingleChoiceDialog dlg(pParent, "Please choose a terrain",
		"Select Terrain", num, choices);
	dlg.SetSelection(first_idx);
	delete [] choices;
	if (dlg.ShowModal() == wxID_OK)
	{
		str = dlg.GetStringSelection();
		return true;
	}
	else
		return false;
}

