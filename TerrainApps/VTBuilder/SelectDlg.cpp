/////////////////////////////////////////////////////////////////////////////
// Name:		SelectDlg.cpp
/////////////////////////////////////////////////////////////////////////////

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
	int i;

	m_iField = 0;
	m_iCondition = 0;
	m_strValue = "";

	vtProjection proj;
	m_pLayer->GetProjection(proj);

	m_iFauxFields = 0;
	int type = m_pLayer->GetEntityType();
	if (type == SHPT_POINT || type == SHPT_POINTZ)
	{
		if (proj.IsGeographic())
		{
			GetField()->Append("X (longitude)", (void *) 900);
			GetField()->Append("Y (latitude)", (void *) 901);
		}
		else
		{
			GetField()->Append("X (easting)", (void *) 900);
			GetField()->Append("Y (northing)", (void *) 901);
		}
		m_iFauxFields = 2;
	}
	if (type == SHPT_POINTZ)
	{
		GetField()->Append("Z (meters)", (void *) 902);
		m_iFauxFields = 3;
	}

	for (i = 0; i < m_pLayer->GetNumFields(); i++)
	{
		Field *field = m_pLayer->GetField(i);
		GetField()->Append((const char *) field->m_name, (void *) 0);
	}
	GetField()->SetSelection(0);

	// The order of these must not change
	GetCondition()->Append(" = ");
	GetCondition()->Append(" > ");
	GetCondition()->Append(" < ");
	GetCondition()->Append(" >= ");
	GetCondition()->Append(" <= ");
	GetCondition()->Append(" <> ");
	GetCondition()->SetSelection(0);

	FillValuesControl();

	AddValidator(ID_CONDITION, &m_iCondition);

	wxDialog::OnInitDialog(event);	// calls TransferValuesToWindow

	m_bSetting = false;
}

void SelectDlg::FillValuesControl()
{
	int i, values = 0;
	vtString str;
	wxString str2;

	GetComboValue()->Clear();

	if (m_iField < 0)
	{
		GetComboValue()->Append("0.0");
		GetComboValue()->SetSelection(0);
		return;
	}
	for (i = 0; i < m_pLayer->NumEntities(); i++)
	{
		m_pLayer->GetValueAsString(i, m_iField, str);

		str2 = (const char *) str;
		if (str2 == "")
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


