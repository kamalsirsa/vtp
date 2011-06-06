//
// Name: ResampleDlg.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ResampleDlg_H__
#define __ResampleDlg_H__

#include "VTBuilder_UI.h"
#include "vtdata/MathTypes.h"
#include "vtui/AutoDialog.h"
#include "TileDlg.h"
#include "ElevDrawOptions.h"

class BuilderView;

// WDR: class declarations

//----------------------------------------------------------------------------
// ResampleDlg
//----------------------------------------------------------------------------

class ResampleDlg: public ResampleDlgBase
{
public:
	// constructors and destructors
	ResampleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ResampleDlg
	wxButton* GetDotdotdot2()  { return (wxButton*) FindWindow( ID_DOTDOTDOT2 ); }
	wxTextCtrl* GetTextToImageFile()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TO_IMAGE_FILE ); }
	wxButton* GetRenderingOptions()  { return (wxButton*) FindWindow( ID_RENDERING_OPTIONS ); }
	wxCheckBox* GetDerivedImages()  { return (wxCheckBox*) FindWindow( ID_DERIVED_IMAGES ); }
	wxButton* GetDotDotDot()  { return (wxButton*) FindWindow( ID_DOTDOTDOT ); }
	wxButton* GetTileOptions()  { return (wxButton*) FindWindow( ID_TILE_OPTIONS ); }
	wxTextCtrl* GetTextToFile()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TO_FILE ); }
	wxTextCtrl* GetTextTileInfo()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TILE_INFO ); }
	wxRadioButton* GetRadioToFile()  { return (wxRadioButton*) FindWindow( ID_RADIO_TO_FILE ); }
	wxRadioButton* GetRadioCreateNew()  { return (wxRadioButton*) FindWindow( ID_RADIO_CREATE_NEW ); }
	wxRadioButton* GetShorts()  { return (wxRadioButton*) FindWindow( ID_SHORTS ); }
	wxTextCtrl* GetVUnits()  { return (wxTextCtrl*) FindWindow( ID_VUNITS ); }
	wxTextCtrl* GetSpacingY()  { return (wxTextCtrl*) FindWindow( ID_SPACINGY ); }
	wxTextCtrl* GetSpacingX()  { return (wxTextCtrl*) FindWindow( ID_SPACINGX ); }
	wxTextCtrl* GetSizeY()  { return (wxTextCtrl*) FindWindow( ID_SIZEY ); }
	wxTextCtrl* GetSizeX()  { return (wxTextCtrl*) FindWindow( ID_SIZEX ); }
	wxButton* GetBigger()  { return (wxButton*) FindWindow( ID_BIGGER ); }
	wxButton* GetSmaller()  { return (wxButton*) FindWindow( ID_SMALLER ); }
	wxCheckBox* GetConstrain()  { return (wxCheckBox*) FindWindow( ID_CONSTRAIN ); }

	void SetView(BuilderView *pView) { m_pView = pView; }
	void RecomputeSize();
	void FormatTilingString();
	void EnableBasedOnConstraint();

	bool m_bNewLayer;
	bool m_bToFile;
	bool m_bToTiles;
	wxString m_strToFile;
	wxString m_strTileInfo;

	double  m_fSpacingX;
	double  m_fSpacingY;
	int  m_iSizeX;
	int  m_iSizeY;
	bool	m_bConstraint;

	double  m_fAreaX;
	double  m_fAreaY;
	double  m_fEstX;
	double  m_fEstY;

	bool	m_bFloats;
	float   m_fVUnits;

	DRECT   m_area;
	bool	m_bFillGaps;

	// Where to write the derived image tiles, if we will create them.
	wxString	m_strToFileImages;

	TilingOptions   m_tileopts;

private:
	// WDR: member variable declarations for ResampleDlg
	wxNumericValidator *spacing1, *spacing2;
	BuilderView *m_pView;
	int		m_power;
	bool	m_bSetting;

private:
	// WDR: handler declarations for ResampleDlg
	void OnDotDotDot2( wxCommandEvent &event );
	void OnRenderingOptions( wxCommandEvent &event );
	void OnCheckDerivedImages( wxCommandEvent &event );
	void OnTileOptions( wxCommandEvent &event );
	void OnDotDotDot( wxCommandEvent &event );
	void OnRadioOutput( wxCommandEvent &event );
	void OnShorts( wxCommandEvent &event );
	void OnFloats( wxCommandEvent &event );
	void OnSpacingXY( wxCommandEvent &event );
	void OnSizeXY( wxCommandEvent &event );
	void OnConstrain( wxCommandEvent &event );
	void OnBigger( wxCommandEvent &event );
	void OnSmaller( wxCommandEvent &event );
	void OnTextToImageFile( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ResampleDlg_H__

