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
	m_pFeatures = pFeatures;
}

void FeatInfoDlg::Clear()
{
	GetList()->DeleteAllItems();
}

void FeatInfoDlg::AddFeature()
{
}


// WDR: handler implementations for FeatInfoDlg

void FeatInfoDlg::OnItemSelected( wxListEvent &event )
{
	
}




