//
// Name: DistribVegDlg.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "DistribVegDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "DistribVegDlg.h"
#include "Frame.h"
#include "VegLayer.h"

// WDR: class implementations

//----------------------------------------------------------------------------
// DistribVegDlg
//----------------------------------------------------------------------------

// WDR: event table for DistribVegDlg

BEGIN_EVENT_TABLE(DistribVegDlg,AutoDialog)
END_EVENT_TABLE()

DistribVegDlg::DistribVegDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	DistribVegFunc( this, TRUE ); 
}

// WDR: handler implementations for DistribVegDlg

void DistribVegDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_iChoiceSpecies = 0;
	m_iChoiceBiotype = 0;
	m_iChoiceBiotypeLayer = 0;
	m_iChoiceDensityLayer = 0;

	m_fFixedSize = 5.0f;
	m_iRandomFrom = 1;
	m_iRandomTo = 100;

	AddNumValidator(IDC_SAMPLING, &m_fSampling);
	AddNumValidator(IDC_SCARCITY, &m_fScarcity);

	AddValidator(ID_CHOICE_SPECIES, &m_iChoiceSpecies);
	AddValidator(ID_CHOICE_BIOTYPE, &m_iChoiceBiotype);
	AddValidator(ID_CHOICE_BIOTYPE_LAYER, &m_iChoiceBiotypeLayer);

	AddValidator(ID_CHOICE_DENSITY_LAYER, &m_iChoiceDensityLayer);

	AddNumValidator(ID_TEXT_FIXED_SIZE, &m_fFixedSize);
	AddValidator(ID_SPIN_RANDOM_FROM, &m_iRandomFrom);
	AddValidator(ID_SPIN_RANDOM_TO, &m_iRandomTo);

	//
	// Populate the Choice controls with necessary values
	//
	MainFrame *frame = GetMainFrame();
	wxString2 str, str2;
	unsigned int i;

	// Single species
	GetChoiceSpecies()->Clear();
	vtPlantList* pl = GetMainFrame()->GetPlantList();
	if (pl != NULL)
	{
		for (i = 0; i < pl->NumSpecies(); i++)
		{
			vtPlantSpecies *spe = pl->GetSpecies(i);
			str = spe->GetSciName();
			GetChoiceSpecies()->Append(str);
		}
	}
	GetChoiceSpecies()->SetSelection(0);

	// Single biotype
	GetChoiceBiotype()->Clear();
	vtBioRegion *br = GetMainFrame()->GetBioRegion();
	if (br != NULL)
	{
		for (i = 0; i < br->m_Types.GetSize(); i++)
		{
			vtBioType *bt = br->m_Types[i];
			str.Printf(_T("(%d) "), i);
			str2 = bt->m_name;
			str += str2;
			GetChoiceBiotype()->Append(str);
		}
	}
	GetChoiceBiotype()->SetSelection(0);

	// Biotype layers
	for (int i = 0; i < frame->NumLayers(); i++)
	{
		vtLayer *lp = frame->GetLayer(i);
		if (lp->GetType() == LT_VEG)
		{
			vtVegLayer *vl = (vtVegLayer *) lp;
			if (vl->GetVegType() == VLT_BioMap)
				GetChoiceBiotypeLayer()->Append(vl->GetLayerFilename());
		}
	}
	GetChoiceBiotypeLayer()->SetSelection(0);

	// Density layers
	int iDensityLayers = 0;
	for (int i = 0; i < frame->NumLayers(); i++)
	{
		vtLayer *lp = frame->GetLayer(i);
		if (lp->GetType() == LT_VEG)
		{
			vtVegLayer *vl = (vtVegLayer *) lp;
			if (vl->GetVegType() == VLT_Density)
			{
				GetChoiceDensityLayer()->Append(vl->GetLayerFilename());
				iDensityLayers ++;
			}
		}
	}
	GetChoiceDensityLayer()->SetSelection(0);

	// Species
	GetSpecies1()->SetValue(false);
	GetSpecies2()->SetValue(false);
	GetSpecies3()->SetValue(true);

	// not yet
	GetSpecies1()->Enable(false);
	GetChoiceSpecies()->Enable(false);
	GetSpecies2()->Enable(false);
	GetChoiceBiotype()->Enable(false);

	// Density
	bool bHaveDensityLayer = (iDensityLayers != 0);
	GetDensity1()->SetValue(!bHaveDensityLayer);
	GetDensity2()->SetValue(bHaveDensityLayer);
	GetDensity1()->Enable(!bHaveDensityLayer);
	GetDensity2()->Enable(bHaveDensityLayer);
	GetChoiceDensityLayer()->Enable(bHaveDensityLayer);

	// Size
	GetSize1()->SetValue(false);
	GetSize2()->SetValue(true);
	GetSize1()->Enable(false);
	GetTextFixedSize()->Enable(false);
	GetSpinRandomFrom()->Enable(false);
	GetSpinRandomTo()->Enable(false);

	wxDialog::OnInitDialog(event);  // transfers data to window
}

