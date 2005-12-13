//
// App.cpp - Main application class for VTBuilder
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Frame.h"
#include "BuilderView.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"
#include "gdal_priv.h"
#include "App.h"

#define HEAPBUSTER 0

#if HEAPBUSTER
#include "../HeapBuster/HeapBuster.h"
#endif

IMPLEMENT_APP(BuilderApp)


void BuilderApp::Args(int argc, wxChar **argv)
{
	for (int i = 0; i < argc; i++)
	{
		wxString2 str = argv[i];
		const char *cstr = str.mb_str();
		if (!strncmp(cstr, "-locale=", 8))
			m_locale_name = cstr+8;
	}
}


class LogCatcher : public wxLog
{
	void DoLogString(const wxChar *szString, time_t t)
	{
		VTLOG(" wxLog: ");
		VTLOG(szString);
		VTLOG("\n");
	}
};

bool BuilderApp::OnInit()
{
#if WIN32 && defined(_MSC_VER) && DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	VTSTARTLOG("debug.txt");
	VTLOG1(APPNAME);
	VTLOG1("\nBuild:");
#if VTDEBUG
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

	// Redirect the wxWindows log messages to our own logging stream
	wxLog *logger = new LogCatcher();
	wxLog::SetActiveTarget(logger);

	Args(argc, argv);

	SetupLocale();

	VTLOG(" Initializing GDAL.");
	g_GDALWrapper.GuessDataPaths();
	g_GDALWrapper.RequestGDALFormats();

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
	wxString2 title = APPNAME;
	VTLOG(" title '%s'\n", title.mb_str());
	MainFrame* frame = new MainFrame((wxFrame *) NULL, title,
							   wxPoint(50, 50), wxSize(900, 500));

	VTLOG(" Setting up the UI.\n");
	frame->SetupUI();

	VTLOG(" Showing the frame.\n");
	frame->Show(TRUE);

	SetTopWindow(frame);

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
//	wxString2 str("E:/Data-USA/Terrains/Hawai`i.xml");
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

//	vtString fname = "E:/Locations-Hawai'i/Hawai`i Island Data/SDTS-DLG/waipahu_HI/transportation/852867.RR.sdts.tar.gz";
//	frame->ImportDataFromArchive(LT_ROAD, fname, true);

#if HEAPBUSTER
	// Pull in the heap buster
	g_HeapBusterDummy = -1;
#endif

	return TRUE;
}

int BuilderApp::OnExit()
{
	VTLOG("App Exit\n");
	return wxApp::OnExit();
}

void BuilderApp::SetupLocale()
{
	wxLog::SetVerbose(true);

	// Locale stuff
	int lang = wxLANGUAGE_DEFAULT;
	int default_lang = m_locale.GetSystemLanguage();

	const wxLanguageInfo *info = wxLocale::GetLanguageInfo(default_lang);
	const char *langdesc = info->Description.mb_str();
	VTLOG("Default language: %d (%s)\n", default_lang, langdesc);

	// After wx2.4.2, wxWidgets looks in the application's directory for
	//  locale catalogs, not the current directory.  Here we force it to
	//  look in the current directory as well.
	wxString cwd = wxGetCwd();
	m_locale.AddCatalogLookupPathPrefix(cwd);

	bool bSuccess=false;
	if (m_locale_name != "")
	{
		VTLOG("Looking up language: %s\n", (const char *) m_locale_name);
		lang = GetLangFromName(wxString2(m_locale_name));
		if (lang == wxLANGUAGE_UNKNOWN)
		{
			VTLOG(" Unknown, falling back on default language.\n");
			lang = wxLANGUAGE_DEFAULT;
		}
		else
		{
			info = m_locale.GetLanguageInfo(lang);
			const char *canonical = info->CanonicalName.mb_str();
			const char *descript = info->Description.mb_str();
			VTLOG("Initializing locale to language %d, Canonical name '%s', Description: '%s':\n", lang,
				canonical, descript);
			bSuccess = m_locale.Init(lang, wxLOCALE_CONV_ENCODING);
		}
	}
	if (lang == wxLANGUAGE_DEFAULT)
	{
		VTLOG("Initializing locale to default language:\n");
		bSuccess = m_locale.Init(wxLANGUAGE_DEFAULT, wxLOCALE_CONV_ENCODING);
		if (bSuccess)
			lang = default_lang;
	}
	if (bSuccess)
		VTLOG(" succeeded.\n");
	else
		VTLOG(" failed.\n");

	if (lang != wxLANGUAGE_ENGLISH_US)
	{
		VTLOG("Attempting to load the 'VTBuilder.mo' catalog for the current locale.\n");
		bSuccess = m_locale.AddCatalog(wxT("VTBuilder"));
		if (bSuccess)
			VTLOG(" succeeded.\n");
		else
			VTLOG(" not found.\n");
		VTLOG("\n");
	}

	// Test it
//	wxString test = _("&File");

	wxLog::SetVerbose(false);
}

