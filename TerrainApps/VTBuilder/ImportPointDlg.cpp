//
// Name: ImportPointDlg.cpp
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "ImportPointDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "ImportPointDlg.h"
#include "Projection2Dlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// ImportPointDlg
//----------------------------------------------------------------------------

// WDR: event table for ImportPointDlg

BEGIN_EVENT_TABLE(ImportPointDlg,AutoDialog)
	EVT_BUTTON( ID_SET_CRS, ImportPointDlg::OnSetCRS )
END_EVENT_TABLE()

ImportPointDlg::ImportPointDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function ImportPointDialogFunc for ImportPointDlg
	ImportPointDialogFunc( this, TRUE ); 

	m_iEasting = 0;
	m_iNorthing = 0;;
	m_proj.SetProjectionSimple(false, 0, EPSG_DATUM_WGS84);
	m_bFormat1 = true;
	m_bFormat2 = false;
	m_bLongitudeWest = false;

	AddValidator(ID_EASTING, &m_iEasting);
	AddValidator(ID_NORTHING, &m_iNorthing);
	AddValidator(ID_CRS, &m_strCRS);
	AddValidator(ID_FORMAT_DECIMAL, &m_bFormat1);
	AddValidator(ID_FORMAT_DMS, &m_bFormat2);
	AddValidator(ID_LONGITUDE_WEST, &m_bLongitudeWest);

	RefreshProjString();
}

void ImportPointDlg::SetCRS(const vtProjection &proj)
{
	m_proj = proj;
	RefreshProjString();
}

void ImportPointDlg::RefreshProjString()
{
	char *str1;
	m_proj.exportToProj4(&str1);
	m_strCRS = str1;
	OGRFree(str1);

	TransferDataToWindow();
}

// WDR: handler implementations for ImportPointDlg

void ImportPointDlg::OnSetCRS( wxCommandEvent &event )
{
	Projection2Dlg dlg(this, -1, _("Please indicate CRS"));
	dlg.SetProjection(m_proj);

	if (dlg.ShowModal() == wxID_OK)
	{
		dlg.GetProjection(m_proj);
		RefreshProjString();
	}
}

