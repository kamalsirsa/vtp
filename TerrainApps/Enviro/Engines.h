//
// Engines.h
// header file for Engines.cpp
//

#ifndef ENVIRO_ENGINESH
#define ENVIRO_ENGINESH

#include "vtlib/core/LocalGrid.h"
#include "vtlib/core/Roads.h"
#include "vtlib/core/Terrain.h"
#include "SwapEngine.h"

enum AirportCodes {ITO, KOA, MUE, UPP};


///////////////////////////////////////////////////

class BgColorEngine : public vtEngine
{
public:
	void Eval();
};

///////////////////////////////////////////////////

class PlaneEngine : public vtEngine
{
public:
	PlaneEngine(float fSpeedExag, AirportCodes code);
	void Eval();
	void SetHoop(int i);

	FPoint3	m_hoop_pos[10];
	float	m_hoop_speed[10];
	int		m_hoops;
	int		m_hoop;

	FPoint3	m_pos, m_dir;
	float	m_fSpeed;		// in meters/sec
//	float	m_fSizeExag;	//  exaggerate size by this amount
	float	m_fSpeedExag;	// this many times normal speed
	float	m_fLastTime;
};

///////////////////////////////////////////////////

class RoadFollowEngine : public vtEngine
{
public:
	RoadFollowEngine(NodeGeom *pStartNode);
	void Eval();

	void	PickRoad(RoadGeom *last_road);

	float	fAmount;
//	RoadMap	*m_pRoadMap;
	NodeGeom	*m_pLastNode;
	RoadGeom	*m_pCurrentRoad;
	float	m_pCurrentRoadLength;
	bool	forwards;
};

//////////////////////////////////////////////////

class BenchEngine : public vtEngine
{
public:
	BenchEngine(const char *loc_fname);
	void Eval();
	void Convert(float result, FILE* fp, bool last);

	FILE* m_fp;
	int m_idone;
	int m_iviewpoint;
	int m_iframesElapsed;
	float m_fstartTime;
	float m_fresults;	//total framerate (to be divided by # of viewpoints)
};

//////////////////////////////////////////////////

// Follow the current route
class RouteFollowerEngine : public vtEngine
{
public:
	RouteFollowerEngine(vtRoute* route, vtCamera* camera);
	void Eval();

	vtHeightField* m_pHeightField;
	vtRoute* m_pRoute;
	vtCamera* m_pCamera;
	bool m_bFollowerOn;
	bool m_bFirstTime;
	long m_lnext;
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

class ProximityEngine : public vtEngine
{
public:
	ProximityEngine(vtTransform* far_model, vtTransform* near_model,
		vtCamera* camera, float distance);
	void Eval();

	vtTransform *m_FarModel;
	vtTransform *m_NearModel;
	vtCamera *m_pCamera;
	bool m_bFarOn;
	float m_distance;
};

class SimpleBBEngine : public vtEngine
{
public:
	SimpleBBEngine(vtTransform* model, vtCamera* camera);
	void Eval();

	vtTransform *m_model;
	vtCamera *m_pCamera;
	float m_fAngle;
};

///////////////////////////////////////////////////

class TerrainPicker : public vtLastMouse
{
public:
	TerrainPicker();
//	void OnMouse(vtMouseEvent &event);
	void Eval();
	void SetHeightField(vtHeightField *pHeight) { m_pHeightField = pHeight; }

	bool GetCurrentPoint(FPoint3 &p);
	bool GetCurrentEarthPos(DPoint3 &p);

protected:
	FPoint3		m_GroundPoint;
	vtHeightField *m_pHeightField;
	DPoint3		m_EarthPos;
	bool		m_bOnTerrain;
};

///////////////////////////////////////////////////

class GlobePicker : public vtLastMouse
{
public:
	GlobePicker();
//	void OnMouse(vtMouseEvent &event);
	void Eval();
	void SetRadius(double fRadius) { m_fRadius = fRadius; }
	void SetGlobeMGeom(vtMovGeom *pMG) { m_pGlobeMGeom = pMG; }

	bool GetCurrentPoint(FPoint3 &p);
	bool GetCurrentEarthPos(DPoint3 &p);

protected:
	FPoint3		m_GroundPoint;
	DPoint3		m_EarthPos;
	bool		m_bOnTerrain;
	double		m_fRadius;
	vtMovGeom	*m_pGlobeMGeom;
};

#endif
