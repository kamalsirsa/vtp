/////////////////////////////////////////////////////////////////////////////
// Name:		SelectDlg.h
// Author:	  XX
// Created:	 XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __SelectDlg_H__
#define __SelectDlg_H__

#ifdef __GNUG__
	#pragma interface "SelectDlg.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"
#include "AutoDialog.h"

class vtRawLayer;

// WDR: class declarations

//----------------------------------------------------------------------------
// SelectDlg
//----------------------------------------------------------------------------

class SelectDlg: public AutoDialog
{
public:
	// constructors and destructors
	SelectDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for SelectDlg
	wxComboBox* GetComboValue()  { return (wxComboBox*) FindWindow( ID_COMBO_VALUE ); }
	wxChoice* GetCondition()  { return (wxChoice*) FindWindow( ID_CONDITION ); }
	wxListBox* GetField()  { return (wxListBox*) FindWindow( ID_FIELD ); }
	void FillValuesControl();
	void SetRawLayer(vtRawLayer *pRL);

	int			m_iField;
	int			m_iCondition;
	wxString	m_strValue;

private:
	// WDR: member variable declarations for SelectDlg
	vtRawLayer	*m_pLayer;
	bool		m_bSetting;

private:
	// WDR: handler declarations for SelectDlg
	void OnChoiceField( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);
	void OnOK( wxCommandEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
