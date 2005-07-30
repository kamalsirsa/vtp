//
// Name:		FeatInfoDlg.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "FeatInfoDlg.h"
#include "vtdata/vtLog.h"
#include "vtdata/Features.h"
#include "vtui/wxString2.h"
#include "BuilderView.h"

// WDR: class implementations

//---------------------------------------------------------------------------
// FeatInfoDlg
//---------------------------------------------------------------------------

// WDR: event table for FeatInfoDlg

BEGIN_EVENT_TABLE(FeatInfoDlg,AutoDialog)
	EVT_INIT_DIALOG (FeatInfoDlg::OnInitDialog)
	EVT_LIST_ITEM_SELECTED( ID_LIST, FeatInfoDlg::OnItemSelected )
	EVT_LIST_ITEM_DESELECTED( ID_LIST, FeatInfoDlg::OnItemSelected )
	EVT_LIST_ITEM_RIGHT_CLICK( ID_LIST, FeatInfoDlg::OnListRightClick )
	EVT_CHOICE( ID_CHOICE_SHOW, FeatInfoDlg::OnChoiceShow )
	EVT_CHOICE( ID_CHOICE_VERTICAL, FeatInfoDlg::OnChoiceVertical )
	EVT_BUTTON( ID_DEL_HIGH, FeatInfoDlg::OnDeleteHighlighted )
	EVT_LEFT_DCLICK( FeatInfoDlg::OnLeftDClick )
END_EVENT_TABLE()

FeatInfoDlg::FeatInfoDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	m_iShow = 1;
	m_iVUnits = 0;
	m_pLayer = NULL;
	m_pFeatures = NULL;

	FeatInfoDialogFunc( this, TRUE );

	AddValidator(ID_CHOICE_SHOW, &m_iShow);
	AddValidator(ID_CHOICE_VERTICAL, &m_iVUnits);
}

void FeatInfoDlg::SetFeatureSet(vtFeatureSet *pFeatures)
{
	if (m_pFeatures == pFeatures)
		return;

	m_pFeatures = pFeatures;

	GetList()->ClearAll();  // clears all items and columns
	m_iCoordColumns = 0;

	if (!m_pFeatures)
	{
		GetList()->Enable(false);
		GetChoiceShow()->Enable(false);
		GetDelHigh()->Enable(false);
		GetTextVertical()->Enable(false);
		GetChoiceVertical()->Enable(false);
		return;
	}

	vtProjection &proj = pFeatures->GetAtProjection();
	m_bGeo = (proj.IsGeographic() != 0);

	GetList()->Enable(true);
	GetChoiceShow()->Enable(true);

	int field = 0;
	OGRwkbGeometryType type = m_pFeatures->GetGeomType();
	if (type == wkbPoint || type == wkbPoint25D)
	{
		GetList()->InsertColumn(field++, _T("X"), wxLIST_FORMAT_LEFT, m_bGeo ? 90 : 60);
		GetList()->InsertColumn(field++, _T("Y"), wxLIST_FORMAT_LEFT, m_bGeo ? 90 : 60);
		m_iCoordColumns = 2;
	}
	if (type == wkbPoint25D)
	{
		GetList()->InsertColumn(field++, _T("Z"), wxLIST_FORMAT_LEFT, 70);
		m_iCoordColumns = 3;
	}

	GetTextVertical()->Enable(type == SHPT_POINTZ);
	GetChoiceVertical()->Enable(type == SHPT_POINTZ);

	unsigned int i;
	for (i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		Field *pField = m_pFeatures->GetField(i);
		wxString2 name = pField->m_name;
		int width1 = pField->m_width * 6;
		int width2 = name.Length() * 8;
		int width = max(width1, width2);
		if (width < 20)
			width = 20;
		GetList()->InsertColumn(field++, name, wxLIST_FORMAT_LEFT, width);
	}
}

void FeatInfoDlg::Clear()
{
	GetList()->DeleteAllItems();
	GetDelHigh()->Enable(false);
}

void FeatInfoDlg::ShowSelected()
{
	if (!m_pFeatures)
		return;

	m_iShow = 0;
	TransferDataToWindow();
	Clear();
	int selected = m_pFeatures->NumSelected();
	if (selected > 2000)
	{
		wxString msg;
		msg.Printf(_("There are %d selected features.  Are you sure you\n want to display them all in the table view?"), selected);
		if (wxMessageBox(msg, _("Warning"), wxYES_NO) == wxNO)
			return;
	}
	int i, num = m_pFeatures->GetNumEntities();
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
	int i, num = m_pFeatures->GetNumEntities();
	for (i = 0; i < num; i++)
	{
		if (m_pFeatures->IsPicked(i))
			ShowFeature(i);
	}
}

void FeatInfoDlg::ShowAll()
{
	m_iShow = 2;
	TransferDataToWindow();
	Clear();
	int i, num = m_pFeatures->GetNumEntities();
	if (num > 2000)
	{
		wxString msg;
		msg.Printf(_("There are %d features in this layer.  Are you sure\n you want to display them all in the table view?"), num);
		if (wxMessageBox(msg, _("Warning"), wxYES_NO) == wxNO)
			return;
	}
	for (i = 0; i < num; i++)
	{
		ShowFeature(i);
	}
}

void FeatInfoDlg::ShowFeature(int iFeat)
{
	int next = GetList()->GetItemCount();
	GetList()->InsertItem(next, _T("temp"));
	GetList()->SetItemData(next, iFeat);

	UpdateFeatureText(next, iFeat);
}

void FeatInfoDlg::UpdateFeatureText(int iItem, int iFeat)
{
	wxString2 str;
	int field = 0;

	wxString strFormat;
	if (m_bGeo)
		strFormat = _T("%.8lf");
	else
		strFormat = _T("%.2lf");

	OGRwkbGeometryType type = m_pFeatures->GetGeomType();
	if (type == wkbPoint)
	{
		DPoint2 p2;
		((vtFeatureSetPoint2D *)m_pFeatures)->GetPoint(iFeat, p2);

		str.Printf(strFormat, p2.x);
		GetList()->SetItem(iItem, field++, str);
		str.Printf(strFormat, p2.y);
		GetList()->SetItem(iItem, field++, str);
	}
	if (type == wkbPoint25D)
	{
		DPoint3 p3;
		((vtFeatureSetPoint3D *)m_pFeatures)->GetPoint(iFeat, p3);

		str.Printf(strFormat, p3.x);
		GetList()->SetItem(iItem, field++, str);
		str.Printf(strFormat, p3.y);
		GetList()->SetItem(iItem, field++, str);

		double scale = GetMetersPerUnit((LinearUnits) (m_iVUnits+1));
		str.Printf(_T("%.2lf"), p3.z / scale);
		GetList()->SetItem(iItem, field++, str);
	}

	for (unsigned int i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		vtString vs;
		m_pFeatures->GetValueAsString(iFeat, i, vs);

		// Using wstring2 here works, but leaks a little memory for some
		//  reason.  I cannot understand why, it is buried deep in insanely
		//  convoluted STL templates.  We'll just use wxString instead.
//		wstring2 wide;
//		wide.from_utf8(vs);
//		str = wide.c_str();
		wxString wide((const char *)vs, wxConvUTF8);

		GetList()->SetItem(iItem, field++, wide);
	}
}

void FeatInfoDlg::RefreshItems()
{
	if (m_iShow == 0)
		ShowSelected();
	else if (m_iShow == 1)
		ShowPicked();
	else if (m_iShow == 2)
		ShowAll();
}

bool FeatInfoDlg::EditValue(int iFeature, int iColumn)
{
	if (iColumn < m_iCoordColumns)
	{
		// TODO: allow numeric entry of point coordinates
	}
	else
	{
		int iField = iColumn - m_iCoordColumns;
		vtString vs;
		m_pFeatures->GetValueAsString(iFeature, iField, vs);

		wxString2 str;
		str.from_utf8(vs);

		Field *pField = m_pFeatures->GetField(iField);

		wxString message, caption;
		message.Printf(_("Enter a new value for field '%hs'"),
			(const char *) pField->m_name);
		caption.Printf(_("Text entry"));
		str = wxGetTextFromUser(message, caption, str, this);
		if (str != _T(""))
		{
			vs = str.to_utf8();
			m_pFeatures->SetValueFromString(iFeature, iField, vs);
			return true;
		}
	}
	return false;
}


// WDR: handler implementations for FeatInfoDlg

void FeatInfoDlg::OnLeftDClick( wxMouseEvent &event )
{
}

void FeatInfoDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetChoiceShow()->Clear();
	GetChoiceShow()->Append(_("Selected"));
	GetChoiceShow()->Append(_("Picked"));
	GetChoiceShow()->Append(_("All"));

	GetChoiceVertical()->Clear();
	GetChoiceVertical()->Append(_("Meter"));
	GetChoiceVertical()->Append(_("Foot"));
	GetChoiceVertical()->Append(_("Foot (US)"));

	GetDelHigh()->Enable(false);

	wxDialog::OnInitDialog(event);
}


void FeatInfoDlg::OnDeleteHighlighted( wxCommandEvent &event )
{
	int iDeleted = 0;
	int iFeat;
	int item = -1;
	for ( ;; )
	{
		item = GetList()->GetNextItem(item, wxLIST_NEXT_ALL,
									 wxLIST_STATE_SELECTED);
		if ( item == -1 )
			break;
		iFeat = (int) GetList()->GetItemData(item);
		m_pFeatures->SetToDelete(iFeat);
		iDeleted++;
	}
	if (iDeleted > 0)
	{
		m_pLayer->SetModified(true);
		m_pFeatures->ApplyDeletion();
		m_pView->Refresh();
		RefreshItems();
	}
}

void FeatInfoDlg::OnChoiceVertical( wxCommandEvent &event )
{
	TransferDataFromWindow();
	RefreshItems();
}

void FeatInfoDlg::OnChoiceShow( wxCommandEvent &event )
{
	TransferDataFromWindow();
	RefreshItems();
}

void FeatInfoDlg::OnListRightClick( wxListEvent &event )
{
/*	int iFeat;
	int item = -1;

	item = GetList()->GetNextItem(item, wxLIST_NEXT_ALL,
								  wxLIST_STATE_SELECTED);
	if ( item == -1 )
		return; */
	int iItem = event.GetIndex();
	int iFeat = (int) GetList()->GetItemData(iItem);
/*	for (int i = 0; i < m_pFeatures->GetNumFields(); i++)
	{
		vtString vs;
		m_pFeatures->GetValueAsString(iFeat, i, vs);
		wstring2 wide;
		wide.from_utf8(vs);
		VTLOG(_T("Field %d, Value '%ls'\n"), i, wide.c_str());
	} */
	wxPoint mouse = event.GetPoint();
	int x = 0;
	int columns = GetList()->GetColumnCount();
	for (int i = 0; i < columns; i++)
	{
		int width = GetList()->GetColumnWidth(i);
		if (x < mouse.x && mouse.x < x+width)
		{
			// Found it;
			VTLOG("Clicked column %d\n", i);
			if (EditValue(iFeat, i))
			{
				// Refresh the display
				m_pLayer->SetModified(true);
				UpdateFeatureText(iItem, iFeat);
			}
			return;
		}
		x += width;
	}
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

