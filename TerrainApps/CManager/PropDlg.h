//
// Name: PropDlg.h
//
// Copyright (c) 2004-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __PropDlg_H__
#define __PropDlg_H__

#include "cmanager_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"

class vtItem;
class vtModel;

// WDR: class declarations

//----------------------------------------------------------------------------
// PropDlg
//----------------------------------------------------------------------------

class PropDlg: public AutoPanel
{
public:
	// constructors and destructors
	PropDlg( wxWindow *parent, wxWindowID id = -1,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL | wxNO_BORDER );
	
	// WDR: method declarations for PropDlg
	wxListCtrl* GetTaglist()  { return (wxListCtrl*) FindWindow( ID_TAGLIST ); }
	wxChoice* GetTypeChoice()  { return (wxChoice*) FindWindow( ID_TYPECHOICE ); }
	void SetCurrentItem(vtItem *item);
	void UpdateFromControls();
	void UpdateTagList();

private:
	// WDR: member variable declarations for PropDlg
	wxString2	m_strItem;
	wxString2	m_strType;
	vtItem	  *m_pCurrentItem;
	wxListCtrl  *m_pTagList;
	wxChoice	*m_pTypeChoice;
	bool		m_bUpdating;

private:
	// WDR: handler declarations for PropDlg
	void OnTagEdit( wxCommandEvent &event );
	void OnRemoveTag( wxCommandEvent &event );
	void OnAddTag( wxCommandEvent &event );
	void OnChoiceType( wxCommandEvent &event );
	void OnTextItem( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
