//
// TreeView.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/treectrl.h"
#include "wx/imaglist.h"
#include "wx/image.h"

#include "Frame.h"
#include "Layer.h"

class MyTreeItemData : public wxTreeItemData
{
public:
	MyTreeItemData(vtLayer *pLayer) { m_pLayer = pLayer; }
	vtLayerPtr m_pLayer;
};

class MyTreeCtrl : public wxTreeCtrl
{
public:
	enum
	{
		TreeCtrlIcon_File,
		TreeCtrlIcon_FileSelected,
		TreeCtrlIcon_Folder,
		TreeCtrlIcon_FolderSelected,
		TreeCtrlIcon_FolderOpened,
		TreeCtrlIcon_Building,
		TreeCtrlIcon_Road,
		TreeCtrlIcon_Grid,
		TreeCtrlIcon_Image,
		TreeCtrlIcon_Veg1,
		TreeCtrlIcon_Water,
		TreeCtrlIcon_Transit,
		TreeCtrlIcon_Raw
	};

	MyTreeCtrl() { }
	MyTreeCtrl(wxWindow *parent, const wxWindowID id,
			   const wxPoint& pos, const wxSize& size,
			   long style);
	virtual ~MyTreeCtrl();

	void SetShowPaths(bool b) { m_bShowPaths = b; }
	bool GetShowPaths() { return m_bShowPaths; }

	void CreateImageList(int size = 16);
	wxString MakeItemName(vtLayerPtr lp);

	wxTreeItemId AddRootItem(int image, const char *text);
	void RefreshTreeItems(MainFrame *pFrame);
	void RefreshTreeStatus(MainFrame *pFrame);

	void DoSortChildren(const wxTreeItemId& item, bool reverse = FALSE)
		{ m_reverseSort = reverse; wxTreeCtrl::SortChildren(item); }
	void DoEnsureVisible() { EnsureVisible(m_lastItem); }

	void DoToggleIcon(const wxTreeItemId& item);

protected:
	void OnBeginDrag(wxTreeEvent& event);
	void OnBeginRDrag(wxTreeEvent& event);
	void OnEndDrag(wxTreeEvent& event);
	void OnBeginLabelEdit(wxTreeEvent& event);
	void OnEndLabelEdit(wxTreeEvent& event);
	void OnDeleteItem(wxTreeEvent& event);
	void OnGetInfo(wxTreeEvent& event);
	void OnSetInfo(wxTreeEvent& event);
	void OnItemExpanded(wxTreeEvent& event);
	void OnItemExpanding(wxTreeEvent& event);
	void OnItemCollapsed(wxTreeEvent& event);
	void OnItemCollapsing(wxTreeEvent& event);
	void OnSelChanged(wxTreeEvent& event);
	void OnSelChanging(wxTreeEvent& event);
	void OnTreeKeyDown(wxTreeEvent& event);
	void OnItemActivated(wxTreeEvent& event);
	void OnRMouseDClick(wxMouseEvent& event);

	virtual int OnCompareItems(const wxTreeItemId& i1, const wxTreeItemId& i2);

	// is this the test item which we use in several event handlers?
	bool IsTestItem(const wxTreeItemId& item)
	{
		// the test item is the first child folder
		return GetParent(item) == GetRootItem() && !GetPrevSibling(item);
	}

private:
	wxImageList *m_imageListNormal;
	bool		 m_reverseSort;			 // flag for OnCompareItems
	wxTreeItemId m_lastItem;				// for OnEnsureVisible()
	bool		 m_bShowPaths;

	// NB: due to an ugly wxMSW hack you _must_ use DECLARE_DYNAMIC_CLASS()
	//	 if you want your overloaded OnCompareItems() to be called.
	//	 OTOH, if you don't want it you may omit the next line - this will
	//	 make default (alphabetical) sorting much faster under wxMSW.
	DECLARE_DYNAMIC_CLASS(MyTreeCtrl)
	DECLARE_EVENT_TABLE()
};

