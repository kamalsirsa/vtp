/////////////////////////////////////////////////////////////////////////////
// Name:        StatePlaneDlg.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "StatePlaneDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "StatePlaneDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// StatePlaneDlg
//----------------------------------------------------------------------------

// WDR: event table for StatePlaneDlg

BEGIN_EVENT_TABLE(StatePlaneDlg,AutoDialog)
    EVT_LISTBOX( ID_STATEPLANES, StatePlaneDlg::OnListBox )
END_EVENT_TABLE()

StatePlaneDlg::StatePlaneDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
    StatePlaneDialogFunc( this, TRUE ); 
}

void StatePlaneDlg::OnInitDialog(wxInitDialogEvent& event)
{
    int num_planes = GetNumStatePlanes();
    StatePlaneInfo *plane_info = GetStatePlaneTable();

    wxString *choices = new wxString[num_planes];
    for (int i = 0; i < num_planes; i++)
    {
        GetStatePlanes()->Append(plane_info[i].name, (void *) plane_info[i].usgs_code);
    }

    m_iStatePlane = 1;
    m_bNAD27 = 1;

    AddValidator(ID_NAD27, &m_bNAD27);

    TransferDataToWindow();
}

// WDR: handler implementations for StatePlaneDlg

void StatePlaneDlg::OnListBox( wxCommandEvent &event )
{
    m_iStatePlane = event.GetInt();
}




