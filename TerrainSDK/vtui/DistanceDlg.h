//
// Name: DistanceDlg.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __DistanceDlg_H__
#define __DistanceDlg_H__

#if defined(__GNUG__) && !defined(__APPLE__)
	#pragma interface "DistanceDlg.cpp"
#endif

#include "vtui_wdr.h"
#include "vtui/AutoDialog.h"
#include "vtdata/Projections.h"

// WDR: class declarations

//----------------------------------------------------------------------------
// DistanceDlg
//----------------------------------------------------------------------------

class DistanceDlg: public AutoDialog
{
public:
	// constructors and destructors
	DistanceDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );
	
	// WDR: method declarations for DistanceDlg
	wxChoice* GetUnits1()  { return (wxChoice*) FindWindow( ID_UNITS1 ); }
	wxChoice* GetUnits2()  { return (wxChoice*) FindWindow( ID_UNITS2 ); }
	wxChoice* GetUnits3()  { return (wxChoice*) FindWindow( ID_UNITS3 ); }
	wxTextCtrl* GetMapOffset()  { return (wxTextCtrl*) FindWindow( ID_MAP_OFFSET ); }
	wxTextCtrl* GetMapDist()  { return (wxTextCtrl*) FindWindow( ID_MAP_DIST ); }
	wxTextCtrl* GetGeodDist()  { return (wxTextCtrl*) FindWindow( ID_GEOD_DIST ); }

	void SetProjection(vtProjection *proj);
	void SetPoints(const DPoint2 &p1, const DPoint2 &p2);
	void SetAvailableUnits();
	void ShowValues();

private:
	// WDR: member variable declarations for DistanceDlg
	vtProjection	*m_pProj;
	int		m_iUnits1;
	int		m_iUnits2;
	int		m_iUnits3;
	DPoint2	m_p1, m_p2;

private:
	// WDR: handler declarations for DistanceDlg
	void OnUnits( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
