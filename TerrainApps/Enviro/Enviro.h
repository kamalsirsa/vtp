
#ifndef ENVIROH
#define ENVIROH

#include "vtlib/core/Engine.h"
#include "vtlib/core/Fence3d.h"
#include "EnviroEnum.h"

// Use forward declarations to minimize rebuild dependencies
class vtTerrain;
class TerrainPicker;
class IcoGlobe;
class vtRoute;

// Engines
class vtTerrainFlyer;
class RouteFollowerEngine;
class VFlyer;
class QuakeFlyer;
class vtTrackball;
class BenchEngine;
class GlobePicker;

// Plants
class vtPlantList3d;

// jump sprite: location in window
#define UPPER_LEFT_X		0.0f
#define UPPER_LEFT_Y		0.87f
#define LOWER_RIGHT_X		0.075f
#define LOWER_RIGHT_Y		1.0f

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
	void LoadTerrainDescriptions();
	void StartControlEngine(const char *filename);
	void DoControl();
	void SetFlightSpeed(float speed);
	float GetFlightSpeed();
	void SetTopDown(bool bTopDown);
	void SetTerrain(vtTerrain *pTerrain);
	void EnableFlyerEngine(bool bEnable);
	void SetCurrentNavigator(vtTerrainFlyer *pE);
	const char *GetStatusText();
	void SetMode(MouseMode mode);

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
	int AddGlobePoints(const char *fname);
	void SetDisplayedArc(const DPoint2 &g1, const DPoint2 &g2);

	vtString GetMessage() { return m_strMessage; }
	void SetMessage(const char *msg, float time = 0.0f);
	void FormatCoordString(vtString &str, const DPoint3 &coord, bool bUTM);
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

	RouteFollowerEngine	*m_pRouteFollower;
	bool			m_bRouteTranslationSet;
	vtCamera		*m_pRouteFollowerCamera;

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
	void SetRouteOptions(float fHeight, float fSpacing);
	void SetFollowerCamera();

	// plants
	vtPlantList3d	*GetPlantList() { return m_pPlantList; }
	void PlantATree(const DPoint2 &epos);
	void SetPlantOptions(int iSpecies, float fSize, float fSpacing);

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
	DPoint3		m_EarthPosDown;
	DPoint3		m_EarthPosLast;
	vtRoute		*m_pCurRoute;

protected:
	// methods
	void SetupScene1();
	void SetupScene2();
	void DoPickers();
	void SetupCameras();
	void MakeGlobe();
	void SetupGlobe();
	void LookUpTerrainLocations();
	void AddTerrainRectangles();
	void SetupTerrain(vtTerrain *pTerr);

	// plants
	vtPlantList3d	*m_pPlantList;
	int			m_iSpecies;
	float		m_fPlantSize;
	float		m_fPlantSpacing;

	// fence members
	bool		m_bActiveFence, m_bFenceClosed;
	vtFence3d	*m_pCurFence;
	FenceType	m_CurFenceType;
	float		m_fFenceHeight, m_fFenceSpacing;

	// route members
	bool		m_bActiveRoute;
	float		m_fRouteOffL, m_fRouteOffR;
	float		m_fRouteStInc;
	vtString	m_sRouteName;

	// linear arc on Earth
	vtGeom *m_pArc;
	vtMesh *m_pArcMesh;
	double m_fArcLength; 

	// view and navigation
	vtCamera	*m_pNormalCamera;
	vtCamera	*m_pTopDownCamera;
	BenchEngine	*m_pBench;

	vtRoot		*m_pRoot;		// top of the scene graph
	vtMovGeom	*m_pGlobeMGeom;
	IcoGlobe	*m_pIcoGlobe;
	bool		m_bShowTime;
	bool		m_bGlobeFlat;
	float		m_fGlobeFac, m_fGlobeDir;
	vtTrackball	*m_pTrackball;

	vtSprite	*m_pMessageSprite;
	vtSprite	*m_pSprite2;
	float			m_fMessageStart, m_fMessageTime;
	ControlEngine	*m_pControlEng;

	bool		m_bSuppliedFilename;	// true if there was a command line
	int			m_iInitStep;			// initialization stage
	vtTerrain	*m_pTargetTerrain;		// terrain we are switching to

	// Error logging.
	vtString	m_msg;
	FILE		*m_log;
	void		_StartLog(const char *fname);
public:
	void		_Log(const char *str);
};

extern Enviro g_App;

#endif
