//
// class Enviro: Main functionality of the Enviro application
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ENVIROH
#define ENVIROH

#include "vtdata/Fence.h"
#include "vtdata/Projections.h"
#include "vtlib/core/Engine.h"
#include "EnviroEnum.h"
#include "PlantingOptions.h"

// Use forward declarations to minimize rebuild dependencies
class vtTerrainScene;
class vtTerrain;
class TerrainPicker;
class IcoGlobe;
class vtRoute;
class vtUtilNode;
class vtFence3d;

// Engines
class vtTerrainFlyer;
class RouteFollowerEngine;
class VFlyer;
class QuakeFlyer;
class GrabFlyer;
class vtTrackball;
class GlobePicker;

// Plants
class vtPlantList3d;

class ControlEngine : public vtEngine
{
public:
	void Eval();
};

class Enviro
{
public:
	Enviro();
	~Enviro();

	// methods
	void Startup();
	void Shutdown();

	void LoadTerrainDescriptions();
	void StartControlEngine();
	void DoControl();
	void SetFlightSpeed(float speed);
	float GetFlightSpeed();
	void SetTopDown(bool bTopDown);
	void SetTerrain(vtTerrain *pTerrain);
	void EnableFlyerEngine(bool bEnable);
	void SetCurrentNavigator(vtTerrainFlyer *pE);
	void SetMode(MouseMode mode);
	void SetRouteFollower(bool bOn);
	bool GetRouteFollower();
	void DumpCameraInfo();

	// go to space or a terrain
	void FlyToSpace();
	bool SwitchToTerrain(const char *name);
	void SwitchToTerrain(vtTerrain *pTerr);

	// these work in space
	bool GetShowTime();
	void SetShowTime(bool bShow);
	void SetGlobeTime(struct tm *gmt);
	void SetEarthShape(bool Flat);
	bool GetEarthShape() { return m_bGlobeFlat; }
	void SetEarthUnfold(bool Flat);
	bool GetEarthUnfold() { return m_bGlobeUnfolded; }
	int AddGlobePoints(const char *fname);
	void SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2);
	vtTerrain *FindTerrainOnEarth(const DPoint2 &p);

	vtString GetMessage() { return m_strMessage; }
	void SetMessage(const char *msg, float time = 0.0f);
	void FormatCoordString(vtString &str, const DPoint3 &coord, LinearUnits units);
	void DescribeCoordinates(vtString &str);
	void DescribeCLOD(vtString &str);

	// location of 3d cursor on the ground, in earth coordinates
	DPoint3			m_EarthPos;
	TerrainPicker	*m_pTerrainPicker;
	GlobePicker		*m_pGlobePicker;
	vtMovGeom		*m_pCursorMGeom;
	FMatrix4		m_SpaceCamLocation;

	// navigation engines
	vtTerrainFlyer	*m_pTFlyer;
	VFlyer			*m_pVFlyer;
	QuakeFlyer		*m_pQuakeFlyer;
	vtTerrainFlyer	*m_pCurrentFlyer;
	GrabFlyer		*m_pGFlyer;

	vtTerrainScene	*m_pTerrainScene;

	// event handlers
	void OnMouse(vtMouseEvent &event);
	void OnMouseLeftDownTerrain(vtMouseEvent &event);
	void OnMouseLeftDownOrbit(vtMouseEvent &event);
	void OnMouseRightDown(vtMouseEvent &event);
	void OnMouseRightUp(vtMouseEvent &event);
	void OnMouseMove(vtMouseEvent &event);

	// fence methods
	void start_new_fence();
	void finish_fence();
	void close_fence();
	void SetFenceOptions(FenceType type, float fHeight, float fSpacing);

	// route methods
	void start_new_route();
	void finish_route();
	void close_route();
	void SetRouteOptions(const vtString &sStructType);

	// plants
	vtPlantList3d	*GetPlantList() { return m_pPlantList; }
	PlantingOptions &GetPlantOptions() { return m_PlantOpt; }
	bool PlantATree(const DPoint2 &epos);
	void SetPlantOptions(PlantingOptions &opt) { m_PlantOpt = opt; }

	// global state
	AppState	m_state;
	MouseMode	m_mode;
	NavType		m_nav;
	bool		m_bOnTerrain;
	vtString	m_strMessage;

	// used for mouse interaction
	bool		m_bDragging;
	bool		m_bSelectedStruct;
	bool		m_bSelectedPlant;
	bool		m_bSelectedUtil;
	DPoint3		m_EarthPosDown;
	DPoint3		m_EarthPosLast;
	vtRoute		*m_pCurRoute;
	vtUtilNode	*m_pSelUtilNode;
	vtRoute		*m_pSelRoute;

protected:
	// startup methods
	void SetupScene1();
	void SetupScene2();
	void DoPickers();
	void MakeGlobe();
	void SetupGlobe();
	void LookUpTerrainLocations();
	void AddTerrainRectangles();
	void SetupTerrain(vtTerrain *pTerr);

	// plants
	vtPlantList3d	*m_pPlantList;
	PlantingOptions m_PlantOpt;

	// fence members
	bool		m_bActiveFence, m_bFenceClosed;
	vtFence3d	*m_pCurFence;
	FenceType	m_CurFenceType;
	float		m_fFenceHeight, m_fFenceSpacing;

	// route members
	bool		m_bActiveRoute;
	vtString	m_sStructType;
	RouteFollowerEngine	*m_pRouteFollower;

	// linear arc on Earth
	vtGeom *m_pArc;
	vtMesh *m_pArcMesh;
	double m_fArcLength; 

	// view and navigation
	vtCamera	*m_pNormalCamera;
	vtCamera	*m_pTopDownCamera;

	vtRoot		*m_pRoot;		// top of the scene graph
	vtTransform	*m_pGlobeXForm;
	IcoGlobe	*m_pIcoGlobe;
	bool		m_bShowTime;
	bool		m_bGlobeFlat;
	float		m_fFlattening, m_fFlattenDir;
	bool		m_bGlobeUnfolded;
	float		m_fFolding, m_fFoldDir;
	vtTrackball	*m_pTrackball;

	vtSprite	*m_pMessageSprite;
	vtSprite	*m_pSprite2;
	float			m_fMessageStart, m_fMessageTime;
	ControlEngine	*m_pControlEng;

	int			m_iInitStep;			// initialization stage
	vtTerrain	*m_pTargetTerrain;		// terrain we are switching to
};

// global singleton
extern Enviro g_App;

// global helper functions
vtTerrain *GetCurrentTerrain();
vtTerrainScene *GetTerrainScene();

#endif	// ENVIROH

