//
// Name: OptionsDlg.h
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __OptionsDlg_H__
#define __OptionsDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "OptionsDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "ElevLayer.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// OptionsDlg
//----------------------------------------------------------------------------

class OptionsDlg: public AutoDialog
{
public:
	// constructors and destructors
	OptionsDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	// WDR: method declarations for OptionsDlg
	wxSpinCtrl* GetSpinCastDirection()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_CAST_DIRECTION ); }
	wxSpinCtrl* GetSpinCastAngle()  { return (wxSpinCtrl*) FindWindow( ID_SPIN_CAST_ANGLE ); }
	wxCheckBox* GetCheckHideUnknown()  { return (wxCheckBox*) FindWindow( ID_CHECK_HIDE_UNKNOWN ); }
	wxRadioButton* GetRadioCastShadows()  { return (wxRadioButton*) FindWindow( ID_RADIO_CAST_SHADOWS ); }
	wxRadioButton* GetRadioSimpleShading()  { return (wxRadioButton*) FindWindow( ID_RADIO_SIMPLE_SHADING ); }
	wxRadioButton* GetRadioNoShading()  { return (wxRadioButton*) FindWindow( ID_RADIO_NO_SHADING ); }
	wxRadioButton* GetRadioColor()  { return (wxRadioButton*) FindWindow( ID_RADIO_COLOR ); }
	wxRadioButton* GetRadioOutlineOnly()  { return (wxRadioButton*) FindWindow( ID_RADIO_OUTLINE_ONLY ); }
	wxChoice* GetElevUnit()  { return (wxChoice*) FindWindow( ID_ELEVUNIT ); }

	void SetElevDrawOptions(const ElevDrawOptions &opt);
	void GetElevDrawOptions(ElevDrawOptions &opt);

	bool m_bShowToolbar;
	bool m_bShowMinutes;
	int  m_iElevUnits;

	bool m_bShowOutlines;
	bool m_bColor;
	bool m_bNoShading;
	bool m_bSimpleShading;
	bool m_bCastShadows;
	int m_iCastAngle;
	int m_iCastDirection;
	bool m_bHideUnknown;

	bool m_bShowRoadWidth;
	bool m_bShowPath;

private:
	// WDR: member variable declarations for OptionsDlg

private:
	void UpdateEnables();

	// WDR: handler declarations for OptionsDlg
	void OnRadio( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
