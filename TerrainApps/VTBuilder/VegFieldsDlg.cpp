//
// Name:		VegFieldsDlg.cpp
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "VegFieldsDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "VegFieldsDlg.h"
#include "Frame.h"		// for Plants List

// WDR: class implementations

//----------------------------------------------------------------------------
// VegFieldsDlg
//----------------------------------------------------------------------------

// WDR: event table for VegFieldsDlg

BEGIN_EVENT_TABLE(VegFieldsDlg,AutoDialog)
	EVT_CHOICE( ID_SPECIES_FIELD, VegFieldsDlg::OnChoice1 )
	EVT_CHOICE( ID_HEIGHT_FIELD, VegFieldsDlg::OnChoice2 )
	EVT_BUTTON( wxID_OK, VegFieldsDlg::OnOK )
	EVT_RADIOBUTTON( ID_USE_SPECIES, VegFieldsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_SPECIES_USE_FIELD, VegFieldsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_HEIGHT_RANDOM, VegFieldsDlg::OnRadio )
	EVT_RADIOBUTTON( ID_HEIGHT_USE_FIELD, VegFieldsDlg::OnRadio )
END_EVENT_TABLE()

VegFieldsDlg::VegFieldsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	VegFieldsDialogFunc( this, TRUE ); 
}

void VegFieldsDlg::SetShapefileName(const char *filename)
{
	m_filename = filename;
}

// WDR: handler implementations for VegFieldsDlg

void VegFieldsDlg::OnRadio( wxCommandEvent &event )
{
	// When a radio button is pressed, we should adjust which controls are
	// enabled and disabled.
	TransferDataFromWindow();

	RefreshEnabled();
}

void VegFieldsDlg::RefreshEnabled()
{
	GetSpeciesChoice()->Enable(m_bUseSpecies);

	GetSpeciesField()->Enable(m_bSpeciesUseField);
	GetSpeciesId()->Enable(m_bSpeciesUseField);
	GetSpeciesName()->Enable(m_bSpeciesUseField);
	GetCommonName()->Enable(m_bSpeciesUseField);
	GetBiotypeInt()->Enable(m_bSpeciesUseField);
	GetBiotypeString()->Enable(m_bSpeciesUseField);

	GetHeightField()->Enable(m_bHeightUseField);
}

void VegFieldsDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// Open the SHP File
	m_hSHP = SHPOpen(m_filename, "rb");
	if (m_hSHP == NULL)
	{
		wxMessageBox("Couldn't open shapefile.");
		return;
	}

	// Get number of polys and type of data
	double  adfMinBound[4], adfMaxBound[4];
	SHPGetInfo(m_hSHP, &m_nElem, &m_nShapeType, adfMinBound, adfMaxBound);

	// Open DBF File
	DBFHandle m_db = DBFOpen(m_filename, "rb");
	if (m_db == NULL)
	{
		wxMessageBox("Couldn't open DBF file.");
		return;
	}

	int i, *pnWidth = 0, *pnDecimals = 0;
	DBFFieldType fieldtype;
	char pszFieldName[20];
	m_iFields = DBFGetFieldCount(m_db);
	for (i = 0; i < m_iFields; i++)
	{
		fieldtype = DBFGetFieldInfo(m_db, i,
			pszFieldName, pnWidth, pnDecimals );

		if (fieldtype == FTString || fieldtype == FTInteger)
			GetSpeciesField()->Append(pszFieldName);

		if (fieldtype == FTInteger || fieldtype == FTDouble)
			GetHeightField()->Append(pszFieldName);
	}

	m_bUseSpecies = true;
	m_bSpeciesUseField = false;

	AddValidator(ID_USE_SPECIES, &m_bUseSpecies);
	AddValidator(ID_SPECIES_USE_FIELD, &m_bSpeciesUseField);

	m_bHeightRandomize = true;
	m_bHeightUseField = false;

	AddValidator(ID_HEIGHT_RANDOM, &m_bHeightRandomize);
	AddValidator(ID_HEIGHT_USE_FIELD, &m_bHeightUseField);

	wxDialog::OnInitDialog(event);  // calls TransferDataToWindow()

	RefreshEnabled();
}

void VegFieldsDlg::OnChoice1( wxCommandEvent &event )
{
	
}

void VegFieldsDlg::OnChoice2( wxCommandEvent &event )
{
	
}

void VegFieldsDlg::OnOK( wxCommandEvent &event )
{
	
}


