//
// Name:     app.cpp
// Purpose:  The application class for our wxWindows application.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "../Enviro.h"		// for g_App, GetTerrainScene
#include "../Options.h"
#include "vtdata/vtLog.h"

#include "app.h"
#include "frame.h"
#include "StartupDlg.h"

IMPLEMENT_APP(vtApp)


vtApp::vtApp()
{
	m_bShowStartupDialog = true;
}

void vtApp::Args(int argc, wxChar **argv)
{
	for (int i = 0; i < argc; i++)
	{
		wxString2 str = argv[i];
		const char *cstr = str.mb_str();
		g_App.StartupArgument(i, cstr);
		if (!strcmp(cstr, "-no_startup_dialog"))
			m_bShowStartupDialog = false;
		else if (!strcmp(cstr, "-fullscreen"))
			g_Options.m_bFullscreen = true;
		else if(!strncmp(cstr, "-terrain=", 9))
		{
			m_bShowStartupDialog = false;
			g_Options.m_strInitTerrain = cstr+9;
		}
	}
}

//
// Initialize the app object
//
bool vtApp::OnInit()
{
	g_Options.Read("Enviro.ini");

	g_App.Startup();	// starts log

	Args(argc, argv);

	g_App.LoadTerrainDescriptions();

/*	class AA { public: virtual void func() {} };
	class BB : public AA {};
	VTLOG("Testing the ability to use dynamic_cast to downcast...\n");
	BB *b = new BB;
	AA *a = (AA *) b;
	BB *result1 = dynamic_cast<BB *>(a);
	VTLOG("  successful (%lx)\n", result1); */

	//
	// Create and show the Startup Dialog
	//
	if (m_bShowStartupDialog)
	{
		VTLOG("Opening the Startup dialog.\n");
		StartupDlg StartDlg(NULL, -1, _T("Enviro Startup"), wxDefaultPosition);
		StartDlg.GetOptionsFrom(g_Options);
		StartDlg.CenterOnParent();
		int result = StartDlg.ShowModal();
		if (result == wxID_CANCEL)
			return FALSE;

		VTLOG("Writing options to Enviro.ini\n");
		StartDlg.PutOptionsTo(g_Options);
		g_Options.Write();
	}

	//
	// Create the main frame window
	//
	wxString title = _T("VTP Enviro");
#if VTLIB_PSM
	title += _T(" PSM");
#elif VTLIB_OSG
	title += _T(" OSG");
#elif VTLIB_SGL
	title += _T(" SGL");
#elif VTLIB_SSG
	title += _T(" SSG");
#endif
	VTLOG("Creating the frame window.\n");
	vtFrame *frame = new vtFrame(NULL, title,
		wxPoint(50, 50), wxSize(800, 600));

	vtGetScene()->Init();

	g_App.StartControlEngine();

	if (g_Options.m_bFullscreen)
		frame->SetFullScreen(true);

	return true;
}

int vtApp::OnExit()
{
	VTLOG("App Exit\n");
#ifdef VTLIB_PSM
	PSWorld3D::Get()->Stop();
	PSGetScene()->SetWindow(NULL);
#endif
	g_App.Shutdown();
	vtGetScene()->Shutdown();

	return wxApp::OnExit();
}

//
// ask the user to choose from a list of known terrain
//
bool AskForTerrainName(wxWindow *pParent, wxString &str)
{
    vtTerrain *pTerr, *pFirst = GetTerrainScene()->GetFirstTerrain();

	// count them
	int num = 0;
    for (pTerr = pFirst; pTerr; pTerr=pTerr->GetNext())
		num++;

	// get their names
	wxString *choices = new wxString[num];
	num = 0;
	int first_idx = 0;
	for (pTerr = pFirst; pTerr; pTerr=pTerr->GetNext())
	{
#if SUPPORT_WSTRING && UNICODE
		wstring2 ws;
		ws.from_utf8(pTerr->GetName());
		choices[num] = wxString2(ws.c_str());
#else
		choices[num] = wxString2(pTerr->GetName());
#endif
		if (str == choices[num]) first_idx = num;
		num++;
	}

	if (!num)
	{
		wxMessageBox(_T("No terrains found (datapath/Terrains/*.ini)"));
		return false;
	}

	wxSingleChoiceDialog dlg(pParent, _T("Please choose a terrain"),
		_T("Select Terrain"), num, choices);
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
