//
// class Enviro: Main functionality of the Enviro application
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ENVIROH
#define ENVIROH

#include "vtlib/core/TerrainScene.h"

#include "vtdata/Fence.h"
#include "vtdata/Projections.h"
#include "vtlib/core/NavEngines.h"
#include "EnviroEnum.h"
#include "PlantingOptions.h"

#define INITIAL_SPACE_DIST	3.1f

// Use forward declarations to minimize rebuild dependencies
class vtTerrainScene;
class vtTerrain;
class TerrainPicker;
class IcoGlobe;
class vtRoute;
class vtUtilNode;
class vtFence3d;

// Engines
class GlobePicker;
class GrabFlyer;
class FlatFlyer;

// Plants
class vtSpeciesList3d;

class ControlEngine : public vtEngine
{
public:
	void Eval();
};

class Enviro : public vtTerrainScene
{
public:
	Enviro();
	virtual ~Enviro();

	// methods
	void Startup();
	void Shutdown();
	virtual void StartupArgument(int i, const char *str);

	void LoadTerrainDescriptions();
	void StartControlEngine();
	void DoControl();
	void DoControlOrbit();
	void SetTerrain(vtTerrain *pTerrain);
	vtGroup *GetRoot() { return m_pRoot; }

	void ShowElevationLegend(bool bShow);
	bool GetShowElevationLegend();

	// navigation and camera
	void SetFlightSpeed(float speed);
	float GetFlightSpeed();
	void SetFlightAccel(bool bAccel);
	bool GetFlightAccel();
	void SetTopDown(bool bTopDown);
	void SetCurrentNavigator(vtTerrainFlyer *pE);
	void EnableFlyerEngine(bool bEnable);
	void SetNavType(NavType nav);
	void SetMaintain(bool bOn);
	bool GetMaintain();

	void SetMode(MouseMode mode);
	void DumpCameraInfo();
	void SetSpeed(float x);
	float GetSpeed();
	vtString GetStatusString(int which);
//	void GetStatusText(vtString &str);

	// go to space or a terrain
	void FlyToSpace();
	bool SwitchToTerrain(const char *name);
	void SwitchToTerrain(vtTerrain *pTerr);

	// these work in space
	IcoGlobe *GetGlobe() { return m_pIcoGlobe; }
	void SetEarthShading(bool bShade);
	bool GetEarthShading();
	void SetEarthTilt(bool bTilt);
	bool GetEarthTilt();
	void SetEarthShape(bool Flat);
	bool GetEarthShape() { return m_bGlobeFlat; }
	void SetEarthUnfold(bool Flat);
	bool GetEarthUnfold() { return m_bGlobeUnfolded; }
	void SetSpaceAxes(bool bShow);
	bool GetSpaceAxes();
	int AddGlobePoints(const char *fname);
	void UpdateEarthArc();
	void SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2);
	void SetEarthLines(double lon, double lat);
	void ShowEarthLines(bool bShow);
	vtTerrain *FindTerrainOnEarth(const DPoint2 &p);

	vtString GetMessage() { return m_strMessage; }
	void SetMessage(const vtString &msg, float time = 0.0f);
	void FormatCoordString(vtString &str, const DPoint3 &coord, LinearUnits units, bool seconds = false);
	void DescribeCoordinatesEarth(vtString &str);
	void DescribeCoordinatesTerrain(vtString &str);
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
	vtOrthoFlyer	*m_pOrthoFlyer;
	QuakeFlyer		*m_pQuakeFlyer;
	vtTerrainFlyer	*m_pCurrentFlyer;
	GrabFlyer		*m_pGFlyer;
	FlatFlyer		*m_pFlatFlyer;
	vtPanoFlyer		*m_pPanoFlyer;
	// related engine to keep the camera above the terrain
	vtHeightConstrain *m_pHeightEngine;

	// event handlers
	void OnMouse(vtMouseEvent &event);
	void OnMouseLeftDownTerrain(vtMouseEvent &event);
	void OnMouseLeftDownTerrainSelect(vtMouseEvent &event);
	void OnMouseLeftDownOrbit(vtMouseEvent &event);
	void OnMouseLeftUp(vtMouseEvent &event);
	void OnMouseRightDown(vtMouseEvent &event);
	void OnMouseRightUp(vtMouseEvent &event);
	void OnMouseMove(vtMouseEvent &event);
	void OnMouseMoveTerrain(vtMouseEvent &event);

	// fence methods
	void start_new_fence();
	void finish_fence();
	void close_fence();
	void SetFenceOptions(const vtLinearParams &param);

	// route methods
	void start_new_route();
	void finish_route();
	void close_route();
	void SetRouteOptions(const vtString &sStructType);

	// plants
	void LoadSpeciesList();
	vtSpeciesList3d	*GetPlantList() { return m_pPlantList; }
	PlantingOptions &GetPlantOptions() { return m_PlantOpt; }
	bool PlantATree(const DPoint2 &epos);
	void SetPlantOptions(PlantingOptions &opt);

	// global state
	AppState	m_state;
	MouseMode	m_mode;
	NavType		m_nav;
	bool		m_bOnTerrain;
	vtString	m_strMessage;

	// used for mouse interaction
	bool		m_bDragging;
	bool		m_bRotating;
	bool		m_bSelectedStruct;
	bool		m_bSelectedPlant;
	bool		m_bSelectedUtil;
	DPoint3		m_EarthPosDown;
	DPoint3		m_EarthPosLast;
	vtRoute		*m_pCurRoute;
	vtUtilNode	*m_pSelUtilNode;
	vtRoute		*m_pSelRoute;
	IPoint2		m_MouseDown;
	float		m_StartRotation;
	vtEngine	*m_pLocEngines;

	// handle to the singleton
	static Enviro *s_pEnviro;

	// The following can be overridden by GUI code, to handle situations
	//  in which the GUI must be informed of what happens in this object.
	virtual void ShowPopupMenu(const IPoint2 &pos) {}
	virtual void SetTerrainToGUI(vtTerrain *pTerrain) {}
	virtual void ShowLayerView() {}
	virtual void RefreshLayerView() {}
	virtual void CameraChanged() {}
	virtual void EarthPosUpdated() {}
	virtual void ShowDistance(const DPoint2 &p1, const DPoint2 &p2,
		float fGround, float fVertical) {}
	virtual vtTagArray *GetInstanceFromGUI() { return NULL; }
	virtual bool OnMouseEvent(vtMouseEvent &event) { return false; }
	virtual void SetTimeEngineToGUI(TimeEngine *pEngine) {}
	virtual bool IsAcceptable(vtTerrain *pTerr) { return true; }
	virtual void ShowMessage(const vtString &str) {}

	// temporary for demo use
	void ToggleLogo(); vtGroup *logo;

protected:
	// startup methods
	void SetupScene1();
	void SetupScene2();
	virtual void SetupScene3() {}
	void DoCursorOnEarth();
	void DoCursorOnTerrain();
	void MakeGlobe();
	void SetupGlobe();
	void LookUpTerrainLocations();
	void SetupTerrain(vtTerrain *pTerr);
	void PlantInstance();
	void SetupArcMesh();
	void FreeArc();
	void FreeArcMesh();
	void SetTerrainMeasure(const DPoint2 &g1, const DPoint2 &g2);
	void CreateElevationLegend();

	// plants
	vtSpeciesList3d	*m_pPlantList;
	PlantingOptions m_PlantOpt;
	bool		m_bPlantsLoaded;
	bool		m_bDoPlants;

	// fence members
	bool		m_bActiveFence, m_bFenceClosed;
	vtFence3d	*m_pCurFence;
	vtLinearParams m_FenceParams;

	// route members
	bool		m_bActiveRoute;
	vtString	m_sStructType;

	// linear arc on Earth (or Distance Tool on the Terrain)
	vtGeom		*m_pArc;
	vtMaterialArray *m_pArcMats;
	double		m_fArcLength;
	float		m_fDistToolHeight;

	// view and navigation
	vtCamera	*m_pNormalCamera;
	vtCamera	*m_pTopDownCamera;
	bool		m_bTopDown;
	vtEngine	*m_pNavEngines;

	vtGroup		*m_pRoot;		// top of the scene graph

	// globe
	TimeEngine	*m_pGlobeTime;
	vtGroup		*m_pGlobeContainer;
	IcoGlobe	*m_pIcoGlobe;
	bool		m_bEarthShade;
	bool		m_bGlobeFlat;
	float		m_fFlattening, m_fFlattenDir;
	bool		m_bGlobeUnfolded;
	float		m_fFolding, m_fFoldDir;
	vtTrackball	*m_pTrackball;
	vtGeom		*m_pSpaceAxes;
	vtGeom		*m_pEarthLines;
	vtMesh		*m_pLineMesh;

	// flattened globe view
	FPQ			m_SpaceLoc;
	FPQ			m_FlatLoc;

	float			m_fMessageStart, m_fMessageTime;
	ControlEngine	*m_pControlEng;
	vtHUD		*m_pHUD;

	int			m_iInitStep;			// initialization stage
	vtTerrain	*m_pTargetTerrain;		// terrain we are switching to

	vtGeom		*m_pLegendGeom;
	bool		m_bCreatedLegend;
};

// global helper functions
vtTerrain *GetCurrentTerrain();

#endif	// ENVIROH

