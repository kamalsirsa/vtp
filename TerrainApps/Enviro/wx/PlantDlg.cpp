//
// Name: PlantDlg.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "PlantDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
#include "wx/image.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "PlantDlg.h"
#include "vtlib/vtlib.h"
#include "vtlib/core/Trees.h"
#include "vtui/wxString2.h"
#include "EnviroGUI.h"

// WDR: class implementations

//---------------------------------------------------------------------------
// PlantDlg
//---------------------------------------------------------------------------

// WDR: event table for PlantDlg

BEGIN_EVENT_TABLE(PlantDlg,AutoDialog)
	EVT_INIT_DIALOG (PlantDlg::OnInitDialog)
	EVT_TEXT( ID_PLANT_HEIGHT_EDIT, PlantDlg::OnHeightEdit )
	EVT_SLIDER( ID_HEIGHT_SLIDER, PlantDlg::OnHeightSlider )
	EVT_CHOICE( ID_SPECIES, PlantDlg::OnSelChangeSpecies )
	EVT_TEXT( ID_PLANT_SPACING_EDIT, PlantDlg::OnSpacingEdit )
	EVT_RADIOBUTTON( ID_PLANT_INDIVIDUAL, PlantDlg::OnRadio )
	EVT_RADIOBUTTON( ID_PLANT_LINEAR, PlantDlg::OnRadio )
	EVT_RADIOBUTTON( ID_PLANT_CONTINUOUS, PlantDlg::OnRadio )
	EVT_TEXT( ID_PLANT_VARIANCE_EDIT, PlantDlg::OnVariance )
	EVT_SLIDER( ID_PLANT_VARIANCE_SLIDER, PlantDlg::OnVarianceSlider )
END_EVENT_TABLE()

PlantDlg::PlantDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	PlantDialogFunc( this, TRUE );	

	m_pHeightSlider = GetHeightSlider();
	m_pSpecies = GetSpecies();
	m_bSetting = false;
}

void PlantDlg::SetPlantList(vtPlantList3d *plants)	
{
	if (m_pPlantList == plants)
		return;

	m_pPlantList = plants;
	if (!plants) return;

	m_pSpecies->Clear();
	wxString2 str;
	vtPlantSpecies *plant;

	int i, num = plants->NumSpecies();
	m_PreferredSizes.SetSize(num);

	for (i = 0; i < num; i++)
	{
		plant = plants->GetSpecies(i);
		str = plant->GetCommonName();
		m_pSpecies->Append(str);

		// Default to 80% of the maximum height of each species
		m_PreferredSizes[i] = plant->GetMaxHeight() * 0.80;
	}
}

void PlantDlg::SetPlantOptions(PlantingOptions &opt)
{
	m_opt = opt;
	if (m_opt.m_fHeight < 0)
		m_opt.m_fHeight = 0;

	vtPlantSpecies *pSpecies = m_pPlantList->GetSpecies(m_opt.m_iSpecies);
	if (pSpecies)
	{
		float size = pSpecies->GetMaxHeight();
		if (m_opt.m_fHeight > size)
			m_opt.m_fHeight = size * 0.80;
	}
}


/////////////////////////////////////////////////////////////////////////////
// WDR: handler implementations for PlantDlg

void PlantDlg::OnVarianceSlider( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_opt.m_iVariance = m_iVarianceSlider;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	g_App.SetPlantOptions(m_opt);
}

void PlantDlg::OnVariance( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	if (m_opt.m_iVariance < 0) m_opt.m_iVariance = 0;
	if (m_opt.m_iVariance > 100) m_opt.m_iVariance = 100;
	m_iVarianceSlider = m_opt.m_iVariance;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	g_App.SetPlantOptions(m_opt);
}

void PlantDlg::OnRadio( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	if (GetPlantIndividual()->GetValue()) m_opt.m_iMode = 0;
	if (GetPlantLinear()->GetValue()) m_opt.m_iMode = 1;
	if (GetPlantContinuous()->GetValue()) m_opt.m_iMode = 2;
	
	g_App.SetPlantOptions(m_opt);
}

void PlantDlg::OnSpacingEdit( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();

	g_App.SetPlantOptions(m_opt);
}

void PlantDlg::OnSelChangeSpecies( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();

	// show a reasonable value for the height
	m_opt.m_fHeight = m_PreferredSizes[m_opt.m_iSpecies];
	HeightToSlider();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	g_App.SetPlantOptions(m_opt);
}

void PlantDlg::OnHeightSlider( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	if (!m_pPlantList) return;

	m_iHeightSlider = m_pHeightSlider->GetValue();
	vtPlantSpecies *pSpecies = m_pPlantList->GetSpecies(m_opt.m_iSpecies);
	if (pSpecies)
		m_opt.m_fHeight = m_iHeightSlider * pSpecies->GetMaxHeight() / 100.0f;
	else
		m_opt.m_fHeight = 0.0f;
	m_PreferredSizes[m_opt.m_iSpecies] = m_opt.m_fHeight;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	g_App.SetPlantOptions(m_opt);
}

void PlantDlg::OnHeightEdit( wxCommandEvent &event )
{
	if (m_bSetting)
		return;

	TransferDataFromWindow();
	m_PreferredSizes[m_opt.m_iSpecies] = m_opt.m_fHeight;
	HeightToSlider();
	g_App.SetPlantOptions(m_opt);
}

void PlantDlg::HeightToSlider()
{
	if (!m_pPlantList) return;

	vtPlantSpecies *pSpecies = m_pPlantList->GetSpecies(m_opt.m_iSpecies);
	if (pSpecies)
		m_iHeightSlider = (int) (m_opt.m_fHeight / pSpecies->GetMaxHeight() * 100.0f);
	else
		m_iHeightSlider = 0;

	m_bSetting = true;
	m_pHeightSlider->SetValue(m_iHeightSlider);
	m_bSetting = false;
}

void PlantDlg::ModeToRadio()
{
	if (m_opt.m_iMode == 0) GetPlantIndividual()->SetValue(true);
	if (m_opt.m_iMode == 1) GetPlantLinear()->SetValue(true);
	if (m_opt.m_iMode == 2) GetPlantContinuous()->SetValue(true);
}

void PlantDlg::OnInitDialog(wxInitDialogEvent& event)	
{
	AddValidator(ID_SPECIES, &m_opt.m_iSpecies);
	AddNumValidator(ID_PLANT_HEIGHT_EDIT, &m_opt.m_fHeight);
	AddNumValidator(ID_PLANT_SPACING_EDIT, &m_opt.m_fSpacing);

	AddNumValidator(ID_PLANT_VARIANCE_EDIT, &m_opt.m_iVariance);
	AddValidator(ID_PLANT_VARIANCE_SLIDER, &m_iVarianceSlider);

	HeightToSlider();
	ModeToRadio();
	m_iVarianceSlider = m_opt.m_iVariance;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	// also keep main Enviro object in synch
	g_App.SetPlantOptions(m_opt);
}

