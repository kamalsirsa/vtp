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
class FenceDlg;
class CameraDlg;
class LocationDlg;
class BuildingDlg;

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

	void OnViewMaintain(wxCommandEvent& event);
	void OnUpdateViewMaintain(wxUpdateUIEvent& event);
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
	void OnViewLocations(wxCommandEvent& event);
	void OnUpdateViewLocations(wxUpdateUIEvent& event);

	void OnToolsSelect(wxCommandEvent& event);
	void OnUpdateToolsSelect(wxUpdateUIEvent& event);
	void OnToolsFences(wxCommandEvent& event);
	void OnUpdateToolsFences(wxUpdateUIEvent& event);
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
	void OnUpdateSaveVeg(wxUpdateUIEvent& event);
	void OnUpdateSaveStruct(wxUpdateUIEvent& event);

	void OnEarthShowTime(wxCommandEvent& event);
	void OnUpdateEarthShowTime(wxUpdateUIEvent& event);
	void OnEarthFlatten(wxCommandEvent& event);
	void OnUpdateEarthFlatten(wxUpdateUIEvent& event);
	void OnEarthPoints(wxCommandEvent& event);
	void OnUpdateEarthPoints(wxUpdateUIEvent& event);

	void OnHelpAbout(wxCommandEvent& event);

	void OnPopupProperties(wxCommandEvent& event);
	void OnPopupDelete(wxCommandEvent& event);

	void OnChar(wxKeyEvent& event);

public:
	class vtGLCanvas	*m_canvas;
	wxToolBar			*m_pToolbar;

	// Modeless dialogs
	SceneGraphDlg		*m_pSceneGraphDlg;
	PlantDlg			*m_pPlantDlg;
	FenceDlg			*m_pFenceDlg;
	CameraDlg			*m_pCameraDlg;
	LocationDlg			*m_pLocationDlg;
	BuildingDlg			*m_pBuildingDlg;

protected:
	bool m_bMaintainHeight;
	bool m_bCulleveryframe;
	bool m_bFullscreen;
	bool m_bTopDown;

DECLARE_EVENT_TABLE()
};

// Helper functions
void GetCwd( char path[], int path_size );
void SetCwd( const char path[] );

#endif

