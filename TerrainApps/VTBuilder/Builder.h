//
// Builder.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef BUILDERH
#define BUILDERH

#include "vtdata/Projections.h"
#include "vtdata/Plants.h"		// for vtSpeciesList
#include "vtdata/Fence.h"		// for LinStructOptions
#include "vtdata/vtLog.h"

#include "Layer.h"
#include "TilingOptions.h"
#include "RenderOptions.h"
#include "BuilderView.h"

class vtDLGFile;
class vtVegLayer;
class vtRawLayer;
class vtElevLayer;
class vtImageLayer;
class vtRoadLayer;
class vtStructureLayer;
class vtUtilityLayer;
class vtFeatureSet;
class VegGenOptions;
class vtElevationGrid;

// dialogs
class InstanceDlg;

/**
* The main frame is the central class of the whole terrain builder.
* Not only does it represent the top window of the application, but it also
* contains many of the powerful methods for working with data.
*/
class Builder
{
public:
	Builder();
	~Builder();

	void ReadDataPath();
	void ReadDatapathsFromXML(ifstream &input, const char *path);

	// Project methods
	bool LoadProject(const vtString &fname, vtScaledView *pView = NULL);
	void DeleteContents();

	// Layer methods
	int NumLayers() const { return m_Layers.GetSize(); }
	vtLayer *GetLayer(int i) const { return m_Layers[i]; }
	vtLayer *LoadLayer(const wxString &fname);
	void AddLayer(vtLayer *lp);
	virtual bool AddLayerWithCheck(vtLayer *pLayer, bool bRefresh = true);
	virtual void RemoveLayer(vtLayer *lp);
	void DeleteLayer(vtLayer *lp);
	virtual void SetActiveLayer(vtLayer *lp, bool refresh = false);
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

	// UI
	virtual void RefreshTreeStatus() {}
	virtual void RefreshStatusBar() {}
	virtual void UpdateFeatureDialog(vtRawLayer *raw, vtFeatureSetPoint2D *set, int iEntity) {}
	virtual void OnSetMode(LBMode m) {}
	virtual void OnSelectionChanged() {}
	virtual void UpdateDistance(const DPoint2 &p1, const DPoint2 &p2) {}
	virtual void UpdateDistance(const DLine2 &path) {}
	virtual void ZoomAll() {}
	bool DrawDisabled() { return m_bDrawDisabled; }
	void AddToMRU(vtStringArray &arr, const vtString &fname);

	// Projection
	virtual void SetProjection(const vtProjection &p);
	void GetProjection(vtProjection &p) { p = m_proj; }
	vtProjection &GetAtProjection() { return m_proj; }
	bool ConfirmValidCRS(vtProjection *pProj);

	// Elevation
	bool SampleCurrentTerrains(vtElevLayer *pTarget);
	float GetHeightFromTerrain(const DPoint2 &p);
	void ExportBitmap(vtElevLayer *pEL, RenderOptions &ropt);
	uint ElevLayerArray(std::vector<vtElevLayer*> &elevs);
	bool FillElevGaps(vtElevLayer *el, DRECT *area = NULL, int iMethod = -1);
	void FlagStickyLayers(const std::vector<vtElevLayer*> &elevs);
	vtElevLayer *ComputeDifference(vtElevLayer *pElev);

	// Images
	bool SampleCurrentImages(vtImageLayer *pTarget);
	bool GetRGBUnderCursor(const DPoint2 &p, RGBi &rgb);

	// Structures
	vtLinearParams m_LSOptions;
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
	bool LoadSpeciesFile(const char *fname);
	bool LoadBiotypesFile(const char *fname);

	vtBioRegion m_BioRegion;
	vtBioRegion *GetBioRegion() { return &m_BioRegion; }

	void GenerateVegetation(const char *vf_file, DRECT area, VegGenOptions &opt);
	void GenerateVegetationPhase2(const char *vf_file, DRECT area, VegGenOptions &opt);

	// Import
	void ImportData(LayerType ltype);
	int ImportDataFromArchive(LayerType ltype, const wxString &fname_org, bool bRefresh);
	vtLayer *ImportDataFromFile(LayerType ltype, const wxString &strFileName, bool bRefresh = false, bool bWarn = false);
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
	vtFeatureSetPoint2D *ImportPointsFromDBF(const char *fname);
	vtFeatureSet *ImportPointsFromCSV(const char *fname);
	vtFeatureSet *ImportPointsFromXYZ(const char *fname, bool progress_callback(int) = NULL);
	void ImportDataPointsFromTable(const char *fname, bool progress_callback(int) = NULL);
	int ImportDataFromTIGER(const wxString &strDirName);
	void ImportDataFromOSM(const wxString &strFileName);
	void ImportDataFromNTF(const wxString &strFileName);
	void ImportDataFromS57(const wxString &strDirName);
	int ImportDataFromSCC(const char *filename);
	bool ImportDataFromDXF(const char *filename);
	LayerType GuessLayerTypeFromDLG(vtDLGFile *pDLG);
	void ElevCopy();
	void ElevPasteNew();

	// Export
	void ExportASC();
	void ExportGeoTIFF();
	void ExportTerragen();
	void ExportBMP();
	void ExportSTM();
	void ExportTIN();
	void ExportPlanet();
	void ExportVRML();
	void ExportXYZ();
	void ExportRAWINF();
	void ExportChunkLOD();
	void ExportPNG16();
	void Export3TX();
	void ElevExportTiles(BuilderView *pView = NULL);
	void ImageExportTiles(BuilderView *pView = NULL);
	void ImageExportPPM();
	void ExportAreaOptimizedElevTileset(BuilderView *pView = NULL);
	void ExportAreaOptimizedImageTileset(BuilderView *pView = NULL);

	// Area tool
	void SetArea(const DRECT &r) { m_area = r; }
	void GetArea(DRECT &r) { r = m_area; }
	DRECT &GetAtArea() { return m_area; }

	// Sampling
	void ScanElevationLayers(int &count, int &floating, int &tins, DPoint2 &spacing);
	void MergeResampleElevation(BuilderView *pView = NULL);
	bool SampleElevationToTilePyramids(BuilderView *pView, TilingOptions &opts, bool bFloat, bool bShowGridMarks = true);
	bool DoSampleElevationToTilePyramids(BuilderView *pView, TilingOptions &opts, bool bFloat, bool bShowGridMarks = true);
	bool SampleImageryToTilePyramids(BuilderView *pView, TilingOptions &opts, bool bShowGridMarks = true);
	bool DoSampleImageryToTilePyramids(BuilderView *pView, TilingOptions &opts, bool bShowGridMarks = true);
	void MergeResampleImages(BuilderView *pView = NULL);

	// Application Data
	wxFrame *m_pParentWindow;
	bool	m_bDrawDisabled;

protected:
	void CheckOptionBounds();

	// Application Data
	DRECT		m_area;
	LayerArray	m_Layers;
	vtLayerPtr	m_pActiveLayer;
	TilingOptions m_tileopts;

	// Most-recently-used files
	vtStringArray m_ProjectFiles, m_LayerFiles, m_ImportFiles;

	vtProjection	m_proj;
	bool	m_bAdoptFirstCRS;	// If true, silenty assume user wants to
								// use the CRS of the first layer they create
};

extern Builder *g_bld;

wxString GetImportFilterString(LayerType ltype);
float ElevLayerArrayValue(std::vector<vtElevLayer*> &elevs, const DPoint2 &p);
void ElevLayerArrayRange(std::vector<vtElevLayer*> &elevs,
						 float &minval, float &maxval);

#endif	// BUILDERH

