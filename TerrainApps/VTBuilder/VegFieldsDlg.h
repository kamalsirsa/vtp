//
// Name:		VegFieldsDlg.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __VegFieldsDlg_H__
#define __VegFieldsDlg_H__

#ifdef __GNUG__
	#pragma interface "VegFieldsDlg.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "vtdata/shapelib/shapefil.h"
#include "vtui/AutoDialog.h"

class vtVegLayer;

// WDR: class declarations

//----------------------------------------------------------------------------
// VegFieldsDlg
//----------------------------------------------------------------------------

class VegFieldsDlg: public AutoDialog
{
public:
	// constructors and destructors
	VegFieldsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetShapefileName(const char *filename);
	void SetVegLayer(vtVegLayer *pLayer) { m_pLayer = pLayer; }

	// WDR: method declarations for VegFieldsDlg
	wxRadioButton* GetHeightRandom()  { return (wxRadioButton*) FindWindow( ID_HEIGHT_RANDOM ); }
	wxChoice* GetSpeciesChoice()  { return (wxChoice*) FindWindow( ID_SPECIES_CHOICE ); }
	wxRadioButton* GetUseSpecies()  { return (wxRadioButton*) FindWindow( ID_USE_SPECIES ); }
	wxRadioButton* GetBiotypeString()  { return (wxRadioButton*) FindWindow( ID_BIOTYPE_STRING ); }
	wxRadioButton* GetBiotypeInt()  { return (wxRadioButton*) FindWindow( ID_BIOTYPE_INT ); }
	wxRadioButton* GetCommonName()  { return (wxRadioButton*) FindWindow( ID_COMMON_NAME ); }
	wxRadioButton* GetSpeciesName()  { return (wxRadioButton*) FindWindow( ID_SPECIES_NAME ); }
	wxRadioButton* GetSpeciesId()  { return (wxRadioButton*) FindWindow( ID_SPECIES_ID ); }
	wxChoice* GetHeightField()  { return (wxChoice*) FindWindow( ID_HEIGHT_FIELD ); }
	wxChoice* GetSpeciesField()  { return (wxChoice*) FindWindow( ID_SPECIES_FIELD ); }
	
private:
	// WDR: member variable declarations for VegFieldsDlg
	vtVegLayer *m_pLayer;

	SHPHandle m_hSHP;
	int	   m_nElem, m_nShapeType;
	wxString  m_filename;
	DBFHandle m_db;
	int	   m_iFields;

	bool	m_bUseSpecies;
	bool	m_bSpeciesUseField;
	bool	m_bHeightRandomize;
	bool	m_bHeightUseField;

	void RefreshEnabled();

private:
	// WDR: handler declarations for VegFieldsDlg
	void OnRadio( wxCommandEvent &event );
	void OnOK( wxCommandEvent &event );
	void OnChoice2( wxCommandEvent &event );
	void OnChoice1( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
