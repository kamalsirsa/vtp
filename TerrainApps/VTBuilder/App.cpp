//
// App.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
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
#include "vtdata/vtLog.h"

#ifdef ENVIRON
  #define APPNAME "EnvironBuilder"
#else
  #define APPNAME "VTBuilder"
#endif


IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	g_Log._StartLog("debug.txt");
	VTLOG("%s\nBuild:", APPNAME);
#if DEBUG
	VTLOG(" Debug");
#else
	VTLOG(" Release");
#endif
#if UNICODE
	VTLOG(" Unicode");
#endif
	VTLOG("\n");

	MainFrame* frame = new MainFrame((wxFrame *) NULL, _T(APPNAME),
							   wxPoint(50, 50), wxSize(900, 500));

	VTLOG(" Setting up the UI.\n");
	frame->SetupUI();

	VTLOG(" Showing the frame.\n");
	frame->Show(TRUE);

	SetTopWindow(frame);

	frame->GetView()->ZoomAll();

	return TRUE;
}

