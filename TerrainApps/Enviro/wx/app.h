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
};

// helpers
int EditTerrainParameters(wxWindow *parent, const char *filename);

