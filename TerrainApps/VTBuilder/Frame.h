//
// Frame.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTBUILDERFRAMEH
#define VTBUILDERFRAMEH

#include "wx/dnd.h"

#include "vtdata/Projections.h"
#include "vtdata/Plants.h"	// for vtSpeciesList
#include "vtdata/Fence.h"	// for LinStructOptions

#include "StatusBar.h"
#include "Layer.h"

#ifdef ENVIRON
  #define APPNAME "EnvironBuilder"
#else
  #define APPNAME "VTBuilder"
#endif

// some shortcuts
#define ADD_TOOL(id, bmp, tooltip, tog) \
	toolBar_main->AddTool(id, bmp, wxNullBitmap, tog, -1, -1, (wxObject *)0, tooltip, tooltip)
#define INSERT_TOOL(place, id, bmp, tooltip, tog) \
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
class vtFeatureSet;
class VegGenOptions;

// dialogs
class SpeciesListDlg;
class BioRegionDlg;
class FeatInfoDlg;
class DistanceDlg;
class LinearStructureDlg;
class LinearStructureDlg2d;
class InstanceDlg;
class RenderDlg;

class MainFrame: public wxFrame
{
public:
	MainFrame(wxFrame *frame, const wxString& title,
		const wxPoint& pos, const wxSize& size);
	virtual ~MainFrame();

	// view
	virtual void CreateView();
	void ZoomAll();

	void ReadEnviroPaths(vtStringArray &paths);
	void SetupUI();
	void CheckForGDALAndWarn();
	virtual void CreateMenus();
	void CreateToolbar();
	void RefreshToolbar();
	virtual void AddMainToolbars();
	bool DrawDisabled() { return m_bDrawDisabled; }

protected:
	void OnClose(wxCloseEvent &event);
	void DeleteContents();

	// Menu commands
	void OnProjectNew(wxCommandEvent& event);
	void OnProjectOpen(wxCommandEvent& event);
	void OnProjectSave(wxCommandEvent& event);
	void OnDymaxTexture(wxCommandEvent &event);
	void OnProcessBillboard(wxCommandEvent &event);
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
	void OnLayerImportMapSource(wxCommandEvent& event);
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
	void OnLayerUp(wxCommandEvent& event);
	void OnViewMagnifier(wxCommandEvent& event);
	void OnViewPan(wxCommandEvent& event);
	void OnViewDistance(wxCommandEvent& event);
	void OnViewSetArea(wxCommandEvent& event);
public:
	void OnViewZoomIn(wxCommandEvent& event);
	void OnViewZoomOut(wxCommandEvent& event);
protected:
	void OnViewZoomAll(wxCommandEvent& event);
	void OnViewZoomToLayer(wxCommandEvent& event);
	void OnViewFull(wxCommandEvent& event);
	void OnViewWorldMap(wxUpdateUIEvent& event);
	void OnViewUTMBounds(wxUpdateUIEvent& event);
	void OnViewOptions(wxUpdateUIEvent& event);

	void OnUpdateLayerShow(wxUpdateUIEvent& event);
	void OnUpdateLayerUp(wxUpdateUIEvent& event);
	void OnUpdateMagnifier(wxUpdateUIEvent& event);
	void OnUpdatePan(wxUpdateUIEvent& event);
	void OnUpdateDistance(wxUpdateUIEvent& event);
	void OnUpdateViewFull(wxUpdateUIEvent& event);
	void OnUpdateViewZoomToLayer(wxUpdateUIEvent& event);
	void OnUpdateCrossingSelection(wxUpdateUIEvent& event);
	void OnUpdateWorldMap(wxUpdateUIEvent& event);
	void OnUpdateUTMBounds(wxUpdateUIEvent& event);

	void OnSelectLink(wxCommandEvent& event);
	void OnSelectNode(wxCommandEvent& event);
	void OnSelectWhole(wxCommandEvent& event);
	void OnDirection(wxCommandEvent& event);
	void OnRoadEdit(wxCommandEvent& event);
	void OnRoadShowNodes(wxCommandEvent& event);
	void OnSelectHwy(wxCommandEvent& event);
	void OnRoadClean(wxCommandEvent& event);
	void OnRoadGuess(wxCommandEvent& event);
	void OnRoadFlatten(wxCommandEvent& event);

	void OnUpdateSelectLink(wxUpdateUIEvent& event);
	void OnUpdateSelectNode(wxUpdateUIEvent& event);
	void OnUpdateSelectWhole(wxUpdateUIEvent& event);
	void OnUpdateDirection(wxUpdateUIEvent& event);
	void OnUpdateRoadEdit(wxUpdateUIEvent& event);
	void OnUpdateRoadShowNodes(wxUpdateUIEvent& event);
	void OnUpdateRoadFlatten(wxUpdateUIEvent& event);

	void OnElevSelect(wxCommandEvent& event);
	void OnRemoveElevRange(wxCommandEvent& event);
	void OnElevSetUnknown(wxCommandEvent& event);
	void OnFillIn(wxCommandEvent& event);
	void OnScaleElevation(wxCommandEvent& event);
	void OnElevExport(wxCommandEvent& event);
	void OnElevExportBitmap(wxCommandEvent& event);
	void OnElevMergeTin(wxCommandEvent& event);

	void OnUpdateElevSelect(wxUpdateUIEvent& event);
	void OnUpdateViewSetArea(wxUpdateUIEvent& event);
	void OnUpdateElevSetUnknown(wxUpdateUIEvent& event);
	void OnUpdateScaleElevation(wxUpdateUIEvent& event);
	void OnUpdateElevMergeTin(wxUpdateUIEvent& event);
	void OnUpdateIsGrid(wxUpdateUIEvent& event);

	void OnTowerSelect(wxCommandEvent& event);
	void OnTowerEdit(wxCommandEvent& event);
	void OnTowerAdd(wxCommandEvent& event);

	void OnUpdateTowerSelect(wxUpdateUIEvent& event);
	void OnUpdateTowerEdit(wxUpdateUIEvent& event);
	void OnUpdateTowerAdd(wxUpdateUIEvent& event);

	void OnVegPlants(wxCommandEvent& event);
	void OnVegBioregions(wxCommandEvent& event);
	void OnVegExportSHP(wxCommandEvent& event);
	void OnUpdateVegExportSHP(wxUpdateUIEvent& event);

	void OnFeatureSelect(wxCommandEvent& event);
	void OnFeaturePick(wxCommandEvent& event);
	void OnFeatureTable(wxCommandEvent& event);
	void OnBuildingEdit(wxCommandEvent& event);
	void OnBuildingAddPoints(wxCommandEvent& event);
	void OnBuildingDeletePoints(wxCommandEvent& event);
	void OnStructureAddLinear(wxCommandEvent& event);
	void OnStructureEditLinear(wxCommandEvent& event);
	void OnStructureAddInstances(wxCommandEvent& event);
	void OnStructureAddFoundation(wxCommandEvent& event);
	void OnStructureConstrain(wxCommandEvent& event);
	void OnUpdateFeatureSelect(wxUpdateUIEvent& event);
	void OnUpdateFeaturePick(wxUpdateUIEvent& event);
	void OnUpdateFeatureTable(wxUpdateUIEvent& event);
	void OnUpdateBuildingEdit(wxUpdateUIEvent& event);
	void OnUpdateBuildingAddPoints(wxUpdateUIEvent& event);
	void OnUpdateBuildingDeletePoints(wxUpdateUIEvent& event);
	void OnUpdateStructureAddLinear(wxUpdateUIEvent& event);
	void OnUpdateStructureAddInstances(wxUpdateUIEvent& event);
	void OnUpdateStructureEditLinear(wxUpdateUIEvent& event);
	void OnUpdateStructureAddFoundation(wxUpdateUIEvent& event);
	void OnUpdateStructureConstrain(wxUpdateUIEvent& event);

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
	void OnAreaExportImage(wxCommandEvent& event);
	void OnAreaGenerateVeg(wxCommandEvent& event);
	void OnAreaRequestLayer(wxCommandEvent& event);
	void OnAreaRequestWMS(wxCommandEvent& event);
	void OnAreaRequestTServe(wxCommandEvent& event);

	void OnUpdateAreaStretch(wxUpdateUIEvent& event);
	void OnUpdateAreaExportElev(wxUpdateUIEvent& event);
	void OnUpdateAreaExportImage(wxUpdateUIEvent& event);
	void OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event);

	void OnHelpAbout(wxCommandEvent& event);

	// keys (used for shortcuts)
	void OnChar(wxKeyEvent& event);

public:
	// project
	void LoadProject(const wxString2 &strPathName);
	void SaveProject(const wxString2 &strPathName);

	// Layer methods
	int NumLayers() const { return m_Layers.GetSize(); }
	vtLayer *GetLayer(int i) const { return m_Layers[i]; }
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
	vtImageLayer *GetActiveImageLayer()
	{
		if (m_pActiveLayer && m_pActiveLayer->GetType() == LT_IMAGE)
			return (vtImageLayer *)m_pActiveLayer;
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
		if (m_pActiveLayer && m_pActiveLayer->GetType() == LT_UTILITY)
			return (vtUtilityLayer *)m_pActiveLayer;
		return NULL;
	}
	vtVegLayer *GetActiveVegLayer()
	{
		if (m_pActiveLayer && m_pActiveLayer->GetType() == LT_VEG)
			return (vtVegLayer *)m_pActiveLayer;
		return NULL;
	}
	int LayersOfType(LayerType lt);
	vtLayer *FindLayerOfType(LayerType lt);
	int NumModifiedLayers();
	DRECT GetExtents();
	LayerType AskLayerType();
	int LayerNum(vtLayer *lp);
	void SwapLayerOrder(int n0, int n1);

	vtFeatureSet *GetActiveFeatureSet();

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
	bool SampleCurrentTerrains(vtElevLayer *pTarget);
	float GetHeightFromTerrain(DPoint2 &p);
	void ExportBitmap(RenderDlg &dlg);

	// Images
	bool SampleCurrentImages(vtImageLayer *pTarget);

	// Structures
	LinearStructureDlg *ShowLinearStructureDlg(bool bShow = true);
	LinearStructureDlg2d *m_pLinearStructureDlg;
	LinStructOptions m_LSOptions;
	InstanceDlg *ShowInstanceDlg(bool bShow);
	InstanceDlg *m_pInstanceDlg;

	// Content items (can be referenced as structures)
	void LookForContentFiles();
	void FreeContentFiles();
	void ResolveInstanceItem(vtStructInstance *inst);
	std::vector<vtContentManager*> m_contents;

	// Vegetation
	vtString m_strSpeciesFilename;
	vtString m_strBiotypesFilename;
	vtSpeciesList m_PlantList;
	vtSpeciesList *GetPlantList() { return &m_PlantList; }
	SpeciesListDlg *m_SpeciesListDlg;
	bool LoadSpeciesFile(const char *fname);
	bool LoadBiotypesFile(const char *fname);

	vtBioRegion m_BioRegions;
	vtBioRegion *GetBioRegion() { return &m_BioRegions; }
	BioRegionDlg *m_BioRegionDlg;

	void GenerateVegetation(const char *vf_file, DRECT area, VegGenOptions &opt);
	void GenerateVegetationPhase2(const char *vf_file, DRECT area, VegGenOptions &opt);

	// import
	void ImportData(LayerType ltype);
	void ImportDataFromArchive(LayerType ltype, const wxString2 &fname_org, bool bRefresh);
	vtLayer *ImportDataFromFile(LayerType ltype, const wxString2 &strFileName, bool bRefresh, bool bWarn = true);
	vtLayer *ImportFromDLG(const wxString2 &strFileName, LayerType ltype);
	vtLayer *ImportFromSHP(const wxString2 &strFileName, LayerType ltype);
	vtLayer *ImportElevation(const wxString2 &strFileName, bool bWarn = true);
	vtLayer *ImportImage(const wxString2 &strFileName);
	vtLayer *ImportFromLULC(const wxString2 &strFileName, LayerType ltype);
	vtLayer *ImportRawFromOGR(const wxString2 &strFileName);
	vtLayer *ImportVectorsWithOGR(const wxString2 &strFileName, LayerType ltype);
	vtStructureLayer *ImportFromBCF(const wxString2 &strFileName);
	void ImportDataFromTIGER(const wxString2 &strDirName);
	void ImportDataFromS57(const wxString2 &strDirName);
	LayerType GuessLayerTypeFromDLG(vtDLGFile *pDLG);

	// Export
	void ExportASC();
	void ExportGeoTIFF();
	void ExportTerragen();
	void ExportBMP();
	void ExportSTM();
	void ExportPlanet();

	// area tool
	void StretchArea();
	void ExportElevation();
	void ExportImage();

	// Application Data
	DRECT		m_area;
	vtStringArray	m_datapaths;

protected:
	// INI File
	bool ReadINI();
	bool WriteINI();
	FILE *m_fpIni;
	const char *m_szIniFilename;

	// Application Data
	LayerArray	m_Layers;
	vtLayerPtr	m_pActiveLayer;

	// UI members
	wxMenu *fileMenu;
	wxMenu *editMenu;
	wxMenu *layerMenu;
	wxMenu *viewMenu;
	wxMenu *elevMenu;
	wxMenu *vegMenu;
	wxMenu *bldMenu;
	wxMenu *roadMenu;
	wxMenu *utilityMenu;
	wxMenu *rawMenu;
	wxMenu *areaMenu;
	wxMenu *helpMenu;
	wxMenuBar *m_pMenuBar;
	wxToolBar *toolBar_main;
	wxToolBar *toolBar_road;
	MyStatusBar *m_statbar;
	int		m_iMainButtons;

	MySplitterWindow *m_splitter;
	MyTreeCtrl	*m_pTree;		// left child of splitter
	BuilderView	*m_pView;		// right child of splitter
	bool	m_bDrawDisabled;

	vtProjection	m_proj;
	bool	m_bAdoptFirstCRS;	// If true, silenty assume user wants to
		// use the CRS of the first layer they create

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

#endif	// VTBUILDERFRAMEH

