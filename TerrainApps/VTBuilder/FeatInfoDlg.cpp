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

BEGIN_EVENT_TABLE(FeatInfoDlg,AutoDialog)
	EVT_LIST_ITEM_SELECTED( ID_LIST, FeatInfoDlg::OnItemSelected )
	EVT_LIST_ITEM_DESELECTED( ID_LIST, FeatInfoDlg::OnItemSelected )
	EVT_LIST_ITEM_RIGHT_CLICK( ID_LIST, FeatInfoDlg::OnListRightClick )
	EVT_CHOICE( ID_CHOICE_SHOW, FeatInfoDlg::OnChoiceShow )
	EVT_CHOICE( ID_CHOICE_VERTICAL, FeatInfoDlg::OnChoiceVertical )
	EVT_BUTTON( ID_DEL_HIGH, FeatInfoDlg::OnDeleteHighlighted )
END_EVENT_TABLE()

FeatInfoDlg::FeatInfoDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_iShow = 1;
	m_iVUnits = 0;
	m_pFeatures = NULL;
	FeatInfoDialogFunc( this, TRUE ); 
}

void FeatInfoDlg::SetFeatureSet(vtFeatures *pFeatures)
{
	if (m_pFeatures == pFeatures)
		return;

	m_pFeatures = pFeatures;

	int field = 0;

	GetList()->ClearAll();	// clears all items and columns

	int type = m_pFeatures->GetEntityType();
	if (type == SHPT_POINT || type == SHPT_POINTZ)
	{
		GetList()->InsertColumn(field++, "X", wxLIST_FORMAT_LEFT, 80);
		GetList()->InsertColumn(field++, "Y", wxLIST_FORMAT_LEFT, 80);
	}
	if (type == SHPT_POINTZ)
		GetList()->InsertColumn(field++, "Z", wxLIST_FORMAT_LEFT, 80);

	GetTextVertical()->Enable(type == SHPT_POINTZ);
	GetChoiceVertical()->Enable(type == SHPT_POINTZ);

	int i;
	for (i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		Field *pField = m_pFeatures->GetField(i);
		const char *name = pField->m_name;
		GetList()->InsertColumn(field++, name, wxLIST_FORMAT_LEFT, 80);
	}
}

void FeatInfoDlg::Clear()
{
	GetList()->DeleteAllItems();
	GetDelHigh()->Enable(false);
}

void FeatInfoDlg::ShowSelected()
{
	m_iShow = 0;
	TransferDataToWindow();
	Clear();
	int selected = m_pFeatures->NumSelected();
	if (selected > 2000)
	{
		wxString msg;
		msg.Printf("There are %d selected features.  Are you sure you\n"
			"want to display them all in the table view?", selected);
		if (wxMessageBox(msg, "Warning", wxYES_NO) == wxNO)
			return;
	}
	int i, num = m_pFeatures->NumEntities();
	for (i = 0; i < num; i++)
	{
		if (m_pFeatures->IsSelected(i))
			ShowFeature(i);
	}
}

void FeatInfoDlg::ShowPicked()
{
	m_iShow = 1;
	TransferDataToWindow();
	Clear();
}

void FeatInfoDlg::ShowAll()
{
	m_iShow = 2;
	TransferDataToWindow();
	Clear();
	int i, num = m_pFeatures->NumEntities();
	if (num > 2000)
	{
		wxString msg;
		msg.Printf("There are %d features in this layer.  Are you sure\n"
			"you want to display them all in the table view?", num);
		if (wxMessageBox(msg, "Warning", wxYES_NO) == wxNO)
			return;
	}
	for (i = 0; i < num; i++)
	{
		ShowFeature(i);
	}
}

void FeatInfoDlg::ShowFeature(int iFeat)
{
	vtString str;
	int i, next;

	next = GetList()->GetItemCount();
	GetList()->InsertItem(next, "temp");

	int field = 0;
	DPoint3 p;
	m_pFeatures->GetPoint(iFeat, p);

	int type = m_pFeatures->GetEntityType();
	if (type == SHPT_POINT || type == SHPT_POINTZ)
	{
		str.Format("%.2lf", p.x);
		GetList()->SetItem(next, field++, (const char *) str);
		str.Format("%.2lf", p.y);
		GetList()->SetItem(next, field++, (const char *) str);
	}
	if (type == SHPT_POINTZ)
	{
		double scale = GetMetersPerUnit((LinearUnits) (m_iVUnits+1));
		str.Format("%.2lf", p.z / scale);
		GetList()->SetItem(next, field++, (const char *) str);
	}

	for (i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		m_pFeatures->GetValueAsString(iFeat, i, str);
		GetList()->SetItem(next, field++, (const char *) str);
	}
}


// WDR: handler implementations for FeatInfoDlg

void FeatInfoDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetChoiceShow()->Clear();
	GetChoiceShow()->Append("Selected");
	GetChoiceShow()->Append("Picked");
	GetChoiceShow()->Append("All");

	GetChoiceVertical()->Clear();
	GetChoiceVertical()->Append("Meter");
	GetChoiceVertical()->Append("Foot");
	GetChoiceVertical()->Append("Foot (US)");

	AddValidator(ID_CHOICE_SHOW, &m_iShow);
	AddValidator(ID_CHOICE_VERTICAL, &m_iVUnits);

	GetDelHigh()->Enable(false);

	wxDialog::OnInitDialog(event);
}


void FeatInfoDlg::OnDeleteHighlighted( wxCommandEvent &event )
{
	int item = -1;
	for ( ;; )
	{
		item = GetList()->GetNextItem(item, wxLIST_NEXT_ALL,
									 wxLIST_STATE_SELECTED);
		if ( item == -1 )
			break;
	}
}

void FeatInfoDlg::OnChoiceVertical( wxCommandEvent &event )
{
	TransferDataFromWindow();
	if (m_iShow == 0)
		ShowSelected();
	else if (m_iShow == 1)
		Clear();
	else if (m_iShow == 2)
		ShowAll();
}

void FeatInfoDlg::OnChoiceShow( wxCommandEvent &event )
{
	TransferDataFromWindow();
	if (m_iShow == 0)
		ShowSelected();
	else if (m_iShow == 1)
		ShowPicked();
	else if (m_iShow == 2)
		ShowAll();
}

void FeatInfoDlg::OnListRightClick( wxListEvent &event )
{
	
}

void FeatInfoDlg::OnItemSelected( wxListEvent &event )
{
	int count = 0;
	int item = -1;
	for ( ;; )
	{
		item = GetList()->GetNextItem(item, wxLIST_NEXT_ALL,
									 wxLIST_STATE_SELECTED);
		if ( item == -1 )
			break;
		count++;
	}
	GetDelHigh()->Enable(count > 0);
}

