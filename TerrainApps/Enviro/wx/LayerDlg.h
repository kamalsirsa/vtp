//
// Name: LayerDlg.h
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LayerDlg_H__
#define __LayerDlg_H__

#ifdef __GNUG__
	#pragma interface "LayerDlg.cpp"
#endif

#include "enviro_wdr.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// LayerDlg
//----------------------------------------------------------------------------

class LayerDlg: public wxDialog
{
public:
	// constructors and destructors
	LayerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for LayerDlg
	wxTreeCtrl *GetTree()  { return (wxTreeCtrl*) FindWindow( ID_LAYER_TREE ); }
	void RefreshTreeContents();
	void RefreshTreeTerrain();
	void RefreshTreeSpace();

private:
	// WDR: member variable declarations for LayerDlg
	wxTreeCtrl *m_pTree;
	
private:
	// WDR: handler declarations for LayerDlg
	void OnSelChanged( wxTreeEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
