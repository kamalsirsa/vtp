//
// RoadDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "RoadDlg.h"
#include "dialog5.h"
#include "Frame.h"

/////////////////////////////////////////////////////////////////////////////
// CRoadDlg dialog

CRoadDlg::CRoadDlg(class RoadEdit *road, vtLayer *pLayer, bool bMultiple)
{
	assert(pLayer != NULL);

	m_iLanes = 1;
	m_iHwyNum = 0;

	m_pRoad = road;
	m_bMultiple = bMultiple;
	m_bSideWalkClicked = false;
	m_bParkingClicked = false;
	m_bMarginClicked = false;
	m_pLayer = pLayer;
}

BEGIN_EVENT_TABLE(CRoadDlg, AutoDialog)
EVT_CHECKBOX(IDC_ROAD_ENABLE_LANES, CRoadDlg::OnRoadEnableLanes)
EVT_CHECKBOX(IDC_ROAD_ENABLE_HWYNUM, CRoadDlg::OnRoadEnableHwynum)
EVT_CHECKBOX(IDC_ROAD_SIDEWALK, CRoadDlg::OnRoadSideWalk)
EVT_CHECKBOX(IDC_ROAD_PARKING, CRoadDlg::OnRoadParking)
EVT_CHECKBOX(IDC_ROAD_MARGIN, CRoadDlg::OnRoadMargin)
EVT_BUTTON(wxID_OK, CRoadDlg::OnOK)
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
// CRoadDlg message handlers

void CRoadDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_pcEnableLabel = (wxStaticText *)FindWindow(IDC_MODIFY_TEXT);
	m_pcEnableHwyNum = (wxCheckBox *)FindWindow(IDC_ROAD_ENABLE_HWYNUM);
	m_pcEnableLanes = (wxCheckBox	*)FindWindow(IDC_ROAD_ENABLE_LANES);
	m_pcHwyNum = (wxTextCtrl *)FindWindow(IDC_ROAD_NUMBER);
	m_pcLanes = (wxTextCtrl *)FindWindow(IDC_ROAD_LANES);
	m_pcMargin = (wxCheckBox	*)FindWindow(IDC_ROAD_MARGIN);
	m_pcParking = (wxCheckBox	*)FindWindow(IDC_ROAD_PARKING);
	m_pcSideWalk = (wxCheckBox *)FindWindow(IDC_ROAD_SIDEWALK);
	m_pcSurfaceChoice = (wxListBox *)FindWindow(IDC_ROAD_SURFACE);

	AddNumValidator(IDC_ROAD_LANES, &m_iLanes);
	AddNumValidator(IDC_ROAD_NUMBER, &m_iHwyNum);

	if (m_bMultiple)
	{
		//road is first road in list for road network.
		RoadEdit *curRoad = m_pRoad;
		bool first = true;
		SurfaceType s;
		bool uniformSurface = true;

		while (curRoad && uniformSurface)
		{
			if (curRoad->IsSelected())
			{
				if (!first)
				{
					if (s != curRoad->m_Surface)
						uniformSurface = false;
				}
				else
				{
					s = curRoad->m_Surface;
					uniformSurface = true;
					first = false;
				}
			}
			curRoad = (RoadEdit*) curRoad->m_pNext;
		}
		if (uniformSurface)
			m_iSurfaceType = s;
		else
			m_iSurfaceType = (SurfaceType)7;
	}
	else
	{
		m_iHwyNum = m_pRoad->m_iHwy;
		m_iLanes = m_pRoad->m_iLanes;
		m_iSurfaceType = m_pRoad->m_Surface;
	}

	m_pcSurfaceChoice->Append("None");
	m_pcSurfaceChoice->Append("Gravel");
	m_pcSurfaceChoice->Append("Trail");
	m_pcSurfaceChoice->Append("2 Track");
	m_pcSurfaceChoice->Append("Dirt");
	m_pcSurfaceChoice->Append("Paved");
	m_pcSurfaceChoice->Append("Railroad");
	m_pcSurfaceChoice->Append("(multiple types)");
	m_pcSurfaceChoice->SetSelection(m_iSurfaceType);
	
	if (m_bMultiple)
	{
		//if we are editing multiple roads, we need to enable some windows.
		//m_pcSurfaceChoice->SetCurSel(-1);

		m_pcEnableLanes->Enable(true);
		m_pcEnableLanes->SetValue(0);
		m_pcEnableHwyNum->Enable(true);
		m_pcEnableHwyNum->SetValue(0);

		m_pcLanes->Enable(false);
		m_pcHwyNum->Enable(false);

		//see if all roads have sidewalks and other properties or not
		//road is first road in list for road network.
		RoadEdit *curRoad = m_pRoad;
		bool bYesSideWalk = false, bNoSideWalk = false;
		bool bYesParking = false, bNoParking = false;
		bool bYesMargin = false, bNoMargin = false;
		while (curRoad) {
			if (curRoad->IsSelected()) {
				if (curRoad->m_iFlags & RF_SIDEWALK) {
					bYesSideWalk = true;
				} else {
					bNoSideWalk = true;
				}
				if (curRoad->m_iFlags & RF_PARKING) {
					bYesParking = true;
				} else {
					bNoParking = true;
				}
				if (curRoad->m_iFlags & RF_MARGIN) {
					bYesMargin = true;
				} else {
					bNoMargin = true;
				}
			}
			curRoad = (RoadEdit*) curRoad->m_pNext;
		}
		//set the appropriate check marks for the properties.
		if (bYesSideWalk && bNoSideWalk) {
//			m_pcSideWalk->SetValue(2);	//there is a mix of roads with/without sidewalks
		} else if (bYesSideWalk) {
			m_pcSideWalk->SetValue(true);
		} else {
			m_pcSideWalk->SetValue(false);
		}

		if (bYesParking && bNoParking) {
//			m_pcParking->SetValue(2);	//there is a mix of roads with/without parking
		} else if (bYesParking) {
			m_pcParking->SetValue(true);
		} else {
			m_pcParking->SetValue(false);
		}

		if (bYesMargin && bNoMargin) {
//			m_pcMargin->SetValue(2);	//there is a mix of roads with/without margins
		} else if (bYesMargin) {
			m_pcMargin->SetValue(true);
		} else {
			m_pcMargin->SetValue(false);
		}
	
	} else {
		//hide features for editing a single road
		m_pcEnableLanes->SetValue(true);
		m_pcEnableHwyNum->SetValue(true);
		m_pcEnableLabel->Show(true);

		if (m_pRoad->m_iFlags & RF_SIDEWALK) {
			m_pcSideWalk->SetValue(1);
		} else {
			m_pcSideWalk->SetValue(0);
		}
		if (m_pRoad->m_iFlags & RF_PARKING) {
			m_pcParking->SetValue(1);
		} else {
			m_pcParking->SetValue(0);
		}
		if (m_pRoad->m_iFlags & RF_MARGIN) {
			m_pcMargin->SetValue(1);
		} else {
			m_pcMargin->SetValue(0);
		}

	}
	TransferDataToWindow();

	wxDialog::OnInitDialog(event);
}

void CRoadDlg::OnOK(wxCommandEvent& event)
{
	TransferDataFromWindow();

	//modify pRoads info with new info	
	m_iSurfaceType = (SurfaceType) m_pcSurfaceChoice->GetSelection();

	int flag = 0;
	int filter =0xffffffff;
	
	if (m_bSideWalkClicked) {
		filter &= ~RF_SIDEWALK; 
		if (m_pcSideWalk->GetValue())
			flag |= RF_SIDEWALK;
		else
			flag &= ~RF_SIDEWALK;
	}
	if (m_bParkingClicked) {
		filter &= ~RF_PARKING;
		if (m_pcParking->GetValue())
			flag |= RF_PARKING;
		else
			flag &= ~RF_PARKING;
	}
	if (m_bMarginClicked) {
		filter &= ~RF_MARGIN;
		if (m_pcMargin->GetValue())
			flag |= RF_MARGIN;
		else
			flag &= ~RF_MARGIN;
	}

	bool bMod = false;
	if (m_bMultiple)
	{
		for (RoadEdit *r = m_pRoad; r; r = r->GetNext())
		{
			if (!r->IsSelected())
				continue;

			RoadEdit r2 = *r;

			if (m_iSurfaceType != 7)
				r->m_Surface = m_iSurfaceType;

			if (m_pcEnableHwyNum->GetValue())
				r->m_iHwy = m_iHwyNum;

			if (m_pcEnableLanes->GetValue())
				r->m_iLanes = m_iLanes;

			r->m_iFlags &= filter;
			r->m_iFlags |= flag;

			if (!(*r == r2))
			{
				bMod = true;
				r->ComputeExtent();
			}
		}
	}
	else
	{
		RoadEdit r2 = *m_pRoad;

		// single road, assign new values
		m_pRoad->m_iHwy = m_iHwyNum;
		m_pRoad->m_iLanes = m_iLanes;
		m_pRoad->m_Surface = m_iSurfaceType;

		m_pRoad->m_iFlags &= filter;
		m_pRoad->m_iFlags |= flag;

		if (!(*m_pRoad == r2))
		{
			bMod = true;
			m_pRoad->ComputeExtent();
		}
	}
	if (bMod)
		m_pLayer->SetModified(true);

	wxDialog::OnOK(event);
}

void CRoadDlg::OnRoadEnableLanes(wxCommandEvent& event) 
{
	m_pcLanes->Enable(m_pcEnableLanes->GetValue());
}

void CRoadDlg::OnRoadEnableHwynum(wxCommandEvent& event) 
{
	m_pcHwyNum->Enable(m_pcEnableHwyNum->GetValue());
}

void CRoadDlg::OnRoadSideWalk(wxCommandEvent& event) 
{
	m_bSideWalkClicked = true;
}

void CRoadDlg::OnRoadParking(wxCommandEvent& event) 
{
	m_bParkingClicked = true;	
}

void CRoadDlg::OnRoadMargin(wxCommandEvent& event) 
{
	m_bMarginClicked = true;
}


