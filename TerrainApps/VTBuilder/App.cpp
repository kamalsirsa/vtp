//
// App.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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

#define HEAPBUSTER 0

#if HEAPBUSTER
#include "../HeapBuster/HeapBuster.h"
#endif

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

	// Fill list of layer type names
	if (vtLayer::LayerTypeNames.IsEmpty())
	{
		// These must correspond to the order of the LayerType enum!
		vtLayer::LayerTypeNames.Add(_("Raw"));
		vtLayer::LayerTypeNames.Add(_("Elevation"));
		vtLayer::LayerTypeNames.Add(_("Image"));
		vtLayer::LayerTypeNames.Add(_("Road"));
		vtLayer::LayerTypeNames.Add(_("Structure"));
		vtLayer::LayerTypeNames.Add(_("Water"));
		vtLayer::LayerTypeNames.Add(_("Vegetation"));
		vtLayer::LayerTypeNames.Add(_("Transit"));
		vtLayer::LayerTypeNames.Add(_("Utility"));
	}

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

	VTLOG(" Initializing GDAL.");
	g_GDALWrapper.RequestGDALFormats();

	VTLOG(" GDAL-supported formats:");
	GDALDriverManager *poDM = GetGDALDriverManager();
    for( int iDriver = 0; iDriver < poDM->GetDriverCount(); iDriver++ )
    {
		if ((iDriver % 13) == 0)
			VTLOG("\n  ");
        GDALDriver *poDriver = poDM->GetDriver( iDriver );
		const char *name = poDriver->GetDescription();
		VTLOG("%s ", name);
	}
	VTLOG("\n");

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

//	vtString fname = "E:/Earth Imagery/NASA BlueMarble/FullRes/MOD09A1.W.interpol.cyl.retouched.topo.3x21600x10800-N.bmp";
//	frame->ImportDataFromFile(LT_IMAGE, fname, true);

	frame->ZoomAll();

#if HEAPBUSTER
	// Pull in the heap buster
	g_HeapBusterDummy = -1;
#endif

	return TRUE;
}

int MyApp::OnExit()
{
	VTLOG("App Exit\n");
	return wxApp::OnExit();
}
