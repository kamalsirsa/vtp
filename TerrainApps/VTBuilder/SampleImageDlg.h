//
// Name: SampleImageDlg.h
//
// Copyright (c) 2003-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __SampleImageDlg_H__
#define __SampleImageDlg_H__

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/MathTypes.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// SampleImageDlg
//----------------------------------------------------------------------------

class SampleImageDlg: public AutoDialog
{
public:
	// constructors and destructors
	SampleImageDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for SampleImageDlg
	wxTextCtrl* GetSpacingY()  { return (wxTextCtrl*) FindWindow( ID_SPACINGY ); }
	wxTextCtrl* GetSpacingX()  { return (wxTextCtrl*) FindWindow( ID_SPACINGX ); }
	wxTextCtrl* GetSizeY()  { return (wxTextCtrl*) FindWindow( ID_SIZEY ); }
	wxTextCtrl* GetSizeX()  { return (wxTextCtrl*) FindWindow( ID_SIZEX ); }
	wxButton* GetBigger()  { return (wxButton*) FindWindow( ID_BIGGER ); }
	wxButton* GetSmaller()  { return (wxButton*) FindWindow( ID_SMALLER ); }
	wxCheckBox* GetTiling()  { return (wxCheckBox*) FindWindow( ID_TILING ); }
	void RecomputeSize();
	void EnableBasedOnConstraint();

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

private:
	// WDR: member variable declarations for SampleImageDlg
	
private:
	// WDR: handler declarations for SampleImageDlg
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

