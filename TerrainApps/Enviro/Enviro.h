//
// class Enviro: Main functionality of the Enviro application
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ENVIROH
#define ENVIROH

#include "vtlib/core/TerrainScene.h"

#include "vtdata/Fence.h"
#include "vtdata/Projections.h"
#include "vtlib/core/AnimPath.h"
#include "vtlib/core/NavEngines.h"
#include "vtlib/core/Vehicles.h"
#include "EnviroEnum.h"
#include "PlantingOptions.h"

#define INITIAL_SPACE_DIST	3.1f

// Use forward declarations to minimize rebuild dependencies
class vtTerrainScene;
class vtTerrain;
class TerrainPicker;
class vtIcoGlobe;
class vtRoute;
class vtUtilNode;
class vtFence3d;
class vtAbstractLayer;
class GlobeLayer;

// Engines
class GlobePicker;
class GrabFlyer;
class FlatFlyer;
class MapOverviewEngine;
class CarEngine;
class vtSpriteSizer;

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

	void LoadAllTerrainDescriptions();
	void LoadGlobalContent();
	void StartControlEngine();
	void DoControl();
	void DoControlOrbit();
	void DoControlTerrain();
	void SetTerrain(vtTerrain *pTerrain);
	vtGroup *GetRoot() { return m_pRoot; }
	void StoreTerrainParameters();
	osgText::Font *GetArial() { return m_pArial; }

	void ShowElevationLegend(bool bShow);
	bool GetShowElevationLegend();
	void ShowCompass(bool bShow);
	bool GetShowCompass();
	void ShowMapOverview(bool bShow);
	bool GetShowMapOverview();
	void TextureHasChanged();

	// navigation and camera
	virtual void SetFlightSpeed(float speed);
	float GetFlightSpeed();
	void SetFlightAccel(bool bAccel);
	bool GetFlightAccel();
	void SetTopDown(bool bTopDown);
	void SetCurrentNavigator(vtTerrainFlyer *pE);
	void EnableFlyerEngine(bool bEnable);
	void SetNavType(NavType nav);
	NavType GetNavType() {return m_nav; }
	void SetMaintain(bool bOn);
	bool GetMaintain();
	void ResetCamera();

	void SetMode(MouseMode mode);
	void DumpCameraInfo();
	void SetSpeed(float x);
	float GetSpeed();
	vtString GetStatusString(int which);

	// go to space or a terrain
	void FlyToSpace();
	bool SwitchToTerrain(const char *name);
	void SwitchToTerrain(vtTerrain *pTerr);

	// these work in space
	vtIcoGlobe *GetGlobe() { return m_pIcoGlobe; }
	vtIcoGlobe *GetOverlayGlobe() { return m_pOverlayGlobe; }
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
	int AddGlobeAbstractLayer(const char *fname);
	void RemoveGlobeAbstractLayer(GlobeLayer *glay);
	void UpdateEarthArc();
	void SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2);
	void SetDisplayedArc(const DLine2 &path);
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
	vtMovGeode		*m_pCursorMGeom;

	// navigation engines
	vtTerrainFlyer	*m_pTFlyer;
	VFlyer			*m_pVFlyer;
	vtOrthoFlyer	*m_pOrthoFlyer;
	QuakeFlyer		*m_pQuakeFlyer;
	vtTerrainFlyer	*m_pCurrentFlyer;
	GrabFlyer		*m_pGFlyer;
	FlatFlyer		*m_pFlatFlyer;
	vtPanoFlyer		*m_pPanoFlyer;
	// engine to keep the camera above the terrain
	vtHeightConstrain *m_pHeightEngine;

	// event handlers
	bool OnMouse(vtMouseEvent &event);
	void OnMouseLeftDownTerrain(vtMouseEvent &event);
	void OnMouseLeftDownBuildings();
	void OnMouseLeftDownTerrainSelect(vtMouseEvent &event);
	void OnMouseLeftDownTerrainMove(vtMouseEvent &event);
	void OnMouseLeftDownOrbit(vtMouseEvent &event);
	void OnMouseLeftUp(vtMouseEvent &event);
	void OnMouseLeftUpBox(vtMouseEvent &event);
	void OnMouseRightDown(vtMouseEvent &event);
	void OnMouseRightUp(vtMouseEvent &event);
	void OnMouseMove(vtMouseEvent &event);
	void OnMouseMoveTerrain(vtMouseEvent &event);
	void OnMouseSelectRayPick(vtMouseEvent &event);
	void OnMouseSelectCursorPick(vtMouseEvent &event);
	bool OnMouseCompass(vtMouseEvent &event);

	// fence methods
	void start_new_fence();
	void finish_fence();
	void close_fence();
	void SetFenceOptions(const vtLinearParams &param, bool bProfileChanged = false);

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
	void SetPlantOptions(const PlantingOptions &opt);

	// vehicles
	void SetVehicleOptions(const VehicleOptions &opt);
	CarEngine *CreateGroundVehicle(const VehicleOptions &opt);
	VehicleManager m_VehicleManager;
	VehicleSet m_Vehicles;

	// abstract layers
	vtAbstractLayer *GetLabelLayer();
	int NumSelectedAbstractFeatures();

	// distance tool
	void SetDistanceToolMode(bool bPath);
	void SetDistanceTool(const DLine2 &line);
	void ResetDistanceTool();
	void UpdateDistanceTool();

	// Wind
	void SetWind(int iDirection, float fSpeed);

	// UI
	void PolygonSelectionAddPoint();
	void PolygonSelectionClose();
	DLine2 &GetPolygonSelection() { return m_NewLine; }
	void UpdateCompass();

	// global state
	AppState	m_state;
	MouseMode	m_mode;
	NavType		m_nav;
	bool		m_bOnTerrain;
	vtString	m_strMessage;

	// used for mouse interaction
	bool		m_bDragging;
	bool		m_bDragUpDown;
	bool		m_bRotating;
	bool		m_bSelectedStruct;
	bool		m_bSelectedPlant;
	bool		m_bSelectedUtil;
	bool		m_bSelectedVehicle;
	vtFence3d	*m_pDraggingFence;
	int			m_iDraggingFencePoint;
	DPoint3		m_EarthPosDown;
	DPoint3		m_EarthPosLast;
	vtRoute		*m_pCurRoute;
	vtUtilNode	*m_pSelUtilNode;
	vtRoute		*m_pSelRoute;
	IPoint2		m_MouseDown, m_MouseLast;
	float		m_StartRotation;

	// handle to the singleton
	static Enviro *s_pEnviro;

	// The following can be overridden by GUI code, to handle situations
	//  in which the GUI may need to be informed of what happens.
	virtual void ShowPopupMenu(const IPoint2 &pos) {}
	virtual void SetTerrainToGUI(vtTerrain *pTerrain) {}
	virtual void ShowLayerView() {}
	virtual void RefreshLayerView() {}
	virtual void UpdateLayerView() {}
	virtual void CameraChanged() {}
	virtual void EarthPosUpdated() {}
	virtual void ShowDistance(const DPoint2 &p1, const DPoint2 &p2,
		double fGround, double fVertical) {}
	virtual void ShowDistance(const DLine2 &path,
		double fGround, double fVertical) {}
	virtual vtTagArray *GetInstanceFromGUI() { return NULL; }
	virtual bool OnMouseEvent(vtMouseEvent &event) { return false; }
	virtual void SetTimeEngineToGUI(vtTimeEngine *pEngine) {}
	virtual bool IsAcceptable(vtTerrain *pTerr) { return true; }
	virtual void OnCreateTerrain(vtTerrain *pTerr) {}
	virtual void ShowMessage(const vtString &str) {}
	virtual void SetState(AppState s) { m_state = s; }
	virtual vtString GetStringFromUser(const vtString &title, const vtString &msg) = 0;
	virtual void ShowProgress(bool bShow) {}
	virtual void SetProgressTerrain(vtTerrain *pTerr) {}
	virtual void UpdateProgress(const char *msg, int amount1, int amount2) {}
	virtual void ExtendStructure(vtStructInstance *si) {}
	virtual void AddVehicle(CarEngine *eng) {}
	virtual void RemoveVehicle(CarEngine *eng) {}

	// temporary for demo use
	void MakeDemoGlobe();
	void ToggleDemo();
	vtGroup *m_pDemoGroup;
	vtGeode *m_pDemoTrails;
	void CreateSomeTestVehicles(vtTerrain *pTerrain);
	void MakeOverlayGlobe(vtImage *image, bool progress_callback(int) = NULL);

protected:
	// startup methods
	void LoadTerrainDescriptions(const vtString &path);
	void SetupScene1();
	void SetupScene2();
	virtual void SetupScene3() {}
	// other implementation methods
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
	void SetTerrainMeasure(const DLine2 &path);
	void CreateElevationLegend();
	void CreateCompass();
	void CreateMapOverview();
	void StartFlyIn();
	void FlyInStage1();
	void FlyInStage2();
	void SetWindowBox(const IPoint2 &ul, const IPoint2 &lr);

	// plants
	vtSpeciesList3d	*m_pPlantList;
	PlantingOptions m_PlantOpt;
	bool		m_bPlantsLoaded;

	// vehicles
	VehicleOptions m_VehicleOpt;

	// fence members
	bool		m_bActiveFence, m_bFenceClosed;
	vtFence3d	*m_pCurFence;
	vtLinearParams m_FenceParams;
	DLine2		m_NewLine;		// Line for building footprint
	bool		m_bLineDrawing;	//is Drawing
	std::vector<vtTransform*> m_Markers;

	// route members
	bool		m_bActiveRoute;
	vtString	m_sStructType;

	// linear arc on Earth (or Distance Tool on the Terrain)
	vtGeode		*m_pArc;
	vtMaterialArray *m_pArcMats;
	double		m_fArcLength;
	float		m_fDistToolHeight;
	bool		m_bMeasurePath;
	DLine2		m_distance_path;

	// view and navigation
	vtCameraPtr	m_pNormalCamera;
	vtCameraPtr	m_pTopDownCamera;
	bool		m_bTopDown;
	vtEngine	*m_pNavEngines;

	vtGroup		*m_pRoot;		// top of the scene graph

	// globe
	vtTimeEngine	*m_pGlobeTime;
	vtGroup		*m_pGlobeContainer;
	vtIcoGlobe	*m_pIcoGlobe;
	vtIcoGlobe	*m_pOverlayGlobe;
	bool		m_bEarthShade;
	bool		m_bGlobeFlat;
	float		m_fFlattening, m_fFlattenDir;
	bool		m_bGlobeUnfolded;
	float		m_fFolding, m_fFoldDir;
	vtTrackball	*m_pTrackball;
	vtGeode		*m_pSpaceAxes;
	vtGeode		*m_pEarthLines;
	vtMesh		*m_pLineMesh;

	// flattened globe view
	FPQ			m_SpaceLoc;
	FPQ			m_FlatLoc;

	float			m_fMessageStart, m_fMessageTime;
	ControlEngine	*m_pControlEng;
	vtHUD		*m_pHUD;
	vtTextMesh	*m_pHUDMessage;
	vtFontPtr	 m_pArial;

	int			m_iInitStep;			// initialization stage
	vtTerrain	*m_pTargetTerrain;		// terrain we are switching to
	bool		m_bFlyIn;
	int			m_iFlightStage;		// 1, 2
	int			m_iFlightStep;		// 0-100
	FPoint3		m_TrackStart[3], m_TrackPosDiff;
	FPoint3		m_SpaceTrackballState[3];
	vtAnimPath	m_FlyInAnim;
	DPoint2		m_FlyInCenter;
	float		m_fTransitionHeight;	// in meters

	// HUD UI
	vtMaterialArrayPtr m_pHUDMaterials;

	vtGeode		*m_pLegendGeom;
	bool		m_bCreatedLegend;

	vtSpriteSizer	*m_pCompassSizer;
	vtGeode		*m_pCompassGeom;
	bool		m_bCreatedCompass;
	bool		m_bDragCompass;
	float		m_fDragAngle;

	vtMesh		*m_pWindowBoxMesh;

	// mapoverviewengine
	MapOverviewEngine *m_pMapOverview;
};

// global helper functions
vtTerrain *GetCurrentTerrain();

#endif	// ENVIROH

