//
// RoadLayer.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ROADLAYER_H
#define ROADLAYER_H

#include "Layer.h"
#include "RoadMapEdit.h"

//////////////////////////////////////////////////////////

class vtRoadLayer : public vtLayer, public RoadMapEdit
{
public:
	vtRoadLayer();
	~vtRoadLayer();

	// implementation of vtLayer methods
	bool GetExtent(DRECT &rect);
	void DrawLayer(wxDC* pDC, vtScaledView *pView);
	bool ConvertProjection(vtProjection &proj);
	bool OnSave();
	bool OnLoad();
	bool AppendDataFrom(vtLayer *pL);
	void GetProjection(vtProjection &proj);
	void SetProjection(const vtProjection &proj);
	void Offset(const DPoint2 &p);
	void GetPropertyText(wxString &strIn);
	void OnLeftDown(BuilderView *pView, UIContext &ui);
	void OnRightUp(BuilderView *pView, UIContext &ui);
	void OnLeftDoubleClick(BuilderView *pView, UIContext &ui);

	static bool GetDrawNodes() { return m_bDrawNodes; }
	static void SetDrawNodes(bool d) { m_bDrawNodes = d; }
	static bool GetShowDirection() { return m_bShowDirection; }
	static void SetShowDirection(bool d) { m_bShowDirection = d; }
	static bool GetDrawWidth() { return m_bDrawWidth; }
	static void SetDrawWidth(bool d) { m_bDrawWidth = d; }

	int GetSelectedNodes();
	int GetSelectedLinks();
	void ToggleLinkDirection(LinkEdit *pLink);
	bool SelectArea(const DRECT &box, bool nodemode, bool crossSelect);
	void MoveSelectedNodes(const DPoint2 &offset);
	void CarveRoadway(class vtElevLayer *pElev);

	//edit a single node
	bool EditNodeProperties(const DPoint2 &point, float error, DRECT &bound);
	//edit a single road
	bool EditLinkProperties(const DPoint2 &point, float error, DRECT &bound);
	//edit all selected nodes
	bool EditNodesProperties();
	//edit all selected roads
	bool EditLinksProperties();

protected:
	static bool	m_bDrawNodes;
	static bool	m_bShowDirection;
	static bool	m_bDrawWidth;
};

#endif
