//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// Define a new application type
class vtApp: public wxApp
{
public:
    bool OnInit();
    int OnExit();
};

// helper
bool AskForTerrainName(wxWindow *pParent, wxString &str);
