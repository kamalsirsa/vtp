//
// Name:     TagDlg.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "TagDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "TagDlg.h"
#include <wx/valgen.h>

// WDR: class implementations

//----------------------------------------------------------------------------
// TagDlg
//----------------------------------------------------------------------------

// WDR: event table for TagDlg

BEGIN_EVENT_TABLE(TagDlg,AutoDialog)
EVT_INIT_DIALOG (TagDlg::OnInitDialog)
END_EVENT_TABLE()

TagDlg::TagDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	TagDialogFunc( this, TRUE ); 

	AddValidator(ID_TAGNAME, &m_strName);
	AddValidator(ID_TAGTEXT, &m_strValue);
}

// WDR: handler implementations for TagDlg

void TagDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetTagName()->Clear();
	GetTagName()->Append(_T("country"));
	GetTagName()->Append(_T("creator"));
	GetTagName()->Append(_T("language"));
	GetTagName()->Append(_T("manufacturer"));
	GetTagName()->Append(_T("model-name"));
	GetTagName()->Append(_T("model-year"));
	GetTagName()->Append(_T("url"));
	GetTagName()->Append(_T("wire_info"));

	GetTagText()->Clear();

	wxDialog::OnInitDialog(event);
}


