//
// Name: LayerDlg.h
//
// Copyright (c) 2003-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __LayerDlg_H__
#define __LayerDlg_H__

#include "enviro_wdr.h"
#include "vtlib/core/Structure3d.h"

class vtNodeBase;

enum LayerType
{
	LT_UNKNOWN = -1,
	LT_ABSTRACT,
	LT_ROAD,
	LT_STRUCTURE,
	LT_VEG,
//	LT_ELEVATION,	// these aren't layer types in enviro.. yet :)
//	LT_IMAGE,
//	LT_WATER,
//	LT_TRANSIT,
//	LT_UTILITY,
	LAYER_TYPES
};

// WDR: class declarations
class LayerItemData : public wxTreeItemData
{
public:
	LayerItemData(vtStructureArray3d *sa, int index, int item)
	{
		Defaults();
		m_type = LT_STRUCTURE;
		m_sa = sa;
		m_index = index;
		m_item = item;
	}
	LayerItemData(vtAbstractLayer *layer, vtFeatureSet *set)
	{
		Defaults();
		m_type = LT_ABSTRACT;
		m_layer = layer;
		m_fset = set;
	}
	LayerItemData(LayerType type)
	{
		Defaults();
		m_type = type;
	}
	void Defaults()
	{
		m_sa = NULL;
		m_layer = NULL;
		m_fset = NULL;
		m_index = -1;
		m_item = -1;
		last_visible = false;
		shadow_last_visible = false;
	}
	LayerType m_type;
	vtStructureArray3d *m_sa;
	vtAbstractLayer *m_layer;
	vtFeatureSet *m_fset;
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
	~LayerDlg();
	
	// WDR: method declarations for LayerDlg
	wxButton* GetLayerRemove()  { return (wxButton*) FindWindow( ID_LAYER_REMOVE ); }
	wxButton* GetLayerCreate()  { return (wxButton*) FindWindow( ID_LAYER_CREATE ); }
	wxButton* GetLayerSaveAs()  { return (wxButton*) FindWindow( ID_LAYER_SAVE_AS ); }
	wxButton* GetLayerSave()  { return (wxButton*) FindWindow( ID_LAYER_SAVE ); }
	wxButton* GetZoomTo()  { return (wxButton*) FindWindow( ID_LAYER_ZOOM_TO ); }
	wxCheckBox* GetVisible()  { return (wxCheckBox*) FindWindow( ID_LAYER_VISIBLE ); }
	wxCheckBox* GetShadow()  { return (wxCheckBox*) FindWindow( ID_SHADOW_VISIBLE ); }
	wxCheckBox* GetShowAll()  { return (wxCheckBox*) FindWindow( ID_SHOW_ALL ); }
	wxTreeCtrl *GetTree()  { return (wxTreeCtrl*) FindWindow( ID_LAYER_TREE ); }
	void RefreshTreeContents();
	void RefreshTreeTerrain();
	void UpdateTreeTerrain();
	void RefreshTreeSpace();

	void SetShowAll(bool bTrue);
	void UpdateEnabling();

private:
	// WDR: member variable declarations for LayerDlg
	wxTreeCtrl *m_pTree;
	wxTreeItemId m_root;
	wxTreeItemId m_item;
	wxImageList *m_imageListNormal;
	bool	m_bShowAll;

private:
	vtNode *GetNodeFromItem(wxTreeItemId item, bool bContainer = false);
	vtStructureArray3d *GetStructureArray3dFromItem(wxTreeItemId item);
	LayerItemData *GetLayerDataFromItem(wxTreeItemId item);
	void ToggleVisible(bool bVis, wxTreeItemId id);
	void CreateImageList(int size = 16);

	// WDR: handler declarations for LayerDlg
	void OnLayerRemove( wxCommandEvent &event );
	void OnLayerCreate( wxCommandEvent &event );
	void OnLayerSave( wxCommandEvent &event );
	void OnLayerSaveAs( wxCommandEvent &event );
	void OnZoomTo( wxCommandEvent &event );
	void OnVisible( wxCommandEvent &event );
	void OnShadowVisible( wxCommandEvent &event );
	void OnShowAll( wxCommandEvent &event );
	void OnSelChanged( wxTreeEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __LayerDlg_H__
