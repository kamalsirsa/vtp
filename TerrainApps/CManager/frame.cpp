//
// Name:	 frame.cpp
// Purpose:  The frame class for the wxWindows application.
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/NavEngines.h"

#include "xmlhelper/easyxml.hpp"

#include "app.h"
#include "frame.h"
#include "canvas.h"
#include "menu_id.h"
#include "TreeView.h"
#include "PropDlg.h"
#include "ModelDlg.h"
#include "SceneGraphDlg.h"

DECLARE_APP(vtApp)

// Window ids
#define WID_SPLITTER	100
#define WID_FRAME		101
#define WID_MAINVIEW	102
#define WID_SCENEGRAPH	103
#define WID_PROPDLG		104
#define WID_MODELDLG	105
#define WID_SPLITTER2	106

void ItemGroup::CreateNodes()
{
	m_pCage = NULL;
	m_pGroup = new vtGroup();
	m_pLOD = new vtLOD();
	m_pTop = new vtGroup;
	m_pTop->SetName2("ItemGroupTop");
	m_pTop->AddChild(m_pLOD);
	m_pTop->AddChild(m_pGroup);
}

void ItemGroup::AttemptToLoadModels()
{
	int i, num_models = m_pItem->NumModels();

	for (i = 0; i < num_models; i++)
	{
		vtModel *mod = m_pItem->GetModel(i);
		vtTransform *trans = GetMainFrame()->m_nodemap[mod];
		if (!trans && !mod->m_attempted_load)
		{
			// haven't tried to load it yet
			GetMainFrame()->AttemptLoad(mod);
		}
	}
}

void ItemGroup::AttachModels()
{
	// Undo previous attachments
	vtNode *pNode;
	while (pNode = m_pLOD->GetChild(0))
		m_pLOD->RemoveChild(pNode);
	while (pNode = m_pGroup->GetChild(0))
		m_pGroup->RemoveChild(pNode);

	// re-attach
	int i, num_models = m_pItem->NumModels();
	FSphere sph(FPoint3(0,0,0), 1.0f);
	for (i = 0; i < num_models; i++)
	{
		vtModel *mod = m_pItem->GetModel(i);
		vtNode *node = GetMainFrame()->m_nodemap[mod];
		if (node)
		{
			m_pGroup->AddChild(node);
			m_pLOD->AddChild(node);
			node->GetBoundSphere(sph);
		}
	}

	// Update ruler
	if (m_pCage)
	{
		m_pTop->RemoveChild(m_pCage);
		delete m_pCage;
	}
	float size = sph.radius * 1.1f;
	m_pCage = Create3DCursor(size, size/40);
	m_pCage->SetName2("Cage");
	m_pTop->AddChild(m_pCage);
}

void ItemGroup::SetRanges()
{
	// Now set the LOD ranges for each model
	int i, num_models = m_pItem->NumModels();
	if (!num_models)
		return;

	// LOD documentation: For N children, you must have N+1 range values.
	// "Note that the last child (n) does not implicitly have a maximum
	//  distance value of infinity.  You must add a n+1'st range value to
	//  specify its maximum distance.  Otherwise, "bad things" will happen."

	m_ranges[0] = 0.0f;
	if (num_models == 1)
		m_ranges[1] = 10000000.0f;
	else
	{
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			m_ranges[i+1] = mod->m_distance;
		}
	}
	m_pLOD->SetRanges(m_ranges, num_models+1);
}

void ItemGroup::ShowLOD(bool bTrue)
{
	m_pLOD->SetEnabled(bTrue);
	m_pGroup->SetEnabled(!bTrue);
	if (bTrue)
	{
		// LOD requires all models to be enabled
		int i, num_models = m_pItem->NumModels();
		for (i = 0; i < num_models; i++)
		{
			vtModel *mod = m_pItem->GetModel(i);
			vtTransform *trans = GetMainFrame()->m_nodemap[mod];
			if (trans)
				trans->SetEnabled(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Splitter window management

void Splitter2::SizeWindows()
{
	int w, h;
	GetClientSize(&w, &h);

	int pos = GetSashPosition();
	if (pos != 0) m_last = pos;
//	if (h > 190)
//		SetSashPosition(h - 180, false);

	wxSplitterWindow::SizeWindows();
}

//////////////////////////////////////////////////////////////////////////
// vtFrame class implementation
//

BEGIN_EVENT_TABLE(vtFrame, wxFrame)
	EVT_CHAR(vtFrame::OnChar)
	EVT_CLOSE(vtFrame::OnClose)
	EVT_MENU(wxID_OPEN, vtFrame::OnOpen)
	EVT_MENU(wxID_SAVE, vtFrame::OnSave)
	EVT_MENU(wxID_EXIT, vtFrame::OnExit)
	EVT_MENU(ID_SCENE_SCENEGRAPH, vtFrame::OnSceneGraph)
	EVT_MENU(ID_TEST_XML, vtFrame::OnTestXML)
	EVT_MENU(ID_SET_DATA_PATH, vtFrame::OnSetDataPath)
	EVT_MENU(ID_ITEM_NEW, vtFrame::OnItemNew)
	EVT_MENU(ID_ITEM_DEL, vtFrame::OnItemDelete)
	EVT_MENU(ID_ITEM_ADDMODEL, vtFrame::OnItemAddModel)
	EVT_UPDATE_UI(ID_ITEM_ADDMODEL, vtFrame::OnUpdateItemAddModel)
	EVT_MENU(ID_ITEM_REMOVEMODEL, vtFrame::OnItemRemoveModel)
	EVT_UPDATE_UI(ID_ITEM_REMOVEMODEL, vtFrame::OnUpdateItemRemoveModel)
	EVT_MENU(ID_ITEM_SAVESOG, vtFrame::OnItemSaveSOG)
	EVT_UPDATE_UI(ID_ITEM_SAVESOG, vtFrame::OnUpdateItemSaveSOG)
	EVT_MENU(ID_HELP_ABOUT, vtFrame::OnHelpAbout)
END_EVENT_TABLE()


vtFrame *GetMainFrame()
{
	return (vtFrame *) wxGetApp().GetTopWindow();
}


// My frame constructor
vtFrame::vtFrame(wxFrame *parent, const wxString& title, const wxPoint& pos,
	const wxSize& size, long style) :
	wxFrame(parent, WID_FRAME, title, pos, size, style)
{
	// Give it an icon
	{
		wxString name = _T("cmanager");
		wxIcon icon(name);
		SetIcon(icon);
	}

	ReadINI();

	m_pCurrentModel = NULL;
	m_pCurrentItem = NULL;

	CreateMenus();
	CreateToolbar();
	CreateStatusBar();

	SetDropTarget(new DnDFile());

	// frame icon
	SetIcon(wxICON(cmanager));

	// Make a vtGLCanvas
#ifdef __WXMSW__	// JACS
	int *gl_attrib = NULL;
#else
	int gl_attrib[20] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1, GLX_DEPTH_SIZE, 1,
			GLX_DOUBLEBUFFER, None };
#endif

	// splitter
	m_splitter = new wxSplitterWindow(this, WID_SPLITTER, wxDefaultPosition,
		wxDefaultSize, wxSP_3D /*| wxSP_LIVE_UPDATE*/);
	m_splitter2 = new Splitter2(m_splitter, WID_SPLITTER2, wxDefaultPosition,
		wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);

	m_pTree = new MyTreeCtrl(m_splitter2, ID_TREECTRL,
		wxPoint(0, 0), wxSize(200, 400),
//		wxTR_HAS_BUTTONS |
		wxTR_EDIT_LABELS |
		wxNO_BORDER);
	m_pTree->SetBackgroundColour(*wxLIGHT_GREY);

	m_canvas = new vtGLCanvas(m_splitter, WID_MAINVIEW, wxPoint(0, 0), wxSize(-1, -1),
		0, _T("vtGLCanvas"), gl_attrib);

	m_pSceneGraphDlg = new SceneGraphDlg(this, WID_SCENEGRAPH, _T("Scene Graph"),
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	m_pSceneGraphDlg->SetSize(250, 350);
	m_canvas->SetCurrent();

	m_pPropDlg = new PropDlg(m_splitter2, WID_PROPDLG,
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);

	m_pModelDlg = new ModelDlg(m_splitter2, WID_MODELDLG,
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
	m_pModelDlg->Show(FALSE);
	m_pModelDlg->InitDialog();

	m_splitter->Initialize(m_splitter2);

	////////////////////////
	m_pTree->Show(TRUE);
	m_canvas->Show(TRUE);
	m_splitter->SplitVertically( m_splitter2, m_canvas, 260);

	m_splitter2->SplitHorizontally( m_pTree, m_pPropDlg, 200);
	m_pPropDlg->Show(TRUE);
	m_pPropDlg->InitDialog();

	// Show the frame
	Show(TRUE);

	m_pTree->RefreshTreeItems(this);
}

vtFrame::~vtFrame()
{
	delete m_canvas;
	delete m_pSceneGraphDlg;
}

#define STR_DATAPATH "DataPath"

extern vtString get_line_from_stream(ifstream &input);

void vtFrame::ReadINI()
{
	ifstream input;
	input.open("CManager.ini", ios::in | ios::binary);
	if (!input.is_open())
	{
		input.clear();
		input.open("Enviro.ini", ios::in | ios::binary);
	}
	if (!input.is_open())
	{
		input.clear();
		input.open("../Enviro/Enviro.ini", ios::in | ios::binary);
	}
	if (!input.is_open())
		return;

	char buf[80];
	while (!input.eof())
	{
		if (input.peek() == '\n')
			input.ignore();
		input >> buf;

		// data value should been separated by a tab or space
		int next = input.peek();
		if (next != '\t' && next != ' ')
			continue;
		while (input.peek() == '\t' || input.peek() == ' ')
			input.ignore();

		if (strcmp(buf, STR_DATAPATH) == 0)
		{
			vtString string = get_line_from_stream(input);
			vtString *path = new vtString(string);
			m_DataPaths.Append(path);
		}
	}
}

void vtFrame::CreateMenus()
{
	// Make menus
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_OPEN, _T("Open Content File"), _T("Open"));
	fileMenu->Append(wxID_SAVE, _T("&Save Content File"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_SCENE_SCENEGRAPH, _T("Scene Graph"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_TEST_XML, _T("Test XML"), _T("Test XML"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_SET_DATA_PATH, _T("Set Data Path"));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, _T("E&xit\tEsc"), _T("Exit"));

	wxMenu *itemMenu = new wxMenu;
	itemMenu->Append(ID_ITEM_NEW, _T("New Item"), _T("New Item"));
	itemMenu->Append(ID_ITEM_DEL, _T("Delete Item"), _T("Delete Item"));
	itemMenu->AppendSeparator();
	itemMenu->Append(ID_ITEM_ADDMODEL, _T("Add Model"), _T("Add Model"));
	itemMenu->Append(ID_ITEM_REMOVEMODEL, _T("Remove Model"), _T("Remove Model"));
	itemMenu->AppendSeparator();
	itemMenu->Append(ID_ITEM_SAVESOG, _T("Save Model as SOG"), _T("Save Model as SOG"));

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(ID_HELP_ABOUT, _T("About VTP Content Manager..."));

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(itemMenu, _T("&Item"));
	menuBar->Append(helpMenu, _T("&Help"));
	SetMenuBar(menuBar);
}

void vtFrame::CreateToolbar()
{
	// tool bar
	m_pToolbar = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_DOCKABLE);
	m_pToolbar->SetMargins(2, 2);
	m_pToolbar->SetToolBitmapSize(wxSize(20, 20));

	ADD_TOOL(wxID_OPEN, wxBITMAP(contents_open), _("Select"), false);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_ITEM_NEW, wxBITMAP(item_new), _("Select"), false);
	ADD_TOOL(ID_ITEM_DEL, wxBITMAP(item_rem), _("Select"), false);
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_ITEM_ADDMODEL, wxBITMAP(item_addmodel), _("Select"), false);
	ADD_TOOL(ID_ITEM_REMOVEMODEL, wxBITMAP(item_remmodel), _("Select"), false);

	m_pToolbar->Realize();
}

//
// Utility methods
//

void vtFrame::OnChar(wxKeyEvent& event)
{
	long key = event.KeyCode();

	if (key == 27)
	{
		// Esc: exit application
		m_canvas->m_bRunning = false;
		Destroy();
	}
}

void vtFrame::OnClose(wxCloseEvent &event)
{
	m_canvas->m_bRunning = false;
	delete m_canvas;
	m_canvas = NULL;
	event.Skip();
}


//
// Intercept menu commands
//

void vtFrame::OnOpen(wxCommandEvent& event)
{
	m_canvas->m_bRunning = false;
	wxFileDialog loadFile(NULL, _T("Load Content File"), _T(""), _T(""),
		_T("Content XML Files (*.vtco)|*.vtco|"), wxOPEN);
	loadFile.SetFilterIndex(1);
	if (loadFile.ShowModal() == wxID_OK)
	{
		wxString2 fname = loadFile.GetPath();
		LoadContentsFile(fname);
	}
	m_canvas->m_bRunning = true;
}


void vtFrame::OnSave(wxCommandEvent& event)
{
	m_canvas->m_bRunning = false;
	wxFileDialog loadFile(NULL, _T("Save Content File"), _T(""), _T(""),
		_T("Content XML Files (*.vtco)|*.vtco|"), wxSAVE);
	loadFile.SetFilterIndex(1);
	if (loadFile.ShowModal() == wxID_OK)
	{
		wxString2 fname = loadFile.GetPath();
		SaveContentsFile(fname);
	}
	m_canvas->m_bRunning = true;
}


void vtFrame::LoadContentsFile(const wxString2 &fname)
{
	m_Man.Empty();
	try 
	{
		m_Man.ReadXML(fname);
	}
	catch (xh_io_exception &e)
	{
		string str = e.getFormattedMessage();
		DisplayMessageBox(str);
		return;
	}
	SetCurrentItem(NULL);
	SetCurrentModel(NULL);
	m_pTree->RefreshTreeItems(this);
}

void vtFrame::SaveContentsFile(const wxString2 &fname)
{
	try 
	{
		m_Man.WriteXML(fname);
	}
	catch (xh_io_exception &e)
	{
		string str = e.getFormattedMessage();
		DisplayMessageBox(str);
	}
}

void vtFrame::AddModelFromFile(const wxString2 &fname)
{
	vtModel *nm = AddModel(fname);
	if (nm)
		SetCurrentItemAndModel(m_pCurrentItem, nm);
}

void vtFrame::OnExit(wxCommandEvent& event)
{
	m_canvas->m_bRunning = false;
	Destroy();
}

void vtFrame::OnSceneGraph(wxCommandEvent& event)
{
	m_pSceneGraphDlg->Show(TRUE);
}

void vtFrame::OnItemNew(wxCommandEvent& event)
{
	AddNewItem();
	m_pTree->RefreshTreeItems(this);
}

void vtFrame::OnItemDelete(wxCommandEvent& event)
{
	if (!m_pCurrentItem)
		return;

	m_Man.RemoveItem(m_pCurrentItem);
	SetCurrentItemAndModel(NULL, NULL);

	m_pTree->RefreshTreeItems(this);
}

void vtFrame::OnItemAddModel(wxCommandEvent& event)
{
	wxFileDialog loadFile(NULL, _T("Load 3d Model"), _T(""), _T(""),
		_T("3DS Files (*.3ds)|*.3ds|")
		_T("FLT Files (*.flt)|*.flt|")
		_T("All Files (*.*)|*.*|"), wxOPEN);
	loadFile.SetFilterIndex(0);
	if (loadFile.ShowModal() != wxID_OK)
		return;

	wxString2 fname = loadFile.GetPath();
	AddModelFromFile(fname);
}

void vtFrame::OnUpdateItemAddModel(wxUpdateUIEvent& event)
{
	event.Enable(m_pCurrentItem != NULL);
}

void vtFrame::OnItemRemoveModel(wxCommandEvent& event)
{
	m_pCurrentItem->RemoveModel(m_pCurrentModel);

	// update tree view
	m_pTree->RefreshTreeItems(this);

	// update 3d scene graph
	UpdateItemGroup(m_pCurrentItem);
}

void vtFrame::OnUpdateItemRemoveModel(wxUpdateUIEvent& event)
{
	event.Enable(m_pCurrentItem && m_pCurrentModel);
}

#include "vtlib/core/vtSOG.h"

void vtFrame::OnItemSaveSOG(wxCommandEvent& event)
{
	vtTransform *trans = m_nodemap[m_pCurrentModel];
	if (!trans)
		return;
	vtGeom *geom = dynamic_cast<vtGeom*>(trans->GetChild(0));
	if (!geom)
		return;

	OutputSOG osog;
	FILE *fp = fopen("output.sog", "wb");
	osog.WriteHeader(fp);
	osog.WriteSingleGeometry(fp, geom);
	fclose(fp);
}

void vtFrame::OnUpdateItemSaveSOG(wxUpdateUIEvent& event)
{
	vtTransform *trans;
	vtGeom *geom;

	bool enable = true;
	if (m_pCurrentModel == NULL)
		enable = false;
	if (enable && !(trans = m_nodemap[m_pCurrentModel]))
		enable = false;
	if (enable && !(geom = dynamic_cast<vtGeom*>(trans->GetChild(0))))
		enable = false;
	event.Enable(enable);
}

void vtFrame::OnHelpAbout(wxCommandEvent& event)
{
	m_canvas->m_bRunning = false;	// stop rendering

	wxString str = _T("VTP Content Manager\n\n");
	str += _T("Manages sources of 3d models for the Virtual Terrain Project software.\n\n");
	str += _T("Please read the HTML documentation and license.\n");
	str += _T("Send feedback to: ben@vterrain.org\n\n");
	str += _T("This version was built with the ");
#if VTLIB_DSM
	str += _T("DSM");
#elif VTLIB_OSG
	str += _T("OSG");
#elif VTLIB_SGL
	str += _T("SGL");
#elif VTLIB_SSG
	str += _T("SSG");
#endif
	str += _T(" Library.\n");
	str += _T("Build date: ");
	str += _T(__DATE__);
	wxMessageBox(str, _T("About CManager"));

	m_canvas->m_bRunning = true;	// start rendering again
	m_canvas->Refresh(FALSE);
}

//////////////////////////////////////////////////////////////////////////

void vtFrame::AddNewItem()
{
	vtItem *pItem = new vtItem();
	pItem->m_name = "untitled";
	m_Man.AddItem(pItem);
	SetCurrentItemAndModel(pItem, NULL);
}

vtModel *vtFrame::AddModel(const wxString2 &fname_in)
{
	const char *fname = StartOfFilename(fname_in.mb_str());

	vtString onpath = FindFileOnPaths(m_DataPaths, fname);
	if (onpath == "")
	{
		// Warning!  May not be on the data path.
		wxString2 str;
		str.Printf(_T("That file:\n%hs\ndoes not appear to be on the data")
			_T(" paths:"), fname);
		for (int i = 0; i < m_DataPaths.GetSize(); i++)
		{
			vtString *vts = m_DataPaths[i];
			const char *cpath = (const char *) *vts;
			wxString2 path = cpath;
			str += _T("\n");
			str += path;
		}
		DisplayMessageBox(str);
		return NULL;
	}

	// If there is no item, make a new one.
	if (!m_pCurrentItem)
		AddNewItem();

	vtModel *new_model = new vtModel();
	new_model->m_filename = fname;

	vtNode *node = AttemptLoad(new_model);
	if (!node)
	{
		delete new_model;
		return NULL;
	}

	// add to current item
	m_pCurrentItem->AddModel(new_model);

	// update tree view
	m_pTree->RefreshTreeItems(this);

	// update 3d scene graph
	UpdateItemGroup(m_pCurrentItem);

	return new_model;
}

vtTransform *vtFrame::AttemptLoad(vtModel *model)
{
	model->m_attempted_load = true;

	vtString fullpath = FindFileOnPaths(m_DataPaths, model->m_filename);
	vtNodeBase *pNode = vtLoadModel(fullpath);
	if (!pNode)
	{
		wxString2 str;
		str.Printf(_T("Sorry, couldn't load model from %hs"), (const char *) model->m_filename);
		DisplayMessageBox(str);
		return NULL;
	}

	// Wrap in a transform nodeso that we can scale/rotate the node
	vtTransform *pTrans = new vtTransform();
	pTrans->AddChild(pNode);

	// Add to map of model -> nodes
	m_nodemap[model] = pTrans;

	UpdateTransform(model);

	return pTrans;
}

void vtFrame::SetCurrentItemAndModel(vtItem *item, vtModel *model)
{
	m_splitter2->Unsplit();
	if (item != NULL && model == NULL)
	{
		SetCurrentItem(item);
		SetCurrentModel(model);
		DisplayCurrentItem();
		m_splitter2->SplitHorizontally( m_pTree, m_pPropDlg, m_splitter2->m_last);
		m_pPropDlg->Show(TRUE);
	}
	else if (item != NULL && model != NULL)
	{
		SetCurrentItem(item);
		SetCurrentModel(model);
		m_splitter2->SplitHorizontally( m_pTree, m_pModelDlg, m_splitter2->m_last);
		m_pModelDlg->Show(TRUE);
	}
}

void vtFrame::SetCurrentItem(vtItem *item)
{
	if (item == m_pCurrentItem)
		return;

	if (m_pCurrentItem)
		GetItemGroup(m_pCurrentItem)->m_pTop->SetEnabled(false);

	m_pCurrentItem = item;
	m_pCurrentModel = NULL;

	if (item)
	{
		UpdateItemGroup(item);
		m_pPropDlg->SetCurrentItem(item);
	}
	m_pTree->RefreshTreeStatus(this);

	if (m_pCurrentItem)
		GetItemGroup(m_pCurrentItem)->m_pTop->SetEnabled(true);
}

ItemGroup *vtFrame::GetItemGroup(vtItem *item)
{
	ItemGroup *ig = m_itemmap[item];
	if (!ig)
	{
		ig = new ItemGroup(item);
		m_itemmap[item] = ig;
		ig->CreateNodes();

		vtScene *pScene = vtGetScene();
		vtRoot *pRoot = pScene->GetRoot();
		pRoot->AddChild(ig->m_pTop);
	}
	return ig;
}

void vtFrame::UpdateItemGroup(vtItem *item)
{
	ItemGroup *ig = GetItemGroup(item);
	ig->AttemptToLoadModels();
	ig->AttachModels();
	ig->SetRanges();
}

void vtFrame::ShowItemGroupLOD(bool bTrue)
{
	if (!m_pCurrentItem)
		return;
	ItemGroup *ig = GetItemGroup(m_pCurrentItem);
	if (ig)
		ig->ShowLOD(bTrue);
}

void vtFrame::SetCurrentModel(vtModel *model)
{
	if (model == m_pCurrentModel)
		return;

	// 3d scene graph: turn off previous node
	if (m_pCurrentModel)
	{
		vtTransform *trans = m_nodemap[m_pCurrentModel];
		if (trans)
			trans->SetEnabled(false);
	}
	m_pCurrentModel = model;

	// update properties dialog
	m_pModelDlg->SetCurrentModel(model);

	// update 3d scene graph
	if (model)
		DisplayCurrentModel();

	// update tree view
	m_pTree->RefreshTreeStatus(this);
}

//
// Update 3d scene graph and 3d view
// also attempt to load any models which have not yet been loaded, and put
// the status to the properties dialog
//
void vtFrame::DisplayCurrentModel()
{
	vtTransform *trans = m_nodemap[m_pCurrentModel];
	if (!trans && !m_pCurrentModel->m_attempted_load)
	{
		trans = AttemptLoad(m_pCurrentModel);
	}
	if (trans)
	{
		trans->SetEnabled(true);

		FSphere sph;
		trans->GetBoundSphere(sph);
		wxGetApp().m_pTrackball->SetRadius(sph.radius * 2.0f);
		wxGetApp().m_pTrackball->SetZoomScale(sph.radius);

		vtCamera *pCamera = vtGetScene()->GetCamera();
		pCamera->SetYon(sph.radius * 100.0f);

		m_pModelDlg->SetModelStatus("Good");
	}
	else
	{
		m_pModelDlg->SetModelStatus("Failed to load.");
	}
	ShowItemGroupLOD(false);
}

void vtFrame::DisplayCurrentItem()
{
	ShowItemGroupLOD(true);
}

void vtFrame::RefreshTreeItems()
{
	m_pTree->RefreshTreeItems(this);
}

//////////////////////////////////////////////////////////////////////////

bool DnDFile::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
	size_t nFiles = filenames.GetCount();
	for ( size_t n = 0; n < nFiles; n++ )
	{
		wxString str = filenames[n];
		GetMainFrame()->AddModelFromFile(str);
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

void vtFrame::OnTestXML(wxCommandEvent& event)
{
	vtContentManager Man;
	try {
		Man.ReadXML("content3.vtco");
		Man.WriteXML("content4.vtco");
	}
	catch (xh_io_exception &e)
	{
		string str = e.getFormattedMessage();
		DisplayMessageBox(str);
		return;
	}
}

void vtFrame::OnSetDataPath(wxCommandEvent& event)
{
#if 0
	m_canvas->m_bRunning = false;

	wxDirDialog dlg(this, _T("Please indicate your data directory"), m_strDataPath);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_strDataPath = dlg.GetPath();
#if WIN32
		wxString path_separator = _T("\\");
#else
		wxString path_separator = _T("/");
#endif
		m_strDataPath += path_separator;
		WriteINI();
	}

	m_canvas->m_bRunning = true;
	m_canvas->Refresh(FALSE);
#endif
}

void vtFrame::DisplayMessageBox(const wxString2 &str)
{
	m_canvas->m_bRunning = false;
	wxMessageBox(str);
	m_canvas->m_bRunning = true;
	m_canvas->Refresh(FALSE);
}


void vtFrame::UpdateCurrentModelLOD()
{
	// safety
	if (!m_pCurrentItem)
		return;

	ItemGroup *ig = m_itemmap[m_pCurrentItem];
	if (!ig)
		return;

	ig->SetRanges();
}

void vtFrame::UpdateScale(vtModel *model)
{
	UpdateTransform(model);
	UpdateItemGroup(m_pCurrentItem);
}

void vtFrame::UpdateTransform(vtModel *model)
{
	// scale may occasionally be 0 while the user is typing a new value.
	if (model->m_scale == 0.0f)
		return;

	vtTransform *trans = m_nodemap[model];
	if (!trans)
		return;

	trans->Identity();

	wxString2 fname = model->m_filename;
	if (fname.AfterLast('.').CmpNoCase(_T("3ds")) == 0)
	{
		// Must rotate by 90 degrees for 3DS MAX -> OpenGL
		trans->Rotate2(FPoint3(1.0f, 0.0f, 0.0f), -PID2f);
	}

	trans->Scale3(model->m_scale, model->m_scale, model->m_scale);
}

void vtFrame::RenderingPause()
{
	m_canvas->m_bRunning = false;
}

void vtFrame::RenderingResume()
{
	m_canvas->m_bRunning = true;
}

