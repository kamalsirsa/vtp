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
#include "gdal_priv.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	g_Log._StartLog("debug.txt");
	VTLOG(APPNAME "\nBuild:");
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

	// prepare to call GDAL format functions
	// only needs to be done once at startup
	GDALAllRegister();

	// Stuff for testing
//	wxString str("E:/Earth Imagery/NASA BlueMarble/MOD09A1.E.interpol.cyl.retouched.topo.3x00054x00027-N.bmp");
//	wxString str("E:/Data-USA/Elevation/crater_0513.bt");
//	vtLayer *pLayer = frame->ImportImage(str);
//	bool success = frame->AddLayerWithCheck(pLayer, true);
//	frame->LoadLayer(str);

	return TRUE;
}

int MyApp::OnExit()
{
	// only needs to be done once at exit
	GDALDestroyDriverManager();

	return wxApp::OnExit();
}
