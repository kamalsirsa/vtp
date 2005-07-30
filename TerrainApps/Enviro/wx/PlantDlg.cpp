//
// Name: PlantDlg.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
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
	EVT_CHECKBOX( ID_COMMON_NAMES, PlantDlg::OnCommonNames )
	EVT_CHOICE( ID_LANGUAGE, PlantDlg::OnLanguage )
END_EVENT_TABLE()

PlantDlg::PlantDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	PlantDialogFunc( this, TRUE );

	m_pHeightSlider = GetHeightSlider();
	m_pSpecies = GetSpecies();
	m_bSetting = false;
	m_bCommonNames = true;
	m_iLanguage = 0;
	m_iSpeciesChoice = -1;
	m_strLang = _T("en");

	m_opt.m_fSpacing = 2;

	AddValidator(ID_SPECIES, &m_iSpeciesChoice);
	AddValidator(ID_COMMON_NAMES, &m_bCommonNames);
	AddValidator(ID_LANGUAGE, &m_iLanguage);
	AddNumValidator(ID_PLANT_HEIGHT_EDIT, &m_opt.m_fHeight);
	AddNumValidator(ID_PLANT_SPACING_EDIT, &m_opt.m_fSpacing);

	AddNumValidator(ID_PLANT_VARIANCE_EDIT, &m_opt.m_iVariance);
	AddValidator(ID_PLANT_VARIANCE_SLIDER, &m_iVarianceSlider);
}

void PlantDlg::SetPlantList(vtSpeciesList3d *plants)	
{
	if (m_pPlantList == plants)
		return;
	m_pPlantList = plants;
	UpdatePlantSizes();
	UpdatePlantNames();
}

void PlantDlg::SetPlantOptions(PlantingOptions &opt)
{
	m_opt = opt;

	if (m_opt.m_iSpecies == -1)
	{
		// select first plant
		m_iSpeciesChoice = 0;
		SpeciesIndexToSpeciesId();
		UpdateHeightFromSpecies();
	}

	// safety check
	if (m_opt.m_fHeight < 0)
		m_opt.m_fHeight = 0;

	vtPlantSpecies *ps = m_pPlantList->GetSpecies(m_opt.m_iSpecies);
	if (ps)
	{
		float size = ps->GetMaxHeight();
		if (m_opt.m_fHeight > size)		// safety check
			m_opt.m_fHeight = size * 0.80f;
	}
	SpeciesIdToSpeciesIndex();

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	// Set them back again, to confirm we're in synch
	g_App.SetPlantOptions(m_opt);
}

void PlantDlg::UpdatePlantSizes()
{
	if (!m_pPlantList)
		return;

	unsigned int num = m_pPlantList->NumSpecies();
	m_PreferredSizes.SetSize(num);
	for (unsigned int i = 0; i < num; i++)
	{
		// Default to 80% of the maximum height of each species
		vtPlantSpecies *plant = m_pPlantList->GetSpecies(i);
		m_PreferredSizes[i] = plant->GetMaxHeight() * 0.80f;
	}
}

void PlantDlg::UpdatePlantNames()
{
	// if we are changing, and the control is already populated, try to keep
	//  the same plant selected, to avoid UI disruption
	vtPlantSpecies *previous = NULL;
	if (m_iSpeciesChoice != -1)
		previous = (vtPlantSpecies *) m_pSpecies->GetClientData(m_iSpeciesChoice);

	m_pSpecies->Clear();
	GetLanguage()->Clear();
	GetLanguage()->Append(_T("en"));

	if (!m_pPlantList)
		return;

	wxString2 str;
	for (unsigned int i = 0; i < m_pPlantList->NumSpecies(); i++)
	{
		vtPlantSpecies *plant = m_pPlantList->GetSpecies(i);

		// filter
		if (m_bOnlyAvailableSpecies)
		{
			vtPlantSpecies3d *plant3d = dynamic_cast<vtPlantSpecies3d *>(plant);
			if (plant3d != NULL)
			{
				if (plant3d->NumAvailableInstances() == 0)
					continue;
			}
		}

		if (m_bCommonNames)
		{
			for (unsigned int j = 0; j < plant->NumCommonNames(); j++)
			{
				vtPlantSpecies::CommonName cname = plant->GetCommonName(j);

				if (cname.m_strLang == m_strLang.mb_str())
				{
					str.from_utf8(cname.m_strName);
					m_pSpecies->Append(str, plant);
				}

				str = cname.m_strLang;
				if (GetLanguage()->FindString(str) == -1)
				{
					GetLanguage()->Append(str);
				}
			}
		}
		else
		{
			str = plant->GetSciName();
			m_pSpecies->Append(str, plant);
		}
	}

	if (previous != NULL)
	{
		// look for a corresponding entry
		for (int j = 0; j < m_pSpecies->GetCount(); j++)
		{
			void *data = m_pSpecies->GetClientData(j);
			if (data == previous)
			{
				m_iSpeciesChoice = j;
				break;
			}
		}
	}
	if (m_bCommonNames)
		m_iLanguage = GetLanguage()->FindString(m_strLang);
}

void PlantDlg::UpdateEnabling()
{
	GetLanguage()->Enable(m_bCommonNames);
}

void PlantDlg::SpeciesIdToSpeciesIndex()
{
	// look up corresponding species choice index
	vtPlantSpecies *ps = m_pPlantList->GetSpecies(m_opt.m_iSpecies);
	for (int i = 0; i < m_pSpecies->GetCount(); i++)
	{
		if (ps == m_pSpecies->GetClientData(i))
		{
			m_iSpeciesChoice = i;
			break;
		}
	}
}
void PlantDlg::SpeciesIndexToSpeciesId()
{
	// convert displayed species index to a real species id
	vtPlantSpecies *ps = (vtPlantSpecies *) m_pSpecies->GetClientData(m_iSpeciesChoice);
	m_opt.m_iSpecies = m_pPlantList->FindSpeciesId(ps);
}

void PlantDlg::UpdateHeightFromSpecies()
{
	// show a reasonable value for the height
	m_opt.m_fHeight = m_PreferredSizes[m_opt.m_iSpecies];
	HeightToSlider();
}


/////////////////////////////////////////////////////////////////////////////
// WDR: handler implementations for PlantDlg

void PlantDlg::OnLanguage( wxCommandEvent &event )
{
	TransferDataFromWindow();
	m_strLang = GetLanguage()->GetStringSelection();
	UpdatePlantNames();
	TransferDataToWindow();
}

void PlantDlg::OnCommonNames( wxCommandEvent &event )
{
	TransferDataFromWindow();
	UpdatePlantNames();
	UpdateEnabling();
	TransferDataToWindow();
}

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

	SpeciesIndexToSpeciesId();
	UpdateHeightFromSpecies();

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
	HeightToSlider();
	ModeToRadio();
	m_iVarianceSlider = m_opt.m_iVariance;

	m_bSetting = true;
	TransferDataToWindow();
	m_bSetting = false;

	UpdateEnabling();

	// also keep main Enviro object in synch
	g_App.SetPlantOptions(m_opt);
}

