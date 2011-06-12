//
// Name: StartupDlg.cpp
//
// Copyright (c) 2001-2011 Virtual Terrain Project
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

#include "vtlib/vtlib.h"	// mostly for gl.h
#include "vtlib/core/TParams.h"
#include "vtdata/DataPath.h"
#include "vtdata/vtLog.h"
#include "vtui/Helper.h"	// for AddFilenamesToComboBox

#include "wx/glcanvas.h"

#include "EnviroApp.h"
#include "OptionsDlg.h"
#include "StartupDlg.h"
#include "TerrManDlg.h"

DECLARE_APP(EnviroApp);


static void ShowOGLInfo2(bool bLog)
{
#if 1
	wxFrame *frame = new wxFrame(NULL, wxID_ANY, wxT(""), wxPoint(0, 0), wxSize(0, 0), 0);
	wxGLCanvas *canvas = new wxGLCanvas(frame, wxID_ANY, wxPoint(0, 0), wxSize(0, 0));
#if defined(__WXMAC__) || defined(__WXGTK__)
		frame->Show(true);
		canvas->SetCurrent();
		frame->Show(false);
#else
		canvas->GetContext()->SetCurrent(*canvas);
#endif
#else
	wxFrame *frame = new wxFrame();
	frame->Create(NULL, -1, _T("Test"));
	wxGLCanvas *canvas = new wxGLCanvas(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	frame->Show();
	canvas->SetCurrent();	// This is necessary as of wx 2.8
	frame->Show(false);		// Minimise the visibility of the opengl test window
#endif

	GLint value;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
	GLenum error = glGetError();
	if (error == GL_INVALID_OPERATION)
		value = 0;

	wxString msg, str;
	if (bLog)
	{
		// send the information to the debug log only
		VTLOG("\tOpenGL Version: %hs\n\tVendor: %hs\n\tRenderer: %hs\n"
			"\tMaximum Texture Dimension: %d\n",
			glGetString(GL_VERSION), glGetString(GL_VENDOR),
			glGetString(GL_RENDERER), value);
	}
	else
	{
		// show the information in a popup dialog
		str.Printf(_("OpenGL Version: %hs\nVendor: %hs\nRenderer: %hs\n"),
			glGetString(GL_VERSION), glGetString(GL_VENDOR),
			glGetString(GL_RENDERER));
		msg += str;
		str.Printf(_("Maximum Texture Dimension: %d\n"), value);
		msg += str;
		msg += _("Extensions: ");
		const char *ext = (const char *) glGetString(GL_EXTENSIONS);
		msg += wxString(ext, wxConvUTF8);
	}
	frame->Close();
	delete frame;

	if (!bLog)
	{
		TextDlgBase dlg(NULL, -1, _("OpenGL Info"), wxDefaultPosition);
		//TextDialogFunc(&dlg, true);
		wxTextCtrl* pText = (wxTextCtrl*) dlg.FindWindow( ID_TEXT );
		pText->SetValue(msg);

		dlg.ShowModal();
	}
}

// WDR: class implementations

//----------------------------------------------------------------------------
// StartupDlg
//----------------------------------------------------------------------------

// WDR: event table for StartupDlg

BEGIN_EVENT_TABLE(StartupDlg,StartupDlgBase)
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
		StartupDlgBase( parent, id, title, position, size, style )
{
	m_psImage = GetImagetext();
	m_pImage = GetImage();

	AddValidator(this, ID_EARTHVIEW, &m_bStartEarth);
	AddValidator(this, ID_IMAGE, &m_strEarthImage);
	AddValidator(this, ID_TERRAIN, &m_bStartTerrain);

	GetSizer()->SetSizeHints(this);
}

void StartupDlg::GetOptionsFrom(EnviroOptions &opt)
{
	m_bStartEarth = opt.m_bEarthView;
	m_bStartTerrain = !opt.m_bEarthView;
	m_strEarthImage = wxString::FromAscii((const char *)opt.m_strEarthImage);
	m_strTName = wxString(opt.m_strInitTerrain, wxConvUTF8);

	// store a copy of all the options
	m_opt = opt;
}

void StartupDlg::PutOptionsTo(EnviroOptions &opt)
{
	m_opt.m_bEarthView = m_bStartEarth;
	m_opt.m_strEarthImage = m_strEarthImage.mb_str(wxConvUTF8);
	m_opt.m_strInitTerrain = m_strTName.mb_str(wxConvUTF8);

	opt = m_opt;
}

void StartupDlg::UpdateState()
{
	m_psImage->Enable(m_bStartEarth);
	m_pImage->Enable(m_bStartEarth);
	GetTname()->Enable(m_bStartTerrain);
	GetEditprop()->Enable(m_bStartTerrain);
}

void StartupDlg::RefreshTerrainChoices()
{
	GetTname()->Clear();

	EnviroApp &app = wxGetApp();

	for (unsigned int i = 0; i < app.terrain_files.size(); i++)
	{
		vtString &name = app.terrain_names[i];
		wxString ws(name, wxConvUTF8);
		GetTname()->Append(ws);
	}
}


// WDR: handler implementations for StartupDlg

void StartupDlg::OnInitDialog(wxInitDialogEvent& event)
{
	VTLOG("StartupDlg Init.\n");

	// log OpenGL info, including max texture size
	ShowOGLInfo2(true);

	// Populate Earth Image files choices
	vtStringArray &paths = vtGetDataPath();
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
	if (dlg.ShowModal() == wxID_OK)
	{
		g_Options = m_opt;
		g_Options.WriteXML();
		wxGetApp().RefreshTerrainList();
		RefreshTerrainChoices();
		int sel = GetTname()->FindString(m_strTName);
		if (sel != -1)
			GetTname()->Select(sel);
	}
}

void StartupDlg::OnEditProp( wxCommandEvent &event )
{
	vtString name_old = (const char *) m_strTName.mb_str(wxConvUTF8);
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
				m_strTName = wxString(name_new, wxConvUTF8);
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
	ShowOGLInfo2(false);
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
	event.Skip();
}

