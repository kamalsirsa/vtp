//
// Name:        BuildingDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
    #pragma implementation "BuildingDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include <wx/colordlg.h>
#include "BuildingDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// BuildingDlg
//----------------------------------------------------------------------------

// WDR: event table for BuildingDlg

BEGIN_EVENT_TABLE(BuildingDlg,AutoDialog)
    EVT_BUTTON( wxID_OK, BuildingDlg::OnOK )
    EVT_BUTTON( ID_COLOR1, BuildingDlg::OnColor1 )
    EVT_BUTTON( ID_COLOR2, BuildingDlg::OnColor2 )
    EVT_BUTTON( ID_COLOR3, BuildingDlg::OnColor3 )
END_EVENT_TABLE()

BuildingDlg::BuildingDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
    BuildingDialogFunc( this, TRUE ); 
}

void BuildingDlg::Setup(vtBuilding *bld)
{
    m_pBuilding = bld;
}

wxBitmap *MakeColorBitmap(int xsize, int ysize, wxColour color)
{
    wxImage pImage(xsize, ysize);
    int i, j;
    for (i = 0; i < xsize; i++)
        for (j = 0; j < ysize; j++)
        {
            pImage.SetRGB(i, j, color.Red(), color.Green(), color.Blue());
        }

    wxBitmap *pBitmap = new wxBitmap(pImage.ConvertToBitmap());
    return pBitmap;
}

void BuildingDlg::EditColor(int i)
{
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(m_Color[i]);

    wxColourDialog dlg(this, &data);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxColourData data2 = dlg.GetColourData();
        m_Color[i] = data2.GetColour();

        wxBitmap *pBitmap = MakeColorBitmap(32, 18, m_Color[i]);
        m_pColorBitmapControl[i]->SetBitmapLabel(*pBitmap);
    }
}

// WDR: handler implementations for BuildingDlg

void BuildingDlg::OnColor1( wxCommandEvent &event )
{
    EditColor(0);
}

void BuildingDlg::OnColor2( wxCommandEvent &event )
{
    EditColor(1);
}

void BuildingDlg::OnColor3( wxCommandEvent &event )
{
    EditColor(2);
}

void BuildingDlg::OnOK( wxCommandEvent &event )
{
    TransferDataFromWindow();

    int c;
    RGBi color[3];
    for (c = 0; c < 3; c++)
        color[c].Set(m_Color[c].Red(), m_Color[c].Green(), m_Color[c].Blue());

    m_pBuilding->SetStories(m_iStories);
    m_pBuilding->m_RoofType = (enum RoofType) m_iRoofType;
    m_pBuilding->m_bMoulding = m_bTrim;
    m_pBuilding->m_bElevated = m_bElevated;
    for (c = 0; c < 3; c++)
        m_pBuilding->SetColor((BldColor) c, color[c]);

    wxDialog::OnOK(event);
}

void BuildingDlg::OnInitDialog(wxInitDialogEvent& event)
{
    m_pColorBitmapControl[0] = (wxBitmapButton *)FindWindow(ID_COLOR1);
    m_pColorBitmapControl[1] = (wxBitmapButton *)FindWindow(ID_COLOR2);
    m_pColorBitmapControl[2] = (wxBitmapButton *)FindWindow(ID_COLOR3);
    m_pcRoofType = (wxChoice *)FindWindow(ID_ROOF);

    int c;
    RGBi col;

	for (c = 0; c < 3; c++)
    {
        col = m_pBuilding->GetColor((BldColor)c);
        m_Color[c].Set(col.r, col.g, col.b);
    }
    m_iStories = m_pBuilding->GetStories();
    m_iRoofType = m_pBuilding->m_RoofType;
    m_bTrim = m_pBuilding->m_bMoulding;
    m_bElevated = m_pBuilding->m_bElevated;

    for (c = 0; c < 3; c++)
    {
        wxBitmap *pBitmap = MakeColorBitmap(32, 18, m_Color[c]);
        m_pColorBitmapControl[c]->SetBitmapLabel(*pBitmap);
    }

    m_pcRoofType->Append("Flat");
    m_pcRoofType->Append("Shed");
    m_pcRoofType->Append("Gable");
    m_pcRoofType->Append("Hip");

//	AddNumValidator(ID_STORIES, &m_iStories);
    AddValidator(ID_STORIES, &m_iStories);
    AddValidator(ID_ROOF, &m_iRoofType);
    AddValidator(ID_TRIM, &m_bTrim);
    AddValidator(ID_ELEVATED, &m_bElevated);

    wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()
}

