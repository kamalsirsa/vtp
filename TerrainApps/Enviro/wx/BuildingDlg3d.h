//
// Name:		BuildingDlg3d.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __BuildingDlg3d_H__
#define __BuildingDlg3d_H__

#ifdef __GNUG__
	#pragma interface "BuildingDlg.cpp"
#endif

#include "vtui/BuildingDlg.h"

//----------------------------------------------------------------------------
// BuildingDlg3d
//----------------------------------------------------------------------------

class BuildingDlg3d: public BuildingDlg
{
public:
	// constructors and destructors
	BuildingDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for BuildingDlg
	void Setup(vtBuilding3d *str);

	// WDR: member variable declarations for BuildingDlg
	vtStructure3d	*m_pStructure3d;
//	vtBuilding3d  *m_pBuilding;

protected:
	// WDR: handler declarations for BuildingDlg
	virtual void OnOK( wxCommandEvent &event );
};

#endif
