//
// TreeView.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "App.h"
#include "TreeView.h"
#include "MenuEnum.h"	// for TreeTest_Ctrl
#include "Frame.h"
#include "BuilderView.h"

DECLARE_APP(MyApp)

// Under Windows, the icons are in the .rc file; on Unix, they are included
// from .xpm files.
#ifndef __WXMSW__
	#include "building.xpm"
	#include "file1.xpm"
	#include "file2.xpm"
	#include "folder1.xpm"
	#include "folder2.xpm"
	#include "folder3.xpm"

	#include "grid.xpm"
	#include "image.xpm"
	#include "raw.xpm"
	#include "road.xpm"
	#include "veg1.xpm"
	#include "water.xpm"
#endif

/////////////////////////////////////////////////


// MyTreeCtrl implementation
IMPLEMENT_DYNAMIC_CLASS(MyTreeCtrl, wxTreeCtrl)

MyTreeCtrl::MyTreeCtrl(wxWindow *parent, const wxWindowID id,
					   const wxPoint& pos, const wxSize& size,
					   long style)
		  : wxTreeCtrl(parent, id, pos, size, style)
{
	m_reverseSort = false;
	m_imageListNormal = NULL;
	m_bShowPaths = true;

	CreateImageList(16);

	// Add some items to the tree
	RefreshTreeItems(NULL);
}

void MyTreeCtrl::CreateImageList(int size)
{
	delete m_imageListNormal;

	if ( size == -1 )
	{
		m_imageListNormal = NULL;
		return;
	}

	// Make an image list containing small icons
	m_imageListNormal = new wxImageList(size, size, TRUE);

	// should correspond to TreeCtrlIcon_xxx enum
	wxIcon icons[13];
	icons[0] = wxICON(file1);
	icons[1] = wxICON(file2);
	icons[2] = wxICON(folder1);
	icons[3] = wxICON(folder2);
	icons[4] = wxICON(folder3);
	icons[5] = wxICON(building);
	icons[6] = wxICON(road);
	icons[7] = wxICON(grid);
	icons[8] = wxICON(image);
	icons[9] = wxICON(veg1);
	icons[10] = wxICON(water);
	icons[11] = wxICON(transit);
	icons[12] = wxICON(raw);

	int sizeOrig = icons[0].GetWidth();
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
	{
		if ( size == sizeOrig )
			m_imageListNormal->Add(icons[i]);
		else
			m_imageListNormal->Add(wxImage(icons[i]).Rescale(size, size).
									ConvertToBitmap());
	}

	SetImageList(m_imageListNormal);
}

MyTreeCtrl::~MyTreeCtrl()
{
	delete m_imageListNormal;
}

int MyTreeCtrl::OnCompareItems(const wxTreeItemId& item1,
							   const wxTreeItemId& item2)
{
	if ( m_reverseSort )
	{
		// just exchange 1st and 2nd items
		return wxTreeCtrl::OnCompareItems(item2, item1);
	}
	else
	{
		return wxTreeCtrl::OnCompareItems(item1, item2);
	}
}

wxTreeItemId rootId;

wxTreeItemId MyTreeCtrl::AddRootItem(int image, const char *text)
{
	wxTreeItemId id = AppendItem(rootId, text, image);
	SetItemBold(id);
	return id;
}

wxString MyTreeCtrl::MakeItemName(vtLayerPtr lp)
{
	wxString str;
	if (lp->GetModified())
		str += "(*) ";
	wxString fullpath = lp->GetFilename();
	if (!m_bShowPaths)
	{
		if (fullpath.Find('/') != -1)
			fullpath = fullpath.AfterLast('/');
		if (fullpath.Find('\\') != -1)
			fullpath = fullpath.AfterLast('\\');
		if (fullpath.Find(':') != -1)
			fullpath = fullpath.AfterLast(':');
	}
	str += fullpath;
	return str;
}

void MyTreeCtrl::RefreshTreeItems(MainFrame *pFrame)
{
	DeleteAllItems();

	rootId = AddRoot("Layers");
	SetItemBold(rootId);

	int	image, imageSel;

	wxTreeItemId elevId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Grid, "Elevation");
//	wxTreeItemId imageId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Image, "Images");
	wxTreeItemId buildId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Building, "Structures");
	wxTreeItemId roadId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Road, "Roads");
	wxTreeItemId vegId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Veg1, "Vegetation");
	wxTreeItemId waterId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Water, "Water");
#if SUPPORT_TRANSIT
	wxTreeItemId transId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Transit, "Transit");
#endif
	wxTreeItemId rawId =	AddRootItem(MyTreeCtrl::TreeCtrlIcon_Raw, "Raw");

	image = TreeCtrlIcon_File;
	imageSel = TreeCtrlIcon_FileSelected;
	vtLayerPtr lp;
	int iLayers = 0;
	if (pFrame) iLayers = pFrame->m_Layers.GetSize();
	for (int i = 0; i < iLayers; i++)
	{
		lp = pFrame->m_Layers.GetAt(i);

		wxString str = MakeItemName(lp);

		wxTreeItemId hItem;
		switch (lp->GetType())
		{
			case LT_ELEVATION:
				hItem = AppendItem(elevId, str, image, imageSel);
				break;
			case LT_ROAD:
				hItem = AppendItem(roadId, str, image, imageSel);
				break;
			case LT_STRUCTURE:
				hItem = AppendItem(buildId, str, image, imageSel);
				break;
			case LT_VEG:
				hItem = AppendItem(vegId, str, image, imageSel);
				break;
			case LT_WATER:
				hItem = AppendItem(waterId, str, image, imageSel);
				break;
#if SUPPORT_TRANSIT
			case LT_TRANSIT:
				hItem = AppendItem(transId, str, image, imageSel);
				break;
#endif
			case LT_RAW:
				hItem = AppendItem(rawId, str, image, imageSel);
				break;
		}
		SetItemData(hItem, new MyTreeItemData(lp));

		if (lp == pFrame->GetActiveLayer())
			SelectItem(hItem);
	}

	Expand(rootId);
	Expand(elevId);
//	Expand(imageId);
	Expand(roadId);
	Expand(buildId);
	Expand(vegId);
	Expand(waterId);
#if SUPPORT_TRANSIT
	Expand(transId);
#endif
	Expand(rawId);
}

void MyTreeCtrl::RefreshTreeStatus(MainFrame *pFrame)
{
	wxTreeItemId root = GetRootItem();
	wxTreeItemId parent, item;
	long cookie = 0, cookie2 = 1;

	for (parent = GetFirstChild(root, cookie); parent; parent = GetNextChild(root, cookie))
	{
//		wxString str = GetItemText(parent);
		for (item = GetFirstChild(parent, cookie2); item; item = GetNextChild(parent, cookie2))
		{
			MyTreeItemData *data = (MyTreeItemData *)GetItemData(item);
//			wxString str2 = GetItemText(item);
			if (data)
			{
				SetItemText(item, MakeItemName(data->m_pLayer));
				if (data->m_pLayer == pFrame->GetActiveLayer())
					SelectItem(item);
			}
		}
	}
}


// avoid repetition
#define TREE_EVENT_HANDLER(name)			\
void MyTreeCtrl::name(wxTreeEvent& event)	\
{											\
 /*	wxLogMessage(#name); */					\
	event.Skip();							\
}

TREE_EVENT_HANDLER(OnBeginRDrag)
TREE_EVENT_HANDLER(OnDeleteItem)
TREE_EVENT_HANDLER(OnGetInfo)
TREE_EVENT_HANDLER(OnSetInfo)
TREE_EVENT_HANDLER(OnItemExpanded)
TREE_EVENT_HANDLER(OnItemExpanding)
TREE_EVENT_HANDLER(OnItemCollapsed)
TREE_EVENT_HANDLER(OnSelChanging)
TREE_EVENT_HANDLER(OnTreeKeyDown)

#undef TREE_EVENT_HANDLER

void MyTreeCtrl::OnSelChanged(wxTreeEvent& event)
{
	wxTreeItemId item = event.GetItem();
	MyTreeItemData *data = (MyTreeItemData *)GetItemData(item);
	vtLayerPtr lp = NULL;
	if (data)
		lp = data->m_pLayer;

	vtLayerPtr last = GetMainFrame()->GetActiveLayer();
	if (lp != last)
		GetMainFrame()->GetView()->SetActiveLayer(lp);

	LayerType last_ltype = last ? last->GetType() : LT_UNKNOWN;
	if (lp && lp->GetType() != last_ltype)
		GetMainFrame()->RefreshToolbar();
}

void MyTreeCtrl::OnBeginDrag(wxTreeEvent& event)
{
}

void MyTreeCtrl::OnEndDrag(wxTreeEvent& event)
{
}

void MyTreeCtrl::OnBeginLabelEdit(wxTreeEvent& event)
{
#if 0
	wxLogMessage("OnBeginLabelEdit");

	// for testing, prevent this items label editing
	wxTreeItemId itemId = event.GetItem();
	if ( IsTestItem(itemId) )
	{
		wxMessageBox("You can't edit this item.");

		event.Veto();
	}
#endif
}

void MyTreeCtrl::OnEndLabelEdit(wxTreeEvent& event)
{
#if 0
	wxLogMessage("OnEndLabelEdit");

	// don't allow anything except letters in the labels
	if ( !event.GetLabel().IsWord() )
	{
		wxMessageBox("The label should contain only letters.");

		event.Veto();
	}
#endif
}

void MyTreeCtrl::OnItemCollapsing(wxTreeEvent& event)
{
#if 0
	wxLogMessage("OnItemCollapsing");

	// for testing, prevent the user from collapsing the first child folder
	wxTreeItemId itemId = event.GetItem();
	if ( IsTestItem(itemId) )
	{
		wxMessageBox("You can't collapse this item.");

		event.Veto();
	}
#endif
}

void MyTreeCtrl::OnItemActivated(wxTreeEvent& event)
{
#if 0
	// show some info about this item
	wxTreeItemId itemId = event.GetItem();
	MyTreeItemData *item = (MyTreeItemData *)GetItemData(itemId);

	if ( item != NULL )
	{
		item->ShowInfo(this);
	}
	wxLogMessage("OnItemActivated");
#endif
}

void MyTreeCtrl::OnRMouseDClick(wxMouseEvent& event)
{
#if 0
	wxTreeItemId id = HitTest(event.GetPosition());
	if ( !id )
		wxLogMessage("No item under mouse");
	else
	{
		MyTreeItemData *item = (MyTreeItemData *)GetItemData(id);
		if ( item )
			wxLogMessage("Item '%s' under mouse", item->GetDesc());
	}
#endif
}

/////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
	EVT_TREE_BEGIN_DRAG(TreeTest_Ctrl, MyTreeCtrl::OnBeginDrag)
	EVT_TREE_BEGIN_RDRAG(TreeTest_Ctrl, MyTreeCtrl::OnBeginRDrag)
	EVT_TREE_END_DRAG(TreeTest_Ctrl, MyTreeCtrl::OnEndDrag)
	EVT_TREE_BEGIN_LABEL_EDIT(TreeTest_Ctrl, MyTreeCtrl::OnBeginLabelEdit)
	EVT_TREE_END_LABEL_EDIT(TreeTest_Ctrl, MyTreeCtrl::OnEndLabelEdit)
	EVT_TREE_DELETE_ITEM(TreeTest_Ctrl, MyTreeCtrl::OnDeleteItem)
	EVT_TREE_SET_INFO(TreeTest_Ctrl, MyTreeCtrl::OnSetInfo)
	EVT_TREE_ITEM_EXPANDED(TreeTest_Ctrl, MyTreeCtrl::OnItemExpanded)
	EVT_TREE_ITEM_EXPANDING(TreeTest_Ctrl, MyTreeCtrl::OnItemExpanding)
	EVT_TREE_ITEM_COLLAPSED(TreeTest_Ctrl, MyTreeCtrl::OnItemCollapsed)
	EVT_TREE_ITEM_COLLAPSING(TreeTest_Ctrl, MyTreeCtrl::OnItemCollapsing)
	EVT_TREE_SEL_CHANGED(TreeTest_Ctrl, MyTreeCtrl::OnSelChanged)
	EVT_TREE_SEL_CHANGING(TreeTest_Ctrl, MyTreeCtrl::OnSelChanging)
	EVT_TREE_KEY_DOWN(TreeTest_Ctrl, MyTreeCtrl::OnTreeKeyDown)
	EVT_TREE_ITEM_ACTIVATED(TreeTest_Ctrl, MyTreeCtrl::OnItemActivated)
	EVT_RIGHT_DCLICK(MyTreeCtrl::OnRMouseDClick)
END_EVENT_TABLE()

