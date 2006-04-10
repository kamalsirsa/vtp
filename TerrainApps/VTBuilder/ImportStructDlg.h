//
// Name: ImportStructDlg.h
//
// Copyright (c) 2003-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ImportStructDlg_H__
#define __ImportStructDlg_H__

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "vtdata/StructArray.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ImportStructDlg
//----------------------------------------------------------------------------

class ImportStructDlg: public AutoDialog
{
public:
	// constructors and destructors
	ImportStructDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for ImportStructDlg
	wxChoice* GetChoiceRoofField()  { return (wxChoice*) FindWindow( ID_CHOICE_ROOF_FIELD ); }
	wxChoice* GetChoiceRoofType()  { return (wxChoice*) FindWindow( ID_CHOICE_ROOF_TYPE ); }
	wxRadioButton* GetRadioRoofField()  { return (wxRadioButton*) FindWindow( ID_RADIO_ROOF_FIELD ); }
	wxRadioButton* GetRadioRoofSingle()  { return (wxRadioButton*) FindWindow( ID_RADIO_ROOF_SINGLE ); }
	wxRadioButton* GetRadioRoofDefault()  { return (wxRadioButton*) FindWindow( ID_RADIO_ROOF_DEFAULT ); }
	wxChoice* GetChoiceHeightType()  { return (wxChoice*) FindWindow( ID_CHOICE_HEIGHT_TYPE ); }
	wxChoice* GetChoiceHeightField()  { return (wxChoice*) FindWindow( ID_CHOICE_HEIGHT_FIELD ); }
	wxChoice* GetChoiceFileField()  { return (wxChoice*) FindWindow( ID_CHOICE_FILE_FIELD ); }
	wxRadioButton* GetTypeInstance()  { return (wxRadioButton*) FindWindow( ID_TYPE_INSTANCE ); }
	wxRadioButton* GetTypeLinear()  { return (wxRadioButton*) FindWindow( ID_TYPE_LINEAR ); }
	wxRadioButton* GetTypeFootprint()  { return (wxRadioButton*) FindWindow( ID_TYPE_FOOTPRINT ); }
	wxRadioButton* GetTypeCenter()  { return (wxRadioButton*) FindWindow( ID_TYPE_CENTER ); }
	void SetFileName(const wxString &str) { m_filename = str; }
	void UpdateEnables();
	bool GetRadio(int id);

	int	m_nShapeType;
	int	m_iType;		// 0 = linear, 1 = instance, 2 = center, 3 = footprint
	int	m_iHeightType;	// 0 = stories, 1 = meters, 2 = feet
	int m_iRoofType;	// 0 = flat, 1 = shed, 2 = gable, 3 = hip
	int m_iRoofMode;	// 0 = default, 1 = single, 2 = from field

public:
	StructImportOptions m_opt;

private:
	// WDR: member variable declarations for ImportStructDlg
	wxString2 m_filename;

private:
	// WDR: handler declarations for ImportStructDlg
	void OnChoiceFileField( wxCommandEvent &event );
	void OnChoiceHeightField( wxCommandEvent &event );
	void OnChoiceHeightType( wxCommandEvent &event );
	void OnChoiceRoofType( wxCommandEvent &event );
	void OnChoiceRoofField( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif

