//
// Name: RoadDlg.h
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef __RoadDlg_H__
#define __RoadDlg_H__

#ifdef __GNUG__
	#pragma interface "RoadDlg.cpp"
#endif

#include "VTBuilder_wdr.h"
#include "AutoDialog.h"

class RoadEdit;
class vtRoadLayer;

// WDR: class declarations

//----------------------------------------------------------------------------
// RoadDlg
//----------------------------------------------------------------------------

class RoadDlg: public AutoDialog
{
public:
	// constructors and destructors
	RoadDlg( wxWindow *parent, wxWindowID id, const wxString &title,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE );

	void SetRoad(RoadEdit *pSingleRoad, vtRoadLayer *pLayer);
	void ClearState();
	void AccumulateState(RoadEdit *pRoad);
	void TransferStateToControls();
	void ApplyState(RoadEdit *pRoad);

	// WDR: method declarations for RoadDlg
	wxListBox* GetSurfType()  { return (wxListBox*) FindWindow( ID_SURFTYPE ); }
	wxChoice* GetMargin()  { return (wxChoice*) FindWindow( ID_MARGIN ); }
	wxChoice* GetParking()  { return (wxChoice*) FindWindow( ID_PARKING ); }
	wxChoice* GetSidewalk()  { return (wxChoice*) FindWindow( ID_SIDEWALK ); }
	wxTextCtrl* GetHwyName()  { return (wxTextCtrl*) FindWindow( ID_HWYNAME ); }
	wxTextCtrl* GetNumLanes()  { return (wxTextCtrl*) FindWindow( ID_NUMLANES ); }
	
private:
	// WDR: member variable declarations for RoadDlg
	RoadEdit *m_pRoad;
	vtRoadLayer *m_pLayer;

	// State
	int m_iLanes;
	int m_iHwy;
	int m_iSidewalk;
	int m_iParking;
	int m_iMargin;
	int m_iSurf;

private:
	// WDR: handler declarations for RoadDlg
	void OnOK( wxCommandEvent &event );
	void OnInitDialog(wxInitDialogEvent& event);

private:
	DECLARE_EVENT_TABLE()
};




#endif
