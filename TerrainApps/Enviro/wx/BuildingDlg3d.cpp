//
// Name:		BuildingDlg3d.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "BuildingDlg3d.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "BuildingDlg3d.h"
#include "../Enviro.h"	// for GetCurrentTerrain
#include "vtlib/core/Building3d.h"

extern void EnableContinuousRendering(bool bTrue);

// WDR: class implementations

//----------------------------------------------------------------------------
// BuildingDlg3d
//----------------------------------------------------------------------------

BuildingDlg3d::BuildingDlg3d( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	BuildingDlg( parent, id, title, position, size, style )
{
}

void BuildingDlg3d::Setup(vtBuilding3d *bld3d, vtHeightField3d *pHeightField)
{
	vtTerrain *pTerr = GetCurrentTerrain();
	m_pStructure3d = bld3d;
	BuildingDlg::Setup(pTerr->GetStructures(), bld3d, pHeightField);
}

void BuildingDlg3d::OnOK( wxCommandEvent &event )
{
	BuildingDlg::OnOK(event);

	vtTerrain *pTerr = GetCurrentTerrain();
	pTerr->GetStructures()->ConstructStructure(m_pStructure3d);
}

void BuildingDlg3d::EnableRendering(bool bEnable)
{
	EnableContinuousRendering(bEnable);
}

void BuildingDlg3d::Modified()
{
	vtTerrain *pTerr = GetCurrentTerrain();
	pTerr->GetStructures()->ConstructStructure(m_pStructure3d);
}
