//
// Name:		PlantDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "PlantDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "PlantDlg.h"
#include "vtlib/vtlib.h"
#include "vtlib/core/Trees.h"
#include "../Enviro.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// PlantDlg
//----------------------------------------------------------------------------

// WDR: event table for PlantDlg

BEGIN_EVENT_TABLE(PlantDlg,AutoDialog)
	EVT_INIT_DIALOG (PlantDlg::OnInitDialog)
	EVT_TEXT( ID_SIZEEDIT, PlantDlg::OnSizeEdit )
	EVT_SLIDER( ID_SIZESLIDER, PlantDlg::OnSizeSlider )
	EVT_CHOICE( ID_SPECIES, PlantDlg::OnSelChangeSpecies )
	EVT_TEXT( ID_PLANT_SPACING_EDIT, PlantDlg::OnSpacingEdit )
END_EVENT_TABLE()

PlantDlg::PlantDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	PlantDialogFunc( this, TRUE ); 

	m_pSizeSlider = GetSizeslider();
	m_pSpecies = GetSpecies();
}

void PlantDlg::SetPlantList(vtPlantList3d *plants) 
{
	m_pPlantList = plants;
	if (!plants) return;

	m_pSpecies->Clear();
	for (int i = 0; i < plants->NumSpecies(); i++)
	{
		vtPlantSpecies3d *plant = plants->GetSpecies(i);
		m_pSpecies->Append(plant->GetCommonName());
	}
}

// WDR: handler implementations for PlantDlg

void PlantDlg::OnSpacingEdit( wxCommandEvent &event )
{
	TransferDataFromWindow();

	g_App.SetPlantOptions(m_iSpecies, m_fSize, m_fSpacing);
}

void PlantDlg::OnSelChangeSpecies( wxCommandEvent &event )
{
	TransferDataFromWindow();

	g_App.SetPlantOptions(m_iSpecies, m_fSize, m_fSpacing);
}

void PlantDlg::OnSizeSlider( wxCommandEvent &event )
{
	m_iSizeSlider = m_pSizeSlider->GetValue();
	vtPlantSpecies *pSpecies = m_pPlantList->GetSpecies(m_iSpecies);
	if (pSpecies)
		m_fSize = m_iSizeSlider * pSpecies->GetMaxHeight() / 100.0f;
	else
		m_fSize = 0.0f;
	TransferDataToWindow();

	g_App.SetPlantOptions(m_iSpecies, m_fSize, m_fSpacing);
}

void PlantDlg::OnSizeEdit( wxCommandEvent &event )
{
	TransferDataFromWindow();

    vtPlantSpecies *pSpecies = m_pPlantList->GetSpecies(m_iSpecies);
	if (pSpecies)
	    m_iSizeSlider = (int) (m_fSize / pSpecies->GetMaxHeight() * 100.0f);
	else
		m_iSizeSlider = 0;
	m_pSizeSlider->SetValue(m_iSizeSlider);

	g_App.SetPlantOptions(m_iSpecies, m_fSize, m_fSpacing);
}

void PlantDlg::OnInitDialog(wxInitDialogEvent& event) 
{
	AddValidator(ID_SPECIES, &m_iSpecies);
	AddNumValidator(ID_SIZEEDIT, &m_fSize);
	AddNumValidator(ID_PLANT_SPACING_EDIT, &m_fSpacing);

	m_iSpecies = 0;
	m_fSize = 2.0f;
	m_fSpacing = 4.0f;
	m_iSizeSlider = 1;

	TransferDataToWindow();

	g_App.SetPlantOptions(m_iSpecies, m_fSize, m_fSpacing);
}

