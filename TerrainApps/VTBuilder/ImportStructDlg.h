//
// Name: ImportStructDlg.h
//
// Copyright (c) 2003-2005 Virtual Terrain Project
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
	wxChoice* GetChoiceHeightType()  { return (wxChoice*) FindWindow( ID_CHOICE_HEIGHT_TYPE ); }
	wxChoice* GetChoiceHeightField()  { return (wxChoice*) FindWindow( ID_CHOICE_HEIGHT_FIELD ); }
	wxCheckBox* GetFlip()  { return (wxCheckBox*) FindWindow( ID_FLIP ); }
	wxChoice* GetChoiceFileField()  { return (wxChoice*) FindWindow( ID_CHOICE_FILE_FIELD ); }
	wxRadioButton* GetTypeInstance()  { return (wxRadioButton*) FindWindow( ID_TYPE_INSTANCE ); }
	wxRadioButton* GetTypeLinear()  { return (wxRadioButton*) FindWindow( ID_TYPE_LINEAR ); }
	wxRadioButton* GetTypeFootprint()  { return (wxRadioButton*) FindWindow( ID_TYPE_FOOTPRINT ); }
	wxRadioButton* GetTypeCenter()  { return (wxRadioButton*) FindWindow( ID_TYPE_CENTER ); }
	void SetFileName(const wxString &str) { m_filename = str; }
	void UpdateEnables();
	bool GetRadio(int id);

	int		m_nShapeType;
	int		m_iType;
	int		m_iHeightType; // 0 = stories, 1 = meters, 2 = feet

public:
	StructImportOptions m_opt;

private:
	// WDR: member variable declarations for ImportStructDlg
	wxString2 m_filename;

private:
	// WDR: handler declarations for ImportStructDlg
	void OnChoiceFileField( wxCommandEvent &event );
	void OnChoiceHeightType( wxCommandEvent &event );
	void OnChoiceHeightField( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif

