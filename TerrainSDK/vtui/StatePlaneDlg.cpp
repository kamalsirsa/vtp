//
// Name: StatePlaneDlg.cpp
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "StatePlaneDlg.h"
#include "vtui/wxString2.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// StatePlaneDlg
//----------------------------------------------------------------------------

// WDR: event table for StatePlaneDlg

BEGIN_EVENT_TABLE(StatePlaneDlg, AutoDialog)
	EVT_INIT_DIALOG (StatePlaneDlg::OnInitDialog)
	EVT_LISTBOX( ID_STATEPLANES, StatePlaneDlg::OnListBox )
	EVT_RADIOBUTTON( ID_NAD27, StatePlaneDlg::OnNad27 )
	EVT_RADIOBUTTON( ID_NAD83, StatePlaneDlg::OnNad83 )
END_EVENT_TABLE()

StatePlaneDlg::StatePlaneDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bSetting = false;

	m_bNAD27 = false;
	m_bNAD83 = true;

	m_bMeters = true;
	m_bFeet = false;
	m_bFeetUS = false;

	StatePlaneDialogFunc( this, TRUE ); 

	AddValidator(ID_NAD27, &m_bNAD27);
	AddValidator(ID_NAD83, &m_bNAD83);
	AddValidator(ID_RADIO_METERS, &m_bMeters);
	AddValidator(ID_RADIO_FEET, &m_bFeet);
	AddValidator(ID_RADIO_FEET_US, &m_bFeetUS);
}

void StatePlaneDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// update state plane control
	m_iStatePlane = 1;
	int num_planes = GetNumStatePlanes();
	StatePlaneInfo *plane_info = GetStatePlaneTable();
	for (int i = 0; i < num_planes; i++)
	{
		wxString2 str = plane_info[i].name;
		GetStatePlanes()->Append(str, (void *) plane_info[i].usgs_code);
	}

	TransferDataToWindow();
}

// WDR: handler implementations for StatePlaneDlg

void StatePlaneDlg::OnNad83( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	m_bMeters = true;
	m_bFeet = false;
	m_bFeetUS = false;
	m_bNAD27 = false;
	m_bNAD83 = true;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void StatePlaneDlg::OnNad27( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	m_bMeters = false;
	m_bFeet = false;
	m_bFeetUS = true;
	m_bNAD27 = true;
	m_bNAD83 = false;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void StatePlaneDlg::OnListBox( wxCommandEvent &event )
{
	m_iStatePlane = event.GetInt();

	// Disable some combinations of zone and datum
	StatePlaneInfo *plane_info = GetStatePlaneTable();

	bool b27 = plane_info[m_iStatePlane].bNAD27;
	bool b83 = plane_info[m_iStatePlane].bNAD83;
	GetNad27()->Enable(b27);
	GetNad83()->Enable(b83);
	if (!b27)
		GetNad83()->SetValue(true);
	if (!b83)
	{
		GetNad27()->SetValue(true);
		GetFeetUs()->SetValue(true);	// NAD27 is always U.S. Survey feet
	}
}

