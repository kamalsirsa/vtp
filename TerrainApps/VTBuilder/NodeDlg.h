//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef NODEDLGH
#define NODEDLGH

#include "Layer.h"
#include "RoadMapEdit.h"
#include "AutoDialog.h"

class CNodeDlg : public AutoDialog
{
// Construction
public:
	CNodeDlg(class NodeEdit *node, vtLayer *pLayer, bool bMultiple=false);
	~CNodeDlg();

	void OnInitDialog(wxInitDialogEvent& event);
	void OnSelchangeNodeRoadnum(wxCommandEvent&);
	void OnSelchangeNodeRoadbeh(wxCommandEvent&);
	void OnSelchangeNodeIntersect(wxCommandEvent&);

	wxListBox	*m_pcRoadBehavior;
	wxListBox	*m_pcRoadNum;
	wxListBox	*m_pcIntersectType;
	float	m_fRoadHeight;

	void OnOK(wxCommandEvent& event);
	bool OnOKMultiple();
	bool OnOKSingle();
	void OnDraw(wxDC &dc);
	void OnPaint(wxPaintEvent& event);

// Implementation
protected:
	//the node to edit.
	NodeEdit *m_pNode;
	//a dummy node to do the editting on
	NodeEdit *m_pTempNode;
	vtLayer *m_pLayer;

	bool m_bMultiple;  //true if we are editing multiple nodes

    DECLARE_EVENT_TABLE()
};

#endif
