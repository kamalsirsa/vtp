//
// Name: StartupDlg.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation "StartupDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif

#if defined(UNIX)
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <GL/glx.h>
#endif

#include "vtlib/vtlib.h"	// mostly for gl.h

#include "StartupDlg.h"
#include "../Options.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"	// for AddFilenamesToComboBox

#include "app.h"
#include "TParamsDlg.h"
#include "TerrManDlg.h"

DECLARE_APP(vtApp);

//
// Helper: find the largest texture size supported by OpenGL
//
#ifdef WIN32
static void ShowOGLInfo(bool bLog, HDC hdc)
#else
static void ShowOGLInfo(bool bLog)
#endif
{
#if defined(WIN32)
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
		1,				  // version number
		PFD_DRAW_TO_WINDOW |	// support window
		PFD_SUPPORT_OPENGL |	// support OpenGL
		PFD_DOUBLEBUFFER,	 // double buffered
		PFD_TYPE_RGBA,	  // RGBA type
		24,			   // 24-bit color depth
		0, 0, 0, 0, 0, 0,	 // color bits ignored
		0, 0, 0,				// no alpha buffer
		0, 0, 0, 0,	   // accum bits ignored
		32, 0, 0,			 // 32-bit z-buffer
		PFD_MAIN_PLANE,   // main layer
		0, 0, 0, 0		  // reserved, layer masks ignored
	};
	int  iPixelFormat;

	// get the best available match of pixel format for the device context 
	iPixelFormat = ChoosePixelFormat(hdc, &pfd);
	// make that the pixel format of the device context
	SetPixelFormat(hdc, iPixelFormat, &pfd);

	HGLRC device = wglCreateContext(hdc);
	if (device == NULL)
	{
		DWORD lasterror = GetLastError();
		// 2000 The pixel format is invalid.  ERROR_INVALID_PIXEL_FORMAT
	}
	wglMakeCurrent(hdc, device);
#elif defined(UNIX)
	static int dblBuf[] =  {GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 1,
		GLX_DOUBLEBUFFER, None};
	Display *dpy;
	Window win;
	XVisualInfo *vi;
	Colormap cmap;
	XSetWindowAttributes swa;
	GLXContext cx;
	XEvent event;
	Bool needRedraw = False, recalcModelView = True;
	int dummy;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		wxFatalError( "could not open display" );

	if (!glXQueryExtension(dpy, &dummy, &dummy))
		wxFatalError( "X server has no OpenGL GLX extension" );

	vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
	if (vi == NULL)
		wxFatalError( "no RGB visual with double and depth buffer" );
	if (vi->c_class != TrueColor)
		wxFatalError( "TrueColor visual required for this program" );

	cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen),
			vi->visual, AllocNone);
	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask;
	win = XCreateWindow(dpy, RootWindow(dpy, vi->screen),
			0, 0, 300, 300, 0, vi->depth,
			InputOutput, vi->visual,
			CWBorderPixel | CWColormap | CWEventMask, &swa);

	XSetStandardProperties(dpy, win, "test", "test",
		None, NULL, 0, NULL);

	cx = glXCreateContext(dpy, vi, None, True);
	if (cx == NULL)
		wxFatalError( "could not create rendering context" );

	glXMakeCurrent(dpy, win, cx);
#else
#  error "I don't know this platform."
#endif

	GLint value;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
	GLenum error = glGetError();
	if (error == GL_INVALID_OPERATION)
		value = 0;

	if (bLog)
	{
		VTLOG("\tOpenGL Version: %hs\n\tVendor: %hs\n\tRenderer: %hs\n"
			"\tMaximum Texture Dimension: %d\n",
			glGetString(GL_VERSION), glGetString(GL_VENDOR),
			glGetString(GL_RENDERER), value);
	}
	else
	{
		wxString msg;

		wxString2 str;
		str.Printf(_T("OpenGL Version: %hs\nVendor: %hs\nRenderer: %hs\n"),
			glGetString(GL_VERSION), glGetString(GL_VENDOR),
			glGetString(GL_RENDERER));
		msg += str;
		str.Printf(_("Maximum Texture Dimension: %d\n"), value);
		msg += str;
		str.Printf(_T("Extensions: %hs"), glGetString(GL_EXTENSIONS));
		msg += str;

		wxDialog dlg(NULL, -1, _("OpenGL Info"), wxDefaultPosition);
		TextDialogFunc(&dlg, true);
		wxTextCtrl* pText = (wxTextCtrl*) dlg.FindWindow( ID_TEXT );
		pText->SetValue(msg);

		dlg.ShowModal();
	}

#ifdef WIN32
	wglDeleteContext(device);
#elif defined(UNIX)
	glXDestroyContext(dpy, cx);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
#endif
}

// WDR: class implementations

//----------------------------------------------------------------------------
// StartupDlg
//----------------------------------------------------------------------------

void StartupDlg::GetOptionsFrom(EnviroOptions &opt)
{
	m_bStartEarth = opt.m_bEarthView;
	m_bStartTerrain = !opt.m_bEarthView;
	m_strImage = wxString::FromAscii((const char *)opt.m_strImage);
	m_strTName.from_utf8(opt.m_strInitTerrain);
	m_bFullscreen = opt.m_bFullscreen;
	m_bHtmlpane = opt.m_bHtmlpane;
	m_bFloatingToolbar = opt.m_bFloatingToolbar;
	m_bTextureCompression = opt.m_bTextureCompression;
	m_bSpeedTest = opt.m_bSpeedTest;
	m_fPlantScale = opt.m_fPlantScale;
	m_bShadows = opt.m_bShadows;
	m_strContentFile = opt.m_strContentFile;
}

void StartupDlg::PutOptionsTo(EnviroOptions &opt)
{
	opt.m_bEarthView = m_bStartEarth;
	opt.m_strImage = m_strImage.mb_str();
	opt.m_strInitTerrain = m_strTName.to_utf8();
	opt.m_bFullscreen = m_bFullscreen;
	opt.m_bHtmlpane = m_bHtmlpane;
	opt.m_bFloatingToolbar = m_bFloatingToolbar;
	opt.m_bTextureCompression = m_bTextureCompression;
	opt.m_bSpeedTest = m_bSpeedTest;
	opt.m_fPlantScale = m_fPlantScale;
	opt.m_bShadows = m_bShadows;
	opt.m_strContentFile = m_strContentFile.mb_str();
}

void StartupDlg::UpdateState()
{
	m_psImage->Enable(m_bStartEarth);
	m_pImage->Enable(m_bStartEarth);
	GetTname()->Enable(m_bStartTerrain);
}

// WDR: event table for StartupDlg

BEGIN_EVENT_TABLE(StartupDlg,AutoDialog)
	EVT_BUTTON( wxID_OK, StartupDlg::OnOK )
	EVT_BUTTON( ID_OPENGL, StartupDlg::OnOpenGLInfo )
	EVT_RADIOBUTTON( ID_EARTHVIEW, StartupDlg::OnEarthView )
	EVT_RADIOBUTTON( ID_TERRAIN, StartupDlg::OnTerrain )
	EVT_BUTTON( ID_EDITPROP, StartupDlg::OnEditProp )
	EVT_BUTTON( ID_TERRMAN, StartupDlg::OnTerrMan )
	EVT_CHOICE( ID_TNAME, StartupDlg::OnTnameChoice )
END_EVENT_TABLE()

StartupDlg::StartupDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
		AutoDialog( parent, id, title, position, size, style )
{
	VTLOG("Constructing StartupDlg.\n");
	StartupDialogFunc( this, TRUE );
}

void StartupDlg::RefreshTerrainChoices()
{
	GetTname()->Clear();

	vtApp &app = wxGetApp();
	wxString2 ws;

	for (unsigned int i = 0; i < app.terrain_files.size(); i++)
	{
		ws.from_utf8(app.terrain_names[i]);
		GetTname()->Append(ws);
	}
}


// WDR: handler implementations for StartupDlg

void StartupDlg::OnTnameChoice( wxCommandEvent &event )
{
	m_strTName = GetTname()->GetStringSelection();
}

void StartupDlg::OnTerrMan( wxCommandEvent &event )
{
	TerrainManagerDlg dlg(this, -1, _("Terrain Manager"), wxDefaultPosition);
	if (dlg.ShowModal() == wxID_OK)
	{
		g_Options.Write();
		wxGetApp().RefreshTerrainList();
		RefreshTerrainChoices();
		int sel = GetTname()->FindString(m_strTName);
		if (sel != -1)
			GetTname()->Select(sel);
	}
}

void StartupDlg::OnEditProp( wxCommandEvent &event )
{
	vtString name_old = m_strTName.to_utf8();
	vtString path_to_ini = wxGetApp().GetIniFileForTerrain(name_old);
	if (path_to_ini == "")
		return;

	int res = EditTerrainParameters(this, path_to_ini);
	if (res == wxID_OK)
	{
		// Name might have changed
		TParams params;
		if (params.LoadFrom(path_to_ini))
		{
			vtString name_new = params.GetValueString(STR_NAME);
			if (name_new != name_old)
				m_strTName.from_utf8(name_new);
		}

		wxGetApp().RefreshTerrainList();
		RefreshTerrainChoices();
		int sel = GetTname()->FindString(m_strTName);
		if (sel != -1)
			GetTname()->Select(sel);
	}
}

void StartupDlg::OnTerrain( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateState();
}

void StartupDlg::OnEarthView( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateState();
}

void StartupDlg::OnOpenGLInfo( wxCommandEvent &event )
{
	// display OpenGL info, including max texture size
#ifdef WIN32
	wxClientDC wdc(this);
	HDC hdc = (HDC) wdc.GetHDC();
	ShowOGLInfo(false, hdc);
#else
	ShowOGLInfo(false);
#endif
}

void StartupDlg::OnOK( wxCommandEvent &event )
{
	VTLOG("StartupDlg pressed OK.\n");
	wxDialog::OnOK(event);
}

void StartupDlg::OnInitDialog(wxInitDialogEvent& event)
{
	int sel;

	VTLOG("StartupDlg Init.\n");

	// display OpenGL info, including max texture size
#ifdef WIN32
	wxClientDC wdc(this);
	HDC hdc = (HDC) wdc.GetHDC();
	ShowOGLInfo(true, hdc);
#else
	ShowOGLInfo(true);
#endif

/*  vtTerrain *pTerr = GetTerrainScene()->FindTerrainByName(m_strTName.to_utf8());
	if (pTerr)
		m_strTName = wxString::FromAscii(pTerr->GetName());
	else
		m_strTName = _T("none");
*/
	m_psImage = GetImagetext();
	m_pImage = GetImage();

	// Populate Earth Image files choices
	vtStringArray &paths = g_Options.m_DataPaths;
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		vtString path = paths[i];
		path += "WholeEarth/";
		AddFilenamesToComboBox(m_pImage, path, "*_0106.png", 9);
		AddFilenamesToComboBox(m_pImage, path, "*_0106.jpg", 9);
	}
	sel = m_pImage->FindString(m_strImage);
	if (sel != -1)
		m_pImage->SetSelection(sel);

	// Populate Content files choices
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		vtString path = paths[i];
		AddFilenamesToChoice(GetContent(), path, "*.vtco");
	}
	m_iContentFile = GetContent()->FindString(m_strContentFile);
	if (m_iContentFile != -1)
		GetContent()->SetSelection(m_iContentFile);

	UpdateState();

	AddValidator(ID_EARTHVIEW, &m_bStartEarth);
	AddValidator(ID_TERRAIN, &m_bStartTerrain);

	AddValidator(ID_FULLSCREEN, &m_bFullscreen);
//	AddValidator(ID_HTML_PANE, &m_bHtmlpane);
//	AddValidator(ID_FLOATING, &m_bFloatingToolbar);
	AddValidator(ID_TEXTURE_COMPRESSION, &m_bTextureCompression);
//	AddValidator(ID_SHADOWS, &m_bShadows);

	AddValidator(ID_CHOICE_CONTENT, &m_iContentFile);
	AddValidator(ID_CHOICE_CONTENT, &m_strContentFile);

	// Terrain choices
	RefreshTerrainChoices();
	sel = GetTname()->FindString(m_strTName);
	if (sel != -1)
		GetTname()->Select(sel);

	AddValidator(ID_IMAGE, &m_strImage);
	AddNumValidator(ID_PLANTSIZE, &m_fPlantScale, 2);

	wxWindow::OnInitDialog(event);
}

