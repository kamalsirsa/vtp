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
#include "vtui/Helper.h"
#include "gdal_priv.h"

IMPLEMENT_APP(MyApp)


bool MyApp::OnInit()
{
#if WIN32 && defined(_MSC_VER) && DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

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
#if WIN32
	VTLOG(" Running on: ");
	LogWindowsVersion();
#endif

	VTLOG("Testing ability to allocate a frame object.\n");
	wxFrame *frametest = new wxFrame(NULL, -1, _T("Title"));
	delete frametest;

	VTLOG(" Creating Main Frame Window,");
	wxString2 title = _T(APPNAME);
	VTLOG(" title '%s'\n", title.mb_str());
	MainFrame* frame = new MainFrame((wxFrame *) NULL, title,
							   wxPoint(50, 50), wxSize(900, 500));

	VTLOG(" Setting up the UI.\n");
	frame->SetupUI();

	VTLOG(" Showing the frame.\n");
	frame->Show(TRUE);

	SetTopWindow(frame);

	// Stuff for testing
//	wxString str("E:/Earth Imagery/NASA BlueMarble/MOD09A1.E.interpol.cyl.retouched.topo.3x00054x00027-N.bmp");
//	wxString str("E:/Data-USA/Elevation/crater_0513.bt");
/*	vtLayer *pLayer = frame->ImportImage(str);
	bool success = frame->AddLayerWithCheck(pLayer, true);
	frame->LoadLayer(str);
*/
//	frame->LoadProject("E:/Locations/Romania/giurgiu.vtb");
//	frame->ImportDataFromFile(LT_ELEVATION, "E:/Earth/NOAA Globe/g10g.hdr", false);
//	wxString str("E:/VTP User's Data/Mike Flaxman/catrct_nur.tif");
//	frame->LoadLayer(str);

//	wxString fname("E:/VTP User's Data/Hangzhou/Data/BuildingData/a-bldgs-18dec-subset1.vtst");
//	frame->LoadLayer(fname);
//	vtStructureLayer *pSL = frame->GetActiveStructureLayer();
//	vtStructure *str = pSL->GetAt(0);
//	str->Select(true);
//	pSL->EditBuildingProperties();
//	wxString fname("E:/Locations-USA/Hawai`i Island Data/DRG/O19154F8.TIF");
//	frame->ImportDataFromFile(LT_IMAGE, fname, true);
//	frame->LoadProject("E:/Locations-USA/Hawai`i Island Content/Honoka`a/latest_temp.vtb");

	frame->GetView()->ZoomAll();

	return TRUE;
}

int MyApp::OnExit()
{
	return wxApp::OnExit();
}
