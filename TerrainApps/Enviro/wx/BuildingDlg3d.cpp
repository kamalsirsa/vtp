//
// Name:		BuildingDlg3d.cpp
//
// Copyright (c) 2001-2006 Virtual Terrain Project
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
#include "vtlib/core/Fence3d.h"

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
	m_pBuilding3d = bld3d;
	BuildingDlg::Setup(pTerr->GetStructureLayer(), bld3d, pHeightField);
}

void BuildingDlg3d::OnOK( wxCommandEvent &event )
{
	BuildingDlg::OnOK(event);

	vtTerrain *pTerr = GetCurrentTerrain();
	pTerr->GetStructureLayer()->ConstructStructure(m_pBuilding3d);
}

void BuildingDlg3d::EnableRendering(bool bEnable)
{
	EnableContinuousRendering(bEnable);
}

void BuildingDlg3d::Modified()
{
	// When a building is modified, we must reconstruct its 3D geometry
	vtTerrain *pTerr = GetCurrentTerrain();
	vtStructureLayer *slay = pTerr->GetStructureLayer();
	slay->ConstructStructure(m_pBuilding3d);

	// We might also have to rebuild any linear structures around or on it
	DLine2 &foot = m_pBuilding3d->GetFootprint(0);
	for (unsigned int i = 0; i < slay->GetSize(); i++)
	{
		vtFence3d *fen = slay->GetFence(i);
		if (fen)
		{
			bool bInside = false;
			DLine2 &pts = fen->GetFencePoints();
			for (unsigned int j = 0; j < pts.GetSize(); j++)
				if (foot.ContainsPoint(pts[j]))
					bInside = true;
			if (bInside)
				slay->ConstructStructure(fen);
		}
	}
}
