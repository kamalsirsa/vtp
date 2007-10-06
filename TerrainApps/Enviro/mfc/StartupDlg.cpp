//
// StartupDlg.cpp : implementation file
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "StdAfx.h"
#include "Charset.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "vtdata/vtLog.h"
#include "StartupDlg.h"
#include "ChooseDlg.h"
#include "CreateDlg.h"
#include "../Enviro.h"	// for GetTerrainScene

#ifdef _MSC_VER
#pragma warning( disable : 4800 )
#endif

CStartupDlg g_StartDlg;

void CStartupDlg::GetOptionsFrom(EnviroOptions &opt)
{
	m_iLaunch = (opt.m_bEarthView ? 0 : 1);
	m_strImage = FromUTF8(opt.m_strEarthImage);
	m_strTName = FromUTF8(opt.m_strInitTerrain);
	m_bFullscreen = opt.m_bFullscreen;
	m_bHtmlpane = opt.m_bHtmlpane;
	m_bFloatingToolbar = opt.m_bFloatingToolbar;
	m_fPlantSize = opt.m_fPlantScale;
	m_bShadows = opt.m_bShadows;
}

void CStartupDlg::PutOptionsTo(EnviroOptions &opt)
{
	opt.m_bEarthView = (m_iLaunch == 0);
	opt.m_strEarthImage = ToUTF8(m_strImage);
	opt.m_strInitTerrain = ToUTF8(m_strTName);
	opt.m_bFullscreen = m_bFullscreen;
	opt.m_bHtmlpane = m_bHtmlpane;
	opt.m_bFloatingToolbar = m_bFloatingToolbar;
	opt.m_fPlantScale = m_fPlantSize;
	opt.m_bShadows = m_bShadows;
}

/////////////////////////////////////////////////////////////////////////////
// CStartupDlg dialog


CStartupDlg::CStartupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStartupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStartupDlg)
	m_bFullscreen = FALSE;
	m_bHtmlpane = FALSE;
	m_bFloatingToolbar = FALSE;
	m_iLaunch = -1;
	m_strTName = _T("");
	m_strImage = _T("");
	m_fPlantSize = 0.0f;
	m_bShadows = FALSE;
	//}}AFX_DATA_INIT
}


void CStartupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStartupDlg)
	DDX_Control(pDX, IDC_IMAGE, m_cbImage);
	DDX_Control(pDX, IDC_STATICIMAGE, m_csImage);
	DDX_Control(pDX, IDC_TSELECT, m_cbTSelect);
	DDX_Control(pDX, IDC_TNAME, m_editTName);
	DDX_Check(pDX, IDC_FULLSCREEN, m_bFullscreen);
	DDX_Check(pDX, IDC_HTMLPANE, m_bHtmlpane);
	DDX_Check(pDX, IDC_TOOLBAR, m_bFloatingToolbar);
	DDX_Radio(pDX, IDC_LAUNCH1, m_iLaunch);
	DDX_Text(pDX, IDC_TNAME, m_strTName);
	DDX_Text(pDX, IDC_IMAGE, m_strImage);
	DDX_Text(pDX, IDC_PLANTSIZE, m_fPlantSize);
	DDX_Check(pDX, IDC_SHADOWS, m_bShadows);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStartupDlg, CDialog)
	//{{AFX_MSG_MAP(CStartupDlg)
	ON_BN_CLICKED(IDC_OPENGL_INFO, OnOpenglInfo)
	ON_BN_CLICKED(IDC_TSELECT, OnTSelect)
	ON_BN_CLICKED(IDC_LAUNCH1, OnChangeLaunch)
	ON_BN_CLICKED(IDC_LAUNCH2, OnChangeLaunch)
	ON_BN_CLICKED(IDC_EDITPROP, OnEditProp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//
// This function replaces CComboBox::Dir, because that method
//  does not support non-8.3 directory names (or file names)
//  in Win9x operating systems.
//
void AddFilesToComboBox(CComboBox *box, CString wildcard)
{
	VTLOG(" AddFilenamesToComboBox:");

	CFileFind finder;
	int entries = 0, matches = 0;
	BOOL bWorking = finder.FindFile(wildcard);

	while (bWorking)
	{
		entries++;
		bWorking = finder.FindNextFile();
		if (!finder.IsDirectory())
		{
			CString str = finder.GetFileName();
			box->AddString(str.Left(str.GetLength()-9));
			matches++;
		}
	}
	VTLOG(" %d entries, %d matches\n", entries, matches);
}

/////////////////////////////////////////////////////////////////////////////
// CStartupDlg message handlers

BOOL CStartupDlg::OnInitDialog()
{
	vtString tname = ToUTF8(m_strTName);
	vtTerrain *pTerr = vtGetTS()->FindTerrainByName(tname);
	if (pTerr)
		m_strTName = FromUTF8(pTerr->GetName());
	else
		m_strTName = "none";

	CDialog::OnInitDialog();

	vtStringArray &paths = vtGetDataPath();
	for (unsigned int i = 0; i < paths.size(); i++)
	{
		CString path = (const char *) paths[i];
		AddFilesToComboBox(&m_cbImage, path + "WholeEarth/*_0106.png");
		AddFilesToComboBox(&m_cbImage, path + "WholeEarth/*_0106.jpg");
	}
	m_cbImage.SelectString(-1, m_strImage);

	UpdateState();

	return TRUE;  // return TRUE unless you set the focus to a control
}

//
// Helper: find the largest texture size supported by OpenGL
//
void ShowOGLInfo(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,						// version number
		PFD_DRAW_TO_WINDOW |	// support window
		PFD_SUPPORT_OPENGL |	// support OpenGL
		PFD_DOUBLEBUFFER,		// double buffered
		PFD_TYPE_RGBA,			// RGBA type
		24,						// 24-bit color depth
		0, 0, 0, 0, 0, 0,		// color bits ignored
		0, 0, 0,				// no alpha buffer
		0, 0, 0, 0,				// accum bits ignored
		32, 0, 0,				// 32-bit z-buffer
		PFD_MAIN_PLANE,			// main layer
		0,						// reserved
		0, 0, 0					// layer masks ignored
	};
	int  iPixelFormat;
	// get the best available match of pixel format for the device context
	iPixelFormat = ChoosePixelFormat(hdc, &pfd);
	// make that the pixel format of the device context
	SetPixelFormat(hdc, iPixelFormat, &pfd);

	HGLRC device = wglCreateContext(hdc);
	if (device == NULL)
	{
		DWORD lasterror = GetLastError();
		// 2000 The pixel format is invalid.  ERROR_INVALID_PIXEL_FORMAT
	}
	wglMakeCurrent(hdc, device);
	GLint value;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
	GLenum error = glGetError();
	if (error == GL_INVALID_OPERATION)
		value = 0;

	CString str;
	str.Format(_T("OpenGL Version: %s\nVendor: %s\nRenderer: %s\nMaximum Texture Dimension: %d\nExtensions: %s"),
		glGetString(GL_VERSION), glGetString(GL_VENDOR),
		glGetString(GL_RENDERER), value, glGetString(GL_EXTENSIONS));
	AfxMessageBox(str);

	wglDeleteContext(device);
}


void CStartupDlg::OnOpenglInfo()
{
	// check the OpenGL max texture size
	HWND hwnd = GetSafeHwnd();
	CDC *pDC = GetDC();
	HDC hdc = pDC->GetSafeHdc();
	ShowOGLInfo(hdc);
}

void CStartupDlg::OnTSelect()
{
	UpdateData(TRUE);
	CChooseDlg dlg;
	dlg.m_strTName = m_strTName;
	if (dlg.DoModal() == IDOK)
		m_strTName = dlg.m_strTName;
	UpdateData(FALSE);
}


void CStartupDlg::OnChangeLaunch()
{
	UpdateData(TRUE);
	UpdateState();
}

void CStartupDlg::UpdateState()
{
	m_csImage.EnableWindow(m_iLaunch == 0);
	m_cbImage.EnableWindow(m_iLaunch == 0);
	m_editTName.EnableWindow(m_iLaunch == 1);
	m_cbTSelect.EnableWindow(m_iLaunch == 1);
}

void CStartupDlg::OnEditProp()
{
	vtString tname = ToUTF8(m_strTName);
	vtTerrain *pTerr = vtGetTS()->FindTerrainByName(tname);
	if (!pTerr)
		return;

	vtString fname = pTerr->GetParamFile();

	TParams Params;
	CCreateDlg dlg;

	if (Params.LoadFrom(fname))
		dlg.SetParams(Params);

	int result = dlg.DoModal();
	if (result == IDOK)
	{
		dlg.GetParams(Params);

		vtString ext = GetExtension(fname, false);
		if (ext.CompareNoCase(".ini") == 0)
		{
			AfxMessageBox(_T("Upgrading the .ini to a .xml file.\nPlease remember to remove the old .ini file."));
			fname = fname.Left(fname.GetLength()-4) + ".xml";
		}

		if (!Params.WriteToXML(fname, STR_TPARAMS_FORMAT_NAME))
		{
			CString str;
			str.Format(_T("Couldn't save to file %s\nPlease make sure the file is not read-only."),
				(const char *)fname);
			::AfxMessageBox(str);
		}
	}
}
