//
// Name: ImportPointDlg.h
//
// Copyright (c) 2004-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ImportPointDlg_H__
#define __ImportPointDlg_H__

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "VTBuilder_wdr.h"
#include "vtdata/Projections.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ImportPointDlg
//----------------------------------------------------------------------------

class ImportPointDlg: public AutoDialog
{
public:
	// constructors and destructors
	ImportPointDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	int m_iEasting;
	int m_iNorthing;
	wxString2 m_strCRS;
	vtProjection m_proj;
	bool m_bFormat1;
	bool m_bFormat2;
	bool m_bLongitudeWest;

	void SetCRS(const vtProjection &proj);
	void RefreshProjString();

	// WDR: method declarations for ImportPointDlg
	wxCheckBox* GetLongitudeWest()  { return (wxCheckBox*) FindWindow( ID_LONGITUDE_WEST ); }
	wxButton* GetSetCrs()  { return (wxButton*) FindWindow( ID_SET_CRS ); }
	wxRadioButton* GetFormatDms()  { return (wxRadioButton*) FindWindow( ID_FORMAT_DMS ); }
	wxRadioButton* GetFormatDecimal()  { return (wxRadioButton*) FindWindow( ID_FORMAT_DECIMAL ); }
	wxTextCtrl* GetCrs()  { return (wxTextCtrl*) FindWindow( ID_CRS ); }
	wxChoice* GetNorthing()  { return (wxChoice*) FindWindow( ID_NORTHING ); }
	wxChoice* GetEasting()  { return (wxChoice*) FindWindow( ID_EASTING ); }

private:
	// WDR: member variable declarations for ImportPointDlg
	
private:
	// WDR: handler declarations for ImportPointDlg
	void OnSetCRS( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif
