//
// Frame.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTBUILDERFRAMEH
#define VTBUILDERFRAMEH

#include "wx/dnd.h"

#include "vtdata/Projections.h"
#include "vtdata/Plants.h"	// for vtPlantList

#include "StatusBar.h"
#include "Layer.h"

// some shortcuts
#define ADD_TOOL(id, bmp, tooltip, tog)     \
    toolBar_main->AddTool(id, bmp, wxNullBitmap, tog, -1, -1, (wxObject *)0, tooltip, tooltip)
#define INSERT_TOOL(place, id, bmp, tooltip, tog)     \
    toolBar_main->InsertTool(place, id, bmp, wxNullBitmap, tog, (wxObject *)0, tooltip, tooltip)

class MyTreeCtrl;
class MySplitterWindow;
class MyStatusBar;
class vtDLGFile;
class vtVegLayer;
class vtRawLayer;
class vtElevLayer;
class vtImageLayer;
class vtRoadLayer;
class vtStructureLayer;
class vtUtilityLayer;
class BuilderView;

// dialogs
class PlantListDlg;
class BioRegionDlg;
class FeatInfoDlg;
class DistanceDlg;

class MainFrame: public wxFrame
{
public:
	MainFrame(wxFrame* frame, const wxString& title,
		const wxPoint& pos, const wxSize& size);
	virtual ~MainFrame();

	virtual void CreateView();
	void SetupUI();
	virtual void CreateMenus();
	void CreateToolbar();
	void RefreshToolbar();
	virtual void AddMainToolbars();

	void OnSize(wxSizeEvent& event);
	void DeleteContents();

	// Menu commands
	void OnProjectNew(wxCommandEvent& event);
	void OnProjectOpen(wxCommandEvent& event);
	void OnProjectSave(wxCommandEvent& event);
	void OnDymaxTexture(wxCommandEvent &event);
	void OnQuit(wxCommandEvent& event);

	void OnEditDelete(wxCommandEvent& event);
	void OnUpdateEditDelete(wxUpdateUIEvent& event);
	void OnEditDeselectAll(wxCommandEvent& event);
	void OnEditInvertSelection(wxCommandEvent& event);
	void OnEditCrossingSelection(wxCommandEvent& event);
	void OnEditOffset(wxCommandEvent& event);

	void OnUpdateEditOffset(wxUpdateUIEvent& event);

	void OnLayerNew(wxCommandEvent& event);
	void OnLayerOpen(wxCommandEvent& event);
	void OnLayerSave(wxCommandEvent& event);
	void OnLayerSaveAs(wxCommandEvent& event);
	void OnLayerImport(wxCommandEvent& event);
	void OnLayerImportTIGER(wxCommandEvent& event);
	void OnLayerImportUtil(wxCommandEvent& event);
	void OnLayerProperties(wxCommandEvent& event);
	void OnLayerConvert(wxCommandEvent& event);
	void OnLayerSetProjection(wxCommandEvent& event);
	void OnLayerFlatten(wxCommandEvent& event);

	void OnUpdateLayerSave(wxUpdateUIEvent& event);
	void OnUpdateLayerSaveAs(wxUpdateUIEvent& event);
	void OnUpdateLayerProperties(wxUpdateUIEvent& event);
	void OnUpdateLayerConvert(wxUpdateUIEvent& event);
	void OnUpdateLayerFlatten(wxUpdateUIEvent& event);

	void OnLayerShow(wxCommandEvent& event);
	void OnViewMagnifier(wxCommandEvent& event);
	void OnViewPan(wxCommandEvent& event);
	void OnViewDistance(wxCommandEvent& event);
	void OnViewSetArea(wxCommandEvent& event);
	void OnViewZoomIn(wxCommandEvent& event);
	void OnViewZoomOut(wxCommandEvent& event);
	void OnViewZoomAll(wxCommandEvent& event);
	void OnViewFull(wxCommandEvent& event);
	void OnViewWorldMap(wxUpdateUIEvent& event);
	void OnViewUTMBounds(wxUpdateUIEvent& event);
	void OnViewOptions(wxUpdateUIEvent& event);

	void OnUpdateLayerShow(wxUpdateUIEvent& event);
	void OnUpdateMagnifier(wxUpdateUIEvent& event);
	void OnUpdatePan(wxUpdateUIEvent& event);
	void OnUpdateDistance(wxUpdateUIEvent& event);
	void OnUpdateViewFull(wxUpdateUIEvent& event);
	void OnUpdateCrossingSelection(wxUpdateUIEvent& event);
	void OnUpdateWorldMap(wxUpdateUIEvent& event);
	void OnUpdateUTMBounds(wxUpdateUIEvent& event);

	void OnSelectLink(wxCommandEvent& event);
	void OnSelectNode(wxCommandEvent& event);
	void OnSelectWhole(wxCommandEvent& event);
	void OnDirection(wxCommandEvent& event);
	void OnRoadEdit(wxCommandEvent& event);
	void OnRoadShowNodes(wxCommandEvent& event);
	void OnRoadShowWidth(wxCommandEvent& event);
	void OnSelectHwy(wxCommandEvent& event);
	void OnRoadClean(wxCommandEvent& event);
	void OnRoadGuess(wxCommandEvent& event);

	void OnUpdateSelectLink(wxUpdateUIEvent& event);
	void OnUpdateSelectNode(wxUpdateUIEvent& event);
	void OnUpdateSelectWhole(wxUpdateUIEvent& event);
	void OnUpdateDirection(wxUpdateUIEvent& event);
	void OnUpdateRoadEdit(wxUpdateUIEvent& event);
	void OnUpdateRoadShowNodes(wxUpdateUIEvent& event);
	void OnUpdateRoadShowWidth(wxUpdateUIEvent& event);

	void OnElevSelect(wxCommandEvent& event);
	void OnRemoveAboveSea(wxCommandEvent& event);
	void OnFillIn(wxCommandEvent& event);
	void OnScaleElevation(wxCommandEvent& event);
	void OnExportTerragen(wxCommandEvent& event);
	void OnElevShow(wxCommandEvent& event);
	void OnElevShading(wxCommandEvent& event);
	void OnElevHide(wxCommandEvent& event);
	void OnElevExportBitmap(wxCommandEvent& event);
	void OnElevMergeTin(wxCommandEvent& event);

	void OnUpdateElevSelect(wxUpdateUIEvent& event);
	void OnUpdateViewSetArea(wxUpdateUIEvent& event);
	void OnUpdateRemoveAboveSea(wxUpdateUIEvent& event);
	void OnUpdateFillIn(wxUpdateUIEvent& event);
	void OnUpdateScaleElevation(wxUpdateUIEvent& event);
	void OnUpdateExportTerragen(wxUpdateUIEvent& event);
	void OnUpdateElevShow(wxUpdateUIEvent& event);
	void OnUpdateElevShading(wxUpdateUIEvent& event);
	void OnUpdateElevHide(wxUpdateUIEvent& event);
	void OnUpdateExportBitmap(wxUpdateUIEvent& event);
	void OnUpdateElevMergeTin(wxUpdateUIEvent& event);

	void OnTowerSelect(wxCommandEvent& event);
	void OnTowerEdit(wxCommandEvent& event);
	void OnTowerAdd(wxCommandEvent& event);

	void OnUpdateTowerSelect(wxUpdateUIEvent& event);
	void OnUpdateTowerEdit(wxUpdateUIEvent& event);
	void OnUpdateTowerAdd(wxUpdateUIEvent& event);

	void OnVegPlants(wxCommandEvent& event);
	void OnVegBioregions(wxCommandEvent& event);

	void OnFeatureSelect(wxCommandEvent& event);
	void OnFeaturePick(wxCommandEvent& event);
	void OnFeatureTable(wxCommandEvent& event);
	void OnBuildingEdit(wxCommandEvent& event);
	void OnStructureAddLinear(wxCommandEvent& event);
	void OnUpdateFeatureSelect(wxUpdateUIEvent& event);
	void OnUpdateFeaturePick(wxUpdateUIEvent& event);
	void OnUpdateFeatureTable(wxUpdateUIEvent& event);
	void OnUpdateBuildingEdit(wxUpdateUIEvent& event);
	void OnUpdateStructureAddLinear(wxUpdateUIEvent& event);

	void OnRawSetType(wxCommandEvent& event);
	void OnRawAddPoints(wxCommandEvent& event);
	void OnRawAddPointText(wxCommandEvent& event);
	void OnRawAddPointsGPS(wxCommandEvent& event);
	void OnRawSelectCondition(wxCommandEvent& event);

	void OnUpdateRawSetType(wxUpdateUIEvent& event);
	void OnUpdateRawAddPoints(wxUpdateUIEvent& event);
	void OnUpdateRawAddPointText(wxUpdateUIEvent& event);
	void OnUpdateRawAddPointsGPS(wxUpdateUIEvent& event);
	void OnUpdateRawSelectCondition(wxUpdateUIEvent& event);

	void OnAreaStretch(wxCommandEvent& event);
	void OnAreaTypeIn(wxCommandEvent& event);
	void OnAreaExportElev(wxCommandEvent& event);
	void OnAreaGenerateVeg(wxCommandEvent& event);
	void OnAreaRequestLayer(wxCommandEvent& event);

	void OnUpdateAreaStretch(wxUpdateUIEvent& event);
	void OnUpdateAreaExportElev(wxUpdateUIEvent& event);
	void OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event);

	void OnHelpAbout(wxCommandEvent& event);

	// keys (used for shortcuts)
	void OnChar(wxKeyEvent& event);

	// project
	void LoadProject(const wxString &strPathName);
	void SaveProject(const wxString &strPathName);

	// Layer methods
	void LoadLayer(const wxString &fname);
	void AddLayer(vtLayer *lp);
	bool AddLayerWithCheck(vtLayer *pLayer, bool bRefresh = true);
	void RemoveLayer(vtLayer *lp);
	void DeleteLayer(vtLayer *lp);
	void SetActiveLayer(vtLayer *lp, bool refresh = false);
	vtLayer *GetActiveLayer() { return m_pActiveLayer; }
	vtElevLayer *GetActiveElevLayer()
	{
		if (m_pActiveLayer && m_pActiveLayer->GetType() == LT_ELEVATION)
			return (vtElevLayer *)m_pActiveLayer;
		return NULL;
	}
	vtRoadLayer *GetActiveRoadLayer()
	{
		if (m_pActiveLayer && m_pActiveLayer->GetType() == LT_ROAD)
			return (vtRoadLayer *)m_pActiveLayer;
		return NULL;
	}
	vtRawLayer *GetActiveRawLayer()
	{
		if (m_pActiveLayer && m_pActiveLayer->GetType() == LT_RAW)
			return (vtRawLayer *)m_pActiveLayer;
		return NULL;
	}
	vtStructureLayer *GetActiveStructureLayer()
	{
		if (m_pActiveLayer && m_pActiveLayer->GetType() == LT_STRUCTURE)
			return (vtStructureLayer *)m_pActiveLayer;
		return NULL;
	}
	vtUtilityLayer *GetActiveUtilityLayer()
	{
		if(m_pActiveLayer &&m_pActiveLayer->GetType() == LT_UTILITY)
			return (vtUtilityLayer *)m_pActiveLayer;
		return NULL;
	}
	int LayersOfType(LayerType lt);
	vtLayer *FindLayerOfType(LayerType lt);
	DRECT GetExtents();
	LayerType AskLayerType();

	// UI
	void RefreshTreeStatus();
	void RefreshTreeView();
	void RefreshStatusBar();
	BuilderView *GetView() { return m_pView; }
	void OnSelectionChanged();

	// Projection
	void SetProjection(const vtProjection &p);
	void GetProjection(vtProjection &p) { p = m_proj; }
	vtProjection &GetAtProjection() { return m_proj; }

	// Raw features
	FeatInfoDlg	*ShowFeatInfoDlg();
	FeatInfoDlg	*m_pFeatInfoDlg;

	// Distance
	DistanceDlg	*ShowDistanceDlg();
	DistanceDlg *m_pDistanceDlg;

	// Elevation
	void SampleCurrentTerrains(vtElevLayer *pTarget);
	double GetHeightFromTerrain(DPoint2 &p);

	// Vegetation
	vtPlantList m_PlantList;
	vtPlantList *GetPlantList() { return &m_PlantList; }
	PlantListDlg *m_PlantListDlg;

	vtBioRegion m_BioRegions;
	vtBioRegion *GetBioRegion() { return &m_BioRegions; }
	BioRegionDlg *m_BioRegionDlg;

	void GenerateVegetation(const char *vf_file, DRECT area,
		vtVegLayer *pLandUse, vtVegLayer *pVegType,
		float fTreeSpacing, float fTreeScarcity);

	void FindVegLayers(vtVegLayer **Density, vtVegLayer **BioMap);

	// import
	void ImportData(LayerType ltype);
	void ImportDataFromFile(LayerType ltype, wxString strFileName, bool bRefresh);
	vtLayer *ImportFromDLG(wxString &strFileName, LayerType ltype);
	vtLayer *ImportFromSHP(wxString &strFileName, LayerType ltype);
	vtLayer *ImportElevation(wxString &strFileName);
	vtLayer *ImportImage(wxString &strFileName);
	vtLayer *ImportFromLULC(wxString &strFileName, LayerType ltype);
	vtLayer *ImportRawFromOGR(wxString &strFileName);
	vtLayer *ImportVectorsWithOGR(wxString &strFileName, LayerType ltype);
	vtStructureLayer *ImportFromBCF(wxString &strFileName);
	void ImportDataFromTIGER(wxString &strDirName);
	LayerType GuessLayerTypeFromDLG(vtDLGFile *pDLG);

	// export
	void StretchArea();
	void ExportElevation();

	// Application Data
	LayerArray	m_Layers;
	DRECT		m_area;

protected:
	// INI File
	bool ReadINI();
	bool WriteINI();
	FILE *m_fpIni;
	const char *m_szIniFilename;

	// Application Data
	vtLayerPtr	m_pActiveLayer;

	// UI members
	wxMenu*    fileMenu;
	wxMenu*    editMenu;
	wxMenu*    layerMenu;
	wxMenu*    viewMenu;
	wxMenu*    elevMenu;
	wxMenu*    vegMenu;
	wxMenu*    bldMenu;
	wxMenu*    roadMenu;
	wxMenu*	   utilityMenu;
	wxMenu*    rawMenu;
	wxMenu*    areaMenu;
	wxMenu*    helpMenu;
	wxMenuBar *m_pMenuBar;
	wxToolBar *toolBar_main;
	wxToolBar *toolBar_road;
	MyStatusBar *m_statbar;
	int		m_iMainButtons;

	MySplitterWindow *m_splitter;
	MyTreeCtrl	*m_pTree;		// left child of splitter
	BuilderView	*m_pView;		// right child of splitter

	vtProjection	m_proj;

	// menu numbers, for each layer type that has a corresponding menu
	int		m_iLayerMenu[LAYER_TYPES];

	DECLARE_EVENT_TABLE()
};

class DnDFile : public wxFileDropTarget
{
public:
    virtual bool OnDropFiles(wxCoord x, wxCoord y,
                             const wxArrayString& filenames);
};

extern MainFrame *GetMainFrame();
wxString GetImportFilterString(LayerType ltype);

#endif
