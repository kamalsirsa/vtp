//
// Name: ImportStructDlg.h
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ImportStructDlg_H__
#define __ImportStructDlg_H__

#ifdef __GNUG__
	#pragma interface "ImportStructDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"

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
	wxChoice* GetChoiceFileField()  { return (wxChoice*) FindWindow( ID_CHOICE_FILE_FIELD ); }
	wxRadioButton* GetTypeInstance()  { return (wxRadioButton*) FindWindow( ID_TYPE_INSTANCE ); }
	wxRadioButton* GetTypeLinear()  { return (wxRadioButton*) FindWindow( ID_TYPE_LINEAR ); }
	wxRadioButton* GetTypeFootprint()  { return (wxRadioButton*) FindWindow( ID_TYPE_FOOTPRINT ); }
	wxRadioButton* GetTypeCenter()  { return (wxRadioButton*) FindWindow( ID_TYPE_CENTER ); }
	void SetFileName(const char *str) { m_filename = str; }

	int		m_iType;
	bool	m_bFlip;
	bool	m_bInsideOnly;

private:
	// WDR: member variable declarations for ImportStructDlg
	wxString m_filename;

private:
	// WDR: handler declarations for ImportStructDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
