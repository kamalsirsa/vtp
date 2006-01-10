//
// Engines.h
// header file for Engines.cpp
//

#ifndef ENVIRO_ENGINESH
#define ENVIRO_ENGINESH

#include "vtlib/core/Engine.h"
#include "vtlib/core/NavEngines.h"

class NodeGeom;
class LinkGeom;
class vtRoute;
class vtHeightField;

///////////////////////////////////////////////////

enum AirportCodes {ITO, KOA, MUE, UPP};

class PlaneEngine : public vtEngine
{
public:
	PlaneEngine(float fSpeedExag, float fScale, AirportCodes code);
	void Eval();
	void SetHoop(int i);

	FPoint3	m_hoop_pos[10];
	float	m_hoop_speed[10];
	int		m_hoops;
	int		m_hoop;

	FPoint3	m_pos, m_dir;
	float	m_fSpeed;		// in meters/sec
	float	m_fSpeedExag;	// this many times normal speed
	float	m_fLastTime;
	float	m_fScale;
};

///////////////////////////////////////////////////

class RoadFollowEngine : public vtEngine
{
public:
	RoadFollowEngine(NodeGeom *pStartNode);
	void Eval();

	void	PickRoad(LinkGeom *last_road);

	float	fAmount;
//	RoadMap	*m_pRoadMap;
	NodeGeom	*m_pLastNode;
	LinkGeom	*m_pCurrentRoad;
	float	m_pCurrentRoadLength;
	bool	forwards;
};

//////////////////////////////////////////////////

class FollowerEngine : public vtEngine
{
public:
	FollowerEngine(vtTransform* model, vtCamera* camera);
	void Eval();

	vtTransform *m_model;
	vtCamera *m_pCamera;
};

///////////////////////////////////////////////////

class TerrainPicker : public vtLastMouse
{
public:
	TerrainPicker();
	void Eval();
	void SetHeightField(vtHeightField3d *pHeight) { m_pHeightField = pHeight; }

	bool GetCurrentPoint(FPoint3 &p);
	bool GetCurrentEarthPos(DPoint3 &p);

	void OnMouse(vtMouseEvent &event);

protected:
	void FindGroundPoint();

	FPoint3			m_GroundPoint;
	vtHeightField3d *m_pHeightField;
	DPoint3			m_EarthPos;
	bool			m_bOnTerrain;
};

///////////////////////////////////////////////////

class IcoGlobe;

class GlobePicker : public vtLastMouse
{
public:
	GlobePicker();
	void Eval();
	void SetRadius(double fRadius) { m_fRadius = fRadius; }
	void SetGlobe(IcoGlobe *pGlobe) { m_pGlobe = pGlobe; }
	void SetTargetScale(float s) { m_fTargetScale = s; }

	bool GetCurrentPoint(FPoint3 &p);
	bool GetCurrentEarthPos(DPoint3 &p);

protected:
	FPoint3		m_GroundPoint;
	DPoint3		m_EarthPos;
	bool		m_bOnTerrain;
	double		m_fRadius;
	IcoGlobe	*m_pGlobe;
	float		m_fTargetScale;
};

///////////////////////////////////////////////////

class FlatFlyer : public vtLastMouse
{
public:
	FlatFlyer();

	void Eval();
	void OnMouse(vtMouseEvent &event);

protected:
	bool m_bDrag;
	bool m_bZoom;
	FPoint3 m_start_wp;
	IPoint2 m_startpos;
};

///////////////////////////////////////////////////

class GrabFlyer : public vtTerrainFlyer
{
public:
	GrabFlyer(float fSpeed);
	void Eval();
	void OnMouse(vtMouseEvent &event);

	void SetTerrainPicker(TerrainPicker *pTP) { m_pTP = pTP; }

protected:
	bool m_bDrag;
	bool m_bPivot;
	bool m_bZoom;
	IPoint2 m_startpos;

	TerrainPicker *m_pTP;
	FPoint3 m_start_wp;
	FMatrix4 m_start_eye;

	// for dragging
	float m_fHeight;
};

/**
 * Provide a view of the terrain from the top
 * and display a cursor to show current canera position
 * User can move the camera by clicking on the map
 * with the mouse middle button.
 */
class MapOverviewEngine : public vtEngine
{
public:
	MapOverviewEngine();
	~MapOverviewEngine();

    void OnMouse(vtMouseEvent &event);	
	void Eval();

	void ShowMapOverview(bool bShow) { m_pMapGroup->SetEnabled(bShow); }
	bool GetShowMapOverview() {return m_pMapGroup->GetEnabled();}
	void SetTerrain(vtTerrain *pTerr);

protected:
	void CreateMapView();
	void RefreshMapView();
	void CreateArrow();

	vtGroup			*m_pMapGroup;
	vtImageSprite	*m_pMapView;
	vtTransform		*m_pArrow;
	vtImage			*m_pOwnedImage;

	float	anglePrec;
	int		MapWidth;
	int		MapMargin;
	float	MapRatio;
	float	ratioMapTerrain;
	bool	m_bDown;
};

#endif	// ENVIRO_ENGINESH

