//
// Name:		FeatInfoDlg.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "FeatInfoDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "FeatInfoDlg.h"
#include "vtdata/Features.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// FeatInfoDlg
//----------------------------------------------------------------------------

// WDR: event table for FeatInfoDlg

BEGIN_EVENT_TABLE(FeatInfoDlg,wxDialog)
	EVT_LIST_ITEM_SELECTED( ID_LIST, FeatInfoDlg::OnItemSelected )
END_EVENT_TABLE()

FeatInfoDlg::FeatInfoDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	m_pFeatures = NULL;
	FeatInfoDialogFunc( this, TRUE ); 
}

void FeatInfoDlg::SetFeatureSet(vtFeatures *pFeatures)
{
	if (m_pFeatures == pFeatures)
		return;

	m_pFeatures = pFeatures;

	GetList()->ClearAll();		// clears all items and columns

	GetList()->InsertColumn(0, "X coordinate", wxLIST_FORMAT_LEFT, 80);
	GetList()->InsertColumn(1, "Y coordinate", wxLIST_FORMAT_LEFT, 80);
	GetList()->InsertColumn(2, "Z coordinate", wxLIST_FORMAT_LEFT, 80);

	int i;
	for (i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		Field *field = m_pFeatures->GetField(i);
		const char *name = field->m_name;
		GetList()->InsertColumn(i+3, name, wxLIST_FORMAT_LEFT, 80);
	}
}

void FeatInfoDlg::Clear()
{
	GetList()->DeleteAllItems();
}

void FeatInfoDlg::ShowFeature(int iFeat)
{
	vtString str;
	int i, next;

	next = GetList()->GetItemCount();
	GetList()->InsertItem(next, "temp");

	DPoint3 p;
	m_pFeatures->GetPoint(iFeat, p);
	str.Format("%.2lf", p.x);
	GetList()->SetItem(next, 0, (const char *) str);
	str.Format("%.2lf", p.y);
	GetList()->SetItem(next, 1, (const char *) str);
	str.Format("%.2lf", p.z);
	GetList()->SetItem(next, 2, (const char *) str);

	for (i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		Field *field = m_pFeatures->GetField(i);
		m_pFeatures->GetValueAsString(iFeat, i, str);
		GetList()->SetItem(next, i+3, (const char *) str);
	}
}


// WDR: handler implementations for FeatInfoDlg

void FeatInfoDlg::OnItemSelected( wxListEvent &event )
{
	
}




