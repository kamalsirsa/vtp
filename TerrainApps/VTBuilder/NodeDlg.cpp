//
// NodeDlg.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "wx/wxprec.h"
#include "NodeDlg.h"
#include "dialog4.h"
#include "frame.h"
#include "BuilderView.h"

/////////////////////////////////////////////////////////////////////////////
// CNodeDlg dialog

CNodeDlg::CNodeDlg(NodeEdit *node, vtLayer *pLayer, bool bMultiple)
{
	//{{AFX_DATA_INIT(CNodeDlg)
	m_fRoadHeight = 0.0f;
	//}}AFX_DATA_INIT
	m_pNode = node;
	//a dummy node to do the editing on.
	m_pTempNode = new NodeEdit();
	m_pTempNode->Copy(m_pNode);
	m_pLayer = pLayer;
	
	m_bMultiple = bMultiple;
}

CNodeDlg::~CNodeDlg()
{
	delete m_pTempNode;
}

BEGIN_EVENT_TABLE(CNodeDlg, AutoDialog)
EVT_LISTBOX 	(IDC_NODE_INTERSECT, CNodeDlg::OnSelchangeNodeIntersect)
EVT_LISTBOX 	(IDC_NODE_ROADNUM, CNodeDlg::OnSelchangeNodeRoadnum)
EVT_LISTBOX 	(IDC_NODE_ROADBEH, CNodeDlg::OnSelchangeNodeRoadbeh)
EVT_BUTTON		(wxID_OK, CNodeDlg::OnOK)
EVT_PAINT		(CNodeDlg::OnPaint)
END_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
// CNodeDlg message handlers

void CNodeDlg::OnPaint(wxPaintEvent& event)
{
//	wxEvtHandler::OnPaint(event);
    wxPaintDC dc(this);
	OnDraw(dc);
}

void CNodeDlg::OnInitDialog(wxInitDialogEvent& event)
{
	m_pcIntersectType = (wxListBox *) FindWindow(IDC_NODE_INTERSECT);
	m_pcRoadNum = (wxListBox *) FindWindow(IDC_NODE_ROADNUM);
	m_pcRoadBehavior = (wxListBox *) FindWindow(IDC_NODE_ROADBEH);

	m_pcIntersectType->Append("Uncontrolled");
	m_pcIntersectType->Append("All Signal Light(s)");
	m_pcIntersectType->Append("All Stop Sign(s)");
	m_pcIntersectType->Append("Signal Light(s)");
	m_pcIntersectType->Append("Stop Sign(s)");

	m_pcRoadBehavior->Append("Uncontrolled");	// IT_NONE
	m_pcRoadBehavior->Append("Signal Light");	// IT_LIGHT
	m_pcRoadBehavior->Append("Stop Sign");	// IT_STOPSIGN

	// if we are editing multiple nodes at once, disable some of the editing abilities
	if (m_bMultiple)
	{
		//multiple edit
		m_pcRoadNum->Enable(false);
		m_pcRoadBehavior->Enable(false);
		m_pcRoadNum->Append("0"); 

		NodeEdit *curNode = m_pNode;
		bool first = true;
		VisualIntersectionType intersect;
		bool uniformIntersection = true;

		//find out if all the selected intersection are of one type.  if so, select it.
		//does not handle partial stop sign
		while (curNode && uniformIntersection)
		{
			if (curNode->IsSelected())
			{
				if (!first) {
					if (intersect != curNode->GetVisual()) {
						uniformIntersection = false;
					}
				} else {
					intersect = curNode->GetVisual();
					uniformIntersection = true;
					first = false;
				}
			}
			curNode = (NodeEdit*) curNode->m_pNext;
		}
		if (uniformIntersection) {
			m_pcIntersectType->SetSelection(intersect);
		}
	}
	else
	{
		int i;
		//assign the behavior of each road at this node.
		m_pcIntersectType->SetSelection(m_pTempNode->GetVisual());

		wxString string;
		for (i=0; i < m_pTempNode->m_iRoads; i++)
		{
			string = wxString::Format("%i",i);
			m_pcRoadNum->Append(string);
		}
		m_pcRoadNum->SetSelection(0);

		m_pcRoadBehavior->SetSelection(m_pTempNode->GetIntersectType(0));
		TransferDataToWindow();
	}

	wxDialog::OnInitDialog(event);
}


void CNodeDlg::OnOK(wxCommandEvent& event)
{
	bool bMod;	// check for modification

	if (m_bMultiple)
		bMod = OnOKMultiple();
	else
		bMod = OnOKSingle();

	if (bMod)
		m_pLayer->SetModified(true);

	//modify pRoads info with new info	
	wxDialog::OnOK(event);
}

bool CNodeDlg::OnOKMultiple()
{
	bool bMod = false;	// check for modification

	// multiple node edit
	VisualIntersectionType vit = m_pTempNode->GetVisual();

	for (; m_pNode; m_pNode = m_pNode->GetNext())
	{
		if (!m_pNode->IsSelected())
			continue;

		// remember previous state
		NodeEdit n2;
		n2.Copy(m_pNode);

		for (int i = 0; i < m_pNode->m_iRoads; i++)
		{
			if (vit == VIT_ALLLIGHTS)
				m_pNode->SetIntersectType(i, IT_LIGHT);
			else
			if (vit == VIT_ALLSTOPS)
				m_pNode->SetIntersectType(i, IT_STOPSIGN);
			else
			if (vit == VIT_NONE)
				m_pNode->SetIntersectType(i, IT_NONE);
		}
		m_pNode->DetermineVisualFromRoads();
		if (m_pTempNode->HasLights())
		{
			//if we just changed behavior to lights, then we need to 
			//properly assign the light behavior relationships.
			m_pNode->AdjustForLights();
		}

		// if state has changed, layer is modified
		if (bMod == false && !(*m_pNode == n2))
			bMod = true;
	}
	return bMod;
}


bool CNodeDlg::OnOKSingle()
{
	bool bMod = false;	// check for modification

	// remember previous state
	int iVisual = m_pNode->GetVisual();

	// double check logic of values, not necessarily fool proof.
	int numLights = 0;
	int numStopSigns = 0;
	int numNone = 0;
	for (int i = 0; i < m_pTempNode->m_iRoads; i++)
	{
		if (m_pTempNode->GetIntersectType(i) == IT_NONE)
			numNone++;
		else if (m_pTempNode->GetIntersectType(i) == IT_STOPSIGN)
			numStopSigns++;
		else if (m_pTempNode->GetIntersectType(i) == IT_LIGHT)
			numLights++;
	}

	if (numLights > 0)
	{
		if (numLights < m_pTempNode->m_iRoads)
			m_pNode->SetVisual(VIT_LIGHTS);
		else
			m_pNode->SetVisual(VIT_ALLLIGHTS);
	}
	else if (numStopSigns > 0)
	{
		if (numStopSigns < m_pTempNode->m_iRoads)
			m_pNode->SetVisual(VIT_STOPSIGN);
		else
			m_pNode->SetVisual(VIT_ALLSTOPS);
	}
	else
	{
		//uncontrolled
		m_pNode->SetVisual(VIT_NONE);
	}

	//save new values
	m_pNode->AdjustForLights();

	for (i=0; i < m_pNode->m_iRoads; i++)
	{
		IntersectionType t = m_pTempNode->GetIntersectType(i);
		if (t != m_pNode->GetIntersectType(i))
		{
			m_pNode->SetIntersectType(i, t);
			bMod = true;
		}
	}

	// if state has changed, layer is modified
//	if (!(*m_pNode == n2))
	if (iVisual != m_pNode->GetVisual())
		bMod = true;

	return bMod;
}


void CNodeDlg::OnSelchangeNodeRoadnum(wxCommandEvent &event) 
{
	int sel = m_pcRoadNum->GetSelection();
	//update what the behavior shows to match that of the road
	m_pcRoadBehavior->SetSelection(m_pTempNode->GetIntersectType(sel));
	TransferDataToWindow();
}

void CNodeDlg::OnSelchangeNodeRoadbeh(wxCommandEvent &event) 
{
	int sel = m_pcRoadNum->GetSelection();
	//select new behavior and redraw it on the dialog.
	m_pTempNode->SetIntersectType(sel, (IntersectionType) m_pcRoadBehavior->GetSelection());
	Refresh();
}

void CNodeDlg::OnSelchangeNodeIntersect(wxCommandEvent &event) 
{
	int i;

	//new node behavior
	VisualIntersectionType itype = (VisualIntersectionType) m_pcIntersectType->GetSelection();
	m_pTempNode->SetVisual(itype);

	//overwrite all behaviors at the roads to match new assigned node behavior.
	switch (itype)
	{
	case IT_NONE:
		//make all intersections uncontrolled
		for (i = 0; i < m_pTempNode->m_iRoads; i++) {
			m_pTempNode->SetIntersectType(i, IT_NONE);
		}
		m_pcRoadBehavior->SetSelection(IT_NONE);
		break;
	case IT_STOPSIGN:
		//make all intersections stop signs
		for (i = 0; i < m_pTempNode->m_iRoads; i++) {
			m_pTempNode->SetIntersectType(i, IT_STOPSIGN);
		}
		m_pcRoadBehavior->SetSelection(IT_STOPSIGN);
		break;
	case IT_LIGHT:
		//make all intersections lights
		for (i = 0; i < m_pTempNode->m_iRoads; i++) {
			m_pTempNode->SetIntersectType(i, IT_LIGHT);
		}
		m_pcRoadBehavior->SetSelection(IT_LIGHT);
		m_pTempNode->AdjustForLights();
		break;
	}
	Refresh();
}

static wxPoint buf[10];

//draw the node structure on the dialog box.
void CNodeDlg::OnDraw(wxDC &dc) 
{
	if (m_bMultiple)
		return;

	vtScaledView *pView = GetMainFrame()->GetView();
	float fSaveScale = pView->GetScale();
	pView->SetScale(1.0f);

	wxPoint off;
	off.x = pView->sx(m_pTempNode->m_p.x);
	off.y = pView->sy(m_pTempNode->m_p.y);
	dc.SetDeviceOrigin(-off.x + 315, -off.y +125);
	dc.DrawLine(0, 0, 400, 400);
	m_pTempNode->Draw(&dc, pView);

	wxString string;
	for (int i = 0; i < m_pTempNode->m_iRoads; i++)
	{
		RoadEdit *pR = m_pTempNode->GetRoad(i);
		pR->Draw(&dc, pView);

		//we need to use the original node here because the roads point to it.
		DPoint2 close = m_pNode->find_adjacent_roadpoint2d(pR);
		DPoint2 vec = close - m_pTempNode->m_p;
		DPoint2 vector;
		vector.x = vec.x;
		vector.y = vec.y;
		float length = vec.Length();
		vector.x = vector.x/length;
		vector.y = vector.y/length;

		vec.x = (float)(off.x + vector.x*20);
		vec.y = (float)(off.y - vector.y*20);

		//draw signal lights or stop signs as necessary.
		dc.SetLogicalFunction(wxCOPY);
		wxPen pen;

		switch (m_pTempNode->GetIntersectType(i))
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
			wxBrush brush;
			switch (m_pTempNode->GetLightStatus(i)) {
			case LT_INVALID:
				pen.SetColour(0,0,0);
				brush.SetColour(0,0,0);
				break;
			case LT_RED:
				pen.SetColour(128,0,0);
				brush.SetColour(128,0,0);
				break;
			case LT_YELLOW:
				pen.SetColour(0,128,128);
				brush.SetColour(0,128,128);
				break;
			case LT_GREEN:
				pen.SetColour(0,128,0);
				brush.SetColour(0,128,0);
				break;
			default:
				//unrecognized
				pen.SetColour(0,0,255);
				brush.SetColour(0,0,255);
			}
		
			dc.SetPen(pen);
			dc.SetBrush(brush);
			wxRect box;
			int radius = 4;
			box.y = vec.y - radius;
			box.height = (radius << 1);
			box.x = vec.x - radius;
			box.width = (radius << 1);
			dc.DrawEllipse(box.x, box.y, box.width, box.height);
			break;
		} 

		
		vec.x = (float)(off.x + vector.x*40);
		vec.y = (float)(off.y - vector.y*40);
		string = wxString::Format("%i", i);
		//draw text labels
		dc.DrawText(string, vec.x-10, vec.y-10);
	}
	pView->SetScale(fSaveScale);
}

