//
// Frame.h
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTBUILDERFRAMEH
#define VTBUILDERFRAMEH

#include "wx/aui/aui.h"
#include "wx/dnd.h"

#include "vtdata/Projections.h"
#include "vtdata/Plants.h"		// for vtSpeciesList
#include "vtdata/Fence.h"		// for LinStructOptions
#include "vtdata/WFSClient.h"	// for OGCServerArray

#include "StatusBar.h"
#include "Layer.h"
#include "TilingOptions.h"

#ifdef ENVIRON
  #define APPNAME "EnvironBuilder"
#else
  #define APPNAME "VTBuilder"
#endif

// some shortcuts
#define ADD_TOOL(bar, id, bmp, label) \
	bar->AddTool(id, label, bmp, wxNullBitmap, wxITEM_NORMAL, label, label)
#define ADD_TOOL2(bar, id, bmp, label, type) \
	bar->AddTool(id, label, bmp, wxNullBitmap, type, label, label)

class MyTreeCtrl;
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
class vtElevationGrid;

// dialogs
class SpeciesListDlg;
class BioRegionDlg;
class FeatInfoDlg;
class DistanceDlg2d;
class LinearStructureDlg;
class LinearStructureDlg2d;
class InstanceDlg;
class MapServerDlg;
class RenderDlg;
class ProfileDlg;
class vtScaleBar;

// option tag names
#define TAG_SHOW_MAP "ShowMap"
#define TAG_SHOW_UTM "ShowUTMBounds"
#define TAG_SHOW_PATHS "ShowFullPaths"
#define TAG_ROAD_DRAW_WIDTH "RoadDrawWidth"

#define TAG_USE_CURRENT_CRS	"UseCurrentCRS"
#define TAG_LOAD_IMAGES_ALWAYS "LoadImagesAlways"
#define TAG_LOAD_IMAGES_NEVER "LoadImagesNever"
#define TAG_REPRO_TO_FLOAT_ALWAYS "ReproToFloatAlways"
#define TAG_REPRO_TO_FLOAT_NEVER "ReproToFloatNever"
#define TAG_SLOW_FILL_GAPS "SlowFillGaps"
#define TAG_SAMPLING_N "MultiSampleN"


class MainFrame: public wxFrame
{
public:
	MainFrame(wxFrame *frame, const wxString& title,
		const wxPoint& pos, const wxSize& size);
	virtual ~MainFrame();

	// view
	virtual void CreateView();
	void ZoomAll();

	void ReadEnviroPaths();
	void ReadDatapathsFromXML(ifstream &input, const char *path);
	void SetupUI();
	virtual void CreateMenus();
	void ManageToolbar(const wxString &name, wxToolBar *bar, bool show);
	wxToolBar *NewToolbar();
	void CreateToolbar();
	void RefreshToolbars();
	virtual void AddMainToolbars();
	bool DrawDisabled() { return m_bDrawDisabled; }

protected:
	void OnClose(wxCloseEvent &event);
	void DeleteContents();

	// Menu commands
	void OnProjectNew(wxCommandEvent& event);
	void OnProjectOpen(wxCommandEvent& event);
	void OnProjectSave(wxCommandEvent& event);
	void OnProjectPrefs(wxCommandEvent& event);
	void OnDymaxTexture(wxCommandEvent &event);
	void OnProcessBillboard(wxCommandEvent &event);
	void OnGeocode(wxCommandEvent &event);
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
	void OnLayerImportNTF(wxCommandEvent& event);
	void OnLayerImportUtil(wxCommandEvent& event);
	void OnLayerImportMapSource(wxCommandEvent& event);
	void OnLayerImportPoint(wxCommandEvent& event);
	void OnLayerImportXML(wxCommandEvent& event);
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
	void OnLayerDown(wxCommandEvent& event);
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
	void OnViewZoomArea(wxCommandEvent& event);
	void OnViewToolbar(wxCommandEvent& event);
	void OnViewLayers(wxCommandEvent& event);
	void OnViewWorldMap(wxCommandEvent& event);
	void OnViewUTMBounds(wxCommandEvent& event);
	void OnViewProfile(wxCommandEvent& event);
	void OnViewScaleBar(wxCommandEvent& event);
	void OnViewOptions(wxCommandEvent& event);

	void OnUpdateLayerShow(wxUpdateUIEvent& event);
	void OnUpdateLayerUp(wxUpdateUIEvent& event);
	void OnUpdateLayerDown(wxUpdateUIEvent& event);
	void OnUpdateMagnifier(wxUpdateUIEvent& event);
	void OnUpdatePan(wxUpdateUIEvent& event);
	void OnUpdateDistance(wxUpdateUIEvent& event);
	void OnUpdateViewFull(wxUpdateUIEvent& event);
	void OnUpdateViewZoomArea(wxUpdateUIEvent& event);
	void OnUpdateViewToolbar(wxUpdateUIEvent& event);
	void OnUpdateViewLayers(wxUpdateUIEvent& event);
	void OnUpdateViewZoomToLayer(wxUpdateUIEvent& event);
	void OnUpdateCrossingSelection(wxUpdateUIEvent& event);
	void OnUpdateWorldMap(wxUpdateUIEvent& event);
	void OnUpdateUTMBounds(wxUpdateUIEvent& event);
	void OnUpdateViewProfile(wxUpdateUIEvent& event);
	void OnUpdateViewScaleBar(wxUpdateUIEvent& event);

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
	void OnElevExportTiles(wxCommandEvent& event);
	void OnElevCopy(wxCommandEvent& event);
	void OnElevPasteNew(wxCommandEvent& event);
	void OnElevMergeTin(wxCommandEvent& event);
	void OnElevTrimTin(wxCommandEvent& event);

	void OnUpdateElevSelect(wxUpdateUIEvent& event);
	void OnUpdateViewSetArea(wxUpdateUIEvent& event);
	void OnUpdateElevSetUnknown(wxUpdateUIEvent& event);
	void OnUpdateScaleElevation(wxUpdateUIEvent& event);
	void OnUpdateElevMergeTin(wxUpdateUIEvent& event);
	void OnUpdateElevTrimTin(wxUpdateUIEvent& event);
	void OnUpdateIsGrid(wxUpdateUIEvent& event);

	void OnImageExportTiles(wxCommandEvent& event);
	void OnUpdateImageExportTiles(wxUpdateUIEvent& event);
	void OnImageExportPPM(wxCommandEvent& event);
	void OnUpdateImageExportPPM(wxUpdateUIEvent& event);

	void OnTowerSelect(wxCommandEvent& event);
	void OnTowerEdit(wxCommandEvent& event);
	void OnTowerAdd(wxCommandEvent& event);

	void OnUpdateTowerSelect(wxUpdateUIEvent& event);
	void OnUpdateTowerEdit(wxUpdateUIEvent& event);
	void OnUpdateTowerAdd(wxUpdateUIEvent& event);

	void OnVegPlants(wxCommandEvent& event);
	void OnVegBioregions(wxCommandEvent& event);
	void OnVegRemap(wxCommandEvent& event);
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
	void OnStructureSelectUsingPolygons(wxCommandEvent& event);
	void OnStructureColourSelectedRoofs(wxCommandEvent& event);
	void OnStructureExportFootprints(wxCommandEvent& event);

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
	void OnUpdateStructureSelectUsingPolygons(wxUpdateUIEvent& event);
	void OnUpdateStructureColourSelectedRoofs(wxUpdateUIEvent& event);
	void OnUpdateStructureExportFootprints(wxUpdateUIEvent& event);

	void OnRawSetType(wxCommandEvent& event);
	void OnRawAddPoints(wxCommandEvent& event);
	void OnRawAddPointText(wxCommandEvent& event);
	void OnRawAddPointsGPS(wxCommandEvent& event);
	void OnRawSelectCondition(wxCommandEvent& event);
	void OnRawExportImageMap(wxCommandEvent& event);
	void OnRawStyle(wxCommandEvent& event);
	void OnRawScale(wxCommandEvent& event);

	void OnUpdateRawSetType(wxUpdateUIEvent& event);
	void OnUpdateRawAddPoints(wxUpdateUIEvent& event);
	void OnUpdateRawAddPointText(wxUpdateUIEvent& event);
	void OnUpdateRawAddPointsGPS(wxUpdateUIEvent& event);
	void OnUpdateRawIsActive(wxUpdateUIEvent& event);

	void OnAreaZoomAll(wxCommandEvent& event);
	void OnAreaZoomLayer(wxCommandEvent& event);
	void OnAreaTypeIn(wxCommandEvent& event);
	void OnAreaMatch(wxCommandEvent& event);
	void OnAreaExportElev(wxCommandEvent& event);
	void OnAreaOptimizedElevTileset(wxCommandEvent& event);
	void OnAreaOptimizedImageTileset(wxCommandEvent& event);
	void OnAreaExportImage(wxCommandEvent& event);
	void OnAreaGenerateVeg(wxCommandEvent& event);
	void OnAreaVegDensity(wxCommandEvent& event);
	void OnAreaRequestWFS(wxCommandEvent& event);
	void OnAreaRequestWMS(wxCommandEvent& event);
	void OnAreaRequestTServe(wxCommandEvent& event);

	void OnUpdateAreaZoomAll(wxUpdateUIEvent& event);
	void OnUpdateAreaZoomLayer(wxUpdateUIEvent& event);
	void OnUpdateAreaMatch(wxUpdateUIEvent& event);
	void OnUpdateAreaExportElev(wxUpdateUIEvent& event);
	void OnUpdateAreaExportImage(wxUpdateUIEvent& event);
	void OnUpdateAreaGenerateVeg(wxUpdateUIEvent& event);
	void OnUpdateAreaVegDensity(wxUpdateUIEvent& event);
	void OnUpdateAreaRequestWMS(wxUpdateUIEvent& event);

	void OnHelpAbout(wxCommandEvent& event);
	void OnHelpDocLocal(wxCommandEvent& event);
	void OnHelpDocOnline(wxCommandEvent& event);

	// Popup menu items
	void OnDistanceClear(wxCommandEvent& event);
	void OnShowAll(wxCommandEvent& event);
	void OnHideAll(wxCommandEvent& event);

	// keys (used for shortcuts)
	void OnChar(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnMouseWheel(wxMouseEvent& event);

public:
	// project
	void LoadProject(const wxString &strPathName);
	void SaveProject(const wxString &strPathName) const;
	void DoGeocode();

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
	DPoint2 EstimateGeoDataCenter();
	LayerType AskLayerType();
	int LayerNum(vtLayer *lp);
	void SwapLayerOrder(int n0, int n1);
	void RefreshLayerInView(vtLayer *pLayer);

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
	bool ConfirmValidCRS(vtProjection *pProj);

	// Raw features
	FeatInfoDlg	*ShowFeatInfoDlg();
	FeatInfoDlg	*m_pFeatInfoDlg;

	// Distance and Elevation Profile
	DistanceDlg2d	*ShowDistanceDlg();
	DistanceDlg2d *m_pDistanceDlg;
	void UpdateDistance(const DPoint2 &p1, const DPoint2 &p2);
	void UpdateDistance(const DLine2 &path);
	void ClearDistance();
	ProfileDlg	*ShowProfileDlg();
	ProfileDlg	*m_pProfileDlg;

	// Elevation
	bool SampleCurrentTerrains(vtElevLayer *pTarget);
	float GetHeightFromTerrain(const DPoint2 &p);
	void ExportBitmap(RenderDlg &dlg);
	int ElevLayerArray(std::vector<vtElevLayer*> &elevs);
	bool FillElevGaps(vtElevLayer *el);

	// Images
	bool SampleCurrentImages(vtImageLayer *pTarget);

	// Structures
	LinearStructureDlg *ShowLinearStructureDlg(bool bShow = true);
	LinearStructureDlg2d *m_pLinearStructureDlg;
	vtLinearParams m_LSOptions;
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

	vtBioRegion m_BioRegion;
	vtBioRegion *GetBioRegion() { return &m_BioRegion; }
	BioRegionDlg *m_BioRegionDlg;

	void GenerateVegetation(const char *vf_file, DRECT area, VegGenOptions &opt);
	void GenerateVegetationPhase2(const char *vf_file, DRECT area, VegGenOptions &opt);

	// import
	void ImportData(LayerType ltype);
	void ImportDataFromArchive(LayerType ltype, const wxString &fname_org, bool bRefresh);
	vtLayer *ImportDataFromFile(LayerType ltype, const wxString &strFileName, bool bRefresh, bool bWarn = true);
	vtLayer *ImportFromDLG(const wxString &strFileName, LayerType ltype);
	vtLayer *ImportFromSHP(const wxString &strFileName, LayerType ltype);
	vtLayer *ImportFromDXF(const wxString &strFileName, LayerType ltype);
	vtLayer *ImportElevation(const wxString &strFileName, bool bWarn = true);
	vtLayer *ImportImage(const wxString &strFileName);
	vtLayer *ImportFromLULC(const wxString &strFileName, LayerType ltype);
	vtLayer *ImportRawFromOGR(const wxString &strFileName);
	vtLayer *ImportVectorsWithOGR(const wxString &strFileName, LayerType ltype);
	vtStructureLayer *ImportFromBCF(const wxString &strFileName);
	void ImportFromMapSource(const char *fname);
	void ImportDataPointsFromTable(const char *fname);
	int ImportDataFromTIGER(const wxString &strDirName);
	int ImportDataFromSCC(const char *filename);
	void ImportDataFromNTF(const wxString &strFileName);
	void ImportDataFromS57(const wxString &strDirName);
	LayerType GuessLayerTypeFromDLG(vtDLGFile *pDLG);
	void ElevCopy();
	void ElevPasteNew();

	// Export
	void ExportASC();
	void ExportGeoTIFF();
	void ExportTerragen();
	void ExportBMP();
	void ExportSTM();
	void ExportPlanet();
	void ExportVRML();
	void ExportRAWINF();
	void ExportChunkLOD();
	void ExportPNG16();
	void Export3TX();
	void ElevExportTiles();
	void ImageExportTiles();
	void ImageExportPPM();
	void ExportAreaOptimizedElevTileset();
	void ExportAreaOptimizedImageTileset();
	void ExportDymaxTexture();

	// area tool
	void ScanElevationLayers(int &count, int &floating, int &tins, DPoint2 &spacing);
	void MergeResampleElevation();
	bool SampleElevationToTilePyramids(const TilingOptions &opts, bool bFloat);
	bool SampleImageryToTilePyramids(const TilingOptions &opts);
	void ExportImage();

	// Web Access
	OGCServerArray m_wms_servers;
	OGCServerArray m_wfs_servers;

	// Application Data
	DRECT		m_area;
	vtStringArray	m_datapaths;

	// User Preferences
	vtTagArray m_Options;

protected:
	// INI File
	bool ReadINI(const char *fname);
	//bool WriteINI();
	bool ReadXML(const char *fname);
	bool WriteXML(const char *fname);
	const char *m_szIniFilename;

	// Application Data
	LayerArray	m_Layers;
	vtLayerPtr	m_pActiveLayer;
	TilingOptions m_tileopts;

	MapServerDlg *m_pMapServerDlg;

	// UI members
	wxMenu *fileMenu;
	wxMenu *editMenu;
	wxMenu *layerMenu;
	wxMenu *viewMenu;
	wxMenu *elevMenu;
	wxMenu *imgMenu;
	wxMenu *vegMenu;
	wxMenu *bldMenu;
	wxMenu *roadMenu;
	wxMenu *utilityMenu;
	wxMenu *rawMenu;
	wxMenu *areaMenu;
	wxMenu *helpMenu;
	wxMenuBar *m_pMenuBar;
	wxToolBar *m_pToolbar;				// main toolbar
	wxToolBar *m_pLayBar[LAYER_TYPES];	// layer type-specific toolbars
	MyStatusBar *m_statbar;
	int		m_iMainButtons;

	MyTreeCtrl	*m_pTree;		// left child of splitter
	BuilderView	*m_pView;		// right child of splitter
	bool	m_bDrawDisabled;
	vtScaleBar *m_pScaleBar;

	vtProjection	m_proj;
	bool	m_bAdoptFirstCRS;	// If true, silenty assume user wants to
		// use the CRS of the first layer they create

	// menu numbers, for each layer type that has a corresponding menu
	int		m_iLayerMenu[LAYER_TYPES];

    wxAuiManager m_mgr;

	DECLARE_EVENT_TABLE()
};

#if wxUSE_DRAG_AND_DROP
class DnDFile : public wxFileDropTarget
{
public:
	virtual bool OnDropFiles(wxCoord x, wxCoord y,
							 const wxArrayString& filenames);
};
#endif

extern MainFrame *GetMainFrame();
wxString GetImportFilterString(LayerType ltype);
float ElevLayerArrayValue(std::vector<vtElevLayer*> &elevs, const DPoint2 &p);
void ElevLayerArrayRange(std::vector<vtElevLayer*> &elevs,
						 float &minval, float &maxval);

#endif	// VTBUILDERFRAMEH

