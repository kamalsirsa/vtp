/////////////////////////////////////////////////////////////////////////////
// Name:		FeatInfoDlg.h
// Author:	  XX
// Created:	 XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __FeatInfoDlg_H__
#define __FeatInfoDlg_H__

#ifdef __GNUG__
	#pragma interface "FeatInfoDlg.cpp"
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "VTBuilder_wdr.h"

class vtFeatures;

// WDR: class declarations

//----------------------------------------------------------------------------
// FeatInfoDlg
//----------------------------------------------------------------------------

class FeatInfoDlg: public wxDialog
{
public:
	// constructors and destructors
	FeatInfoDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetFeatureSet(vtFeatures *pFeatures);
	void Clear();
	void ShowFeature(int iFeat);

	// WDR: method declarations for FeatInfoDlg
	wxListCtrl* GetList()  { return (wxListCtrl*) FindWindow( ID_LIST ); }
	
private:
	// WDR: member variable declarations for FeatInfoDlg
	vtFeatures *m_pFeatures;

private:
	// WDR: handler declarations for FeatInfoDlg
	void OnItemSelected( wxListEvent &event );

private:
	DECLARE_EVENT_TABLE()
};




#endif
