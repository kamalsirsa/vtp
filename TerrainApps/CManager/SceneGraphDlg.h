//
// Name:	SceneGraphDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __SceneGraphDlg_H__
#define __SceneGraphDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "SceneGraphDlg.cpp"
#endif

#include "wx/imaglist.h"
#include "cmanager_wdr.h"

class vtEngine;
class vtNode;

// WDR: class declarations

//----------------------------------------------------------------------------
// SceneGraphDlg
//----------------------------------------------------------------------------

class SceneGraphDlg: public wxDialog
{
public:
	// constructors and destructors
	SceneGraphDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	~SceneGraphDlg();

	void OnInitDialog(wxInitDialogEvent& event);
	wxButton	*m_pZoomTo;
	wxCheckBox  *m_pEnabled;
	wxTreeCtrl  *m_pTree;

	vtEngine *m_pSelectedEngine;
	vtNode *m_pSelectedNode;

	void CreateImageList(int size = 16);
	void RefreshTreeContents();
	void AddNodeItemsRecursively(wxTreeItemId hParentItem,
								 vtNode *pNode, int depth);

	// WDR: method declarations for SceneGraphDlg
	wxButton* GetZoomto()  { return (wxButton*) FindWindow( ID_ZOOMTO ); }
	wxCheckBox* GetEnabled()  { return (wxCheckBox*) FindWindow( ID_ENABLED ); }
	wxTreeCtrl* GetScenetree()  { return (wxTreeCtrl*) FindWindow( ID_SCENETREE ); }

private:
	// WDR: member variable declarations for SceneGraphDlg
	wxImageList *m_imageListNormal;
	bool m_bFirst;

private:
	// WDR: handler declarations for SceneGraphDlg
	void OnRefresh( wxCommandEvent &event );
	void OnZoomTo( wxCommandEvent &event );
	void OnEnabled( wxCommandEvent &event );
	void OnTreeSelChanged( wxTreeEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
