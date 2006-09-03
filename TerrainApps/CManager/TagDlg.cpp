//
// Name:     TagDlg.cpp
//
// Copyright (c) 2002-2006 Virtual Terrain Project
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

	GetTagName()->Clear();

	char buf[80];
	FILE *fp = fopen("tags.txt", "rb");
	if (fp)
	{
		while (fgets(buf, 80, fp))
		{
			// string EOL
			if (buf[strlen(buf)-1] == 10) buf[strlen(buf)-1] = 0;
			if (buf[strlen(buf)-1] == 13) buf[strlen(buf)-1] = 0;
			GetTagName()->Append(wxString2(buf));
		}
		fclose(fp);
	}
	GetTagName()->SetSelection(0);
	GetTagText()->Clear();
}

// WDR: handler implementations for TagDlg

void TagDlg::OnInitDialog(wxInitDialogEvent& event)
{
	wxDialog::OnInitDialog(event);
}


