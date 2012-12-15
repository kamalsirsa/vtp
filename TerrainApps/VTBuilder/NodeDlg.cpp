//
// Name: NodeDlg.cpp
//
// Copyright (c) 2002-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "vtui/AutoDialog.h"

#include "NodeDlg.h"
#include "RoadLayer.h"
#include "Builder.h"
#include "BuilderView.h"

#define MULTIPLE	5000

static wxPoint buf[10];

void NodeDlgView::OnDraw(wxDC &dc)
{
	if (!m_pNode)
		return;

	wxPoint center;
	screen(m_pNode->Pos(), center);
	m_pNode->Draw(&dc, this);

	wxString string;
	for (int i = 0; i < m_pNode->NumLinks(); i++)
	{
		LinkEdit *pR = m_pNode->GetLink(i);
		pR->Draw(&dc, this);

		//we need to use the original node here because the roads point to it.
		const DPoint2 close = m_pNode->GetAdjacentLinkPoint2d(i);
		DPoint2 vector = close - m_pNode->Pos();
		vector.Normalize();
		IPoint2 vec;

		vec.x = (int)(center.x + vector.x*20);
		vec.y = (int)(center.y - vector.y*20);

		//draw signal lights or stop signs as necessary.
		dc.SetLogicalFunction(wxCOPY);
		wxPen pen;
		wxBrush brush;

		switch (m_pNode->GetIntersectType(i))
		{
		case IT_STOPSIGN:
			pen.SetColour(128,0,0);
			dc.SetPen(pen);
			vec.x += 2;
			vec.y += 6;
			buf[0].x = vec.x; buf[0].y = vec.y;
			vec.x -= 4;
			buf[1].x = vec.x; buf[1].y = vec.y;
			vec.x -= 3;
			vec.y -= 3;
			buf[2].x = vec.x; buf[2].y = vec.y;
			vec.y -= 4;
			buf[3].x = vec.x; buf[3].y = vec.y;
			vec.x += 3;
			vec.y -= 3;
			buf[4].x = vec.x; buf[4].y = vec.y;
			vec.x += 4;
			buf[5].x = vec.x; buf[5].y = vec.y;
			vec.x += 3;
			vec.y += 3;
			buf[6].x = vec.x; buf[6].y = vec.y;
			vec.y += 4;
			buf[7].x = vec.x; buf[7].y = vec.y;
			vec.x -= 3;
			vec.y += 3;
			buf[8].x = vec.x; buf[8].y = vec.y;
			dc.DrawLines(9, buf);
			break;
		case IT_LIGHT:
			pen.SetColour(128,0,0);
			brush.SetColour(128,0,0);
			dc.SetPen(pen);
			dc.SetBrush(brush);
			{
				wxRect box;
				int radius = 4;
				box.y = vec.y - radius;
				box.height = (radius << 1);
				box.x = vec.x - radius;
				box.width = (radius << 1);
				dc.DrawEllipse(box.x, box.y, box.width, box.height);
			}
			break;
		}

		//draw text labels
		vec.x = (int)(center.x + vector.x*40);
		vec.y = (int)(center.y - vector.y*40);
		string.Printf(_T("%i"), i);
		dc.DrawText(string, vec.x-10, vec.y-10);
	}
}

// WDR: class implementations

//----------------------------------------------------------------------------
// NodeDlg
//----------------------------------------------------------------------------

// WDR: event table for NodeDlg

BEGIN_EVENT_TABLE(NodeDlg, NodeDlgBase)
	EVT_INIT_DIALOG (NodeDlg::OnInitDialog)
	EVT_LISTBOX( ID_INTTYPE, NodeDlg::OnIntType )
	EVT_LISTBOX( ID_ROADNUM, NodeDlg::OnLinkNum )
	EVT_LISTBOX( ID_BEHAVIOR, NodeDlg::OnBehavior )
END_EVENT_TABLE()

NodeDlg::NodeDlg( wxWindow *parent, wxWindowID id, const wxString &title,
	const wxPoint &position, const wxSize& size, long style ) :
	NodeDlgBase( parent, id, title, position, size, style )
{
	GetIntType()->Append(_("Unknown"));
	GetIntType()->Append(_("Uncontrolled"));
	GetIntType()->Append(_("All Traffic Light(s)"));
	GetIntType()->Append(_("All Stop Sign(s)"));
	GetIntType()->Append(_("Traffic Light(s)"));
	GetIntType()->Append(_("Stop Sign(s)"));
	GetIntType()->Append(_("(multiple)"));

	GetBehavior()->Append(_("Uncontrolled"));	// IT_NONE
	GetBehavior()->Append(_("Traffic Light"));	// IT_LIGHT
	GetBehavior()->Append(_("Stop Sign"));		// IT_STOPSIGN

	m_pView = (NodeDlgView *) FindWindow( ID_SCROLLED );

	GetSizer()->SetSizeHints(this);
}

void NodeDlg::SetScale(float fScale)
{
	m_pView->SetScale(fScale);
}

void NodeDlg::SetNode(NodeEdit *pSingleNode, vtRoadLayer *pLayer)
{
	m_pNode = pSingleNode;
	m_pLayer = pLayer;
	m_pView->m_pNode = m_pNode;
	if (NULL != m_pNode)
		m_pView->ZoomToPoint(m_pNode->Pos());
}

// WDR: handler implementations for NodeDlg

void NodeDlg::OnBehavior( wxCommandEvent &event )
{
	int sel = GetLinkNum()->GetSelection();
	//select new behavior and redraw it on the dialog.
	int itype = GetBehavior()->GetSelection();
	m_pNode->SetIntersectType(sel, (IntersectionType)itype );

	// this may have changed the VIT, so check
	m_pNode->DetermineVisualFromLinks();
	GetIntType()->SetSelection(m_pNode->GetVisual());

	Refresh();
}

void NodeDlg::OnLinkNum( wxCommandEvent &event )
{
	int sel = GetLinkNum()->GetSelection();
	//update what the behavior shows to match that of the road
	GetBehavior()->SetSelection(m_pNode->GetIntersectType(sel));
}

void NodeDlg::OnIntType( wxCommandEvent &event )
{
	//new node behavior
	VisualIntersectionType vitype;
	int sel = GetIntType()->GetSelection();
	if (sel == 0 || sel == 6)	// unknown or multiple not allowed
		return;

	vitype = (VisualIntersectionType) sel;

	if (m_pNode)
		ApplyVisualToNode(m_pNode, vitype);
	else
	{
		for (NodeEdit *n = m_pLayer->GetFirstNode(); n; n=n->GetNext())
		{
			if (!n->IsSelected())
				continue;
			ApplyVisualToNode(n, vitype);
		}
	}
	Refresh();
}

void NodeDlg::ApplyVisualToNode(NodeEdit *pNode, VisualIntersectionType vitype)
{
	int i;

	pNode->SetVisual(vitype);

	//overwrite all behaviors at the roads to match new assigned node behavior.
	switch (vitype)
	{
	case VIT_NONE:
		//make all intersections uncontrolled
		for (i = 0; i < pNode->NumLinks(); i++) {
			pNode->SetIntersectType(i, IT_NONE);
		}
		GetBehavior()->SetSelection(IT_NONE);
		break;
	case VIT_ALLSTOPS:
		//make all intersections stop signs
		for (i = 0; i < pNode->NumLinks(); i++) {
			pNode->SetIntersectType(i, IT_STOPSIGN);
		}
		GetBehavior()->SetSelection(IT_STOPSIGN);
		break;
	case VIT_ALLLIGHTS:
		//make all intersections lights
		for (i = 0; i < pNode->NumLinks(); i++) {
			pNode->SetIntersectType(i, IT_LIGHT);
		}
		GetBehavior()->SetSelection(IT_LIGHT);
		pNode->AdjustForLights();
		break;
	}
}

void NodeDlg::OnInitDialog(wxInitDialogEvent& event)
{
	// if we are editing multiple nodes at once, disable some of the
	// editing abilities
	if (!m_pNode)
	{
		GetLinkNum()->Enable(false);
		GetBehavior()->Enable(false);

		// Accumulate state
		int viz = -1;
		for (NodeEdit *n = m_pLayer->GetFirstNode(); n; n=n->GetNext())
		{
			if (!n->IsSelected())
				continue;
			if (viz == -1)
				viz = n->GetVisual();
			if (n->GetVisual() != viz)
				viz = MULTIPLE;
		}

		// Transfer state to control
		if (viz == MULTIPLE)
			GetIntType()->SetSelection(6);
		else
			GetIntType()->SetSelection(viz);
	}
	else
	{
		// single road
		wxString string;
		for (int i = 0; i < m_pNode->NumLinks(); i++)
		{
			string.Printf(_T("%i"), i);
			GetLinkNum()->Append(string);
		}
		if (m_pNode->NumLinks() > 0)
			GetLinkNum()->SetSelection(0);
		GetIntType()->SetSelection(m_pNode->GetVisual());
		int itype = m_pNode->GetIntersectType(0);
		GetBehavior()->SetSelection(itype);
	}
}

