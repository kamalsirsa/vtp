//
// Name: GeocodeDlg.cpp
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "GeocodeDlg.h"
#include "Vtdata/config_vtdata.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// GeocodeDlg
//----------------------------------------------------------------------------

// WDR: event table for GeocodeDlg

BEGIN_EVENT_TABLE(GeocodeDlg,wxDialog)
	EVT_BUTTON( ID_GET_FILE_DATA, GeocodeDlg::OnGetFileData )
	EVT_BUTTON( ID_GET_FILE_GAZ, GeocodeDlg::OnGetFileGaz )
	EVT_BUTTON( ID_GET_FILE_GNS, GeocodeDlg::OnGetFileGNS )
	EVT_BUTTON( ID_GET_FILE_ZIP, GeocodeDlg::OnGetFileZip )
END_EVENT_TABLE()

GeocodeDlg::GeocodeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function GeocodeDialogFunc for GeocodeDlg
	GeocodeDialogFunc( this, TRUE ); 

	AddValidator(ID_CHECK_USE1, &m_bGeocodeUS);
	AddValidator(ID_CHECK_USE2, &m_bGazetteer);
	AddValidator(ID_CHECK_USE3, &m_bGNS);
	AddValidator(ID_FILE_DATA, &m_strData);
	AddValidator(ID_FILE_GAZ, &m_strGaz);
	AddValidator(ID_FILE_ZIP, &m_strZip);
	AddValidator(ID_FILE_GNS, &m_strGNS);

	// If no web, then no Geocode.us
	GetCheckUse1()->Enable(SUPPORT_HTTP);
}

// WDR: handler implementations for GeocodeDlg

void GeocodeDlg::OnGetFileData( wxCommandEvent &event )
{
	wxFileDialog dlg(this, _T(""), _T(""), _T(""),
		_T("SHP and DBF Files (*.shp,*.dbf)|*.shp;*.dbf|SHP Files (*.shp)|*.shp|DBF Files (*.dbf)|*.dbf"), wxOPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		TransferDataFromWindow();
		m_strData = dlg.GetPath();
		TransferDataToWindow();
	}
}

void GeocodeDlg::OnGetFileGaz( wxCommandEvent &event )
{
	wxFileDialog dlg(this, _T(""), _T(""), _T(""),
		_T("Gazetteer Files (*.txt)|*.txt"), wxOPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		TransferDataFromWindow();
		m_strGaz = dlg.GetPath();
		TransferDataToWindow();
	}
}

void GeocodeDlg::OnGetFileZip( wxCommandEvent &event )
{
	wxFileDialog dlg(this, _T(""), _T(""), _T(""),
		_T("Gazetteer Zipcode Files (*.txt)|*.txt"), wxOPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		TransferDataFromWindow();
		m_strZip = dlg.GetPath();
		TransferDataToWindow();
	}
}

void GeocodeDlg::OnGetFileGNS( wxCommandEvent &event )
{
	wxFileDialog dlg(this, _T(""), _T(""), _T(""),
		_T("VTP GNS Files (*.gcf)|*.gcf"), wxOPEN);
	if (dlg.ShowModal() == wxID_OK)
	{
		TransferDataFromWindow();
		m_strGNS = dlg.GetPath();
		TransferDataToWindow();
	}
}


