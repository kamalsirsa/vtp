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
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"	// for AddFilenamesToComboBox

#include "EnviroApp.h"
#include "TParamsDlg.h"
#include "TerrManDlg.h"
#include "OptionsDlg.h"

DECLARE_APP(EnviroApp);

//
// Helper: find the largest texture size supported by OpenGL
//
#ifdef WIN32
static void ShowOGLInfo(bool bLog, HDC hdc)
#else
static void ShowOGLInfo(bool bLog)
#endif
{
	VTLOG("Opening an OpenGL context:\n");
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

	// get the best available match of pixel format for the device context
	VTLOG("ChoosePixelFormat, ");
	int iPixelFormat = ChoosePixelFormat(hdc, &pfd);

	// make that the pixel format of the device context
	VTLOG("SetPixelFormat, ");
	SetPixelFormat(hdc, iPixelFormat, &pfd);

	VTLOG("wglCreateContext, ");
	HGLRC device = wglCreateContext(hdc);
	if (device == NULL)
	{
		DWORD lasterror = GetLastError();
		VTLOG(" failed with error %d\n", lasterror);
		// 2000 The pixel format is invalid.  ERROR_INVALID_PIXEL_FORMAT
		return;
	}
	VTLOG("wglMakeCurrent.\n");
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
	int dummy;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		wxLogFatalError( _T("could not open display") );

	if (!glXQueryExtension(dpy, &dummy, &dummy))
		wxLogFatalError( _T("X server has no OpenGL GLX extension") );

	vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
	if (vi == NULL)
		wxLogFatalError( _T("no RGB visual with double and depth buffer") );
	if (vi->c_class != TrueColor)
		wxLogFatalError( _T("TrueColor visual required for this program") );

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
		wxLogFatalError( _T("could not create rendering context") );

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

// WDR: event table for StartupDlg

BEGIN_EVENT_TABLE(StartupDlg,AutoDialog)
	EVT_INIT_DIALOG (StartupDlg::OnInitDialog)
	EVT_BUTTON( wxID_OK, StartupDlg::OnOK )
	EVT_BUTTON( ID_OPENGL, StartupDlg::OnOpenGLInfo )
	EVT_BUTTON( ID_OPTIONS, StartupDlg::OnOptions )
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

	m_psImage = GetImagetext();
	m_pImage = GetImage();

	AddValidator(ID_EARTHVIEW, &m_bStartEarth);
	AddValidator(ID_IMAGE, &m_strEarthImage);
	AddValidator(ID_TERRAIN, &m_bStartTerrain);
}

void StartupDlg::GetOptionsFrom(EnviroOptions &opt)
{
	m_bStartEarth = opt.m_bEarthView;
	m_bStartTerrain = !opt.m_bEarthView;
	m_strEarthImage = wxString::FromAscii((const char *)opt.m_strEarthImage);
	m_strTName.from_utf8(opt.m_strInitTerrain);

	// store a copy of all the options
	m_opt = opt;
}

void StartupDlg::PutOptionsTo(EnviroOptions &opt)
{
	m_opt.m_bEarthView = m_bStartEarth;
	m_opt.m_strEarthImage = m_strEarthImage.mb_str();
	m_opt.m_strInitTerrain = m_strTName.to_utf8();

	opt = m_opt;
}

void StartupDlg::UpdateState()
{
	m_psImage->Enable(m_bStartEarth);
	m_pImage->Enable(m_bStartEarth);
	GetTname()->Enable(m_bStartTerrain);
}

void StartupDlg::RefreshTerrainChoices()
{
	GetTname()->Clear();

	EnviroApp &app = wxGetApp();
	wxString2 ws;

	for (unsigned int i = 0; i < app.terrain_files.size(); i++)
	{
		vtString &name = app.terrain_names[i];
		ws.from_utf8(name);
		GetTname()->Append(ws);
	}
}


// WDR: handler implementations for StartupDlg

void StartupDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG("StartupDlg Init.\n");

	// log OpenGL info, including max texture size
#ifdef WIN32
	wxClientDC wdc(this);
	HDC hdc = (HDC) wdc.GetHDC();
	ShowOGLInfo(true, hdc);
#else
	ShowOGLInfo(true);
#endif

	// Populate Earth Image files choices
	vtStringArray &paths = g_Options.m_DataPaths;
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		vtString path = paths[i];
		path += "WholeEarth/";
		AddFilenamesToComboBox(m_pImage, path, "*_0106.png", 9);
		AddFilenamesToComboBox(m_pImage, path, "*_0106.jpg", 9);
	}
	int sel = m_pImage->FindString(m_strEarthImage);
	if (sel != -1)
		m_pImage->SetSelection(sel);

	UpdateState();

	// Terrain choices
	RefreshTerrainChoices();
	sel = GetTname()->FindString(m_strTName);
	if (sel != -1)
		GetTname()->Select(sel);

	wxWindow::OnInitDialog(event);
}

void StartupDlg::OnTnameChoice( wxCommandEvent &event )
{
	m_strTName = GetTname()->GetStringSelection();
}

void StartupDlg::OnTerrMan( wxCommandEvent &event )
{
	TerrainManagerDlg dlg(this, -1, _("Terrain Manager"), wxDefaultPosition);
	dlg.m_DataPaths = m_opt.m_DataPaths;
	if (dlg.ShowModal() == wxID_OK)
	{
		m_opt.m_DataPaths = dlg.m_DataPaths;
		g_Options = m_opt;
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

void StartupDlg::OnOptions( wxCommandEvent &event )
{
	OptionsDlg dlg(this, -1, _("Global Options"));
	dlg.GetOptionsFrom(m_opt);

	int result = dlg.ShowModal();
	if (result == wxID_OK)
		dlg.PutOptionsTo(m_opt);
}

void StartupDlg::OnOK( wxCommandEvent &event )
{
	VTLOG("StartupDlg pressed OK.\n");
	wxDialog::OnOK(event);
}

