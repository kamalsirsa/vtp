//
// Name: StatePlaneDlg.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "StatePlaneDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "StatePlaneDlg.h"
#include "vtui/wxString2.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// StatePlaneDlg
//----------------------------------------------------------------------------

// WDR: event table for StatePlaneDlg

BEGIN_EVENT_TABLE(StatePlaneDlg,AutoDialog)
	EVT_LISTBOX( ID_STATEPLANES, StatePlaneDlg::OnListBox )
	EVT_RADIOBUTTON( ID_NAD27, StatePlaneDlg::OnNad27 )
	EVT_RADIOBUTTON( ID_NAD83, StatePlaneDlg::OnNad83 )
END_EVENT_TABLE()

StatePlaneDlg::StatePlaneDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bSetting = false;
	StatePlaneDialogFunc( this, TRUE ); 
}

void StatePlaneDlg::OnInitDialog(wxInitDialogEvent& event)
{
	int num_planes = GetNumStatePlanes();
	StatePlaneInfo *plane_info = GetStatePlaneTable();

	wxString2 str;
	for (int i = 0; i < num_planes; i++)
	{
		str = plane_info[i].name;
		GetStatePlanes()->Append(str, (void *) plane_info[i].usgs_code);
	}

	m_iStatePlane = 1;

	m_bNAD27 = false;
	m_bNAD83 = true;

	m_bMeters = true;
	m_bFeet = false;
	m_bFeetUS = false;

	AddValidator(ID_NAD27, &m_bNAD27);
	AddValidator(ID_NAD83, &m_bNAD83);
	AddValidator(ID_RADIO_METERS, &m_bMeters);
	AddValidator(ID_RADIO_FEET, &m_bFeet);
	AddValidator(ID_RADIO_FEET_US, &m_bFeetUS);

	TransferDataToWindow();
}

// WDR: handler implementations for StatePlaneDlg

void StatePlaneDlg::OnNad83( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	m_bSetting = true;

	m_bMeters = true;
	m_bFeet = false;
	m_bFeetUS = false;
	m_bNAD27 = false;
	m_bNAD83 = true;
	TransferDataToWindow();

	m_bSetting = false;
}

void StatePlaneDlg::OnNad27( wxCommandEvent &event )
{
	if (m_bSetting)
		return;
	m_bSetting = true;

	m_bMeters = false;
	m_bFeet = false;
	m_bFeetUS = true;
	m_bNAD27 = true;
	m_bNAD83 = false;
	TransferDataToWindow();

	m_bSetting = false;
}

void StatePlaneDlg::OnListBox( wxCommandEvent &event )
{
	m_iStatePlane = event.GetInt();
}




