/////////////////////////////////////////////////////////////////////////////
// Name:        Projection2Dlg.cpp
// Author:    XX
// Created:  XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "Projection2Dlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Projection2Dlg.h"

//
// Must offset the values we use for Datum because a Choice
// control cannot handle "client data" values less than 0.
//
#define CHOICE_OFFSET 10

// WDR: class implementations

//----------------------------------------------------------------------------
// Projection2Dlg
//----------------------------------------------------------------------------

// WDR: event table for Projection2Dlg

BEGIN_EVENT_TABLE(Projection2Dlg,AutoDialog)
    EVT_CHOICE( ID_PROJCHOICE, Projection2Dlg::OnProjChoice )
END_EVENT_TABLE()

Projection2Dlg::Projection2Dlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
	m_bInitializedUI = false;
    ProjectionDialog2Func( this, TRUE ); 
}

// Re-arrange the UI for a given projection
void Projection2Dlg::SetProjectionUI(ProjType type)
{
    m_eProj = type;
	m_iProj = m_eProj;

	if (m_bInitializedUI)
		UpdateControlStatus();
}

void Projection2Dlg::UpdateControlStatus()
{
	int i;

    switch (m_eProj)
    {
    case PT_GEO:
        m_pParamCtrl->Enable(false);
        m_pZoneCtrl->Enable(false);
        break;
    case PT_UTM:
        m_pParamCtrl->Enable(false);
        m_pZoneCtrl->Enable(true);
		m_pZoneCtrl->Clear();
		for (i = 0; i < 60; i++)
		{
			wxString str;
			str.Printf("Zone %d", i+1);
			m_pZoneCtrl->Append(str, (void *) (i+1));
		}
		break;
    case PT_ALBERS:
        m_pParamCtrl->Enable(true);
        m_pZoneCtrl->Enable(true);
        break;
    case PT_STATE:
		{
        m_pParamCtrl->Enable(false);
        m_pZoneCtrl->Enable(true);
		// Fill in choices for State Plane
		int num_planes = GetNumStatePlanes();
		StatePlaneInfo *plane_info = GetStatePlaneTable();

		m_pZoneCtrl->Clear();
		for (i = 0; i < num_planes; i++)
		{
			m_pZoneCtrl->Append(plane_info[i].name, (void *) plane_info[i].usgs_code);
		}
		for (i = 0; i < num_planes; i++)
		{
			if (plane_info[i].usgs_code == m_iZone)
			   m_pZoneCtrl->SetStringSelection(plane_info[i].name);
		}
		}
        break;
    }
    TransferDataToWindow();
}

void Projection2Dlg::SetProjection(vtProjection &proj)
{
    if (proj.IsGeographic())
        SetProjectionUI(PT_GEO);
    else
    {
        const char *proj_string = proj.GetProjectionName();
        if (!strcmp(proj_string, "Transverse_Mercator"))
        {
            SetProjectionUI(PT_UTM);
//          m_iZone = proj.GetUTMZone();
        }
        if (!strcmp(proj_string, "Albers_Conic_Equal_Area"))
        {
            SetProjectionUI(PT_ALBERS);
//          m_fPar1 = proj.GetProjParm("standard_parallel_1");
//          m_fPar2 = proj.GetProjParm("standard_parallel_2");
//          m_fMeridian = proj.GetProjParm("longitude_of_center");
            // what about "latitude_of_center"? "latitude of projection's origin"
        }
        // TODO: state plane? is that possible or desirable to detect?
    }
	m_iDatum = (int) proj.GetDatum();
}

void Projection2Dlg::GetProjection(vtProjection &proj)
{
/*  // First set Datum
    switch ((DATUM) m_iDatum)
    {
    case NAD27:  proj.SetWellKnownGeogCS( "NAD27" ); break;
    case NAD83:  proj.SetWellKnownGeogCS( "NAD83" ); break;
    case WGS_72: proj.SetWellKnownGeogCS( "WGS72" ); break;
    case WGS_84: proj.SetWellKnownGeogCS( "WGS84" ); break;
    // TODO: Need to support more datums here!
    default:     proj.SetWellKnownGeogCS( "WGS84" ); break;
    }

    if (m_iProj == PT_GEO)
    {
        // nothing more to do
    }
    else if (m_iProj == PT_UTM)
    {
        proj.SetUTM( m_iZone, TRUE );
    }
    else if (m_iProj == PT_ALBERS)
    {
        proj.SetACEA( m_fPar1, m_fPar2,
                         59.0, m_fMeridian,
                         500000, 500000 );
    }
    else if (m_iProj == PT_STATE)
    {
        proj.SetStatePlane( m_iStatePlane );
    }*/
}


// WDR: handler implementations for Projection2Dlg

void Projection2Dlg::OnProjChoice( wxCommandEvent &event )
{
    TransferDataFromWindow();
	SetProjectionUI( (ProjType) m_iProj );
}

void Projection2Dlg::OnInitDialog(wxInitDialogEvent& event)
{
	int i;

    m_pParamCtrl = GetProjparam();
    m_pZoneCtrl = GetZonechoice();
    m_pDatumCtrl = GetDatumchoice();
    m_pProjCtrl = GetProjchoice();

    AddValidator(ID_PROJCHOICE, &m_iProj);

	m_pProjCtrl->Append("Geographic");
	m_pProjCtrl->Append("UTM");
	m_pProjCtrl->Append("Albers Equal Area Conic");
	m_pProjCtrl->Append("State Plane Coordinates");

    // Fill in choices for Datum
    for (i = NO_DATUM; i <= WGS_84; i++)
    {
        m_pDatumCtrl->Append(datumToString((DATUM) i), (void *) (i+CHOICE_OFFSET));
    }
    m_pDatumCtrl->SetStringSelection(datumToString((DATUM)m_iDatum));

	m_bInitializedUI = true;

	UpdateControlStatus();

	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}

