//
// Name:	 app.cpp
// Purpose:  The application class for our wxWindows application.
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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
#define LoadAppCatalog(locale)
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
		else if (!strncmp(cstr, "-terrain=", 9))
			m_bShowStartupDialog = false;
		else if (!strncmp(cstr, "-locale=", 8))
			m_locale_name = cstr+8;

		// also let the core application check the command line
		g_App.StartupArgument(i, cstr);
	}
}


WX_DECLARE_EXPORTED_STRING_HASH_MAP(wxString, wxMessagesHash);
class MyMsgCatalogFile
{
public:
    // fills the hash with string-translation pairs
    void TestHash(bool convertEncoding)
	{
		wxCSConv *csConv = new wxCSConv(_T("iso-8859-1"));

		wxString key = wxString("Key1", *wxConvCurrent);
		m_messages[key] = _T("Value1");

		const wxChar *result = GetString(_T("Key1"));
		VTLOG(result == NULL? "test1: bad\n" : "test1: good\n");

		wxString key2 = wxString("Key2", *csConv);
		m_messages[key2] = _T("Value2");

		const wxChar *result2 = GetString(_T("Key2"));
		VTLOG(result2 == NULL? "test2: bad\n" : "test2: good\n");
	}
	const wxChar *GetString(const wxChar *sz) const
	{
		wxMessagesHash::const_iterator i = m_messages.find(sz);
		if ( i != m_messages.end() )
		{
			return i->second.c_str();
		}
		else
			return NULL;
	}
	wxMessagesHash m_messages;
};

void TestLocale()
{
	MyMsgCatalogFile catfile;
	catfile.TestHash(true);
}

void vtApp::SetupLocale()
{
	wxLog::SetVerbose(true);
//	wxLog::AddTraceMask(_T("i18n"));

	TestLocale();

	// Locale stuff
	VTLOG("\n");
	int lang = wxLANGUAGE_DEFAULT;
	if (m_locale_name != "")
	{
		VTLOG("Initializing locale to language: %s\n", (const char *) m_locale_name);
		if (m_locale_name == "swedish")
			lang = wxLANGUAGE_SWEDISH;
		else
		{
			VTLOG(" Unknown, falling back on default language.\n");
			lang = wxLANGUAGE_DEFAULT;
		}
		if (lang != wxLANGUAGE_DEFAULT)
			m_locale.Init(lang);
	}
	if (lang == wxLANGUAGE_DEFAULT)
	{
		// auto detect the language
		lang = m_locale.GetSystemLanguage();
		VTLOG("Initializing locale to default language: %d\n", lang);
		m_locale.Init(wxLANGUAGE_DEFAULT);
	}

	const wxLanguageInfo *info = m_locale.GetLanguageInfo(lang);
	if (info)
	{
		VTLOG("Locale info: Canonical name '%s', Description: '%s'\n",
			info->CanonicalName.mb_str(), info->Description.mb_str());
	}
	else
		VTLOG("Locale info: unavailable.\n");

	VTLOG("Attempting to load the 'Enviro.mo' catalog for the current locale.\n");
	bool success = m_locale.AddCatalog(wxT("Enviro"));
	if (success)
		VTLOG(" succeeded.\n");
	else
		VTLOG(" not found.\n");
	VTLOG("\n");

	// Load any other catalogs which may be specific to this application.
	LoadAppCatalog(m_locale);

	// Test it
//	wxString test = _("&File");

	wxLog::SetVerbose(false);
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

	VTLOG("Specific application name: %s\n", STRING_APPNAME);
	VTLOG("Application framework: wxWindows v" wxVERSION_NUM_DOT_STRING "\n");
#if WIN32
	VTLOG(" Running on: ");
	LogWindowsVersion();
#endif
	VTLOG("\n");

	Args(argc, argv);

	SetupLocale();

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
		wxString appname = _T(STRING_APPNAME);
		appname += _(" Startup");
		StartupDlg StartDlg(NULL, -1, appname, wxDefaultPosition);
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

	bool go = true;
	while (go)
		go = ProcessIdle();

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

			// only look for terrain parameters files
			vtString ext = GetExtension(name, false);
			if (ext != ".ini" && ext != ".xml")
				continue;

			TParams params;
			vtString path = directory + "/" + name;
			if (params.LoadFrom(path))
			{
				terrain_files.push_back(name);
				terrain_paths.push_back(path);
				terrain_names.push_back(params.GetValueString(STR_NAME));
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
		wxMessageBox(_("No terrains found (datapath/Terrains/*.xml)"));
		return false;
	}

	wxSingleChoiceDialog dlg(pParent, _("Please choose a terrain"),
		_("Select Terrain"), num, &(choices.front()));
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
	vtString fname = filename;

	TParamsDlg dlg(parent, -1, _("Terrain Creation Parameters"), wxDefaultPosition);
	dlg.SetDataPaths(g_Options.m_DataPaths);

	TParams Params;
	if (!Params.LoadFrom(fname))
	{
		wxMessageBox(_("Couldn't load from that file."));
		return wxID_CANCEL;
	}
	dlg.SetParams(Params);
	dlg.CenterOnParent();
	int result = dlg.ShowModal();
	if (result == wxID_OK)
	{
		dlg.GetParams(Params);

		vtString ext = GetExtension(fname, false);
		if (ext.CompareNoCase(".ini") == 0)
		{
			wxString2 str = _("Upgrading the .ini to a .xml file.\n");
			str += _("Deleting old file: ");
			str += fname;
			wxMessageBox(str);

			// Try to get rid of it.  Hope they aren't on read-only FS.
			vtDeleteFile(fname);

			fname = fname.Left(fname.GetLength()-4) + ".xml";
		}

		if (!Params.WriteToXML(fname, STR_TPARAMS_FORMAT_NAME))
		{
			wxString str;
			str.Printf(_("Couldn't save to file %hs.\n"), (const char *)fname);
			str += _("Please make sure the file is not read-only.");
			wxMessageBox(str);
			result = wxID_CANCEL;
		}
	}
	return result;
}

