//
// Name: ImageMapDlg.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ImageMapDlg_H__
#define __ImageMapDlg_H__

#ifdef __GNUG__
	#pragma interface "ImageMapDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ImageMapDlg
//----------------------------------------------------------------------------

class ImageMapDlg: public AutoDialog
{
public:
	// constructors and destructors
	ImageMapDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for ImageMapDlg
	wxChoice* GetLinkField()  { return (wxChoice*) FindWindow( ID_LINK_FIELD ); }
	void SetFields(class vtFeatureSet *pSet);

public:
	// WDR: member variable declarations for ImageMapDlg
	int m_iField;
	
private:
	// WDR: handler declarations for ImageMapDlg

private:
	DECLARE_EVENT_TABLE()
};




#endif
