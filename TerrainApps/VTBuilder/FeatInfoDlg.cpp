/////////////////////////////////////////////////////////////////////////////
// Name:		FeatInfoDlg.cpp
// Author:	  XX
// Created:	 XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

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

	int i;
	for (i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		Field *field = m_pFeatures->GetField(i);
		const char *name = field->m_name;
		GetList()->InsertColumn(i, name, wxLIST_FORMAT_LEFT, 80);
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

	for (i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		Field *field = m_pFeatures->GetField(i);
		m_pFeatures->GetValueAsString(iFeat, i, str);
		GetList()->SetItem(next, i, (const char *) str);
	}
}


// WDR: handler implementations for FeatInfoDlg

void FeatInfoDlg::OnItemSelected( wxListEvent &event )
{
	
}




