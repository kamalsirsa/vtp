//
// Name:		BuildingDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __BuildingDlg_H__
#define __BuildingDlg_H__

#ifdef __GNUG__
	#pragma interface "BuildingDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"

class vtBuilding3d;
class vtStructure3d;

// WDR: class declarations

//----------------------------------------------------------------------------
// BuildingDlg
//----------------------------------------------------------------------------

class BuildingDlg: public AutoDialog
{
public:
	// constructors and destructors
	BuildingDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for BuildingDlg
	void Setup(vtStructure3d *str);
	void BuildingDlg::EditColor(int i);

private:
	// WDR: member variable declarations for BuildingDlg
	vtStructure3d	*m_pStructure;
	vtBuilding3d  *m_pBuilding;

	wxColour	m_Color[3];
	int		 m_iStories;
	int		 m_iRoofType;

	wxBitmapButton  *m_pColorBitmapControl[3];
	wxChoice		*m_pcRoofType;

private:
	// WDR: handler declarations for BuildingDlg
	void OnColor1( wxCommandEvent &event );
	void OnColor2( wxCommandEvent &event );
	void OnColor3( wxCommandEvent &event );
	void OnOK( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif
