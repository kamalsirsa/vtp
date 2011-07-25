//
// Name: EnviroApp.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

class EnviroFrame;
class vtTerrain;

/** The wxWidgets application for Enviro. */
class EnviroApp: public wxApp
{
public:
	EnviroApp();

	bool OnInit();
	int OnExit();
	void Args(int argc, wxChar **argv);

	void RefreshTerrainList();
	bool AskForTerrainName(wxWindow *pParent, wxString &strTerrainName);
	vtString GetIniFileForTerrain(const vtString &name);
	wxString GetLanguageCode() { return m_locale.GetCanonicalName(); }
	wxString MakeFrameTitle(vtTerrain *terrain = NULL);

	bool m_bShowStartupDialog;

	vtStringArray terrain_files;
	vtStringArray terrain_paths;
	vtStringArray terrain_names;

protected:
	void StartLog();
	void LoadOptions();
	void SetupLocale();
	EnviroFrame *CreateMainFrame();

	wxLocale m_locale; // locale we'll be using
	vtString m_locale_name;
};

// helpers
int EditTerrainParameters(wxWindow *parent, const char *filename);

#ifndef STRING_ORGNAME
#define STRING_ORGNAME "VTP"
#endif

#ifndef STRING_APPNAME
#define STRING_APPNAME "Enviro"
#define ENVIRO_NATIVE
#else
void SetupCustomOptions();
#endif

#define STRING_APPORG STRING_ORGNAME " " STRING_APPNAME

