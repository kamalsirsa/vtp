//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// Define a new application type
class vtApp: public wxApp
{
public:
    bool OnInit();
    int OnExit();
};

// helper
bool AskForTerrainName(wxWindow *pParent, wxString &str);
