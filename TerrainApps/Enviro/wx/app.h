//
// Name: app.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// Define a new application type
class vtApp: public wxApp
{
public:
	vtApp();

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
    wxLocale m_locale; // locale we'll be using
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

#define WSTRING_APPORG _T(STRING_ORGNAME) _T(" ") _T(STRING_APPNAME)

