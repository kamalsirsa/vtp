//
// Name:		BuildingDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "BuildingDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/colordlg.h>

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

void BuildingDlg3d::Setup(vtStructure3d *str)
{
	m_pStructure = str;
	m_pBuilding = (vtBuilding3d *) str->GetBuilding();
}

void BuildingDlg3d::OnOK( wxCommandEvent &event )
{
	BuildingDlg::OnOK(event);

	vtTerrain *pTerr = GetCurrentTerrain();
	pTerr->GetStructures().ReConstructStructure(m_pStructure, "roof walls detail");
}

