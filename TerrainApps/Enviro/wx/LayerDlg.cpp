//
// Name: LayerDlg.cpp
//
// Copyright (c) 2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "LayerDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "LayerDlg.h"

#include "vtlib/vtlib.h"
#include "vtlib/core/Terrain.h"
#include "vtui/wxString2.h"
#include "../Enviro.h"	// for GetCurrentTerrain

// WDR: class implementations

//----------------------------------------------------------------------------
// LayerDlg
//----------------------------------------------------------------------------

// WDR: event table for LayerDlg

BEGIN_EVENT_TABLE(LayerDlg,wxDialog)
	EVT_INIT_DIALOG (LayerDlg::OnInitDialog)
	EVT_TREE_SEL_CHANGED( ID_LAYER_TREE, LayerDlg::OnSelChanged )
END_EVENT_TABLE()

LayerDlg::LayerDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function LayerDialogFunc for LayerDlg
	LayerDialogFunc( this, TRUE ); 
}

void LayerDlg::OnInitDialog(wxInitDialogEvent& event) 
{
	m_pTree = GetTree();

	RefreshTreeContents();

	wxWindow::OnInitDialog(event);
}

void LayerDlg::RefreshTreeContents()
{
	// start with a blank slate
	m_pTree->DeleteAllItems();

	vtTerrain *terr = GetCurrentTerrain();
	if (!terr)
		return;

	wxTreeItemId hRoot = m_pTree->AddRoot(_T("Layers"));

	StructureSet &set = terr->GetStructureSet();
	vtStructureArray3d *sa;
	for (int i = 0; i < set.GetSize(); i++)
	{
		wxString2 str;

		sa = set[i];

		str = sa->GetFilename();
		wxTreeItemId hItem = m_pTree->AppendItem(hRoot, str, -1, -1);
		if (sa == terr->GetStructures())
			m_pTree->SetItemBold(hItem, true);
	}
	m_pTree->Expand(hRoot);
}

// WDR: handler implementations for LayerDlg

void LayerDlg::OnSelChanged( wxTreeEvent &event )
{
}

