//
// Name:		StyleDlg.h
//
// Copyright (c) 2004 Virtual Terrain Project.
// Free for all uses, see license.txt for details.
//

#ifndef __StyleDlg_H__
#define __StyleDlg_H__

#ifdef __GNUG__
	#pragma interface "StyleDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/Features.h"

class vtTagArray;

// WDR: class declarations

//----------------------------------------------------------------------------
// StyleDlg
//----------------------------------------------------------------------------

class StyleDlg: public AutoDialog
{
public:
	// constructors and destructors
	StyleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	void SetRawLayer(vtStringArray &datapaths, const vtTagArray &Layer);
	void GetRawLayer(vtTagArray &pLayer);

private:
	// WDR: method declarations for StyleDlg
	wxTextCtrl* GetLabelSize()  { return (wxTextCtrl*) FindWindow( ID_LABEL_SIZE ); }
	wxTextCtrl* GetLabelHeight()  { return (wxTextCtrl*) FindWindow( ID_LABEL_HEIGHT ); }
	wxChoice* GetColorField()  { return (wxChoice*) FindWindow( ID_COLOR_FIELD ); }
	wxChoice* GetTextField()  { return (wxChoice*) FindWindow( ID_TEXT_FIELD ); }
	wxCheckBox* GetTextLabels()  { return (wxCheckBox*) FindWindow( ID_TEXT_LABELS ); }
	wxBitmapButton* GetColor()  { return (wxBitmapButton*) FindWindow( ID_COLOR ); }
	wxCheckBox* GetGeometry()  { return (wxCheckBox*) FindWindow( ID_GEOMETRY ); }
	wxTextCtrl* GetFeatureType()  { return (wxTextCtrl*) FindWindow( ID_FEATURE_TYPE ); }

private:
	// WDR: member variable declarations for StyleDlg
	vtString m_strFilename, m_strResolved;

	OGRwkbGeometryType m_type;
	wxString m_strFeatureType;

	bool m_bGeometry;
	RGBi m_Color;

	bool m_bTextLabels;
	int m_iTextField;
	int m_iColorField;
	float m_fLabelHeight;
	float m_fLabelSize;
	
private:
	void RefreshFields();
	void UpdateEnabling();

	// WDR: handler declarations for StyleDlg
	void OnCheck( wxCommandEvent &event );
	void OnColor( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __StyleDlg_H__

