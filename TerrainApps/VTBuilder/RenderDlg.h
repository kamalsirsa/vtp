/////////////////////////////////////////////////////////////////////////////
// Name:		RenderDlg.h
// Author:	  XX
// Created:	 XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __RenderDlg_H__
#define __RenderDlg_H__

#ifdef __GNUG__
	#pragma interface "RenderDlg.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// RenderDlg
//----------------------------------------------------------------------------

class RenderDlg: public AutoDialog
{
public:
	// constructors and destructors
	RenderDlg( wxWindow *parent, wxWindowID id,
		const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for RenderDlg
	wxRadioButton* GetJpeg()  { return (wxRadioButton*) FindWindow( ID_JPEG ); }
	wxRadioButton* GetGeotiff()  { return (wxRadioButton*) FindWindow( ID_GEOTIFF ); }
	wxButton* GetDotdotdot()  { return (wxButton*) FindWindow( ID_DOTDOTDOT ); }
	wxTextCtrl* GetTextToFile()  { return (wxTextCtrl*) FindWindow( ID_TEXT_TO_FILE ); }
	wxCheckBox* GetTiling()  { return (wxCheckBox*) FindWindow( ID_TILING ); }
	wxButton* GetBigger()  { return (wxButton*) FindWindow( ID_BIGGER ); }
	wxButton* GetSmaller()  { return (wxButton*) FindWindow( ID_SMALLER ); }
	wxTextCtrl* GetSizeY()  { return (wxTextCtrl*) FindWindow( ID_SIZEY ); }
	wxTextCtrl* GetSizeX()  { return (wxTextCtrl*) FindWindow( ID_SIZEX ); }
	wxChoice* GetColorMap()  { return (wxChoice*) FindWindow( ID_CHOICE_COLORS ); }

	void RecomputeSize();
	void UpdateEnabling();
	void UpdateColorMapChoice();

	bool m_bToFile;
	wxString2 m_strToFile;
	bool m_bJPEG;

	vtStringArray m_datapaths;
	wxString2   m_strColorMap;
	bool m_bShading;

	int	 m_iSizeX;
	int	 m_iSizeY;
	bool m_bConstraint;
	bool m_bTiling;
	int	 m_power;

	bool m_bSetting;

private:
	// WDR: member variable declarations for RenderDlg
	
private:
	// WDR: handler declarations for RenderDlg
	void OnEditColors( wxCommandEvent &event );
	void OnBigger( wxCommandEvent &event );
	void OnSmaller( wxCommandEvent &event );
	void OnConstrain( wxCommandEvent &event );
	void OnDotdotdot( wxCommandEvent &event );
	void OnRadio( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
