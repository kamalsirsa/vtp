//
// Name: frame.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FRAMEH
#define FRAMEH

#include "../EnviroEnum.h"
#include "vtui/wxString2.h"

class SceneGraphDlg;
class PlantDlg;
class LinearStructureDlg3d;
class CameraDlg;
class LocationDlg;
class BuildingDlg3d;
class UtilDlg;
class LayerDlg;
class InstanceDlg;
class DistanceDlg;
class vtTerrain;

// some shortcuts
#define ADD_TOOL(id, bmp, tooltip, tog)	 \
	m_pToolbar->AddTool(id, bmp, wxNullBitmap, tog, -1, -1, (wxObject *)0, tooltip, tooltip)

class vtFrame: public wxFrame
{
public:
	vtFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size,
		long style = wxDEFAULT_FRAME_STYLE);
	~vtFrame();

	void CreateMenus();
	void CreateToolbar();

	void SetMode(MouseMode mode);
	void ToggleNavigate();
	void ChangeFlightSpeed(float factor);
	void ChangeTerrainDetail(bool bIncrease);
	void SetFullScreen(bool bFull);
	void ShowPopupMenu(const IPoint2 &pos);
	void SetTerrainToGUI(vtTerrain *pTerrain);
	void Snapshot(bool bNumbered);
	virtual void EarthPosUpdated(const DPoint3 &pos);
	virtual void UpdateStatus();
	virtual bool OnTerrainSelect() { return false; }

	// command handlers
	void OnExit(wxCommandEvent& event);
	void OnClose(wxCloseEvent &event);

	void OnFileLayers(wxCommandEvent& event);

	void OnViewMaintain(wxCommandEvent& event);
	void OnViewWireframe(wxCommandEvent& event);
	void OnViewFullscreen(wxCommandEvent& event);
	void OnViewTopDown(wxCommandEvent& event);
	void OnViewFramerate(wxCommandEvent& event);
	void OnViewSettings(wxCommandEvent& event);
	void OnViewFollowRoute(wxCommandEvent& event);
	void OnViewLocations(wxCommandEvent& event);
	void OnViewSnapshot(wxCommandEvent& event);
	void OnViewSnapAgain(wxCommandEvent& event);

	void OnUpdateViewMaintain(wxUpdateUIEvent& event);
	void OnUpdateViewWireframe(wxUpdateUIEvent& event);
	void OnUpdateViewFullscreen(wxUpdateUIEvent& event);
	void OnUpdateViewTopDown(wxUpdateUIEvent& event);
	void OnUpdateViewFramerate(wxUpdateUIEvent& event);
	void OnUpdateViewFollowRoute(wxUpdateUIEvent& event);
	void OnUpdateViewLocations(wxUpdateUIEvent& event);

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
	void OnToolsFences(wxCommandEvent& event);
	void OnUpdateToolsFences(wxUpdateUIEvent& event);
	void OnToolsRoutes(wxCommandEvent& event);
	void OnUpdateToolsRoutes(wxUpdateUIEvent& event);
	void OnToolsTrees(wxCommandEvent& event);
	void OnUpdateToolsTrees(wxUpdateUIEvent& event);
	void OnToolsInstances(wxCommandEvent& event);
	void OnUpdateToolsInstances(wxUpdateUIEvent& event);
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

	void OnTimeStop(wxCommandEvent& event);
	void OnTimeFaster(wxCommandEvent& event);

	void OnDynamic(wxCommandEvent& event);
	void OnCullEvery(wxCommandEvent& event);
	void OnCullOnce(wxCommandEvent& event);
	void OnSky(wxCommandEvent& event);
	void OnOcean(wxCommandEvent& event);
	void OnTrees(wxCommandEvent& event);
	void OnStructures(wxCommandEvent& event);
	void OnRoads(wxCommandEvent& event);
	void OnFog(wxCommandEvent& event);
	void OnIncrease(wxCommandEvent& event);
	void OnDecrease(wxCommandEvent& event);
	void OnSaveVeg(wxCommandEvent& event);
	void OnSaveStruct(wxCommandEvent& event);
	void OnToggleFoundations(wxCommandEvent& event);

	void OnUpdateDynamic(wxUpdateUIEvent& event);
	void OnUpdateCullEvery(wxUpdateUIEvent& event);
	void OnUpdateSky(wxUpdateUIEvent& event);
	void OnUpdateOcean(wxUpdateUIEvent& event);
	void OnUpdateTrees(wxUpdateUIEvent& event);
	void OnUpdateStructures(wxUpdateUIEvent& event);
	void OnUpdateRoads(wxUpdateUIEvent& event);
	void OnUpdateFog(wxUpdateUIEvent& event);
	void OnUpdateSaveVeg(wxUpdateUIEvent& event);
	void OnUpdateSaveStruct(wxUpdateUIEvent& event);
	void OnUpdateFoundations(wxUpdateUIEvent& event);

	void OnEarthShowShading(wxCommandEvent& event);
	void OnEarthShowAxes(wxCommandEvent& event);
	void OnEarthTilt(wxCommandEvent& event);
	void OnEarthFlatten(wxCommandEvent& event);
	void OnEarthUnfold(wxCommandEvent& event);
	void OnEarthPoints(wxCommandEvent& event);

	void OnUpdateEarthShowShading(wxUpdateUIEvent& event);
	void OnUpdateEarthShowAxes(wxUpdateUIEvent& event);
	void OnUpdateEarthTilt(wxUpdateUIEvent& event);
	void OnUpdateInOrbit(wxUpdateUIEvent& event);

	void OnHelpAbout(wxCommandEvent& event);

	void OnPopupProperties(wxCommandEvent& event);
	void OnPopupFlip(wxCommandEvent& event);
	void OnPopupReload(wxCommandEvent& event);
	void OnPopupDelete(wxCommandEvent& event);

	void OnChar(wxKeyEvent& event);

public:
	class vtGLCanvas	*m_canvas;
	wxToolBar			*m_pToolbar;
	wxMenuBar			*m_pMenuBar;

	// Modeless dialogs
	SceneGraphDlg		*m_pSceneGraphDlg;
	PlantDlg			*m_pPlantDlg;
	LinearStructureDlg3d *m_pFenceDlg;
	UtilDlg				*m_pUtilDlg;
	CameraDlg			*m_pCameraDlg;
	LocationDlg			*m_pLocationDlg;
	BuildingDlg3d		*m_pBuildingDlg;
	LayerDlg			*m_pLayerDlg;
	InstanceDlg			*m_pInstanceDlg;
	DistanceDlg			*m_pDistanceDlg;
	MouseMode			m_ToggledMode;

protected:
	bool m_bAlwaysMove;
	bool m_bCulleveryframe;
	bool m_bFullscreen;
	bool m_bTopDown;

	// snapshot members
	wxString2 m_strSnapshotFilename;
	int		m_iSnapshotNumber;
	int		m_iFormat;

DECLARE_EVENT_TABLE()
};

#endif

