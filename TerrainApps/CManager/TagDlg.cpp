/////////////////////////////////////////////////////////////////////////////
// Name:	TagDlg.cpp
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
	#pragma implementation "TagDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "TagDlg.h"
#include <wx/valgen.h>

// WDR: class implementations

//----------------------------------------------------------------------------
// TagDlg
//----------------------------------------------------------------------------

// WDR: event table for TagDlg

BEGIN_EVENT_TABLE(TagDlg,wxDialog)
EVT_INIT_DIALOG (TagDlg::OnInitDialog)
END_EVENT_TABLE()

TagDlg::TagDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	TagDialogFunc( this, TRUE ); 
}

void TagDlg::OnInitDialog(wxInitDialogEvent& event)
{
	wxChoice *pName;
	wxWindow *pWin;
	wxGenericValidator *gv;

	pName = (wxChoice *) FindWindow(ID_TAGNAME);
	gv = new wxGenericValidator(&m_strName);
	pName->SetValidator(*gv);

	pName->Append("manufacturer");
	pName->Append("model-name");
	pName->Append("model-year");
	pName->Append("url");
	pName->Append("language");
	pName->Append("country");

	pWin = FindWindow(ID_TAGTEXT);
	gv = new wxGenericValidator(&m_strValue);
	pWin->SetValidator(*gv);

	wxDialog::OnInitDialog(event);
}

// WDR: handler implementations for TagDlg




