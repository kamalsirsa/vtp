/////////////////////////////////////////////////////////////////////////////
// Name:        ProjectionDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "ProjectionDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "ProjectionDlg.h"

//
// Must offset the values we use for Datum because a Choice
// control cannot handle "client data" values less than 0.
//
#define CHOICE_OFFSET 10

// WDR: class implementations

//----------------------------------------------------------------------------
// ProjectionDlg
//----------------------------------------------------------------------------

// WDR: event table for ProjectionDlg

BEGIN_EVENT_TABLE(ProjectionDlg,AutoDialog)
    EVT_RADIOBUTTON( ID_GEO, ProjectionDlg::OnRadio )
    EVT_RADIOBUTTON( ID_UTM, ProjectionDlg::OnRadio )
    EVT_RADIOBUTTON( ID_ALBERS, ProjectionDlg::OnRadio )
    EVT_RADIOBUTTON( ID_STATEPLANE, ProjectionDlg::OnRadio )
    EVT_CHOICE( ID_SPCHOICE, ProjectionDlg::OnStatePlaneChoice )
    EVT_CHOICE( ID_DATUM, ProjectionDlg::OnDatumChoice )
END_EVENT_TABLE()

ProjectionDlg::ProjectionDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
    m_iProj = PT_GEO;
    m_fPar1 = 0;
    m_fPar2 = 0;
    m_fMeridian = 0;
    m_iStatePlane = 101;
    ProjectionDialogFunc( this, TRUE );
}

// WDR: handler implementations for ProjectionDlg

void ProjectionDlg::OnDatumChoice( wxCommandEvent &event )
{
	int sel = GetDatum()->GetSelection();
	m_iDatum = ((int) GetDatum()->GetClientData(sel)) - CHOICE_OFFSET;
}

void ProjectionDlg::OnStatePlaneChoice( wxCommandEvent &event )
{
	int sel = GetSpchoice()->GetSelection();
	m_iStatePlane = ((int) GetSpchoice()->GetClientData(sel));
}

void ProjectionDlg::OnRadio( wxCommandEvent &event )
{
    GetRadioButtons();
    DoEnables();
}

void ProjectionDlg::OnInitDialog(wxInitDialogEvent& event)
{
    int i;

    AddValidator(ID_CAPTION, &m_strCaption);
    AddNumValidator(ID_UTMZONE, &m_iZone);
    AddNumValidator(ID_PAR1, &m_fPar1);
    AddNumValidator(ID_PAR2, &m_fPar2);
    AddNumValidator(ID_MERIDIAN, &m_fMeridian);

    // Fill in choices for Datum
    for (i = NO_DATUM; i <= WGS_84; i++)
    {
        GetDatum()->Append(datumToString((DATUM) i), (void *) (i+CHOICE_OFFSET));
    }
    GetDatum()->SetStringSelection(datumToString((DATUM)m_iDatum));

    // Fill in choices for State Plane
    int num_planes = GetNumStatePlanes();
    StatePlaneInfo *plane_info = GetStatePlaneTable();

    for (i = 0; i < num_planes; i++)
    {
        GetSpchoice()->Append(plane_info[i].name, (void *) plane_info[i].usgs_code);
    }
    for (i = 0; i < num_planes; i++)
    {
        if (plane_info[i].usgs_code == m_iStatePlane)
           GetSpchoice()->SetStringSelection(plane_info[i].name);
    }

    SetRadioButtons();
    DoEnables();

    wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}

void ProjectionDlg::SetRadioButtons()
{
    GetGeo()->SetValue(m_iProj == PT_GEO);
    GetUtm()->SetValue(m_iProj == PT_UTM);
    GetAlbers()->SetValue(m_iProj == PT_ALBERS);
    GetStateplane()->SetValue(m_iProj == PT_STATE);
}

void ProjectionDlg::DoEnables()
{
    GetSpchoice()->Enable(m_iProj == PT_STATE);
    GetMeridian()->Enable(m_iProj == PT_ALBERS);
    GetPar2()->Enable(m_iProj == PT_ALBERS);
    GetPar1()->Enable(m_iProj == PT_ALBERS);
    GetUtmzone()->Enable(m_iProj == PT_UTM);
}

void ProjectionDlg::GetRadioButtons()
{
    if (GetGeo()->GetValue())
        m_iProj = PT_GEO;
    if (GetUtm()->GetValue())
        m_iProj = PT_UTM;
    if (GetAlbers()->GetValue())
        m_iProj = PT_ALBERS;
    if (GetStateplane()->GetValue())
        m_iProj = PT_STATE;
}


void ProjectionDlg::SetProjection(vtProjection &proj)
{
    if (proj.IsGeographic())
        m_iProj = PT_GEO;
    else
    {
        const char *proj_string = proj.GetProjectionName();
        if (!strcmp(proj_string, "Transverse_Mercator"))
        {
            m_iProj = PT_UTM;
            m_iZone = proj.GetUTMZone();
        }
        if (!strcmp(proj_string, "Albers_Conic_Equal_Area"))
            m_iProj = PT_ALBERS;
        // TODO: state plane
    }
    m_iDatum = (int) proj.GetDatum();
}

void ProjectionDlg::GetProjection(vtProjection &proj)
{
    // First set Datum
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
    }
}
