//
// Name:		SelectDlg.cpp
//
// Copyright (c) 2002-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "SelectDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "SelectDlg.h"
#include "RawLayer.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// SelectDlg
//----------------------------------------------------------------------------

// WDR: event table for SelectDlg

BEGIN_EVENT_TABLE(SelectDlg,AutoDialog)
	EVT_BUTTON( wxID_OK, SelectDlg::OnOK )
	EVT_LISTBOX( ID_FIELD, SelectDlg::OnChoiceField )
END_EVENT_TABLE()

SelectDlg::SelectDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	SelectDialogFunc( this, TRUE );
	m_bSetting = true;
	m_pLayer = NULL;
}

void SelectDlg::SetRawLayer(vtRawLayer *pRL)
{
	m_pLayer = pRL;
}

void SelectDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_iField = 0;
	m_iCondition = 0;
	m_strValue = _T("");

	vtProjection proj;
	m_pLayer->GetProjection(proj);

	m_iFauxFields = 0;
	vtFeatureSet *pSet = m_pLayer->GetFeatureSet();

	OGRwkbGeometryType type = pSet->GetGeomType();
	if (type == wkbPoint || type == wkbPoint25D)
	{
		if (proj.IsGeographic())
		{
			GetField()->Append(_T("X (longitude)"), (void *) 900);
			GetField()->Append(_T("Y (latitude)"), (void *) 901);
		}
		else
		{
			GetField()->Append(_T("X (easting)"), (void *) 900);
			GetField()->Append(_T("Y (northing)"), (void *) 901);
		}
		m_iFauxFields = 2;
	}
	if (type == wkbPoint25D)
	{
		GetField()->Append(_T("Z (meters)"), (void *) 902);
		m_iFauxFields = 3;
	}

	wxString2 str;
	for (unsigned int i = 0; i < pSet->GetNumFields(); i++)
	{
		Field *field = pSet->GetField(i);
		str = field->m_name;
		GetField()->Append(str, (void *) 0);
	}
	GetField()->SetSelection(0);

	// The order of these must not change
	GetCondition()->Append(_T(" = "));
	GetCondition()->Append(_T(" > "));
	GetCondition()->Append(_T(" < "));
	GetCondition()->Append(_T(" >= "));
	GetCondition()->Append(_T(" <= "));
	GetCondition()->Append(_T(" <> "));
	GetCondition()->SetSelection(0);

	FillValuesControl();

	AddValidator(ID_CONDITION, &m_iCondition);

	wxDialog::OnInitDialog(event);	// calls TransferValuesToWindow

	m_bSetting = false;
}

void SelectDlg::FillValuesControl()
{
	int values = 0;
	vtString str;
	wxString2 str2;

	GetComboValue()->Clear();

	if (m_iField < 0)
	{
		GetComboValue()->Append(_T("0.0"));
		GetComboValue()->SetSelection(0);
		return;
	}

	vtFeatureSet *pSet = m_pLayer->GetFeatureSet();
	for (unsigned int i = 0; i < pSet->GetNumEntities(); i++)
	{
		pSet->GetValueAsString(i, m_iField, str);

		str2 = str;
		if (str2 == _T(""))
			continue;

		if (GetComboValue()->FindString(str2) == -1)
		{
			GetComboValue()->Append(str2);
			values++;
		}
		if (values == 100)
			break;
	}
	GetComboValue()->SetSelection(0);
}


// WDR: handler implementations for SelectDlg

void SelectDlg::OnChoiceField( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	m_iField = GetField()->GetSelection();

	// work around the 2-3 "special" fields
	void *data = GetField()->GetClientData(m_iField);
	if (data == (void*) 900)
		m_iField = -1;
	else if (data == (void*) 901)
		m_iField = -2;
	else if (data == (void*) 902)
		m_iField = -3;
	else
		m_iField -= m_iFauxFields;

	TransferDataFromWindow();
	FillValuesControl();
}

void SelectDlg::OnOK( wxCommandEvent &event )
{
	// Using a validator for Combo Box doesn't work perfectly in last version
	// of wxWindows.  Get the value directly instead.
	m_strValue = GetComboValue()->GetValue();

	wxDialog::OnOK(event);
}


