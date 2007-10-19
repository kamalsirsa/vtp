//
// EnviroApp.cpp : Defines the class behaviors for the application.
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "vtlib/vtlib.h"
#include "vtdata/vtLog.h"

#include "EnviroApp.h"
#include "SimpleDoc.h"
#include "CameraDlg.h"
#include "EnviroView.h"
#include "EnviroFrame.h"
#include "EnviroGUI.h"	// for GetCurrentTerrain

/////////////////////////////////////////////////////////////////////////////
// EnviroApp

BEGIN_MESSAGE_MAP(EnviroApp, CWinApp)
	//{{AFX_MSG_MAP(EnviroApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//	DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EnviroApp construction

EnviroApp::EnviroApp() : CWinApp()
{
	m_pMainView = NULL;
}

EnviroApp::~EnviroApp()
{
}


/////////////////////////////////////////////////////////////////////////////
// The one and only EnviroApp object

EnviroApp theApp;

/////////////////////////////////////////////////////////////////////////////
// EnviroApp initialization

BOOL EnviroApp::InitInstance()
{
#if WIN32 && defined(_MSC_VER) && _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	AfxEnableControlContainer();

	// Standard initialization
	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Virtual Terrain Project"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	if (!OnInit())
		return FALSE;

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate *pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSimpleDoc),
		RUNTIME_CLASS(EnviroFrame),	// main SDI frame window
		RUNTIME_CLASS(EnviroView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	CDocument *pDoc = ((EnviroFrame *)m_pMainWnd)->GetActiveDocument();
	POSITION pos = pDoc->GetFirstViewPosition();
	m_pMainView = (EnviroView *)(pDoc->GetNextView( pos ));
	m_pMainView->m_bRunning = true;

	m_pMainWnd->UpdateWindow();

	return CWinApp::InitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// EnviroApp methods

bool EnviroApp::OnInit()
{
	if (!g_Options.ReadXML("Enviro.xml"))
		g_Options.ReadINI("Enviro.ini");

	g_App.Startup();	// starts log

	VTLOG("Application framework: MFC\n");

	LoadOptions();

	vtGetScene()->Init();
	g_App.LoadAllTerrainDescriptions();

	//
	// Create and show the Startup Dialog
	//
	g_StartDlg.GetOptionsFrom(g_Options);
	int result = g_StartDlg.DoModal();
	if (result == IDCANCEL)
		return FALSE;

	g_StartDlg.PutOptionsTo(g_Options);
	g_Options.WriteXML();

	g_App.StartControlEngine();

	return TRUE;
}

void EnviroApp::LoadOptions()
{
	char buf1[MAX_PATH];
	SHGetFolderPathA(
		NULL,               // parent window, not used
		CSIDL_APPDATA,
		NULL,               // access token (current user)
		SHGFP_TYPE_CURRENT, // current path, not just default value
		buf1
		);
	vtString AppDataUser = buf1;

	// Read the vt datapaths
	vtStringArray &dp = vtGetDataPath();
	bool bLoadedDataPaths = vtLoadDataPath(AppDataUser, NULL);

	// Now look for the Enviro options file.  There are two supported places for it.
	//  1. In the same directory as the executable.
	//  2. On Windows, in the user's "Application Data" folder.
	vtString OptionsFile = "Enviro.xml";

	bool bFound = FileExists(OptionsFile);
	if (!bFound && AppDataUser != "")
	{
		OptionsFile = AppDataUser + "/Enviro.xml";
		bFound = FileExists(OptionsFile);
	}
	if (bFound)
	{
		g_Options.ReadXML(OptionsFile);
		g_Options.m_strFilename = OptionsFile;
	}
	else
	{
		// Not found anywhere.  Default to current directory.
		g_Options.m_strFilename = "Enviro.xml";
	}

	if (!bLoadedDataPaths)
	{
		if (bFound)
		{
			// We have paths in Enviro.xml, but not in vtp.xml; move them
			dp = g_Options.m_oldDataPaths;
			g_Options.m_oldDataPaths.clear();
		}
		else
		{
			// Set default data path
			dp.push_back(vtString("../Data/"));
		}
		vtSaveDataPath(AppDataUser + "/vtp.xml");
	}

	// Supply the special symbols {appdata} and {appdatacommon}
	for (unsigned int i = 0; i < dp.size(); i++)
	{
		dp[i].Replace("{appdata}", AppDataUser);
		//dp[i].Replace("{appdatacommon}", AppDataCommon);
	}

	VTLOG("Using Datapaths:\n");
	int n = dp.size();
	if (n == 0)
		VTLOG("   none.\n");
	for (int d = 0; d < n; d++)
		VTLOG("   %s\n", (const char *) dp[d]);
	VTLOG1("\n");
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_strDate;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_strDate = _T("");
	//}}AFX_DATA_INIT
	m_strDate = __DATE__;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_STATICDATE, m_strDate);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void EnviroApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL EnviroApp::OnIdle(LONG lCount)
{
	// Important: let the default Idle processing occur first
	while (CWinApp::OnIdle(lCount) == TRUE)
		lCount++;

	// Then we can repaint on idle
	if (m_pMainView)
		m_pMainView->PaintOnIdle();

	return FALSE;	// FALSE no more idle time needed
}

/////////////////////////////////////////////////////////////////////////////
// EnviroApp commands


int EnviroApp::ExitInstance()
{
	VTLOG("App Exit\n");

	g_App.Shutdown();
	vtGetScene()->Shutdown();

	return CWinApp::ExitInstance();
}
