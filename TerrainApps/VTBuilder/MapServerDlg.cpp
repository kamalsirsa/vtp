//
// Name: MapServerDlg.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "MapServerDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "MapServerDlg.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// MapServerDlg.cpp
//----------------------------------------------------------------------------

// WDR: event table for MapServerDlg.cpp

BEGIN_EVENT_TABLE(MapServerDlg,AutoDialog)
	EVT_COMBOBOX( ID_BASE_URL, MapServerDlg::OnBaseUrlText )
	EVT_TEXT( ID_BASE_URL, MapServerDlg::OnBaseUrlText )
	EVT_TEXT( ID_WIDTH, MapServerDlg::OnSize )
	EVT_TEXT( ID_HEIGHT, MapServerDlg::OnSize )
	EVT_CHOICE( ID_CHOICE_FORMAT, MapServerDlg::OnFormat )
	EVT_CHOICE( ID_CHOICE_LAYERS, MapServerDlg::OnLayer )
	EVT_BUTTON( ID_QUERY_LAYERS, MapServerDlg::OnQueryLayers )
END_EVENT_TABLE()

MapServerDlg::MapServerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_iFormat = 0;
	m_bSetting = false;
	MapServerDialogFunc( this, TRUE ); 
}

// WDR: handler implementations for MapServerDlg.cpp

void MapServerDlg::OnQueryLayers( wxCommandEvent &event )
{
	
}

void MapServerDlg::OnLayer( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateURL();
}

void MapServerDlg::OnFormat( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateURL();
}

void MapServerDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_iXSize = 1024;
	m_iYSize = 1024;

	AddNumValidator(ID_WIDTH, &m_iXSize);
	AddNumValidator(ID_HEIGHT, &m_iYSize);

	AddValidator(ID_QUERY, &m_query);
	AddValidator(ID_CHOICE_FORMAT, &m_iFormat);

	GetBaseUrl()->Append(_("http://wmt.jpl.nasa.gov/cgi-bin/wmt.cgi"));
	GetBaseUrl()->Append(_("http://globe.digitalearth.gov/viz-bin/wmt.cgi"));
	GetBaseUrl()->Append(_("http://grid.cr.usgs.gov/cgi-bin/mapserver/elsalvador"));
	GetBaseUrl()->Append(_("http://www.cubewerx.com/demo/cubeserv/cubeserv.cgi"));
	GetBaseUrl()->Append(_("http://demo.cubewerx.com/demo/cubexplor/cubexplor.cgi"));
	GetBaseUrl()->SetSelection(0);

	GetLayers()->Append(_("<none>"));
	GetLayers()->SetSelection(0);

	GetFormat()->Append(_("JPEG"));
	GetFormat()->Append(_("PNG"));
	GetFormat()->SetSelection(0);

	wxWindow::OnInitDialog(event);
}

void MapServerDlg::OnSize( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateURL();
}

void MapServerDlg::OnBaseUrlText( wxCommandEvent &event )
{
	UpdateURL();
}

void MapServerDlg::UpdateURL()
{
	wxString2 val = GetBaseUrl()->GetValue();

	vtString url = val.mb_str(), str;

	url += "?WMTVER=1.0.0&REQUEST=map";
	url += "&LAYERS=africa";
	url += "&STYLES=&SRS=EPSG:4326";

	str.Format("&BBOX=%lf,%lf,%lf,%lf", m_area.left, m_area.bottom, m_area.right, m_area.top);
	url += str;

	str.Format("&WIDTH=%d&HEIGHT=%d", m_iXSize, m_iYSize);
	url += str;
	if (m_iFormat == 0)
		url += "&FORMAT=JPEG";
	else
		url += "&FORMAT=PNG";
	url += "&TRANSPARENT=TRUE&EXCEPTIONS=WMS_XML&";

	m_query = url;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

