//
// VegDlg.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "VegDlg.h"
#include "Frame.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SpeciesListDlg
//----------------------------------------------------------------------------

// WDR: event table for SpeciesListDlg

BEGIN_EVENT_TABLE(SpeciesListDlg,wxDialog)
	EVT_INIT_DIALOG (SpeciesListDlg::OnInitDialog)
	EVT_BUTTON( wxID_OK, SpeciesListDlg::OnOK )
	EVT_LIST_ITEM_SELECTED( ID_LISTCTRL_SPECIES, SpeciesListDlg::OnSelect )
END_EVENT_TABLE()

SpeciesListDlg::SpeciesListDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function SpeciesListFunc for SpeciesListDlg
	SpeciesListFunc( this, TRUE ); 

	m_idx = 0;
}

// WDR: handler implementations for SpeciesListDlg

void SpeciesListDlg::OnSelect( wxListEvent &event )
{
	// Get index of that item
	m_idx = event.GetIndex();

	// Clear all item data from the Appearance table on right.
	m_PATable->DeleteAllItems();

	RefreshAppeances();
}

void SpeciesListDlg::OnOK( wxCommandEvent &event )
{
	Show(false);
}

void SpeciesListDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_PSplitter = (wxSplitterWindow *) FindWindow( ID_SPLITTER1 );

	// Setup left side with common name and max height per species.
	m_PSTable = (wxListCtrl *) FindWindow( ID_LISTCTRL_SPECIES );
	m_PSTable->ClearAll();
	m_PSTable->SetSingleStyle(wxLC_REPORT);
	m_PSTable->InsertColumn(0, _T("SID"));
	m_PSTable->SetColumnWidth(0, 34);
	m_PSTable->InsertColumn(1, _T("Common Name"));
	m_PSTable->SetColumnWidth(1, 115);
	m_PSTable->InsertColumn(2, _T("Scientific Name"));
	m_PSTable->SetColumnWidth(2, 160);
	m_PSTable->InsertColumn(3, _T("Max Height"));
	m_PSTable->SetColumnWidth(3, 80);

	// Setup right side with each plant appearance's attributes.
	m_PATable = (wxListCtrl *) FindWindow( ID_LISTCTRL_APPEARANCES );
	m_PATable->ClearAll();
	m_PATable->SetSingleStyle(wxLC_REPORT);
	m_PATable->InsertColumn(0, _T("Billboard"));
	m_PATable->SetColumnWidth(0, 60);
	m_PATable->InsertColumn(1, _T("FileName"));
	m_PATable->SetColumnWidth(1, 150);
	m_PATable->InsertColumn(2, _T("Width"));
	m_PATable->SetColumnWidth(2, 60);
	m_PATable->InsertColumn(3, _T("Height"));
	m_PATable->SetColumnWidth(3, 60);
	m_PATable->InsertColumn(4, _T("Shadow Radius"));
	m_PATable->SetColumnWidth(4, 100);
	m_PATable->InsertColumn(5, _T("Shadow Darkness"));
	m_PATable->SetColumnWidth(5, 100);

	// Read data imported from plantlist file and display in tables.
	vtSpeciesList* pl = GetMainFrame()->GetPlantList();

	long item1 = m_PSTable->InsertItem(0, _T(""), 0);
	m_PSTable->SetItem(item1, 1, _T("(All species)"));

	for (unsigned int i = 0; i < pl->NumSpecies(); i++)
	{
		// Display species and max height in left table.
		wxString str;
		long item;
		vtPlantSpecies *spe = pl->GetSpecies(i);

		str.Printf(_T("%d"), spe->GetSpecieID() );
		item = m_PSTable->InsertItem(i+1, str, 0);
		str.Printf(_T("%hs"), spe->GetCommonName() );
		item = m_PSTable->SetItem(i+1, 1, str);
		str.Printf(_T("%hs"), spe->GetSciName() );
		item = m_PSTable->SetItem(i+1, 2, str);
		str.Printf(_T("%4.2f m"), spe->GetMaxHeight() );
		item = m_PSTable->SetItem(i+1, 3, str);
	}

	// Start out with "All Species" selected
	m_PSTable->SetItemState(item1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

	return wxDialog::OnInitDialog(event);
}

void SpeciesListDlg::RefreshAppeances()
{
	if (m_idx == 0)
	{
		vtSpeciesList* pl = GetMainFrame()->GetPlantList();
		for (unsigned int i = 0; i < pl->NumSpecies(); i++)
			AddAppeance(i);
	}
	else
		AddAppeance(m_idx-1);
}

void SpeciesListDlg::AddAppeance(int idx)
{
	vtSpeciesList *pl = GetMainFrame()->GetPlantList();
	vtPlantSpecies *spe = pl->GetSpecies(idx);
	for (int j = 0; j < spe->NumAppearances(); j++)
	{
		vtPlantAppearance* app = spe->GetAppearance(j);
		wxString2 str1;
		long item1;

		str1.Printf(_T("%d"), app->m_eType == AT_BILLBOARD);
		item1 = m_PATable->InsertItem(j, str1, 0);
		str1 = app->m_filename;
		item1 = m_PATable->SetItem(j, 1, str1);
		str1.Printf(_T("%4.2f"), app->m_width);
		item1 = m_PATable->SetItem(j, 2, str1);
		str1.Printf(_T("%4.2f"), app->m_height);
		item1 = m_PATable->SetItem(j, 3, str1);
		str1.Printf(_T("%4.2f"), app->m_shadow_radius);
		item1 = m_PATable->SetItem(j, 4, str1);
		str1.Printf(_T("%4.2f"), app->m_shadow_darkness);
		item1 = m_PATable->SetItem(j, 5, str1);
	}
}


////////////////////////////////////////////////////////////////

void BioRegionDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Create tree control window as child of dialog.
	int width, height;
	GetClientSize(&width, &height);
	m_BTree = new wxTreeCtrl(this, WID_BIOREGIONTREE, wxPoint(10, 10), 
		wxSize(width - 20, height - 20), wxTR_EDIT_LABELS | wxTR_MULTIPLE);

	// Create root of tree.
	wxTreeItemId rootId;
	rootId = m_BTree->AddRoot(_T("BioRegions"));
	m_BTree->SetItemBold(rootId);

	vtBioRegion *br = GetMainFrame()->GetBioRegion();

	// Read data imported from bioregion file and display on tree.
	int numregions = br->m_Types.GetSize();
	for (int i = 0; i < numregions; i++)
	{
		// Display biotype as level 1 of tree.
		wxTreeItemId region;
		wxString bt;
		bt.Printf(_T("Type %d"), i);
		region = m_BTree->AppendItem(rootId, bt);

		int numspecies = br->m_Types[i]->m_Densities.GetSize();
		for (int j = 0; j < numspecies; j++)
		{
			// Display all species and it's density under each biotype as
			//  level 2 of the tree.
			wxString s;
			s.Printf(_T("%hs (%1.4f /m^2)"),
				(const char *) br->m_Types[i]->m_Densities[j]->m_common_name,
				br->m_Types[i]->m_Densities[j]->m_plant_per_m2);

			wxTreeItemId specie = m_BTree->AppendItem(region, s);
			m_BTree->Expand(specie);
		}

		m_BTree->Expand(region);
	}

	m_BTree->Expand(rootId);
	Show(TRUE);
}
