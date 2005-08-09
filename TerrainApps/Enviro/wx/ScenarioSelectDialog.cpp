//
// Name: ScenarioSelectDialog.cpp
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "vtui/wxString2.h"

#include "ScenarioSelectDialog.h"
#include "ScenarioParamsDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CScenarioListValidator
//----------------------------------------------------------------------------
bool CScenarioListValidator::TransferToWindow()
{
	wxListBox* pListBox = wxDynamicCast(GetWindow(), wxListBox);
	if (NULL != pListBox)
	{
		wxString2 str;
		pListBox->Clear();
		for (unsigned int i = 0; i < m_pScenarios->size(); i++)
		{
			str.from_utf8((*m_pScenarios)[i].GetValueString(STR_SCENARIO_NAME));
			pListBox->Append(str);
		}
		return true;
	}
	else
		return false;
}

bool CScenarioListValidator::TransferFromWindow()
{
	// Just do a sanity check
	// the control should update its data real time
	wxListBox* pListBox = wxDynamicCast(GetWindow(), wxListBox);
	if (NULL != pListBox)
		return true;
	else
		return false;
}

//----------------------------------------------------------------------------
// CScenarioSelectDialog
//----------------------------------------------------------------------------

// WDR: event table for CScenarioSelectDialog

BEGIN_EVENT_TABLE(CScenarioSelectDialog,wxDialog)
	EVT_LISTBOX( ID_SCENARIO_LIST, CScenarioSelectDialog::OnScenarioList )
	EVT_BUTTON( ID_NEW_SCENARIO, CScenarioSelectDialog::OnNewScenario )
	EVT_BUTTON( ID_DELETE_SCENARIO, CScenarioSelectDialog::OnDeleteScenario )
	EVT_BUTTON( ID_EDIT_SCENARIO, CScenarioSelectDialog::OnEditScenario )
	EVT_BUTTON( ID_MOVEUP_SCENARIO, CScenarioSelectDialog::OnMoveUpScenario )
	EVT_BUTTON( ID_MOVEDOWN_SCENARIO, CScenarioSelectDialog::OnMoveDownScenario )
	EVT_BUTTON( ID_SCENARIO_PREVIOUS, CScenarioSelectDialog::OnScenarioPrevious )
	EVT_BUTTON( ID_SCENARIO_NEXT, CScenarioSelectDialog::OnScenarioNext )
	EVT_BUTTON(wxID_OK, CScenarioSelectDialog::OnOK)
	EVT_BUTTON(wxID_APPLY, CScenarioSelectDialog::OnApply)
	EVT_BUTTON(wxID_CANCEL, CScenarioSelectDialog::OnCancel)
END_EVENT_TABLE()

CScenarioSelectDialog::CScenarioSelectDialog( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	wxDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function ScenarioSelectDialogFunc for CScenarioSelectDialog
	ScenarioSelectDialogFunc( this, TRUE );

	GetScenarioList()->SetValidator(CScenarioListValidator(&m_Scenarios));

	m_bModified = false;
}

void CScenarioSelectDialog::SetTerrain(vtTerrain *pTerrain)
{
	m_pTerrain = pTerrain;
	m_Scenarios = pTerrain->GetParams().m_Scenarios;
	m_bModified = false;
	UpdateEnableState();
}

bool CScenarioSelectDialog::TransferDataToWindow()
{
	bool bRet = wxDialog::TransferDataToWindow();

	UpdateEnableState();

	return bRet;
}


void CScenarioSelectDialog::UpdateEnableState()
{
	int iSelected = GetScenarioList()->GetSelection();
	if (iSelected != wxNOT_FOUND)
	{
		GetEditScenario()->Enable(true);
		GetDeleteScenario()->Enable(true);
		if (iSelected != (GetScenarioList()->GetCount() - 1))
			GetMovedownScenario()->Enable(true);
		else
			GetMovedownScenario()->Enable(false);
		if (iSelected != 0)
			GetMoveupScenario()->Enable(true);
		else
			GetMoveupScenario()->Enable(false);
		if (m_bModified)
		{
			GetScenarioNext()->Enable(false);
			GetScenarioPrevious()->Enable(false);
		}
		else
		{
			GetScenarioNext()->Enable(true);
			GetScenarioPrevious()->Enable(true);
		}
	}
	else
	{
		GetEditScenario()->Enable(false);
		GetDeleteScenario()->Enable(false);
		GetMoveupScenario()->Enable(false);
		GetMovedownScenario()->Enable(false);
		GetScenarioNext()->Enable(false);
		GetScenarioPrevious()->Enable(false);
	}
	if (m_bModified)
		GetApply()->Enable(true);
	else
		GetApply()->Enable(false);
}


// WDR: handler implementations for CScenarioSelectDialog

void CScenarioSelectDialog::OnScenarioNext( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iCount = pScenarioList->GetCount();
	if (!m_bModified)
	{
		pScenarioList->SetSelection((pScenarioList->GetSelection() + 1) % iCount);
		m_pTerrain->ActivateScenario(GetScenarioList()->GetSelection());
		UpdateEnableState();
	}
}

void CScenarioSelectDialog::OnScenarioPrevious( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iCount = pScenarioList->GetCount();
	if (!m_bModified)
	{
		pScenarioList->SetSelection((pScenarioList->GetSelection() + iCount - 1) % iCount);
		m_pTerrain->ActivateScenario(GetScenarioList()->GetSelection());
		UpdateEnableState();
	}
}

void CScenarioSelectDialog::OnMoveDownScenario( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iSelected = pScenarioList->GetSelection();

	if ((iSelected != wxNOT_FOUND) && (iSelected != (pScenarioList->GetCount() - 1)))
	{
		ScenarioParams TempParams = m_Scenarios[iSelected];
		wxString TempString = pScenarioList->GetString(iSelected);
		pScenarioList->Delete(iSelected);
// Bug in wxWindows
//	  pScenarioList->SetSelection(pScenarioList->Insert(TempString, iSelected + 1));
		pScenarioList->Insert(TempString, iSelected + 1);
		pScenarioList->SetSelection(iSelected + 1);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_Scenarios.insert(m_Scenarios.begin() + iSelected + 1,TempParams);
		m_bModified = true;
		UpdateEnableState();
	}
}

void CScenarioSelectDialog::OnMoveUpScenario( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iSelected = pScenarioList->GetSelection();

	if ((iSelected != wxNOT_FOUND) && (iSelected != 0))
	{
		ScenarioParams TempParams = m_Scenarios[iSelected];
		wxString TempString = pScenarioList->GetString(iSelected);
		pScenarioList->Delete(iSelected);
// Bug in wxWindows
//	  pScenarioList->SetSelection(pScenarioList->Insert(TempString, iSelected - 1));
		pScenarioList->Insert(TempString, iSelected - 1);
		pScenarioList->SetSelection(iSelected - 1);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_Scenarios.insert(m_Scenarios.begin() + iSelected - 1,TempParams);
		m_bModified = true;
		UpdateEnableState();
	}
}

void CScenarioSelectDialog::OnEditScenario( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	CScenarioParamsDialog ScenarioParamsDialog(this, -1, _("Scenario Parameters"));
	int iSelected = pScenarioList->GetSelection();
	
	if (iSelected != wxNOT_FOUND)
	{
		ScenarioParamsDialog.SetAvailableLayers(m_pTerrain->GetParams().m_Layers);
		ScenarioParamsDialog.SetParams(m_Scenarios[iSelected]);

		if (wxID_OK == ScenarioParamsDialog.ShowModal())
		{
			if (ScenarioParamsDialog.IsModified())
			{
				wxString2 str;
				str.from_utf8(m_Scenarios[iSelected].GetValueString(STR_SCENARIO_NAME));
				m_Scenarios[iSelected] = ScenarioParamsDialog.GetParams();
				pScenarioList->SetString(iSelected, str);
				m_bModified = true;
			}
		}
	}
}

void CScenarioSelectDialog::OnDeleteScenario( wxCommandEvent &event )
{
	wxListBox *pScenarioList = GetScenarioList();
	int iSelected = pScenarioList->GetSelection();

	if (iSelected != wxNOT_FOUND)
	{
		pScenarioList->Delete(iSelected);
		m_Scenarios.erase(m_Scenarios.begin() + iSelected);
		m_bModified = true;
		UpdateEnableState();
	}
}

void CScenarioSelectDialog::OnNewScenario( wxCommandEvent &event )
{
	wxString2 ScenarioName = wxGetTextFromUser(_("Enter Scenario Name"), _("New Scenario"));
	wxListBox *pScenarioList = GetScenarioList();

	if (!ScenarioName.IsEmpty())
	{
		ScenarioParams Scenario;

		Scenario.SetValueString(STR_SCENARIO_NAME, ScenarioName.to_utf8(), true);
		m_Scenarios.push_back(Scenario);
		pScenarioList->SetSelection(pScenarioList->Append(ScenarioName));
		m_bModified = true;
		UpdateEnableState();
	}
}

void CScenarioSelectDialog::OnScenarioList( wxCommandEvent &event )
{
	UpdateEnableState();
	if (!m_bModified)
	{
		// Activate the selected scenario
		m_pTerrain->ActivateScenario(GetScenarioList()->GetSelection());
	}
}

void CScenarioSelectDialog::OnApply(wxCommandEvent& event)
{
	std::vector<ScenarioParams> TempParams = m_pTerrain->GetParams().m_Scenarios;

	m_pTerrain->GetParams().m_Scenarios = m_Scenarios;
	if (m_pTerrain->GetParams().WriteToXML(m_pTerrain->GetParamFile(), STR_TPARAMS_FORMAT_NAME))
	{
		m_bModified = false;
		UpdateEnableState();
		wxDialog::OnApply(event);
	}
	else
	{
		wxString str;
		str.Printf(_("Couldn't save to file %hs.\n"), (const char *)m_pTerrain->GetParamFile());
		str += _("Please make sure the file is not read-only.");
		wxMessageBox(str);
		m_pTerrain->GetParams().m_Scenarios = TempParams;
	}
}

void CScenarioSelectDialog::OnOK(wxCommandEvent& event)
{
	if (m_bModified)
	{
		std::vector<ScenarioParams> TempParams = m_pTerrain->GetParams().m_Scenarios;

		m_pTerrain->GetParams().m_Scenarios = m_Scenarios;
		if (m_pTerrain->GetParams().WriteToXML(m_pTerrain->GetParamFile(), STR_TPARAMS_FORMAT_NAME))
		{
			m_bModified = false;
			UpdateEnableState();
			wxDialog::OnOK(event);
		}
		else
		{
			wxString str;
			str.Printf(_("Couldn't save to file %hs.\n"), (const char *)m_pTerrain->GetParamFile());
			str += _("Please make sure the file is not read-only.");
			wxMessageBox(str);
			m_pTerrain->GetParams().m_Scenarios = TempParams;
		}
	}
}

void CScenarioSelectDialog::OnCancel(wxCommandEvent& event)
{
	if (m_bModified)
	{
		// Reset the data
		SetTerrain(m_pTerrain);
		TransferDataToWindow();
	}
	wxDialog::OnCancel(event);
}




