// PlantDlg.cpp : implementation file
//

#include "StdAfx.h"

#include "resource.h"

#include "PlantDlg.h"
#include "vtlib/vtlib.h"
#include "vtlib/core/Trees.h"
#include "../Enviro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlantDlg dialog


CPlantDlg::CPlantDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlantDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlantDlg)
	m_iSpecies = -1;
	m_fSize = 0.0f;
	m_iSize = 0;
	m_fSpacing = 0.0f;
	//}}AFX_DATA_INIT
}


void CPlantDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlantDlg)
	DDX_Control(pDX, IDC_SPECIES, m_cbSpecies);
	DDX_CBIndex(pDX, IDC_SPECIES, m_iSpecies);
	DDX_Text(pDX, IDC_SIZEEDIT, m_fSize);
	DDX_Slider(pDX, IDC_SIZESLIDER, m_iSize);
	DDX_Text(pDX, IDC_SPACINGEDIT, m_fSpacing);
	//}}AFX_DATA_MAP
}

void CPlantDlg::SetPlantList(vtPlantList3d *plants) 
{
	m_pPlantList = plants;

	m_cbSpecies.ResetContent();
	for (int i = 0; i <	plants->NumSpecies(); i++)
	{
		vtPlantSpecies3d *plant = plants->GetSpecies(i);
		int index = m_cbSpecies.AddString(plant->GetCommonName());
		m_cbSpecies.SetItemData(index, i);
	}
	UpdateData(FALSE);
	OnSelchangeSpecies();
}

BEGIN_MESSAGE_MAP(CPlantDlg, CDialog)
	//{{AFX_MSG_MAP(CPlantDlg)
	ON_CBN_SELCHANGE(IDC_SPECIES, OnSelchangeSpecies)
	ON_EN_CHANGE(IDC_SIZEEDIT, OnChangeSizeedit)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_SPACINGEDIT, OnChangeSpacingedit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlantDlg message handlers

BOOL CPlantDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_iSpecies = 0;
	m_fSize = 2.0f;
	m_fSpacing = 4.0f;
	m_iSize = 1;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CPlantDlg::OnSelchangeSpecies() 
{
	UpdateData(TRUE);
	SizeToSizer();
	UpdateData(FALSE);

	int iSpecies = m_cbSpecies.GetItemData(m_iSpecies);
	g_App.SetPlantOptions(iSpecies, m_fSize, m_fSpacing);
}

void CPlantDlg::OnChangeSizeedit() 
{
	OnSelchangeSpecies();
}

void CPlantDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	UpdateData(TRUE);
	SizerToSize();
	UpdateData(FALSE);

	int iSpecies = m_cbSpecies.GetItemData(m_iSpecies);
	g_App.SetPlantOptions(iSpecies, m_fSize, m_fSpacing);
}

void CPlantDlg::SizerToSize()
{
	if (!m_pPlantList)
		return;
	int iSpecies = m_cbSpecies.GetItemData(m_iSpecies);
	vtPlantSpecies *pSpecies = m_pPlantList->GetSpecies(iSpecies);
	m_fSize = m_iSize * pSpecies->GetMaxHeight() / 100.0f;
}

void CPlantDlg::SizeToSizer()
{
	if (!m_pPlantList)
		return;
	int iSpecies = m_cbSpecies.GetItemData(m_iSpecies);
	vtPlantSpecies *pSpecies = m_pPlantList->GetSpecies(iSpecies);
	m_iSize = (int) (m_fSize / pSpecies->GetMaxHeight() * 100.0f);
}


void CPlantDlg::OnChangeSpacingedit() 
{
	OnSelchangeSpecies();
}
