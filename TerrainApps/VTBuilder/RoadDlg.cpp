//
// Name:		RoadDlg.cpp
//
// Copyright (c) 2002-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifdef __GNUG__
	#pragma implementation "RoadDlg.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#include "RoadDlg.h"
#include "RoadLayer.h"

#define MULTIPLE	5000

// WDR: class implementations

//----------------------------------------------------------------------------
// RoadDlg
//----------------------------------------------------------------------------

// WDR: event table for RoadDlg

BEGIN_EVENT_TABLE(RoadDlg,AutoDialog)
	EVT_BUTTON( wxID_OK, RoadDlg::OnOK )
END_EVENT_TABLE()

RoadDlg::RoadDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	AutoDialog( parent, id, title, position, size, style )
{
	RoadPropDialogFunc( this, TRUE ); 
}

void RoadDlg::SetRoad(LinkEdit *pSingleRoad, vtRoadLayer *pLayer)
{
	m_pRoad = pSingleRoad;
	m_pLayer = pLayer;
}

void RoadDlg::ClearState()
{
	m_iLanes = -1;
	m_iHwy = -1;
	m_iSidewalk = -1;
	m_iParking = -1;
	m_iMargin = -1;
	m_iSurf = -1;
}

void RoadDlg::AccumulateState(LinkEdit *pRoad)
{
	if (m_iLanes == -1)
		m_iLanes = pRoad->m_iLanes;
	if (pRoad->m_iLanes != m_iLanes)
		m_iLanes = MULTIPLE;

	if (m_iHwy == -1)
		m_iHwy = pRoad->m_iHwy;
	if (pRoad->m_iHwy != m_iHwy)
		m_iHwy = MULTIPLE;

	if (m_iSidewalk == -1)
		m_iSidewalk = pRoad->GetFlag(RF_SIDEWALK);
	if (pRoad->GetFlag(RF_SIDEWALK) != m_iSidewalk)
		m_iSidewalk = MULTIPLE;

	if (m_iParking == -1)
		m_iParking = pRoad->GetFlag(RF_PARKING);
	if (pRoad->GetFlag(RF_PARKING) != m_iParking)
		m_iParking = MULTIPLE;

	if (m_iMargin == -1)
		m_iMargin = pRoad->GetFlag(RF_MARGIN);
	if (pRoad->GetFlag(RF_MARGIN) != m_iMargin)
		m_iMargin = MULTIPLE;

	if (m_iSurf == -1)
		m_iSurf = pRoad->m_Surface;
	if (pRoad->m_Surface != m_iSurf)
		m_iSurf = MULTIPLE;
}

void RoadDlg::TransferStateToControls()
{
	wxString str;

	if (m_iLanes == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%d"), m_iLanes);
	GetNumLanes()->SetValue(str);

	if (m_iHwy == MULTIPLE)
		str = _("(multiple)");
	else
		str.Printf(_T("%d"), m_iHwy);
	GetHwyName()->SetValue(str);

	if (m_iSidewalk == MULTIPLE)
		GetSidewalk()->SetSelection(2);
	else
		GetSidewalk()->SetSelection(m_iSidewalk);

	if (m_iParking == MULTIPLE)
		GetParking()->SetSelection(2);
	else
		GetParking()->SetSelection(m_iParking);

	if (m_iMargin == MULTIPLE)
		GetMargin()->SetSelection(2);
	else
		GetMargin()->SetSelection(m_iMargin);

	if (m_iSurf == MULTIPLE)
		GetSurfType()->SetSelection(7);
	else
		GetSurfType()->SetSelection(m_iSurf);
}

void RoadDlg::OnInitDialog(wxInitDialogEvent& event)
{
	GetSurfType()->Append(_("None"));
	GetSurfType()->Append(_("Gravel"));
	GetSurfType()->Append(_("Trail"));
	GetSurfType()->Append(_("2 Track"));
	GetSurfType()->Append(_("Dirt"));
	GetSurfType()->Append(_("Paved"));
	GetSurfType()->Append(_("Railroad"));
	GetSurfType()->Append(_("(multiple types)"));
	GetSurfType()->SetSelection(0);

	GetSidewalk()->Append(_("No"));
	GetSidewalk()->Append(_("Yes"));
	GetSidewalk()->Append(_("(multiple)"));
	GetSidewalk()->SetSelection(0);

	GetParking()->Append(_("No"));
	GetParking()->Append(_("Yes"));
	GetParking()->Append(_("(multiple)"));
	GetParking()->SetSelection(0);

	GetMargin()->Append(_("No"));
	GetMargin()->Append(_("Yes"));
	GetMargin()->Append(_("(multiple)"));
	GetMargin()->SetSelection(0);

	ClearState();
	if (m_pRoad)
		AccumulateState(m_pRoad);
	else
	{
		LinkEdit *pRoad;
		for (pRoad = m_pLayer->GetFirstLink(); pRoad; pRoad=pRoad->GetNext())
		{
			if (pRoad->IsSelected())
				AccumulateState(pRoad);
		}
	}
	TransferStateToControls();
}

// WDR: handler implementations for RoadDlg

void RoadDlg::OnOK( wxCommandEvent &event )
{
	if (m_pRoad)
		ApplyState(m_pRoad);
	else
	{
		LinkEdit *pRoad;
		for (pRoad = m_pLayer->GetFirstLink(); pRoad; pRoad=pRoad->GetNext())
		{
			if (pRoad->IsSelected())
				ApplyState(pRoad);
		}
	}
	wxDialog::OnOK(event);
}

//
// Apply the state directly from the controls to a given road.
//
void RoadDlg::ApplyState(LinkEdit *pRoad)
{
	wxString2 str;
	int val;

	str = GetNumLanes()->GetValue();
	val = atoi(str.mb_str());
	if (val != 0)
		pRoad->m_iLanes = val;

	str = GetHwyName()->GetValue();
	val = atoi(str.mb_str());
		pRoad->m_iHwy = val;

	val = GetSidewalk()->GetSelection();
	if (val != 2)
		pRoad->SetFlag(RF_SIDEWALK, (val != 0));

	val = GetParking()->GetSelection();
	if (val != 2)
		pRoad->SetFlag(RF_PARKING, (val != 0));

	val = GetMargin()->GetSelection();
	if (val != 2)
		pRoad->SetFlag(RF_MARGIN, (val != 0));

	val = GetSurfType()->GetSelection();
	if (val != 7)
		pRoad->m_Surface = (SurfaceType) val;
}



