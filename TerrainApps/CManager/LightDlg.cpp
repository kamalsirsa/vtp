//
// Name:		LightDlg.cpp
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "LightDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "vtlib/vtlib.h"
#include "vtui/Helper.h"	// for FillWithColor
#include "LightDlg.h"
#include <wx/colordlg.h>

// WDR: class implementations

//----------------------------------------------------------------------------
// LightDlg
//----------------------------------------------------------------------------

// WDR: event table for LightDlg

BEGIN_EVENT_TABLE(LightDlg,AutoDialog)
	EVT_CHOICE( ID_LIGHT, LightDlg::OnLight )
	EVT_BUTTON( ID_AMBIENT, LightDlg::OnAmbient )
	EVT_BUTTON( ID_DIFFUSE, LightDlg::OnDiffuse )
	EVT_TEXT_ENTER( ID_DIRX, LightDlg::OnText )
	EVT_TEXT_ENTER( ID_DIRY, LightDlg::OnText )
	EVT_TEXT_ENTER( ID_DIRZ, LightDlg::OnText )
END_EVENT_TABLE()

LightDlg::LightDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function LightDialogFunc for LightDlg
	LightDialogFunc( this, TRUE ); 

	AddNumValidator(ID_DIRX, &m_dir.x);
	AddNumValidator(ID_DIRY, &m_dir.y);
	AddNumValidator(ID_DIRZ, &m_dir.z);

	GetLight()->SetSelection(0);
}

void LightDlg::UseLight(vtMovLight *pMovLight)
{
	m_pMovLight = pMovLight;
	m_pLight = pMovLight->m_pLight;

	unsigned char r, g, b;
	RGBf col;
	col = m_pLight->GetAmbient();
	r = (unsigned char) col.r * 255;
	g = (unsigned char) col.g * 255;
	b = (unsigned char) col.b * 255;
	m_ambient.Set(r, g, b);

	col = m_pLight->GetDiffuse();
	r = (unsigned char) col.r * 255;
	g = (unsigned char) col.g * 255;
	b = (unsigned char) col.b * 255;
	m_diffuse.Set(r, g, b);

	UpdateColorBitmaps();

	m_dir = m_pMovLight->GetDirection();

	TransferDataToWindow();
}

void LightDlg::UpdateColorBitmaps()
{
	// Case of a single color, simple
	FillWithColor(GetAmbient(), m_ambient);
	FillWithColor(GetDiffuse(), m_diffuse);
}


// WDR: handler implementations for LightDlg

void LightDlg::OnText( wxCommandEvent &event )
{
	TransferDataFromWindow();
	if (m_pMovLight)
		m_pMovLight->SetDirection(m_dir);
}

void LightDlg::OnDiffuse( wxCommandEvent &event )
{
	m_data.SetChooseFull(true);
	m_data.SetColour(m_diffuse);

	wxColourDialog dlg(this, &m_data);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_data = dlg.GetColourData();
		m_diffuse = m_data.GetColour();

		RGBi result(m_diffuse.Red(), m_diffuse.Green(), m_diffuse.Blue());
		if (m_pLight)
			m_pLight->SetDiffuse(result);
		UpdateColorBitmaps();
	}
}

void LightDlg::OnAmbient( wxCommandEvent &event )
{
	m_data.SetChooseFull(true);
	m_data.SetColour(m_ambient);

	wxColourDialog dlg(this, &m_data);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_data = dlg.GetColourData();
		m_ambient = m_data.GetColour();

		RGBi result(m_ambient.Red(), m_ambient.Green(), m_ambient.Blue());
		if (m_pLight)
			m_pLight->SetAmbient(result);
		UpdateColorBitmaps();
	}
}

void LightDlg::OnLight( wxCommandEvent &event )
{
	
}

