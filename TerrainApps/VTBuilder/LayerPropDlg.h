//
// Name: LayerPropDlg.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LayerPropDlg_H__
#define __LayerPropDlg_H__

#ifdef __GNUG__
	#pragma interface "LayerPropDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// LayerPropDlg
//----------------------------------------------------------------------------

class LayerPropDlg: public AutoDialog
{
public:
	// constructors and destructors
	LayerPropDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for LayerPropDlg
	double m_fLeft, m_fTop, m_fRight, m_fBottom;
	wxString m_strText;

private:
	// WDR: member variable declarations for LayerPropDlg
	
private:
	// WDR: handler declarations for LayerPropDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __LayerPropDlg_H__

