//
// Name:		LocationDlg.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LocationDlg_H__
#define __LocationDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "LocationDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtlib/core/Location.h"

class vtLocationSaver;

// WDR: class declarations

//----------------------------------------------------------------------------
// LocationDlg
//----------------------------------------------------------------------------

class LocationDlg: public wxDialog
{
public:
	// constructors and destructors
	LocationDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~LocationDlg();

	// WDR: method declarations for LocationDlg
	wxButton* GetStoreas()  { return (wxButton*) FindWindow( ID_STOREAS ); }
	wxButton* GetStore()  { return (wxButton*) FindWindow( ID_STORE ); }
	wxButton* GetRecall()  { return (wxButton*) FindWindow( ID_RECALL ); }
	wxListBox* GetLoclist()  { return (wxListBox*) FindWindow( ID_LOCLIST ); }
	wxButton* GetRemove()  { return (wxButton*) FindWindow( ID_REMOVE ); }
	void RefreshList();
	void SetTarget(vtTransform *pTarget, const vtProjection &proj,
				   const vtLocalConversion &conv);
	void SetLocFile(const vtString &fname);
	void RefreshButtons();
	void RecallFrom(const vtString &locname);

private:
	// WDR: member variable declarations for LocationDlg
	vtLocationSaver *m_pSaver;

	wxButton* m_pStoreAs;
	wxButton* m_pStore;
	wxButton* m_pRecall;
	wxButton* m_pRemove;
	wxListBox* m_pLocList;
   
private:
	// WDR: handler declarations for LocationDlg
	void OnRemove( wxCommandEvent &event );
	void OnListDblClick( wxCommandEvent &event );
	void OnLoad( wxCommandEvent &event );
	void OnSave( wxCommandEvent &event );
	void OnStoreAs( wxCommandEvent &event );
	void OnStore( wxCommandEvent &event );
	void OnRecall( wxCommandEvent &event );
	void OnLocList( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif

