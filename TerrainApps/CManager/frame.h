//
// Name:		frame.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FRAMEH
#define FRAMEH

#include <wx/dnd.h>
#include <wx/splitter.h>
#include "vtdata/Content.h"
#include <map>

class vtGLCanvas;
class MyTreeCtrl;
class vtNode;
class vtTransform;
class SceneGraphDlg;
class PropDlg;
class ModelDlg;
class vtGroup;
class vtLOD;
class vtGeom;

class Splitter2 : public wxSplitterWindow
{
public:
	Splitter2(wxWindow *parent, wxWindowID id = -1,
			  const wxPoint& pos = wxDefaultPosition,
			  const wxSize& size = wxDefaultSize,
			  long style = wxSP_3D|wxCLIP_CHILDREN,
			  const wxString& name = "splitter") :
		wxSplitterWindow(parent, id, pos, size, style, name) {}
	virtual void SizeWindows();

	int m_last;
};

//////////////////////////////////////////////////////////////////////////
// ItemGroup
//
class ItemGroup
{
public:
	ItemGroup(vtItem *pItem) { m_pItem = pItem; }
	void CreateNodes();
	void AttemptToLoadModels();
	void AttachModels();
	void ShowLOD(bool bTrue);
	void SetRanges();

	vtItem	*m_pItem;
	vtGroup	*m_pTop;
	vtGroup *m_pGroup;
	vtLOD	*m_pLOD;
	vtGeom	*m_pCage;

	float m_ranges[20];		// One LOD distance for each child
};

// some shortcuts
#define ADD_TOOL(id, bmp, tooltip, tog)	 \
	m_pToolbar->AddTool(id, bmp, wxNullBitmap, tog, -1, -1, (wxObject *)0, tooltip, tooltip)

class vtFrame: public wxFrame
{
public:
	vtFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size,
		long style = wxDEFAULT_FRAME_STYLE);
	~vtFrame();

protected:
	void CreateMenus();
	void CreateToolbar();
	void ReadINI();
	void WriteINI();

	// command handlers
	void OnOpen(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnTestXML(wxCommandEvent& event);
	void OnSetDataPath(wxCommandEvent& event);
	void OnItemNew(wxCommandEvent& event);
	void OnItemDelete(wxCommandEvent& event);
	void OnItemAddModel(wxCommandEvent& event);
	void OnItemRemoveModel(wxCommandEvent& event);
	void OnItemSaveSOG(wxCommandEvent& event);
	void OnSceneGraph(wxCommandEvent& event);
	void OnHelpAbout(wxCommandEvent& event);

	void OnUpdateItemAddModel(wxUpdateUIEvent& event);
	void OnUpdateItemRemoveModel(wxUpdateUIEvent& event);
	void OnUpdateItemSaveSOG(wxUpdateUIEvent& event);

	void LoadContentsFile(wxString &fname);
	void SaveContentsFile(wxString &fname);

	void DisplayMessageBox(const char *string);

public:
	vtGLCanvas		*m_canvas;
	wxToolBar		*m_pToolbar;

	wxSplitterWindow *m_splitter;
	Splitter2		 *m_splitter2;
	MyTreeCtrl		*m_pTree;		// left child of splitter

	// Modeless dialogs
	SceneGraphDlg	*m_pSceneGraphDlg;
	PropDlg			*m_pPropDlg;
	ModelDlg		*m_pModelDlg;

public:
	void RenderingPause();
	void RenderingResume();
	void AddModelFromFile(wxString &fname);
	void OnChar(wxKeyEvent& event);

public:
	void		UpdateCurrentModelLOD();
	void		UpdateScale(vtModel *model);
	void		UpdateTransform(vtModel *model);
	void		RefreshTreeItems();

	// Models
	void		SetCurrentItemAndModel(vtItem *item, vtModel *model);
	void		SetCurrentItem(vtItem *item);
	void		SetCurrentModel(vtModel *mod);
	vtModel		*AddModel(wxString &fname);
	vtTransform	*AttemptLoad(vtModel *model);
	ItemGroup	*GetItemGroup(vtItem *item);
	void		UpdateItemGroup(vtItem *item);
	void		ShowItemGroupLOD(bool bTrue);
	void		AddNewItem();
	void		DisplayCurrentModel();
	void		DisplayCurrentItem();

	vtContentManager	m_Man;
	vtItem				*m_pCurrentItem;
	vtModel				*m_pCurrentModel;

	std::map<vtItem *, ItemGroup *> m_itemmap;
	std::map<vtModel *, vtTransform *> m_nodemap;

	vtString	m_strDataPath;

	DECLARE_EVENT_TABLE()
};

class DnDFile : public wxFileDropTarget
{
public:
    virtual bool OnDropFiles(wxCoord x, wxCoord y,
                             const wxArrayString& filenames);
};

// Helper
extern vtFrame *GetMainFrame();

#endif	// FRAMEH

