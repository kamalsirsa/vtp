//
// App.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "App.h"
#include "Frame.h"
#include "BuilderView.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	MainFrame* frame = new MainFrame((wxFrame *) NULL, "VTBuilder",
							   wxPoint(50, 50), wxSize(700, 500));

	// Show the frame
	frame->Show(TRUE);

	SetTopWindow(frame);

	frame->GetView()->ZoomAll();

	return TRUE;
}

