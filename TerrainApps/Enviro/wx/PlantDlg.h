//
// Name:		PlantDlg.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __PlantDlg_H__
#define __PlantDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "PlantDlg.cpp"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"
#include "../PlantingOptions.h"
#include "vtdata/Array.h"

class vtSpeciesList3d;
class vtPlantInstance3d;

// WDR: class declarations

//---------------------------------------------------------------------------
// PlantDlg
//---------------------------------------------------------------------------

class PlantDlg: public AutoDialog
{
public:
	// constructors and destructors
	PlantDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	void SetPlantList(vtSpeciesList3d *plants);
	void SetPlantOptions(PlantingOptions &opt);

//	void SetActivePlant(vtPlantInstance3d *p);

	void HeightToSlider();
	void ModeToRadio();

	void OnInitDialog(wxInitDialogEvent& event);
	wxSlider	*m_pHeightSlider;
	wxChoice	*m_pSpecies;
	vtSpeciesList3d   *m_pPlantList;

	// planting options
	PlantingOptions m_opt;
	int	 m_iHeightSlider;
	int	 m_iVarianceSlider;

	// WDR: method declarations for PlantDlg
	wxSlider* GetPlantVarianceSlider()  { return (wxSlider*) FindWindow( ID_PLANT_VARIANCE_SLIDER ); }
	wxRadioButton* GetPlantContinuous()  { return (wxRadioButton*) FindWindow( ID_PLANT_CONTINUOUS ); }
	wxRadioButton* GetPlantLinear()  { return (wxRadioButton*) FindWindow( ID_PLANT_LINEAR ); }
	wxRadioButton* GetPlantIndividual()  { return (wxRadioButton*) FindWindow( ID_PLANT_INDIVIDUAL ); }
	wxSlider* GetHeightSlider()  { return (wxSlider*) FindWindow( ID_HEIGHT_SLIDER ); }
	wxChoice* GetSpecies()  { return (wxChoice*) FindWindow( ID_SPECIES ); }

private:
	// WDR: member variable declarations for PlantDlg
//	vtPlantInstance3d *m_pPlant;
	bool m_bSetting;

	// Each species has a size (height) which the user has indicated
	Array<float> m_PreferredSizes;

private:
	// WDR: handler declarations for PlantDlg
	void OnVarianceSlider( wxCommandEvent &event );
	void OnVariance( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnSpacingEdit( wxCommandEvent &event );
	void OnSelChangeSpecies( wxCommandEvent &event );
	void OnHeightSlider( wxCommandEvent &event );
	void OnHeightEdit( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __PlantDlg_H__

