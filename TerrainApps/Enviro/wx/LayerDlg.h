//
// Name: LayerDlg.h
//
// Copyright (c) 2003-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LayerDlg_H__
#define __LayerDlg_H__

#ifdef __GNUG__
	#pragma interface "LayerDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtlib/core/Structure3d.h"

class vtNodeBase;

// WDR: class declarations
class LayerItemData : public wxTreeItemData
{
public:
	LayerItemData(vtStructureArray3d *sa, int index, int item)
	{
		m_sa = sa;
		m_index = index;
		m_item = item;
	}
	vtStructureArray3d *m_sa;
	int m_index;
	int m_item;
	bool last_visible;
	bool shadow_last_visible;
};



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
	wxButton* GetLayerRemove()  { return (wxButton*) FindWindow( ID_LAYER_REMOVE ); }
	wxButton* GetLayerCreate()  { return (wxButton*) FindWindow( ID_LAYER_CREATE ); }
	wxButton* GetLayerSave()  { return (wxButton*) FindWindow( ID_LAYER_SAVE ); }
	wxButton* GetLayerActivate()  { return (wxButton*) FindWindow( ID_LAYER_ACTIVE ); }
	wxButton* GetZoomTo()  { return (wxButton*) FindWindow( ID_LAYER_ZOOM_TO ); }
	wxCheckBox* GetVisible()  { return (wxCheckBox*) FindWindow( ID_LAYER_VISIBLE ); }
	wxCheckBox* GetShadow()  { return (wxCheckBox*) FindWindow( ID_SHADOW_VISIBLE ); }
	wxCheckBox* GetShowAll()  { return (wxCheckBox*) FindWindow( ID_SHOW_ALL ); }
	wxTreeCtrl *GetTree()  { return (wxTreeCtrl*) FindWindow( ID_LAYER_TREE ); }
	void RefreshTreeContents();
	void RefreshTreeTerrain();
	void RefreshTreeSpace();

	void SetShowAll(bool bTrue);
	void UpdateEnabling();

private:
	// WDR: member variable declarations for LayerDlg
	wxTreeCtrl *m_pTree;
	wxTreeItemId m_item;
	bool	m_bShowAll;

private:
	vtNode *LayerDlg::GetNodeFromItem(wxTreeItemId item);
	vtStructureArray3d *LayerDlg::GetStructureArray3dFromItem(wxTreeItemId item);
	LayerItemData *LayerDlg::GetLayerDataFromItem(wxTreeItemId item);
	void ToggleVisible(bool bVis, wxTreeItemId id);
	// WDR: handler declarations for LayerDlg
	void OnLayerRemove( wxCommandEvent &event );
	void OnLayerCreate( wxCommandEvent &event );
	void OnLayerSave( wxCommandEvent &event );
	void OnLayerActivate( wxCommandEvent &event );
	void OnZoomTo( wxCommandEvent &event );
	void OnVisible( wxCommandEvent &event );
	void OnShadowVisible( wxCommandEvent &event );
	void OnShowAll( wxCommandEvent &event );
	void OnSelChanged( wxTreeEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
