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
	EVT_CHOICE( ID_FIELD, SelectDlg::OnChoiceField )
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

	for (i = 0; i < m_pLayer->GetNumFields(); i++)
	{
		Field *field = m_pLayer->GetField(i);
		GetField()->Append((const char *) field->m_name);
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

	AddValidator(ID_FIELD, &m_iField);
	AddValidator(ID_CONDITION, &m_iCondition);
	AddValidator(ID_COMBO_VALUE, &m_strValue);

	wxDialog::OnInitDialog(event);	// calls TransferValuesToWindow

	m_bSetting = false;
}

void SelectDlg::FillValuesControl()
{
	int i, values = 0;
	vtString str;
	wxString str2;

	GetComboValue()->Clear();

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

	TransferDataFromWindow();
	FillValuesControl();
}

