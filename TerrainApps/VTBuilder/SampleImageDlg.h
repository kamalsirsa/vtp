//
// Name: SampleImageDlg.h
//
// Copyright (c) 2003-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __SampleImageDlg_H__
#define __SampleImageDlg_H__

#include "VTBuilder_UI.h"
#include "vtdata/MathTypes.h"
#include "TileDlg.h"

class BuilderView;

// WDR: class declarations

//----------------------------------------------------------------------------
// SampleImageDlg
//----------------------------------------------------------------------------

class SampleImageDlg: public SampleImageDlgBase
{
public:
	// constructors and destructors
	SampleImageDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for SampleImageDlg
	wxButton* GetDotDotDot()  { return (wxButton*) FindWindow( ID_DOTDOTDOT ); }
	wxButton* GetTileOptions()  { return (wxButton*) FindWindow( ID_TILE_OPTIONS ); }
	wxTextCtrl* GetTextToFile()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TO_FILE ); }
	wxTextCtrl* GetTextTileInfo()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TILE_INFO ); }
	wxRadioButton* GetRadioToFile()  { return (wxRadioButton*) FindWindow( ID_RADIO_TO_FILE ); }
	wxRadioButton* GetRadioCreateNew()  { return (wxRadioButton*) FindWindow( ID_RADIO_CREATE_NEW ); }
	wxTextCtrl* GetSpacingY()  { return (wxTextCtrl*) FindWindow( ID_SPACINGY ); }
	wxTextCtrl* GetSpacingX()  { return (wxTextCtrl*) FindWindow( ID_SPACINGX ); }
	wxTextCtrl* GetSizeY()  { return (wxTextCtrl*) FindWindow( ID_SIZEY ); }
	wxTextCtrl* GetSizeX()  { return (wxTextCtrl*) FindWindow( ID_SIZEX ); }
	wxButton* GetBigger()  { return (wxButton*) FindWindow( ID_BIGGER ); }
	wxButton* GetSmaller()  { return (wxButton*) FindWindow( ID_SMALLER ); }
	wxCheckBox* GetConstrain()  { return (wxCheckBox*) FindWindow( ID_CONSTRAIN ); }
	wxCheckBox* Get4x4Tiling()  { return (wxCheckBox*) FindWindow( ID_TILING ); }

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
	int	 m_iSizeX;
	int	 m_iSizeY;
	bool	m_bConstraint;
	bool	m_bTiling;

	double  m_fAreaX;
	double  m_fAreaY;
	double  m_fEstX;
	double  m_fEstY;

	DRECT   m_area;
	int		m_power;
	bool	m_bSetting;

	TilingOptions	m_tileopts;

private:
	// WDR: member variable declarations for SampleImageDlg
	BuilderView *m_pView;

private:
	// WDR: handler declarations for SampleImageDlg
	void OnRadioOutput( wxCommandEvent &event );
	void OnTileOptions( wxCommandEvent &event );
	void OnDotDotDot( wxCommandEvent &event );
	void OnSpacingXY( wxCommandEvent &event );
	void OnSizeXY( wxCommandEvent &event );
	void OnConstrain( wxCommandEvent &event );
	void OnBigger( wxCommandEvent &event );
	void OnSmaller( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif	// __SampleImageDlg_H__

