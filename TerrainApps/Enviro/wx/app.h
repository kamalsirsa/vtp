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

	bool m_bShowStartupDialog;
};

// helper
bool AskForTerrainName(wxWindow *pParent, wxString &str);

