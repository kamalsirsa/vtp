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
#include "vtdata/vtLog.h"

#include "xmlhelper/easyxml.hpp"

#include "app.h"
#include "frame.h"
#include "canvas.h"
#include "menu_id.h"
#include "TreeView.h"
#include "PropDlg.h"
#include "ModelDlg.h"
#include "SceneGraphDlg.h"
#include "ItemGroup.h"

DECLARE_APP(vtApp)

// Window ids
#define WID_SPLITTER	100
#define WID_FRAME		101
#define WID_MAINVIEW	102
#define WID_SCENEGRAPH	103
#define WID_PROPDLG		104
#define WID_MODELDLG	105
#define WID_SPLITTER2	106

//////////////////////////////////////////////////////////////////////////
// Splitter window management

void Splitter2::SizeWindows()
{
	if (!bResetting)
	{
		int pos = GetSashPosition();
		if (pos != 0) m_last = pos;
//		int w, h;
//		GetClientSize(&w, &h);
//		if (h > 190)
//			SetSashPosition(h - 180, false);
	}
	wxSplitterWindow::SizeWindows();
}

//////////////////////////////////////////////////////////////////////////
// vtFrame class implementation
//

StringArray vtFrame::m_DataPaths;

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

	EVT_MENU(ID_VIEW_ORIGIN, vtFrame::OnViewOrigin)
	EVT_UPDATE_UI(ID_VIEW_ORIGIN, vtFrame::OnUpdateViewOrigin)
	EVT_MENU(ID_VIEW_RULERS, vtFrame::OnViewRulers)
	EVT_UPDATE_UI(ID_VIEW_RULERS, vtFrame::OnUpdateViewRulers)

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

	m_bShowOrigin = true;
	m_bShowRulers = false;

	CreateMenus();
	CreateToolbar();
	CreateStatusBar();

	SetDropTarget(new DnDFile());

	// frame icon
	SetIcon(wxICON(cmanager));

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

	// We definitely want full color and a 24-bit Z-buffer!
	int gl_attrib[7] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER,
		WX_GL_BUFFER_SIZE, 24, WX_GL_DEPTH_SIZE, 24, 0	};

	// Make a vtGLCanvas
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
	fileMenu->Append(ID_TEST_XML, _T("Test XML"));
	fileMenu->AppendSeparator();
	fileMenu->Append(ID_SET_DATA_PATH, _T("Set Data Path"));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, _T("E&xit\tEsc"), _T("Exit"));

	wxMenu *itemMenu = new wxMenu;
	itemMenu->Append(ID_ITEM_NEW, _T("New Item"));
	itemMenu->Append(ID_ITEM_DEL, _T("Delete Item"));
	itemMenu->AppendSeparator();
	itemMenu->Append(ID_ITEM_ADDMODEL, _T("Add Model"));
	itemMenu->Append(ID_ITEM_REMOVEMODEL, _T("Remove Model"));
	itemMenu->AppendSeparator();
	itemMenu->Append(ID_ITEM_SAVESOG, _T("Save Model as SOG"));

	wxMenu *viewMenu = new wxMenu;
	viewMenu->AppendCheckItem(ID_VIEW_ORIGIN, _T("Show Local Origin"));
	viewMenu->AppendCheckItem(ID_VIEW_RULERS, _T("Show Rulers"));

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(ID_HELP_ABOUT, _T("About VTP Content Manager..."));

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(itemMenu, _T("&Item"));
	menuBar->Append(viewMenu, _T("&View"));
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
	m_pToolbar->AddSeparator();
	ADD_TOOL(ID_VIEW_ORIGIN, wxBITMAP(show_axes), _("Show Axes"), true);
	ADD_TOOL(ID_VIEW_RULERS, wxBITMAP(show_rulers), _("Show Rulers"), true);

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
	VTLOG("Frame OnClose\n");
	if (m_canvas)
	{
		m_canvas->m_bRunning = false;
		delete m_canvas;
		m_canvas = NULL;
	}
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
	VTLOG("LoadContentsFile '%s'\n", fname.mb_str());
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
	VTLOG("SaveContentsFile '%s'\n", fname.mb_str());
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
	VTLOG("AddModelFromFile '%s'\n", fname.mb_str());
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

void vtFrame::UpdateWidgets()
{
	if (!m_pCurrentItem)
		return;
	ItemGroup *ig = m_itemmap[m_pCurrentItem];
	if (ig)
	{
		ig->ShowOrigin(m_bShowOrigin);
		ig->ShowRulers(m_bShowRulers);
	}
}

void vtFrame::OnViewOrigin(wxCommandEvent& event)
{
	m_bShowOrigin = !m_bShowOrigin;
	if (m_bShowOrigin)
		m_bShowRulers = false;
	UpdateWidgets();
}

void vtFrame::OnUpdateViewOrigin(wxUpdateUIEvent& event)
{
	event.Check(m_bShowOrigin);
}

void vtFrame::OnViewRulers(wxCommandEvent& event)
{
	m_bShowRulers = !m_bShowRulers;
	if (m_bShowRulers)
		m_bShowOrigin = false;
	UpdateWidgets();
}

void vtFrame::OnUpdateViewRulers(wxUpdateUIEvent& event)
{
	event.Check(m_bShowRulers);
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
#if 0
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
#else
	// data path code is too complicated, just store absolute paths
	const char *fname = fname_in.mb_str();
#endif

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

	// Wrap in a transform node so that we can scale/rotate the node
	vtTransform *pTrans = new vtTransform();
	pTrans->AddChild(pNode);

	// Add to map of model -> nodes
	m_nodemap[model] = pTrans;

	UpdateTransform(model);

	return pTrans;
}

void vtFrame::SetCurrentItemAndModel(vtItem *item, vtModel *model)
{
	m_splitter2->bResetting = true;
	m_splitter2->Unsplit();
	m_splitter2->bResetting = false;
	if (item != NULL && model == NULL)
	{
		SetCurrentItem(item);
		SetCurrentModel(model);
		DisplayCurrentItem();
		m_splitter2->SplitHorizontally( m_pTree, m_pPropDlg, m_splitter2->m_last);
		m_pPropDlg->Show(TRUE);
		ZoomToCurrentItem();
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
		GetItemGroup(m_pCurrentItem)->GetTop()->SetEnabled(false);

	m_pCurrentItem = item;
	m_pCurrentModel = NULL;

	if (item)
	{
		UpdateItemGroup(item);
		m_pPropDlg->SetCurrentItem(item);
	}
	m_pTree->RefreshTreeStatus(this);

	if (m_pCurrentItem)
		GetItemGroup(m_pCurrentItem)->GetTop()->SetEnabled(true);
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
		pRoot->AddChild(ig->GetTop());
	}
	return ig;
}

void vtFrame::UpdateItemGroup(vtItem *item)
{
	ItemGroup *ig = GetItemGroup(item);
	ig->AttemptToLoadModels();
	ig->AttachModels();
	ig->ShowOrigin(m_bShowOrigin);
	ig->ShowRulers(m_bShowRulers);
	ig->SetRanges();
}

//
// True to show the current item as an LOD'd object
//
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
	{
		DisplayCurrentModel();
		ZoomToCurrentModel();
	}
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
	// show this individual model, not the LOD'd item
	ShowItemGroupLOD(false);

	vtTransform *trans = m_nodemap[m_pCurrentModel];
	if (!trans && !m_pCurrentModel->m_attempted_load)
	{
		trans = AttemptLoad(m_pCurrentModel);
	}
	if (trans)
	{
		trans->SetEnabled(true);
		m_pModelDlg->SetModelStatus("Good");
	}
	else
	{
		m_pModelDlg->SetModelStatus("Failed to load.");
	}
}

void vtFrame::ZoomToCurrentModel()
{
	ZoomToModel(m_pCurrentModel);
}

void vtFrame::ZoomToModel(vtModel *model)
{
	vtTransform *trans = m_nodemap[model];
	if (!trans)
		return;

	vtCamera *pCamera = vtGetScene()->GetCamera();
	float fYon = pCamera->GetFOV();

	FSphere sph;
	trans->GetBoundSphere(sph);

	// consider the origin-center bounding sphere
	float origin_centered = sph.center.Length() + sph.radius;

	// how far back does the camera have to be to see the whole sphere
	float dist = origin_centered / sinf(fYon / 2);

	wxGetApp().m_pTrackball->SetRadius(dist);
	wxGetApp().m_pTrackball->SetZoomScale(sph.radius);

	pCamera->SetYon(sph.radius * 100.0f);
}

void vtFrame::DisplayCurrentItem()
{
	ShowItemGroupLOD(true);
}

void vtFrame::ZoomToCurrentItem()
{
	if (!m_pCurrentItem)
		return;
	if (m_pCurrentItem->NumModels() < 1)
		return;

	vtModel *model = m_pCurrentItem->GetModel(0);
	if (model)
		ZoomToModel(model);
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

