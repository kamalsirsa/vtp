//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ROADDLGH
#define ROADDLGH

#include "Layer.h"
#include "RoadMapEdit.h"
#include "AutoDialog.h"

class CRoadDlg : public AutoDialog
{
public:
	CRoadDlg(class RoadEdit *road, vtLayer *pLayer, bool bMultiple=false);

	wxStaticText	*m_pcEnableLabel;
	wxCheckBox	*m_pcEnableHwyNum;
	wxCheckBox	*m_pcEnableLanes;
	wxTextCtrl 	*m_pcHwyNum;
	wxTextCtrl 	*m_pcLanes;
	wxCheckBox	*m_pcMargin;
	wxCheckBox	*m_pcParking;
	wxCheckBox	*m_pcSideWalk;
	wxListBox	*m_pcSurfaceChoice;
	int			m_iLanes;
	int		m_iHwyNum;

	void OnInitDialog(wxInitDialogEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnRoadCheckLanes(wxCommandEvent& event);
	void OnRoadEnableLanes(wxCommandEvent& event);
	void OnRoadEnableHwynum(wxCommandEvent& event);
	void OnUpdateRoadLanes(wxCommandEvent& event);
	void OnRoadSideWalk(wxCommandEvent& event);
	void OnRoadParking(wxCommandEvent& event);
	void OnRoadMargin(wxCommandEvent& event);

protected:
	SurfaceType m_iSurfaceType;
	bool m_bSideWalkClicked;
	bool m_bParkingClicked;
	bool m_bMarginClicked;

	RoadEdit	*m_pRoad;
	bool		m_bMultiple;  //true if we are editing multiple roads at once
	vtLayer		*m_pLayer;

    DECLARE_EVENT_TABLE()
};

#endif