//
// VegDlg.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <wx/wxprec.h>
#include "VegDlg.h"
#include "frame.h"

////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(PlantListCtrl, wxListCtrl)
	EVT_LIST_ITEM_SELECTED(WID_PSTABLE,	PlantListCtrl::OnSelect)
END_EVENT_TABLE()


void PlantListCtrl::OnSelect(wxListEvent &event)
{
	// Get index of that item
	int idx = event.GetIndex();

	// Clear all item data from the Appearance table on right.
	wxListCtrl *apps = GetMainFrame()->m_PlantListDlg->m_PATable;
	apps->DeleteAllItems();

	// Get species that was selected from left table.
	vtPlantSpecies* ps = GetMainFrame()->GetPlantList()->GetSpecies(idx);

	// Display appearance in right table of selected species from left table.
	for (int j = 0; j < ps->NumAppearances(); j++)
	{
		vtPlantAppearance* app = ps->GetAppearance(j);
		wxString str1;
		long item1;
		str1.Printf("%d", app->m_bBillboard);
		item1 = apps->InsertItem(j, str1, 0);
		str1.Printf("%s", app->m_filename);
		item1 = apps->SetItem(j, 1, str1);
		str1.Printf("%4.2f", app->m_width);
		item1 = apps->SetItem(j, 2, str1);
		str1.Printf("%4.2f", app->m_height);
		item1 = apps->SetItem(j, 3, str1);
		str1.Printf("%4.2f", app->m_shadow_radius);
		item1 = apps->SetItem(j, 4, str1);
		str1.Printf("%4.2f", app->m_shadow_darkness);
		item1 = apps->SetItem(j, 5, str1);
	}
}

////////////////////////////////////////////////////////////////

void PlantListDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Setup Splitter Window.
	m_PSplitter = new wxSplitterWindow(this, WID_PSPLITTER, wxPoint(0,0), wxSize(950,400),
		wxSP_FULLSASH | wxSP_NOBORDER, "Plant List Dialog Splitter");
	
	int z = 0;

	// Setup left side with common name and max height per species.
	m_PSTable = new PlantListCtrl(m_PSplitter, WID_PSTABLE, wxPoint(0, 0), wxSize(400, 400),
		wxLC_REPORT | wxLC_ALIGN_TOP);
	m_PSTable->ClearAll();
	m_PSTable->SetSingleStyle(wxLC_REPORT);
	m_PSTable->InsertColumn(0, "SID");
	m_PSTable->SetColumnWidth(0, 30);
	m_PSTable->InsertColumn(1, "Common Name");
	m_PSTable->SetColumnWidth(1, 110);
	m_PSTable->InsertColumn(2, "Scientific Name");
	m_PSTable->SetColumnWidth(2, 150);
	m_PSTable->InsertColumn(3, "Max Height");
	m_PSTable->SetColumnWidth(3, 80);

	// Setup right side with each plant appearance's attributes.
	m_PATable = new wxListCtrl(m_PSplitter, WID_PATABLE, wxPoint(0, 0), wxSize(550, 400),
		wxLC_REPORT | wxLC_ALIGN_TOP);
	m_PATable->ClearAll();
	m_PATable->SetSingleStyle(wxLC_REPORT);
	m_PATable->InsertColumn(0, "Billboard");
	m_PATable->SetColumnWidth(0, 60);
	m_PATable->InsertColumn(1, "FileName");
	m_PATable->SetColumnWidth(1, 150);
	m_PATable->InsertColumn(2, "Width");
	m_PATable->SetColumnWidth(2, 60);
	m_PATable->InsertColumn(3, "Height");
	m_PATable->SetColumnWidth(3, 60);
	m_PATable->InsertColumn(4, "Shadow Radius");
	m_PATable->SetColumnWidth(4, 100);
	m_PATable->InsertColumn(5, "Shadow Darkness");
	m_PATable->SetColumnWidth(5, 100);

	// Initialize the splitter window.
	m_PATable->Show(FALSE);
	m_PSplitter->Initialize(m_PSTable);

	vtPlantList* pl = GetMainFrame()->GetPlantList();

	// Read data imported from plantlist file and display in tables.
	for (int i = 0; i < pl->NumSpecies(); i++)
	{
		// Display species and max height in left table.
		wxString str;
		long item;
		vtPlantSpecies *spe = pl->GetSpecies(i);

		str.Printf("%d", spe->GetSpecieID() );
        item = m_PSTable->InsertItem(i, str, 0);
		str.Printf("%s", spe->GetCommonName() );
        item = m_PSTable->SetItem(i, 1, str);
		str.Printf("%s", spe->GetSciName() );
        item = m_PSTable->SetItem(i, 2, str);
		str.Printf("%4.2f m", spe->GetMaxHeight() );
        item = m_PSTable->SetItem(i, 3, str);
		
		// Display plant appearances per species in right table.
		for (int j = 0; j < pl->GetSpecies(i)->NumAppearances(); j++)
		{
			vtPlantAppearance* app = spe->GetAppearance(j);
			wxString str1;
			long item1;

			str1.Printf("%d", app->m_bBillboard);
			item1 = m_PATable->InsertItem(j, str1, 0);
			str1.Printf("%s", app->m_filename);
			item1 = m_PATable->SetItem(j, 1, str1);
			str1.Printf("%4.2f", app->m_width);
			item1 = m_PATable->SetItem(j, 2, str1);
			str1.Printf("%4.2f", app->m_height);
			item1 = m_PATable->SetItem(j, 3, str1);
			str1.Printf("%4.2f", app->m_shadow_radius);
			item1 = m_PATable->SetItem(j, 4, str1);
			str1.Printf("%4.2f", app->m_shadow_darkness);
			item1 = m_PATable->SetItem(j, 5, str1);
		}
	}
	// Show all windows.
	m_PSTable->Show(TRUE);
	m_PATable->Show(TRUE);
	m_PSplitter->SplitVertically(m_PSTable, m_PATable, 400);
}

////////////////////////////////////////////////////////////////

void BioRegionDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Create tree control window as child of dialog.
	int width, height;
	GetClientSize(&width, &height);
	m_BTree = new wxTreeCtrl(this, WID_BTREE, wxPoint(10, 10), 
		wxSize(width - 20, height - 20), wxTR_EDIT_LABELS | wxTR_MULTIPLE);

	// Create root of tree.
	wxTreeItemId rootId;
	rootId = m_BTree->AddRoot("BioRegions");
	m_BTree->SetItemBold(rootId);

	vtBioRegion* br = GetMainFrame()->GetBioRegion();

	// Read data imported from bioregion file and display on tree.
	int numregions = br->m_Types.GetSize();
	for (int i = 0; i < numregions; i++)
	{
		// Display biotype as level 1 of tree.
		wxTreeItemId region;
		wxString bt;
		bt.Printf("Type %d", i+1);
		region = m_BTree->AppendItem(rootId, bt);

		int numspecies = br->m_Types[i]->m_Densities.GetSize();
		for (int j = 0; j < numspecies; j++)
		{
			// Display all species and it's density under each biotype as level 2 of tree.
			wxString s;
			s.Printf("%s (%1.4f /m^2)",
				br->m_Types[i]->m_Densities[j]->m_common_name, 
				br->m_Types[i]->m_Densities[j]->m_plant_per_m2);

			wxTreeItemId specie = m_BTree->AppendItem(region, s);
			m_BTree->Expand(specie);
		}

		m_BTree->Expand(region);
	}

	m_BTree->Expand(rootId);
	Show(TRUE);
}
