// ChooseDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Charset.h"

#include "vtlib/vtlib.h"
#include "EnviroApp.h"
#include "ChooseDlg.h"
#include "vtlib/core/Terrain.h"
#include "vtlib/core/TerrainScene.h"
#include "../Enviro.h"	// for GetTerrainScene

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseDlg dialog


CChooseDlg::CChooseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseDlg)
	//}}AFX_DATA_INIT
}


void CChooseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseDlg)
	DDX_Control(pDX, IDOK, m_cbOK);
	DDX_Control(pDX, IDC_TLIST, m_lbList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseDlg)
	ON_LBN_SELCHANGE(IDC_TLIST, OnSelchangeTlist)
	ON_LBN_DBLCLK(IDC_TLIST, OnDblclkTlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseDlg message handlers

BOOL CChooseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// list each known terrain
	for (unsigned int i = 0; i < vtGetTS()->NumTerrains(); i++)
	{
		vtTerrain *pTerr = vtGetTS()->GetTerrain(i);
		CString str = FromUTF8(pTerr->GetName());
		m_lbList.AddString(str);
	}
	m_lbList.SelectString(-1, m_strTName);

	UpdateEdit();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseDlg::OnOK()
{
	CDialog::OnOK();
}

void CChooseDlg::OnSelchangeTlist()
{
	int sel = m_lbList.GetCurSel();
	if (sel != LB_ERR)
		m_lbList.GetText(sel, m_strTName);
	UpdateEdit();
}

void CChooseDlg::UpdateEdit()
{
	int sel = m_lbList.GetCurSel();
	m_cbOK.EnableWindow(sel != LB_ERR);
}

void CChooseDlg::OnDblclkTlist()
{
	OnSelchangeTlist();
	OnOK();
}
