//
// Name:		frame.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FRAMEH
#define FRAMEH

#include "../EnviroEnum.h"

class SceneGraphDlg;
class PlantDlg;
class LinearStructureDlg3d;
class CameraDlg;
class LocationDlg;
class BuildingDlg3d;
class UtilDlg;

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
	void ChangeFlightSpeed(float factor);
	void ChangeTerrainDetail(bool bIncrease);
	void SetFullScreen(bool bFull);
	void ShowPopupMenu(const IPoint2 &pos);

	// command handlers
	void OnExit(wxCommandEvent& event);
	void OnClose(wxCloseEvent &event);

	void OnViewMaintain(wxCommandEvent& event);
	void OnUpdateViewMaintain(wxUpdateUIEvent& event);
	void OnViewGrabPivot(wxCommandEvent& event);
	void OnUpdateViewGrabPivot(wxUpdateUIEvent& event);
	void OnViewWireframe(wxCommandEvent& event);
	void OnUpdateViewWireframe(wxUpdateUIEvent& event);
	void OnViewFullscreen(wxCommandEvent& event);
	void OnUpdateViewFullscreen(wxUpdateUIEvent& event);
	void OnViewTopDown(wxCommandEvent& event);
	void OnUpdateViewTopDown(wxUpdateUIEvent& event);
	void OnViewFramerate(wxCommandEvent& event);
	void OnUpdateViewFramerate(wxUpdateUIEvent& event);
	void OnViewSlower(wxCommandEvent& event);
	void OnUpdateViewSlower(wxUpdateUIEvent& event);
	void OnViewFaster(wxCommandEvent& event);
	void OnUpdateViewFaster(wxUpdateUIEvent& event);
	void OnViewSettings(wxCommandEvent& event);
	void OnViewFollowRoute(wxCommandEvent& event);
	void OnUpdateViewFollowRoute(wxUpdateUIEvent& event);
	void OnViewLocations(wxCommandEvent& event);
	void OnUpdateViewLocations(wxUpdateUIEvent& event);

	void OnToolsSelect(wxCommandEvent& event);
	void OnUpdateToolsSelect(wxUpdateUIEvent& event);
	void OnToolsFences(wxCommandEvent& event);
	void OnUpdateToolsFences(wxUpdateUIEvent& event);
	void OnToolsRoutes(wxCommandEvent& event);
	void OnUpdateToolsRoutes(wxUpdateUIEvent& event);
	void OnToolsTrees(wxCommandEvent& event);
	void OnUpdateToolsTrees(wxUpdateUIEvent& event);
	void OnToolsMove(wxCommandEvent& event);
	void OnUpdateToolsMove(wxUpdateUIEvent& event);
	void OnToolsNavigate(wxCommandEvent& event);
	void OnUpdateToolsNavigate(wxUpdateUIEvent& event);

	void OnSceneGraph(wxCommandEvent& event);
	void OnSceneTerrain(wxCommandEvent& event);
	void OnUpdateSceneTerrain(wxUpdateUIEvent& event);
	void OnSceneSpace(wxCommandEvent& event);
	void OnUpdateSceneSpace(wxUpdateUIEvent& event);
	void OnSceneSave(wxCommandEvent& event);

	void OnRegular(wxCommandEvent& event);
	void OnDynamic(wxCommandEvent& event);
	void OnCullEvery(wxCommandEvent& event);
	void OnCullOnce(wxCommandEvent& event);
	void OnSky(wxCommandEvent& event);
	void OnOcean(wxCommandEvent& event);
	void OnTrees(wxCommandEvent& event);
	void OnRoads(wxCommandEvent& event);
	void OnFog(wxCommandEvent& event);
	void OnIncrease(wxCommandEvent& event);
	void OnDecrease(wxCommandEvent& event);
	void OnSaveVeg(wxCommandEvent& event);
	void OnSaveStruct(wxCommandEvent& event);

	void OnUpdateRegular(wxUpdateUIEvent& event);
	void OnUpdateDynamic(wxUpdateUIEvent& event);
	void OnUpdateCullEvery(wxUpdateUIEvent& event);
	void OnUpdateSky(wxUpdateUIEvent& event);
	void OnUpdateOcean(wxUpdateUIEvent& event);
	void OnUpdateTrees(wxUpdateUIEvent& event);
	void OnUpdateRoads(wxUpdateUIEvent& event);
	void OnUpdateFog(wxUpdateUIEvent& event);
	void OnUpdateSaveVeg(wxUpdateUIEvent& event);
	void OnUpdateSaveStruct(wxUpdateUIEvent& event);

	void OnEarthShowTime(wxCommandEvent& event);
	void OnUpdateInOrbit(wxUpdateUIEvent& event);
	void OnEarthFlatten(wxCommandEvent& event);
	void OnEarthPoints(wxCommandEvent& event);
	void OnEarthLinear(wxCommandEvent& event);

	void OnHelpAbout(wxCommandEvent& event);

	void OnPopupProperties(wxCommandEvent& event);
	void OnPopupFlip(wxCommandEvent& event);
	void OnPopupDelete(wxCommandEvent& event);

	void OnChar(wxKeyEvent& event);

public:
	class vtGLCanvas	*m_canvas;
	wxToolBar			*m_pToolbar;

	// Modeless dialogs
	SceneGraphDlg		*m_pSceneGraphDlg;
	PlantDlg			*m_pPlantDlg;
	LinearStructureDlg3d *m_pFenceDlg;
	UtilDlg				*m_pUtilDlg;
	CameraDlg			*m_pCameraDlg;
	LocationDlg			*m_pLocationDlg;
	BuildingDlg3d		*m_pBuildingDlg;

protected:
	bool m_bMaintainHeight;
	bool m_bAlwaysMove;
	bool m_bCulleveryframe;
	bool m_bFullscreen;
	bool m_bTopDown;

DECLARE_EVENT_TABLE()
};

#endif

