//
// EnviroApp.cpp : Defines the class behaviors for the application.
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
}

EnviroApp::~EnviroApp()	// try to close ole
{
}


/////////////////////////////////////////////////////////////////////////////
// The one and only EnviroApp object

EnviroApp theApp;

/////////////////////////////////////////////////////////////////////////////
// EnviroApp initialization

BOOL EnviroApp::InitInstance()
{
#if WIN32 && defined(_MSC_VER) && DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	AfxEnableControlContainer();

	// Standard initialization
	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Virtual Terrain Project"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

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

	if (!OnInit())
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
	g_Options.Read("Enviro.ini");

	g_App.Startup();	// starts log

	VTLOG("Application framework: MFC\n");
	g_App.LoadTerrainDescriptions();

	//
	// Create and show the Startup Dialog
	//
	g_StartDlg.GetOptionsFrom(g_Options);
	int result = g_StartDlg.DoModal();
	if (result == IDCANCEL)
		return FALSE;

	g_StartDlg.PutOptionsTo(g_Options);
	g_Options.Write();

	vtGetScene()->Init();
	g_App.StartControlEngine();

	return TRUE;
}

#if 0
void EnviroApp::OnMouse(ISMVec2f pos, uint32 flags)
{
	int foo = 1;

	// Points of Interest
	if (g_App.m_mode == MM_NAVIGATE && m_pHTMLView && (event.button & VT_LEFT) && (event.flags & VT_MF_SHIFT))
	{
		vtPointOfInterest *poi = GetCurrentTerrain()->FindPointOfInterest(g_App.m_EarthPos);
		if (poi)
		{
			vtString url;
			if (poi->m_url[0] == 'h')	// start with "http"
				url = poi->m_url;
			else
			{
				// build a URL containing a local relative path
				char buf[256];
				GetCurrentDirectory(256, buf);
				vtString buf2 = buf;
				url = "file://" + buf2 + poi->m_url;
			}
			m_pHTMLView->Navigate2(url);
		}
	}
	g_App.OnMouse(event);
}
#endif

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

/////////////////////////////////////////////////////////////////////////////
// EnviroApp commands

