// FenceDlg.cpp : implementation file
//

#include "StdAfx.h"

#include "vtlib/vtlib.h"

#include "EnviroGUI.h"

#include "EnviroApp.h"
#include "FenceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HEIGHT_MIN	1.0f
#define HEIGHT_MAX	6.0f
#define SPACING_MIN	1.0f
#define SPACING_MAX	4.0f

/////////////////////////////////////////////////////////////////////////////
// CFenceDlg dialog


CFenceDlg::CFenceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFenceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFenceDlg)
	m_iType = -1;
	m_fHeight = 0.0f;
	m_fSpacing = 0.0f;
	m_iHeight = 0;
	m_iSpacing = 0;
	//}}AFX_DATA_INIT
}


void CFenceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFenceDlg)
	DDX_Control(pDX, IDC_FENCETYPE, m_cbType);
	DDX_CBIndex(pDX, IDC_FENCETYPE, m_iType);
	DDX_Text(pDX, IDC_HEIGHTEDIT, m_fHeight);
	DDX_Text(pDX, IDC_SPACINGEDIT, m_fSpacing);
	DDX_Slider(pDX, IDC_HEIGHTSLIDER, m_iHeight);
	DDX_Slider(pDX, IDC_SPACINGSLIDER, m_iSpacing);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFenceDlg, CDialog)
	//{{AFX_MSG_MAP(CFenceDlg)
	ON_CBN_SELCHANGE(IDC_FENCETYPE, OnSelchangeFencetype)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_HEIGHTEDIT, OnChangeHeightedit)
	ON_EN_CHANGE(IDC_SPACINGEDIT, OnChangeSpacingedit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFenceDlg message handlers

BOOL CFenceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_iType = 0;
	m_fHeight = FENCE_DEFAULT_HEIGHT;
	m_fSpacing = FENCE_DEFAULT_SPACING;

	ValuesToSliders();

	m_cbType.ResetContent();
	int index;

	index = m_cbType.AddString("Wooden posts, 3 wires");
	m_cbType.SetItemData(index, FT_WIRE);

	index = m_cbType.AddString("Metal poles, chain-link");
	m_cbType.SetItemData(index, FT_CHAINLINK);

	UpdateData(FALSE);

	m_fencetype = (FenceType) m_cbType.GetItemData(m_iType);
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CFenceDlg::OnSelchangeFencetype() 
{
	UpdateData(TRUE);
	m_fencetype = (FenceType) m_cbType.GetItemData(m_iType);
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);
}

void CFenceDlg::SlidersToValues()
{
	m_fHeight = HEIGHT_MIN + m_iHeight * (HEIGHT_MAX - HEIGHT_MIN) / 100.0f;
	m_fSpacing = SPACING_MIN + m_iSpacing * (SPACING_MAX - SPACING_MIN) / 100.0f;
}

void CFenceDlg::ValuesToSliders()
{
	m_iHeight = (int) ((m_fHeight - HEIGHT_MIN) / (HEIGHT_MAX - HEIGHT_MIN) * 100.0f);
	m_iSpacing = (int) ((m_fSpacing - SPACING_MIN) / (SPACING_MAX - SPACING_MIN) * 100.0f);
}


void CFenceDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	UpdateData(TRUE);
	SlidersToValues();
	UpdateData(FALSE);
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);
}


void CFenceDlg::OnChangeHeightedit() 
{
	UpdateData(TRUE);
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);
}

void CFenceDlg::OnChangeSpacingedit() 
{
	UpdateData(TRUE);
	g_App.SetFenceOptions(m_fencetype, m_fHeight, m_fSpacing);
}
