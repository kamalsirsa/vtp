//
// Name:	 app.cpp
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
#include "../Options.h"
#include "EnviroGUI.h"		// for g_App, GetTerrainScene
#include "vtui/Helper.h"	// for LogWindowsVersion
#include "vtdata/vtLog.h"

#include "app.h"
#include "frame.h"
#include "StartupDlg.h"
#include "TParamsDlg.h"

#if INFRA
#include "Infra/InfraFrame.h"
#define FRAME_NAME InfraFrame
#else
#define FRAME_NAME vtFrame
#endif

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
		if (!strcmp(cstr, "-no_startup_dialog"))
			m_bShowStartupDialog = false;
		else if (!strncmp(str, "-terrain=", 9))
			m_bShowStartupDialog = false;

		// also let the core application check the command line
		g_App.StartupArgument(i, cstr);
	}
}

//
// Initialize the app object
//
bool vtApp::OnInit()
{
#if WIN32 && defined(_MSC_VER) && DEBUG
	// sometimes, MSVC seems to need to be told to show unfreed memory on exit
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	g_Options.Read(STRING_APPNAME ".ini");

	g_App.Startup();	// starts log

	VTLOG("Application framework: wxWindows v" wxVERSION_NUM_DOT_STRING "\n");
#if WIN32
	VTLOG(" Running on: ");
	LogWindowsVersion();
#endif

	Args(argc, argv);

	// Look for all terrains on all data paths
	RefreshTerrainList();

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
		StartupDlg StartDlg(NULL, -1, _T(STRING_APPNAME) _T(" Startup"), wxDefaultPosition);
		StartDlg.GetOptionsFrom(g_Options);
		StartDlg.CenterOnParent();
		int result = StartDlg.ShowModal();
		if (result == wxID_CANCEL)
			return FALSE;

		VTLOG("Writing options to " STRING_APPNAME ".ini\n");
		StartDlg.PutOptionsTo(g_Options);
		g_Options.Write();
	}

	// Now we can create vtTerrain objects for each terrain
	g_App.LoadTerrainDescriptions();

	//
	// Create the main frame window
	//
	wxString title = WSTRING_APPORG;
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
	vtFrame *frame = new FRAME_NAME(NULL, title,
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
// Look for all terrains on all data paths
//
void vtApp::RefreshTerrainList()
{
	vtStringArray &paths = g_Options.m_DataPaths;

	terrain_files.clear();
	terrain_paths.clear();
	terrain_names.clear();

	for (unsigned int i = 0; i < paths.size(); i++)
	{
		vtString directory = paths[i] + "Terrains";
		for (dir_iter it((const char *)directory); it != dir_iter(); ++it)
		{
			if (it.is_hidden() || it.is_directory())
				continue;

			std::string name1 = it.filename();
			vtString name = name1.c_str();

			// only look for ".ini" files
			if (name.GetLength() < 5 || name.Right(4).CompareNoCase(".ini"))
				continue;

			TParams params;
			vtString path = directory + "/" + name;
			if (params.LoadFromFile(path))
			{
				terrain_files.push_back(name);
				terrain_paths.push_back(path);
				terrain_names.push_back(params.m_strName);
			}
		}
	}
}

//
// ask the user to choose from a list of known terrain
//
bool vtApp::AskForTerrainName(wxWindow *pParent, wxString &strTerrainName)
{
	// convert all the terrain names to wxStrings
	int num = 0, first_idx = 0;
	std::vector<wxString> choices;

	for (unsigned int i = 0; i < terrain_files.size(); i++)
	{
		wxString2 wstr;
		wstr.from_utf8(terrain_names[i]);
		choices.push_back(wstr);
		if (wstr.Cmp(strTerrainName) == 0)
			first_idx = num;
		num++;
	}

	if (!num)
	{
		wxMessageBox(_T("No terrains found (datapath/Terrains/*.ini)"));
		return false;
	}

	wxSingleChoiceDialog dlg(pParent, _T("Please choose a terrain"),
		_T("Select Terrain"), num, &(choices.front()));
	dlg.SetSelection(first_idx);

	if (dlg.ShowModal() == wxID_OK)
	{
		strTerrainName = dlg.GetStringSelection();
		return true;
	}
	else
		return false;
}

vtString vtApp::GetIniFileForTerrain(const vtString &name)
{
	for (unsigned int i = 0; i < terrain_files.size(); i++)
	{
		if (name == terrain_names[i])
			return terrain_paths[i];
	}
	return vtString("");
}

int EditTerrainParameters(wxWindow *parent, const char *filename)
{
	TParamsDlg dlg(parent, -1, _T("Terrain Creation Parameters"), wxDefaultPosition);
	dlg.SetDataPaths(g_Options.m_DataPaths);

	TParams Params;
	if (Params.LoadFromFile(filename))
		dlg.SetParams(Params);

	dlg.CenterOnParent();
	int result = dlg.ShowModal();
	if (result == wxID_OK)
	{
		dlg.GetParams(Params);
		if (!Params.SaveToFile(filename))
		{
			wxString str;
			str.Printf(_T("Couldn't save to file %hs.\n")
				_T("Please make sure the file is not read-only."), filename);
			wxMessageBox(str);
			result = wxID_CANCEL;
		}
	}
	return result;
}

