//
// Name: OptionsDlg.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __OptionsDlg_H__
#define __OptionsDlg_H__

#ifdef __GNUG__
	#pragma interface "OptionsDlg.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "enviro_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtui/wxString2.h"
#include "../Options.h"

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
	wxChoice* GetContent()  { return (wxChoice*) FindWindow( ID_CHOICE_CONTENT ); }

	void GetOptionsFrom(EnviroOptions &opt);
	void PutOptionsTo(EnviroOptions &opt);

private:
	// WDR: member variable declarations for OptionsDlg
	bool	m_bFullscreen;
//	bool	m_bHtmlpane;
//	bool	m_bFloatingToolbar;
	bool	m_bTextureCompression;
//	bool	m_bSpeedTest;
	bool	m_bDisableMipmaps;

	float	m_fSelectionCutoff;
	float	m_fMaxPickableInstanceRadius;

	float	m_fPlantScale;
//	bool	m_bShadows;
	bool	m_bOnlyAvailableSpecies;

	int			m_iContentFile;
	wxString2	m_strContentFile;

private:
	// WDR: handler declarations for OptionsDlg
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
