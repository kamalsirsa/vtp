//
// Name: EnviroApp.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// Define a new application type
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

	bool m_bShowStartupDialog;

	vtStringArray terrain_files;
	vtStringArray terrain_paths;
	vtStringArray terrain_names;

protected:
	void SetupLocale();

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
#endif

#ifndef ICON_NAME
#define ICON_NAME "enviro"
#endif

#define STRING_APPORG STRING_ORGNAME " " STRING_APPNAME

