/////////////////////////////////////////////////////////////////////////////
// Name:        ResampleDlg.cpp
// Author:    XX
// Created:  XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "ResampleDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "ResampleDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ResampleDlg
//----------------------------------------------------------------------------

// WDR: event table for ResampleDlg

BEGIN_EVENT_TABLE(ResampleDlg,AutoDialog)
    EVT_BUTTON( ID_SMALLER, ResampleDlg::OnSmaller )
    EVT_BUTTON( ID_BIGGER, ResampleDlg::OnBigger )
    EVT_CHECKBOX( ID_CONSTRAIN, ResampleDlg::OnConstrain )
    EVT_TEXT( ID_SIZEX, ResampleDlg::OnSizeXY )
    EVT_TEXT( ID_SIZEY, ResampleDlg::OnSizeXY )
    EVT_TEXT( ID_SPACINGX, ResampleDlg::OnSpacingXY )
    EVT_TEXT( ID_SPACINGY, ResampleDlg::OnSpacingXY )
    EVT_RADIOBUTTON( ID_FLOATS, ResampleDlg::OnFloats )
    EVT_RADIOBUTTON( ID_SHORTS, ResampleDlg::OnShorts )
END_EVENT_TABLE()

ResampleDlg::ResampleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    AutoDialog( parent, id, title, position, size, style )
{
    ResampleDialogFunc( this, TRUE );
    m_bSetting = false;
}

void ResampleDlg::OnInitDialog(wxInitDialogEvent& event)
{
    m_power = 8;
    m_bConstraint = false;
    m_fVUnits = 1.0f;

    m_fAreaX = m_area.Width();
    m_fAreaY = m_area.Height();

    // initial value: based on estimate spacing
    m_fSpacingX = m_fEstX;
    m_fSpacingY = m_fEstY;
    m_iSizeX = ((int) (m_fAreaX / m_fSpacingX + 0.5)) + 1;
    m_iSizeY = ((int) (m_fAreaY / m_fSpacingY + 0.5)) + 1;

    // sampling
    AddNumValidator(ID_SPACINGX, &m_fSpacingX);
    AddNumValidator(ID_SPACINGY, &m_fSpacingY);
    AddNumValidator(ID_SIZEX, &m_iSizeX);
    AddNumValidator(ID_SIZEY, &m_iSizeY);
    AddValidator(ID_CONSTRAIN, &m_bConstraint);

    // output grid
    AddValidator(ID_FLOATS, &m_bFloats);
    AddNumValidator(ID_VUNITS, &m_fVUnits);

    // informations
    AddNumValidator(ID_AREAX, &m_fAreaX);
    AddNumValidator(ID_AREAY, &m_fAreaY);

    AddNumValidator(ID_ESTX, &m_fEstX);
    AddNumValidator(ID_ESTY, &m_fEstY);

    m_bSetting = true;
    TransferDataToWindow();
    m_bSetting = false;

    EnableBasedOnConstraint();

	GetShorts()->SetValue(!m_bFloats);
}

void ResampleDlg::RecomputeSize()
{
    if (m_bConstraint)  // powers of 2 + 1
        m_iSizeX = m_iSizeY = (1 << m_power) + 1;

    m_fSpacingX = m_fAreaX / (m_iSizeX - 1);
    m_fSpacingY = m_fAreaY / (m_iSizeY - 1);
}

// WDR: handler implementations for ResampleDlg

void ResampleDlg::OnShorts( wxCommandEvent &event )
{
    GetVUnits()->Enable(true);
}

void ResampleDlg::OnFloats( wxCommandEvent &event )
{
    if (m_bSetting)
        return;

    TransferDataFromWindow();
    m_fVUnits = 1.0f;
    m_bSetting = true;
    TransferDataToWindow();
    m_bSetting = false;
    EnableBasedOnConstraint();
}

void ResampleDlg::OnSpacingXY( wxCommandEvent &event )
{
    if (m_bSetting)
        return;

    TransferDataFromWindow();
    m_iSizeX = (int) (m_fAreaX / m_fSpacingX)+1;
    m_iSizeY = (int) (m_fAreaY / m_fSpacingY)+1;

    m_bSetting = true;
    TransferDataToWindow();
    m_bSetting = false;
}

void ResampleDlg::OnSizeXY( wxCommandEvent &event )
{
    if (m_bSetting)
        return;

    TransferDataFromWindow();
    RecomputeSize();

    m_bSetting = true;
    TransferDataToWindow();
    m_bSetting = false;
}

void ResampleDlg::OnConstrain( wxCommandEvent &event )
{
    if (m_bSetting)
        return;

    TransferDataFromWindow();
    if (m_bConstraint)
    {
        // round up to a value at least as great as the current size
        m_power = 1;
        while (((1 << m_power) + 1) < m_iSizeX ||
               ((1 << m_power) + 1) < m_iSizeY)
            m_power++;
    }
    RecomputeSize();
    EnableBasedOnConstraint();

    m_bSetting = true;
    TransferDataToWindow();
    m_bSetting = false;
}

void ResampleDlg::EnableBasedOnConstraint()
{
    GetSmaller()->Enable(m_bConstraint);
    GetBigger()->Enable(m_bConstraint);
    GetSizeX()->SetEditable(!m_bConstraint);
    GetSizeY()->SetEditable(!m_bConstraint);
    GetSpacingX()->SetEditable(!m_bConstraint);
    GetSpacingY()->SetEditable(!m_bConstraint);
    GetVUnits()->Enable(!m_bFloats);
}

void ResampleDlg::OnBigger( wxCommandEvent &event )
{
    m_power++;
    RecomputeSize();

    m_bSetting = true;
    TransferDataToWindow();
    m_bSetting = false;
}

void ResampleDlg::OnSmaller( wxCommandEvent &event )
{
    m_power--;
    RecomputeSize();

    m_bSetting = true;
    TransferDataToWindow();
    m_bSetting = false;
}

