//
// Name: MapServerDlg.h
//
// Copyright (c) 2003-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __MapServerDlg_H__
#define __MapServerDlg_H__

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "vtdata/MathTypes.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// MapServerDlg.cpp
//----------------------------------------------------------------------------

class MapServerDlg: public AutoDialog
{
public:
	// constructors and destructors
	MapServerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for MapServerDlg.cpp
	wxTextCtrl* GetQuery()  { return (wxTextCtrl*) FindWindow( ID_QUERY ); }
	wxComboBox* GetBaseUrl()  { return (wxComboBox*) FindWindow( ID_BASE_URL ); }
	wxChoice* GetLayers()  { return (wxChoice*) FindWindow( ID_CHOICE_LAYERS ); }
	wxChoice* GetFormat()  { return (wxChoice*) FindWindow( ID_CHOICE_FORMAT ); }

	void UpdateURL();

	int m_iXSize;
	int m_iYSize;
	wxString2 m_query;
	DRECT m_area;
	bool m_bSetting;
	int m_iFormat;

private:
	// WDR: member variable declarations for MapServerDlg.cpp
	
private:
	// WDR: handler declarations for MapServerDlg.cpp
	void OnQueryLayers( wxCommandEvent &event );
	void OnLayer( wxCommandEvent &event );
	void OnFormat( wxCommandEvent &event );
	void OnSize( wxCommandEvent &event );
	void OnBaseUrlText( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __MapServerDlg_H__

