//
// Name: ScenarioParamsDialog.cpp
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/filename.h"

#include "vtui/wxString2.h"
#include "ScenarioParamsDialog.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// CScenarioParamsDialog
//----------------------------------------------------------------------------

// WDR: event table for CScenarioParamsDialog

BEGIN_EVENT_TABLE(CScenarioParamsDialog,AutoDialog)
	EVT_INIT_DIALOG (CScenarioParamsDialog::OnInitDialog)
	EVT_BUTTON( ID_SCENARIO_ADD_VISIBLE_LAYER, CScenarioParamsDialog::OnScenarioAddVisibleLayer )
	EVT_BUTTON( ID_SCENARIO_REMOVE_VISIBLE_LAYER, CScenarioParamsDialog::OnScenarioRemoveVisibleLayer )
	EVT_LISTBOX( ID_SCENARIO_VISIBLE_LAYERS, CScenarioParamsDialog::OnScenarioVisibleLayers )
	EVT_LISTBOX( ID_SCENARIO_AVAILABLE_LAYERS, CScenarioParamsDialog::OnScenarioAvailableLayers )
	EVT_TEXT( ID_SCENARIO_NAME, CScenarioParamsDialog::OnScenarioNameText )
END_EVENT_TABLE()

CScenarioParamsDialog::CScenarioParamsDialog( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	// WDR: dialog function ScenarioParamsDialogFunc for CScenarioParamsDialog
	ScenarioParamsDialogFunc( this, TRUE );

	m_bModified = false;
}

// WDR: handler implementations for CScenarioParamsDialog

void CScenarioParamsDialog::OnScenarioNameText( wxCommandEvent &event )
{
	wxString2 str = event.GetString();
	m_Params.SetValueString(STR_SCENARIO_NAME, str.to_utf8());
	m_bModified = true;
}

void CScenarioParamsDialog::OnScenarioAvailableLayers( wxCommandEvent &event )
{
	UpdateEnableState();	
}

void CScenarioParamsDialog::OnScenarioVisibleLayers( wxCommandEvent &event )
{
	UpdateEnableState();
}

void CScenarioParamsDialog::OnScenarioRemoveVisibleLayer( wxCommandEvent &event )
{
	wxListBox *pAvailableLayers = GetScenarioAvailableLayers();
	wxListBox *pVisibleLayers = GetScenarioVisibleLayers();
	int iOffset;

	if (wxNOT_FOUND != (iOffset = pVisibleLayers->GetSelection()))
	{
		wxString Layer = pVisibleLayers->GetStringSelection();
		pVisibleLayers->Delete(iOffset);
		m_Params.GetActiveLayers().erase(m_Params.GetActiveLayers().begin() + iOffset);
		pAvailableLayers->Append(Layer);
		m_bModified = true;
	}
	UpdateEnableState();
}

void CScenarioParamsDialog::OnScenarioAddVisibleLayer( wxCommandEvent &event )
{
	wxListBox *pAvailableLayers = GetScenarioAvailableLayers();
	wxListBox *pVisibleLayers = GetScenarioVisibleLayers();
	int iOffset;

	if (wxNOT_FOUND != (iOffset = pAvailableLayers->GetSelection()))
	{
		wxString2 Layer = pAvailableLayers->GetStringSelection();
		pAvailableLayers->Delete(iOffset);
		pVisibleLayers->Append(Layer);
		m_Params.GetActiveLayers().push_back(Layer.to_utf8());
		m_bModified = true;
	}
	UpdateEnableState();
}

void CScenarioParamsDialog::OnInitDialog(wxInitDialogEvent& event)
{
	wxTextCtrl *pScenarioName = GetScenarioName();
	wxListBox *pAvailableLayers = GetScenarioAvailableLayers();
	wxListBox *pVisibleLayers = GetScenarioVisibleLayers();
	vtStringArray TempVisibleLayers = m_Params.GetActiveLayers();

	wxString2 str;
	str.from_utf8(m_Params.GetValueString(STR_SCENARIO_NAME));
	pScenarioName->SetValue(str);

	for (unsigned int i = 0; i < m_Layers.size(); i++)
	{
		// Look for structure layers
		vtString LayerType = m_Layers[i].GetValueString("Type");
		if (LayerType != TERR_LTYPE_STRUCTURE)
			continue;

		wxFileName FileName;
		str.from_utf8(m_Layers[i].GetValueString("Filename"));

		FileName.Assign(str);
		pAvailableLayers->Append(FileName.GetName());
	}

	// Validate the passed in visible layers and reject any that are not available
	vtStringArray::iterator iTr = TempVisibleLayers.begin();
	while (iTr != TempVisibleLayers.end())
	{
		int iOffset;
		str.from_utf8((pcchar)(*iTr));
		if (wxNOT_FOUND != (iOffset = pAvailableLayers->FindString(str)))
		{
			pVisibleLayers->Append(str);
			pAvailableLayers->Delete(iOffset);
			iTr++;
		}
		else
		{
			iTr = TempVisibleLayers.erase(iTr);
			m_bModified = true;
		}
	}

	m_Params.GetActiveLayers() = TempVisibleLayers;

	UpdateEnableState();
}

void CScenarioParamsDialog::UpdateEnableState()
{
	if (wxNOT_FOUND != GetScenarioAvailableLayers()->GetSelection())
		GetScenarioAddVisibleLayer()->Enable(true);
	else
		GetScenarioAddVisibleLayer()->Enable(false);
	if (wxNOT_FOUND != GetScenarioVisibleLayers()->GetSelection())
		GetScenarioRemoveVisibleLayer()->Enable(true);
	else
		GetScenarioRemoveVisibleLayer()->Enable(false);
}




