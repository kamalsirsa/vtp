//
// Name: EnviroFrame.h
//
// Copyright (c) 2001-2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FRAMEH
#define FRAMEH

#include "wx/aui/aui.h"

#include "../EnviroEnum.h"
#include "vtlib/core/Event.h"

// forward declare dialogs
class BuildingDlg3d;
class CameraDlg;
class DistanceDlg3d;
class EphemDlg;
class FeatureTableDlg3d;
class InstanceDlg;
class LODDlg;
class LinearStructureDlg3d;
class LocationDlg;
class LayerDlg;
class PlantDlg;
class CScenarioSelectDialog;
class SceneGraphDlg;
class TagDlg;
class TimeDlg;
class UtilDlg;
class ProfileDlg;
class VehicleDlg;
class vtAbstractLayer;
class vtFeatureSet;
class vtTerrain;
class vtTimeEngine;

// some shortcuts
#define ADD_TOOL2(bar, id, bmp, label, type) \
	bar->AddTool(id, label, bmp, wxNullBitmap, type, label, label)

class EnviroFrame: public wxFrame
{
public:
	EnviroFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size,
		long style = wxDEFAULT_FRAME_STYLE, bool bVerticalToolbar = false, bool bEnableEarth = true);
	~EnviroFrame();

	void CreateMenus();
	void CreateToolbar();
	virtual void RefreshToolbar();

	virtual void SetMode(MouseMode mode);
	void ToggleNavigate();
	void ChangeFlightSpeed(float factor);
	void SetTerrainDetail(int iMetric);
	int GetTerrainDetail();
	void ChangePagingRange(float prange);
	void SetFullScreen(bool bFull);
	ProfileDlg *ShowProfileDlg();
	void OpenFenceDialog();
	void ShowPopupMenu(const IPoint2 &pos);
	void SetTimeEngine(vtTimeEngine *pEngine);
	void Snapshot(bool bNumbered);
	void CameraChanged();
	virtual void SetTerrainToGUI(vtTerrain *pTerrain);
	virtual void EarthPosUpdated(const DPoint3 &pos);
	virtual void UpdateStatus();
	virtual bool OnMouseEvent(vtMouseEvent &event) { return false; }
	virtual void Setup3DScene();
	virtual bool IsAcceptable(vtTerrain *pTerr) { return true; }
	virtual void OnCreateTerrain(vtTerrain *pTerr) {}
	virtual void FrameArgument(int i, const char *str) { }
	virtual void PostConstruction() { }
	virtual void AddTool(int id, const wxBitmap &bmp, const wxString &tooltip, bool tog);
	virtual void ExtendStructure(vtStructInstance *si) {}

	void UpdateLODInfo();
	void ShowTable(vtAbstractLayer *alay);
	void OnSetDelete(vtFeatureSet *set);
	void DeleteAllSelected();

	// command handlers
	void OnExit(wxCommandEvent& event);
	void OnClose(wxCloseEvent &event);
	void OnIdle(wxIdleEvent& event);
	void OnChar(wxKeyEvent& event);

#ifdef __WXMSW__
	// Catch special events, or calls an appropriate default window procedure
	virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif

protected:
	void OnFileLayers(wxCommandEvent& event);

	void OnLayerCreate(wxCommandEvent& event);
	void OnLayerLoad(wxCommandEvent& event);
	void OnSaveTerrain(wxCommandEvent& event);
	void OnSaveTerrainAs(wxCommandEvent& event);

	void OnUpdateLayerCreate(wxUpdateUIEvent& event);
	void OnUpdateLayerLoad(wxUpdateUIEvent& event);
	void OnUpdateSaveTerrain(wxUpdateUIEvent& event);

	void OnViewMaintain(wxCommandEvent& event);
	void OnViewWireframe(wxCommandEvent& event);
	void OnViewFullscreen(wxCommandEvent& event);
	void OnViewTopDown(wxCommandEvent& event);
	void OnViewFramerate(wxCommandEvent& event);
	void OnViewElevLegend(wxCommandEvent& event);
	void OnViewCompass(wxCommandEvent& event);
	void OnViewMapOverView(wxCommandEvent& event);
	void OnViewSettings(wxCommandEvent& event);
	void OnViewLocations(wxCommandEvent& event);
	void OnViewReset(wxCommandEvent& event);
	void OnViewSnapshot(wxCommandEvent& event);
	void OnViewSnapAgain(wxCommandEvent& event);
	void OnViewSnapHigh(wxCommandEvent& event);
	void OnViewStatusBar(wxCommandEvent& event);
	void OnViewScenarios(wxCommandEvent& event);
	void OnViewProfile(wxCommandEvent& event);
	void OnViewToolCulture(wxCommandEvent& event);
	void OnViewToolSnapshot(wxCommandEvent& event);
	void OnViewToolTime(wxCommandEvent& event);

	void OnUpdateViewMaintain(wxUpdateUIEvent& event);
	void OnUpdateViewWireframe(wxUpdateUIEvent& event);
	void OnUpdateViewFullscreen(wxUpdateUIEvent& event);
	void OnUpdateViewTopDown(wxUpdateUIEvent& event);
	void OnUpdateViewFramerate(wxUpdateUIEvent& event);
	void OnUpdateViewElevLegend(wxUpdateUIEvent& event);
	void OnUpdateViewCompass(wxUpdateUIEvent& event);
	void OnUpdateViewMapOverView(wxUpdateUIEvent& event);
	void OnUpdateViewLocations(wxUpdateUIEvent& event);
	void OnUpdateViewStatusBar(wxUpdateUIEvent& event);
	void OnUpdateViewProfile(wxUpdateUIEvent& event);
	void OnUpdateViewToolCulture(wxUpdateUIEvent& event);
	void OnUpdateViewToolSnapshot(wxUpdateUIEvent& event);
	void OnUpdateViewToolTime(wxUpdateUIEvent& event);

	void OnViewSlower(wxCommandEvent& event);
	void OnViewFaster(wxCommandEvent& event);
	void OnNavNormal(wxCommandEvent& event);
	void OnNavVelo(wxCommandEvent& event);
	void OnNavGrabPivot(wxCommandEvent& event);
	void OnNavPano(wxCommandEvent& event);

	void OnUpdateViewSlower(wxUpdateUIEvent& event);
	void OnUpdateViewFaster(wxUpdateUIEvent& event);
	void OnUpdateNavNormal(wxUpdateUIEvent& event);
	void OnUpdateNavVelo(wxUpdateUIEvent& event);
	void OnUpdateNavGrabPivot(wxUpdateUIEvent& event);
	void OnUpdateNavPano(wxUpdateUIEvent& event);

	void OnToolsSelect(wxCommandEvent& event);
	void OnUpdateToolsSelect(wxUpdateUIEvent& event);
	void OnToolsSelectBox(wxCommandEvent& event);
	void OnUpdateToolsSelectBox(wxUpdateUIEvent& event);
	void OnToolsSelectMove(wxCommandEvent& event);
	void OnUpdateToolsSelectMove(wxUpdateUIEvent& event);
	void OnToolsFences(wxCommandEvent& event);
	void OnUpdateToolsFences(wxUpdateUIEvent& event);
	void OnToolsBuildings(wxCommandEvent& event);
	void OnUpdateToolsBuildings(wxUpdateUIEvent& event);
	void OnToolsRoutes(wxCommandEvent& event);
	void OnUpdateToolsRoutes(wxUpdateUIEvent& event);
	void OnToolsPlants(wxCommandEvent& event);
	void OnUpdateToolsPlants(wxUpdateUIEvent& event);
	void OnToolsPoints(wxCommandEvent& event);
	void OnUpdateToolsPoints(wxUpdateUIEvent& event);
	void OnToolsInstances(wxCommandEvent& event);
	void OnUpdateToolsInstances(wxUpdateUIEvent& event);
	void OnToolsVehicles(wxCommandEvent& event);
	void OnUpdateToolsVehicles(wxUpdateUIEvent& event);
	void OnToolsMove(wxCommandEvent& event);
	void OnUpdateToolsMove(wxUpdateUIEvent& event);
	void OnToolsNavigate(wxCommandEvent& event);
	void OnUpdateToolsNavigate(wxUpdateUIEvent& event);
	void OnToolsMeasure(wxCommandEvent& event);
	void OnUpdateToolsMeasure(wxUpdateUIEvent& event);

	void OnSceneGraph(wxCommandEvent& event);
	void OnSceneTerrain(wxCommandEvent& event);
	void OnUpdateSceneTerrain(wxUpdateUIEvent& event);
	void OnSceneSpace(wxCommandEvent& event);
	void OnUpdateSceneSpace(wxUpdateUIEvent& event);
	void OnSceneSave(wxCommandEvent& event);
	void OnSceneEphemeris(wxCommandEvent& event);
	void OnUpdateSceneEphemeris(wxUpdateUIEvent& event);

	void OnTimeDialog(wxCommandEvent& event);
	void OnTimeStop(wxCommandEvent& event);
	void OnTimeFaster(wxCommandEvent& event);

	void OnDynamic(wxCommandEvent& event);
	void OnCullEvery(wxCommandEvent& event);
	void OnCullOnce(wxCommandEvent& event);
	void OnSky(wxCommandEvent& event);
	void OnHorizon(wxCommandEvent& event);
	void OnOcean(wxCommandEvent& event);
	void OnPlants(wxCommandEvent& event);
	void OnStructures(wxCommandEvent& event);
	void OnRoads(wxCommandEvent& event);
	void OnFog(wxCommandEvent& event);
	void OnIncrease(wxCommandEvent& event);
	void OnDecrease(wxCommandEvent& event);
	void OnLOD(wxCommandEvent& event);
	void OnToggleFoundations(wxCommandEvent& event);
	void OnTerrainReshade(wxCommandEvent& event);
	void OnTerrainChangeTexture(wxCommandEvent& event);
	void OnTerrainDistribVehicles(wxCommandEvent& event);
	void OnTerrainWriteElevation(wxCommandEvent& event);

	void OnUpdateDynamic(wxUpdateUIEvent& event);
	void OnUpdateCullEvery(wxUpdateUIEvent& event);
	void OnUpdateSky(wxUpdateUIEvent& event);
	void OnUpdateHorizon(wxUpdateUIEvent& event);
	void OnUpdateOcean(wxUpdateUIEvent& event);
	void OnUpdatePlants(wxUpdateUIEvent& event);
	void OnUpdateStructures(wxUpdateUIEvent& event);
	void OnUpdateRoads(wxUpdateUIEvent& event);
	void OnUpdateFog(wxUpdateUIEvent& event);
	void OnUpdateLOD(wxUpdateUIEvent& event);
	void OnUpdateSaveVeg(wxUpdateUIEvent& event);
	void OnUpdateSaveStruct(wxUpdateUIEvent& event);
	void OnUpdateFoundations(wxUpdateUIEvent& event);
	void OnUpdateIsTerrainView(wxUpdateUIEvent& event);
	void OnUpdateIsDynTerrain(wxUpdateUIEvent& event);

	void OnEarthShowShading(wxCommandEvent& event);
	void OnEarthShowAxes(wxCommandEvent& event);
	void OnEarthTilt(wxCommandEvent& event);
	void OnEarthFlatten(wxCommandEvent& event);
	void OnEarthUnfold(wxCommandEvent& event);
	void OnEarthClouds(wxCommandEvent& event);
	void OnEarthClouds2(wxCommandEvent& event);

	void OnUpdateEarthShowShading(wxUpdateUIEvent& event);
	void OnUpdateEarthShowAxes(wxUpdateUIEvent& event);
	void OnUpdateEarthTilt(wxUpdateUIEvent& event);
	void OnUpdateEarthFlatten(wxUpdateUIEvent& event);
	void OnUpdateEarthUnfold(wxUpdateUIEvent& event);
	void OnUpdateInOrbit(wxUpdateUIEvent& event);
	void OnUpdateInOrbitOrTerrain(wxUpdateUIEvent& event);
	void OnUpdateEarthClouds(wxUpdateUIEvent& event);

	virtual void OnHelpAbout(wxCommandEvent& event);
	virtual void OnHelpDocLocal(wxCommandEvent& event);
	virtual void OnHelpDocOnline(wxCommandEvent& event);

	void OnPopupProperties(wxCommandEvent& event);
	void OnPopupFlip(wxCommandEvent& event);
	void OnPopupReload(wxCommandEvent& event);
	void OnPopupShadow(wxCommandEvent& event);
	void OnPopupAdjust(wxCommandEvent& event);
	void OnPopupStart(wxCommandEvent& event);
	void OnPopupDelete(wxCommandEvent& event);
	void OnPopupURL(wxCommandEvent& event);

	void DoTestCode();
	void LoadClouds(const char *fname);
	void CarveTerrainToFitNode(vtNode *node);

public:
	class vtGLCanvas	*m_canvas;
	wxToolBar			*m_pToolbar;
	wxMenuBar			*m_pMenuBar;
	class MyStatusBar	*m_pStatusBar;

	wxMenu				*m_pFileMenu;
	wxMenu				*m_pToolsMenu;
	wxMenu				*m_pViewMenu;
	wxMenu				*m_pNavMenu;
	wxMenu				*m_pEarthMenu;
	wxMenu				*m_pTerrainMenu;
	wxMenu				*m_pSceneMenu;

	// Modeless dialogs
	SceneGraphDlg		*m_pSceneGraphDlg;
	PlantDlg			*m_pPlantDlg;
	LinearStructureDlg3d *m_pFenceDlg;
	UtilDlg				*m_pUtilDlg;
	CameraDlg			*m_pCameraDlg;
	LocationDlg			*m_pLocationDlg;
	BuildingDlg3d		*m_pBuildingDlg;
	TagDlg				*m_pTagDlg;
	LayerDlg			*m_pLayerDlg;
	InstanceDlg			*m_pInstanceDlg;
	DistanceDlg3d		*m_pDistanceDlg;
	EphemDlg			*m_pEphemDlg;
	TimeDlg				*m_pTimeDlg;
	CScenarioSelectDialog *m_pScenarioSelectDialog;
	LODDlg				*m_pLODDlg;
	ProfileDlg			*m_pProfileDlg;
	VehicleDlg			*m_pVehicleDlg;
	MouseMode			m_ToggledMode;

	// There can be any number of feature dialogs, one for each abstract layer
	std::vector<FeatureTableDlg3d*> m_FeatureDlgs;

protected:
	wxAuiManager m_mgr;

	bool m_bAlwaysMove;
	bool m_bCulleveryframe;
	bool m_bFullscreen;
	bool m_bTopDown;
	bool m_bEnableEarth;
	bool m_bEarthLines;
	bool m_bVerticalToolbar;

	// snapshot members
	wxString m_strSnapshotFilename;
	int		m_iSnapshotNumber;
	int		m_iFormat;

	bool m_bCloseOnIdle;

DECLARE_EVENT_TABLE()
};

// Helper
EnviroFrame *GetFrame();

#endif

