//
// BuilderView.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef BUILDERVIEWH
#define BUILDERVIEWH

#include "ScaledView.h"
#include "vtdata/Projections.h"
#include "vtdata/Building.h"
#include "vtdata/Fence.h"
#include "vtdata/Icosa.h"

class vtLayer;
class vtRoadLayer;
class vtElevLayer;
class vtImageLayer;
class vtStructureLayer;
class vtUtilityLayer;
class LinkEdit;

//
// Left-button modes
//
enum LBMode {
	LB_None,	// none
	LB_Link,	// select/edit links
	LB_Node,	// select/edit nodes
	LB_Move,	// move selected nodes
	LB_Pan,		// pan the view
	LB_Dist,	// measure distance
	LB_Mag,		// zoom into rectangle
	LB_Dir,		// show/change road direction
	LB_LinkEdit,	// edit the points of a road centerline
	LB_LinkExtend,  //extend a link selection,
	LB_TSelect,		// select elevation layer
	LB_Box,			// set area box
	LB_FSelect,		// select feature (raw or structure)
	LB_BldEdit,		// edit built structures
	LB_BldAddPoints,		// add footprint points
	LB_BldDeletePoints,		// delete footprint points
	LB_AddLinear,	// structures: add linear features
	LB_EditLinear,	// structures: add linear features
	LB_AddInstance,	// structures: add structure instances
	LB_AddPoints,	// add raw points
	LB_TowerSelect,	// selectTowers
	LB_TowerAdd,	// Add Tower to layer
	LB_FeatInfo,	// Get info about features by picking them
	LB_TrimTIN		// Trim TIN triangles by indicating them
};

// A useful class to contain an array of bools
typedef vtArray<bool> BoolArray;

struct UIContext
{
	bool	m_bLMouseButton;
	bool	m_bMMouseButton;
	bool	m_bRMouseButton;

	bool	m_bRubber;

	// Used while editing buildings
	vtBuilding	*m_pCurBuilding, m_EditBuilding;
	int		m_iCurCorner;
	bool	m_bDragCenter;
	bool	m_bRotate;
	bool	m_bControl;
	bool	m_bShift;
	bool	m_bAlt;

	// Used for linear structures
	vtFence *m_pCurLinear, m_EditLinear;

	// Used while editing roads
	LinkEdit *m_pEditingRoad;
	int		m_iEditingPoint;

	// Left Mouse Button Mode
	LBMode	mode;

	// Mouse in world coordinates
	DPoint2 m_DownLocation;
	DPoint2 m_CurLocation;
	DPoint2 m_PrevLocation;

	// Mouse in canvas coords
	wxPoint	m_DownPoint;
	wxPoint m_CurPoint;		// current position of mouse
	wxPoint m_LastPoint;	// last position of mouse

	bool m_bDistanceToolMode;
};

class BuilderView : public vtScaledView
{
	friend class vtRoadLayer;

public:
	BuilderView(wxWindow *parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, const wxString& name = _T(""));
	virtual ~BuilderView();

	virtual void OnDraw(wxDC& dc); // overridden to draw this view

	// Cursor
	void SetCorrectCursor();

	// Mouse
	void SetMode(LBMode m);
	LBMode GetMode() { return m_ui.mode; }
	void GetMouseLocation(DPoint2 &p);

	// UTM zone boundary display
	void DrawUTMBounds(wxDC *pDC);
	void DrawDymaxionOutline(wxDC *pDC);

	// World Map
	void SetWMProj(const vtProjection &p);

	// Key handler
	void OnChar(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnMouseWheel(wxMouseEvent& event);

	// More public methods
	void DeselectAll();
	void DeleteSelected(vtRoadLayer *pRL);
	void SetActiveLayer(vtLayer *lp);
	void MatchZoomToElev(vtElevLayer *pEL);
	void MatchZoomToImage(vtImageLayer *pEL);
	void SetShowMap(bool bShow);
	bool GetShowMap() { return m_bShowMap; }
	void SetShowScaleBar(bool bShow);
	bool GetShowScaleBar() { return m_bScaleBar; }
	void InvertAreaTool(const DRECT &rect);
	void ShowGridMarks(const DRECT &area, int cols, int rows, int active_col, int active_row);
	void HideGridMarks();
	void SetDistanceToolMode(bool bPath);
	bool GetDistanceToolMode();
	void SetDistancePoints(const DPoint2 &p1, const DPoint2 &p2)
	{ m_distance_p1 = p1; m_distance_p2 = p2; }
	void SetDistancePath(const DLine2 &path)
	{ m_distance_path = path; }
	void ClearDistanceTool();
	void UpdateDistance();
	void DrawInvertedLine(const DPoint2 &ep1, const DPoint2 &ep2);

	void RunTest();

	bool	m_bCrossSelect;
	bool	m_bShowUTMBounds;
	bool	m_bConstrain;

protected:
	// Edit
	void UpdateResizeScale();
	void UpdateRotate();
	void OnDragDistance();

	// Elevation
	void CheckForTerrainSelect(const DPoint2 &loc);
	void HighlightArea(wxDC *pDC, const DRECT &rect);

	// Pan handlers
	void BeginPan();
	void EndPan();
	void DoPan(wxPoint point);

	// Box handlers
	void BeginBox();
	void EndBox(const wxMouseEvent& event);
	void EndBoxFeatureSelect(const wxMouseEvent& event);
	void DoBox(wxPoint point);
	void BeginArea();
	void DoArea(wxPoint delta);

	// Mouse handlers
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnLeftDoubleClick(wxMouseEvent& event);
	void OnMiddleDown(wxMouseEvent& event);
	void OnMiddleUp(wxMouseEvent& event);
	void OnRightDown(wxMouseEvent& event);
	void OnRightUp(wxMouseEvent& event);

	void OnLButtonClick(wxMouseEvent& event);
	void OnLButtonDragRelease(wxMouseEvent& event);
	void OnLButtonClickElement(vtRoadLayer *pRL);
	void OnLButtonClickLinkEdit(vtRoadLayer *pRL);
	void OnLButtonClickFeature(vtLayer *pL);
	void OnRightUpStructure(vtStructureLayer *pSL);

	void OnMouseMove(wxMouseEvent& event);
	void OnMouseMoveLButton(const wxPoint &point);

	void OnIdle(wxIdleEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);

	void InvertRect(wxDC *pDC, const wxRect &r, bool bDashed = false);
	void InvertRect(wxDC *pDC, const wxPoint &one, const wxPoint &two, bool bDashed = false);
	void DrawAreaTool(wxDC *pDC, const DRECT &area);
	void DrawDistanceTool(wxDC *pDC);
	void DrawDistanceTool();
	void BeginDistance();

	bool m_bGotFirstIdle;
	bool m_bSkipNextDraw;
	bool m_bSkipNextRefresh;
	wxSize m_previous_size;
	DRECT m_world_rect;		// rectangle box drawn by mouse
	DPoint2 m_distance_p1, m_distance_p2;
	DLine2 m_distance_path;

	// Mouse in window coords
	wxPoint m_DownClient;

	// Used while mouse button is down
	bool	m_bMouseMoved;
	bool	m_bPanning;		// currently panning
	bool	m_bScrolling;	// currently scrolling
	bool	m_bBoxing;		// currently drawing a rubber box
	int		m_iDragSide;	// which side of the area box being dragged

	// Used while editing roads
	void RefreshRoad(LinkEdit *pRoad);

	wxCursor	*m_pCursorPan;
	bool		m_bMouseCaptured;

	// World Map
	bool	m_bShowMap;
	bool	m_bScaleBar;
	DLine2Array		WMPoly;		// Original data from SHP file
	DLine2Array		WMPolyDraw; // This is the WM that is drawn
	std::vector<DRECT>	WMPolyExtents;
	uint	m_iEntities;
	bool	m_bAttemptedLoad;
	DymaxIcosa		m_icosa;
	OCT		*m_pMapToCurrent, *m_pCurrentToMap;

	bool ImportWorldMap();
	void DrawWorldMap(wxDC *pDC);

	// Grid marks
	bool m_bShowGridMarks;
	DRECT m_GridArea;
	int m_iGridCols, m_iGridRows, m_iActiveCol, m_iActiveRow;
	void DrawGridMarks(wxDC &dc);

	//--------------------------------
	void DrawScaleBar(wxDC * dc);
	void OnBeginScroll(wxScrollWinEvent & event);
	void OnEndScroll(wxScrollWinEvent& event);
	void OnOtherScrollEvents(wxScrollWinEvent & event);
	wxRect m_ScaleBarArea;

	UIContext m_ui;

	DECLARE_EVENT_TABLE()
};

#endif

