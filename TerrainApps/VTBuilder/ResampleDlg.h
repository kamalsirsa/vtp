//
// Name: ResampleDlg.h
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __ResampleDlg_H__
#define __ResampleDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "ResampleDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "vtdata/MathTypes.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// ResampleDlg
//----------------------------------------------------------------------------

class ResampleDlg: public AutoDialog
{
public:
	// constructors and destructors
	ResampleDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for ResampleDlg
	wxButton* GetDotDotDot()  { return (wxButton*) FindWindow( ID_DOTDOTDOT ); }
	wxTextCtrl* GetTextToFile()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TO_FILE ); }
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
	void RecomputeSize();
	void EnableBasedOnConstraint();

	bool m_bNewLayer;
	bool m_bToFile;
	wxString2 m_strToFile;

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
	int	 m_power;
	bool	m_bSetting;

private:
	// WDR: member variable declarations for ResampleDlg
	wxNumericValidator *spacing1, *spacing2;
	
private:
	// WDR: handler declarations for ResampleDlg
	void OnDotDotDot( wxCommandEvent &event );
	void OnRadioOutput( wxCommandEvent &event );
	void OnShorts( wxCommandEvent &event );
	void OnFloats( wxCommandEvent &event );
	void OnSpacingXY( wxCommandEvent &event );
	void OnSizeXY( wxCommandEvent &event );
	void OnConstrain( wxCommandEvent &event );
	void OnBigger( wxCommandEvent &event );
	void OnSmaller( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};

#endif  // __ResampleDlg_H__

