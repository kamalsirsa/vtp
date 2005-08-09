//
// Name: ScenarioParamsDialog.h
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ScenarioParamsDialog_H__
#define __ScenarioParamsDialog_H__

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/TParams.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// CScenarioParamsDialog
//----------------------------------------------------------------------------

class CScenarioParamsDialog: public AutoDialog
{
public:
	// constructors and destructors
	CScenarioParamsDialog( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for CScenarioParamsDialog
	wxTextCtrl* GetScenarioName()  { return (wxTextCtrl*) FindWindow( ID_SCENARIO_NAME ); }
	wxButton* GetScenarioRemoveVisibleLayer()  { return (wxButton*) FindWindow( ID_SCENARIO_REMOVE_VISIBLE_LAYER ); }
	wxButton* GetScenarioAddVisibleLayer()  { return (wxButton*) FindWindow( ID_SCENARIO_ADD_VISIBLE_LAYER ); }
	wxListBox* GetScenarioAvailableLayers()  { return (wxListBox*) FindWindow( ID_SCENARIO_AVAILABLE_LAYERS ); }
	wxListBox* GetScenarioVisibleLayers()  { return (wxListBox*) FindWindow( ID_SCENARIO_VISIBLE_LAYERS ); }
	ScenarioParams& GetParams() { return m_Params; }
	void SetParams(ScenarioParams& Params)
	{
		m_Params = Params;
		m_bModified = false;
	}
	void SetAvailableLayers(std::vector<vtTagArray>& Layers) { m_Layers = Layers; }
	bool IsModified() { return m_bModified; }
	
private:
	// WDR: member variable declarations for CScenarioParamsDialog
	
private:
	// WDR: handler declarations for CScenarioParamsDialog
	void OnScenarioNameText( wxCommandEvent &event );
	void OnScenarioAvailableLayers( wxCommandEvent &event );
	void OnScenarioVisibleLayers( wxCommandEvent &event );
	void OnScenarioRemoveVisibleLayer( wxCommandEvent &event );
	void OnScenarioAddVisibleLayer( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()

protected:
	void UpdateEnableState();

	ScenarioParams m_Params;
	std::vector<vtTagArray> m_Layers;
	bool m_bModified;
};

#endif
