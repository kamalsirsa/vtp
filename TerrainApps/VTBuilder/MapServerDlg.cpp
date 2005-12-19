//
// Name: MapServerDlg.cpp
//
// Copyright (c) 2003-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "MapServerDlg.h"
#if SUPPORT_HTTP
#include "vtdata/WFSClient.h"
#endif
#include "vtui/Helper.h"	// for progress dialog
#include "FileFilters.h"	// for FSTRING filters
#include "Helper.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// MapServerDlg.cpp
//----------------------------------------------------------------------------

// WDR: event table for MapServerDlg.cpp

BEGIN_EVENT_TABLE(MapServerDlg, AutoDialog)
	EVT_INIT_DIALOG (MapServerDlg::OnInitDialog)
	EVT_COMBOBOX( ID_BASE_URL, MapServerDlg::OnServer )
	EVT_TEXT( ID_BASE_URL, MapServerDlg::OnBaseUrlText )
	EVT_TEXT( ID_WIDTH, MapServerDlg::OnSize )
	EVT_TEXT( ID_HEIGHT, MapServerDlg::OnSize )
	EVT_CHOICE( ID_CHOICE_FORMAT, MapServerDlg::OnFormat )
	EVT_LISTBOX( ID_LIST_LAYERS, MapServerDlg::OnLayer )
	EVT_BUTTON( ID_QUERY_LAYERS, MapServerDlg::OnQueryLayers )
	EVT_RADIOBUTTON( ID_RADIO_CREATE_NEW, MapServerDlg::OnRadio )
	EVT_RADIOBUTTON( ID_RADIO_TO_FILE, MapServerDlg::OnRadio )
	EVT_BUTTON( ID_DOTDOTDOT, MapServerDlg::OnDotDotDot )
	EVT_TEXT( ID_TEXT_TO_FILE, MapServerDlg::OnTextToFile )
END_EVENT_TABLE()

MapServerDlg::MapServerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_bSetting = false;
	m_iXSize = 1024;
	m_iYSize = 1024;
	m_iServer = -1;
	m_iLayer = -1;
	m_iStyle = -1;
	m_iFormat = 0;
	m_bNewLayer = true;
	m_bToFile = false;

	MapServerDialogFunc( this, TRUE );

	AddValidator(ID_RADIO_CREATE_NEW, &m_bNewLayer);
	AddValidator(ID_RADIO_TO_FILE, &m_bToFile);

	AddNumValidator(ID_WIDTH, &m_iXSize);
	AddNumValidator(ID_HEIGHT, &m_iYSize);

	AddValidator(ID_QUERY, &m_strQueryURL);   // query url string
	AddValidator(ID_CHOICE_FORMAT, &m_iFormat);
	AddValidator(ID_TEXT_TO_FILE, &m_strToFile);

	GetListLayers()->Clear();
	GetListLayers()->Append(_T("<none>"));
	GetListLayers()->SetSelection(0);

	GetFormat()->Clear();
	GetFormat()->Append(_T("JPEG"));
	GetFormat()->Append(_T("PNG"));
	GetFormat()->Append(_T("GeoTIFF"));
	GetFormat()->SetSelection(0);
}

void MapServerDlg::SetServerArray(OGCServerArray &array)
{
	m_pServers = &array;

	// default to first server
	if (m_pServers->size() > 0)
		m_iServer = 0;
}

void MapServerDlg::UpdateEnabling()
{
	GetTextToFile()->Enable(m_bToFile);
	GetDotdotdot()->Enable(m_bToFile);
}


// WDR: handler implementations for MapServerDlg.cpp

void MapServerDlg::OnTextToFile( wxCommandEvent &event )
{
	TransferDataFromWindow();
}

void MapServerDlg::OnDotDotDot( wxCommandEvent &event )
{
	wxString filter = _("All Files|*.*");
	if (m_iFormat == 0)
		AddType(filter, FSTRING_JPEG);
	if (m_iFormat == 1)
		AddType(filter, FSTRING_PNG);
	if (m_iFormat == 2)
		AddType(filter, FSTRING_TIF);

	// ask the user for a filename
	wxFileDialog saveFile(NULL, _("Save Imagery"), _T(""), _T(""), filter, wxSAVE);
	saveFile.SetFilterIndex(1);
	bool bResult = (saveFile.ShowModal() == wxID_OK);
	if (!bResult)
		return;

	m_strToFile = saveFile.GetPath();

	// update controls
	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

void MapServerDlg::OnRadio( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdateEnabling();
}

void MapServerDlg::OnQueryLayers( wxCommandEvent &event )
{
#if SUPPORT_HTTP
	OpenProgressDialog(_("Querying server..."), false, this);

	wxString2 val = GetBaseUrl()->GetValue();
	vtString url = val.mb_str();
	vtString msg;
	bool success = GetLayersFromWMS(url, m_pServers->at(m_iServer).m_layers,
		msg, progress_callback);

	CloseProgressDialog();

	if (success)
	{
		UpdateLayerList();
		UpdateLayerDescription();
		UpdateURL();
	}
	else
	{
		wxString2 str = msg;
		wxMessageBox(str);
	}
#endif // SUPPORT_HTTP
}

void MapServerDlg::OnServer( wxCommandEvent &event )
{
	m_iServer = GetBaseUrl()->GetSelection();
	m_pServers->m_iSelected = m_iServer;
	UpdateLayerList();
	UpdateLayerDescription();
	UpdateURL();
}

void MapServerDlg::OnBaseUrlText( wxCommandEvent &event )
{
	TransferDataFromWindow();
	wxString2 urlvalue = GetBaseUrl()->GetValue();
	m_pServers->at(m_iServer).m_url = urlvalue.mb_str();
	UpdateURL();
}

void MapServerDlg::OnLayer( wxCommandEvent &event )
{
	if (m_iServer == -1)
		return;

	OGCLayerArray &layers = m_pServers->at(m_iServer).m_layers;
	int num = layers.size();
	if (num == 0)
		return;

	m_iLayer = GetListLayers()->GetSelection();
	layers.m_iSelected = m_iLayer;
	UpdateLayerDescription();
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
	GetBaseUrl()->Clear();
	int numservers = m_pServers->size();
	for (int i = 0; i < numservers; i++)
	{
		wxString2 str = m_pServers->at(i).m_url;
		GetBaseUrl()->Append(str);
	}
	// If no selection, pick the first server
	if (m_pServers->m_iSelected == -1 && numservers > 0)
		m_pServers->m_iSelected = 0;
	// Keep selection in range
	if (m_pServers->m_iSelected >= numservers)
		m_pServers->m_iSelected = numservers-1;
	// Apply selection
	if (m_pServers->m_iSelected != -1)
		GetBaseUrl()->SetSelection(m_pServers->m_iSelected);
	m_iServer = m_pServers->m_iSelected;

	UpdateEnabling();
	UpdateLayerList();
	UpdateLayerDescription();

	wxWindow::OnInitDialog(event);
}

void MapServerDlg::OnSize( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	UpdateURL();
}

void MapServerDlg::UpdateLayerList()
{
	GetListLayers()->Clear();
	if (m_iServer == -1)
		return;

	OGCLayerArray &layers = m_pServers->at(m_iServer).m_layers;
	int num = layers.size();
	if (num == 0)
	{
		GetListLayers()->Append(_("<none>"));
		m_iLayer = -1;
	}
	else
	{
		for (int i = 0; i < num; i++)
		{
			vtString str;
			vtTagArray *tags = layers[i];
			vtTag *tag = tags->FindTag("Name");
			if (tag)
				GetListLayers()->Append(wxString2(tag->value));
		}
	}
	// If no selection, pick the first layer
	if (layers.m_iSelected == -1 && num > 0)
		layers.m_iSelected = 0;
	// Keep selection in range
	if (layers.m_iSelected >= num)
		layers.m_iSelected = num-1;
	// Apply selection
	if (layers.m_iSelected != -1)
		GetListLayers()->SetSelection(layers.m_iSelected);
	m_iLayer = layers.m_iSelected;
}

void MapServerDlg::UpdateLayerDescription()
{
	wxString2 str;
	GetLayerDesc()->SetValue(str);

	GetLayerDesc()->Clear();
	if (m_iServer == -1 || m_iLayer == -1)
		return;

	vtTag *tag;
	tag = m_pServers->at(m_iServer).m_layers.at(m_iLayer)->FindTag("Title");
	if (tag)
	{
		str += "Title: ";
		str += tag->value;
		str += "\n";
	}
	tag = m_pServers->at(m_iServer).m_layers.at(m_iLayer)->FindTag("Abstract");
	if (tag)
	{
		str += "Abstract: ";
		str += tag->value;
		str += "\n";
	}
	GetLayerDesc()->SetValue(str);
}

void MapServerDlg::UpdateURL()
{
	if (m_iServer == -1)
		return;

	OGCServer &server = m_pServers->at(m_iServer);
	vtString url = server.m_url;
	url += "?REQUEST=GetMap";

	url += "&LAYERS=";  // required, even if left blank
	if (m_iLayer != -1)
	{
		vtTagArray *layer = server.m_layers.at(m_iLayer);
		vtString layername = layer->GetValueString("Name");
		url += layername;

		url += "&STYLES=";  // required, even if left blank
		if (m_iStyle != -1)
		{
			// TODO
		}
	}
	url += "&SRS=EPSG:4326";	// 4326 = WGS84

	vtString str;
	str.Format("&BBOX=%lf,%lf,%lf,%lf", m_area.left, m_area.bottom, m_area.right, m_area.top);
	url += str;

	str.Format("&WIDTH=%d&HEIGHT=%d", m_iXSize, m_iYSize);
	url += str;
	if (m_iFormat == 0) url += "&FORMAT=image/jpeg";
	if (m_iFormat == 1) url += "&FORMAT=image/png";
	if (m_iFormat == 2) url += "&FORMAT=image/geotiff";

	url += "&TRANSPARENT=TRUE&EXCEPTIONS=WMS_XML&";

	m_strQueryURL = url;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;
}

