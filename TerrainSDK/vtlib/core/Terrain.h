//
// Terrain.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINH
#define TERRAINH

#include "vtdata/TParams.h"
#include "vtdata/vtTime.h"
#include "vtdata/FilePath.h"

#include "Trees.h"
#include "Structure3d.h"
#include "Route.h"
#include "DynTerrain.h"
#include "Content3d.h"

// Try to reduce compile-time dependencies with these forward declarations
class vtDIB;
class vtTerrainGeom;
class vtTextureCoverage;
class vtFence3d;
class vtRoadMap3d;
class vtLodGrid;
class vtElevationGrid;
class vtTin;
class vtTin3d;
class vtFeatureSet;
class SimpleBillboardEngine;

typedef Array<vtStructureArray3d *> StructureSet;

/**
 * Defines a Point of Interest, which is a rectangular area of the terrain
 * which can have addition attributes associated with it, such as a name
 * and URL.
 */
class vtPointOfInterest
{
public:
	vtPointOfInterest() { m_pGeom = NULL; }

	DRECT m_rect;
	vtString m_name;
	vtString m_url;
	vtGeom *m_pGeom;
};
typedef class vtPointOfInterest *POIPtr;

// Terrain Feature Types
enum TFType
{
	TFT_TERRAINSURFACE,
	TFT_OCEAN,
	TFT_VEGETATION,
	TFT_STRUCTURES,
	TFT_ROADS,
	TFT_TOWERS
};


/**
 * The vtTerrain class represents a rectangular area of terrain.
 * \par
 * It is described by a set of parameters such as elevation, vegetation,
 * and time of day.  These terrain parameters are contained in the class TParams.
 * \par
 * To create a new terrain, first construct a vtTerrain and set its
 * parameters with SetParams() or SetParamFile().  Then call CreateScene()
 * to build the visual representation, which returns a vtGroup which can be
 * added to a scene graph.
 */
class vtTerrain
{
public:
	vtTerrain();
	virtual ~vtTerrain();

	/********************** Public Methods ******************/

	// parameters for terrain creation
	bool SetParamFile(const char *fname);
	bool LoadParams();
	vtString GetParamFile()  { return m_strParamFile; }
	void SetParams(const TParams &pParams);
	TParams &GetParams() { return m_Params; }

	// each terrain can have a long descriptive name
	void SetName(const vtString &str) { m_Params.SetValueString(STR_NAME, str); }
	vtString GetName() { return m_Params.GetValueString(STR_NAME); }

	// you can alternately give it a grid to use instead of loading a BT
	void SetLocalGrid(vtElevationGrid *pGrid, bool bPreserve);
	void SetTin(vtTin3d *pTin);
	vtTin3d *GetTin() { return m_pTin; }

	/// primary creation function
	bool CreateStep1();
	bool CreateStep2(vtTransform *pSunLight);
	bool CreateStep3();
	bool CreateStep4();
	bool CreateStep5();
	vtString GetLastError() { return m_strErrorMsg; }

	/** Set the array of colors to be used when automatically generating the
		terrain texture from the elevation values.  The colors go from the
		lowest elevation value to the highest. */
	void SetTextureColors(Array<RGBi> *brackets) { m_pTextureColors = brackets; }
	virtual void CustomizeDib() {}

	/// return true if the terrain has been created
	bool IsCreated();

	/// set the enabled state of the terrain (whether it is shown or not)
	void Enable(bool bVisible);

	/// load an external geometry file
	vtTransform *LoadModel(const char *filename);

	/// add a model (or any node) to the terrain
	void AddNode(vtNode *pNode);

	/// place a model on the terrain
	void PlantModel(vtTransform *model);
	/// place a model on the terrain at a specific point
	void PlantModelAtPoint(vtTransform *model, const DPoint2 &pos);

	// test whether a given point is within the current terrain
	bool PointIsInTerrain(const DPoint2 &p);

	// set global projection based on this terrain
	void SetGlobalProjection();

	bool LoadHeaderIntoGrid(vtElevationGrid &grid);

	// fences
	void AddFence(vtFence3d *f);
	void AddFencepoint(vtFence3d *f, const DPoint2 &epos);
	void RedrawFence(vtFence3d *f);

	// Route
	void AddRoute(vtRoute *f);
	void add_routepoint_earth(vtRoute *f, const DPoint2 &epos, const char *structname);
	void RedrawRoute(vtRoute *f);
	void SaveRoute();
	vtRoute* GetLastRoute() { return m_Routes.GetSize()>0?m_Routes[m_Routes.GetSize()-1]:0; }
	vtRouteMap &GetRouteMap() { return m_Routes; }

	// plants
	bool AddPlant(const DPoint2 &pos, int iSpecies, float fSize);
	void DeleteSelectedPlants();
	void SetPlantList(vtSpeciesList3d *pPlantList);
	vtPlantInstanceArray3d &GetPlantInstances() { return m_PIA; }
	bool AddNodeToVegGrid(vtTransform *pTrans);

	// structures
	StructureSet &GetStructureSet() { return m_StructureSet; }
	vtStructureArray3d *GetStructures();
	int GetStructureIndex();
	void SetStructureIndex(int index);
	vtStructureArray3d *NewStructureArray();
	vtStructureArray3d *CreateStructuresFromXML(const vtString &strFilename);
	void CreateStructures(vtStructureArray3d *structures);
	bool CreateStructure(vtStructureArray3d *structures, int index);
	void DeleteSelectedStructures();
	void DeleteStructureSet(int index);
	bool FindClosestStructure(const DPoint2 &point, double epsilon,
							  int &structure, double &closest);
	bool AddNodeToStructGrid(vtTransform *pTrans);
	bool AddNodeToStructGrid(vtGeom *pGeom);
	void RemoveNodeFromStructGrid(vtNode *pNode);
	vtLodGrid *GetStructureGrid() { return m_pStructGrid; }

	// Terrain-specific content
	vtContentManager3d m_Content;

	// overridable by subclasses to extend culture
	virtual void CreateCustomCulture();

	// manage engines specific to this terrain
	void AddEngine(vtEngine *pE);
	void ActivateEngines(bool bActive);

	// reports world coordinates
	FPoint3 GetCenter();
	float GetRadius();

	// turn various features on/off
	void SetFeatureVisible(TFType ftype, bool bOn);
	bool GetFeatureVisible(TFType ftype);

	// control LOD
	void SetLODDistance(TFType ftype, float fDistance);
	float GetLODDistance(TFType ftype);

	// query
	vtDynTerrainGeom *GetDynTerrain() { return m_pDynGeom; }
	vtGroup *GetTopGroup() { return m_pTerrainGroup; }
	vtHeightField3d *GetHeightField();
	vtHeightFieldGrid3d *GetHeightFieldGrid3d();
	vtProjection &GetProjection() { return m_proj; }

	// Points of interest
	void AddPointOfInterest(double left, double bottom, double right,
					   double top, const char *name, const char *url);
	vtPointOfInterest *FindPointOfInterest(DPoint2 utm);
	void SetShowPOI( bool flag ) { m_bShowPOI = flag; }
	bool GetShowPOI() { return m_bShowPOI; }
	void ShowPOI(vtPointOfInterest *poi, bool bShow);
	void HideAllPOI();

	// symbols and labels for abstract data
	float AddSurfaceLineToMesh(vtMesh *pMesh, const DLine2 &line, float fOffset, bool bCurve = false);
	void CreateStyledFeatures(const vtFeatureSet &feat, const char *fontname, const PointStyle &style);

	// Access the viewpoint associated with this terrain
	void SetCamLocation(FMatrix4 &mat) { m_CamLocation = mat; }
	FMatrix4 &GetCamLocation() { return m_CamLocation; }

	// Sky and Fog
	void SetFog(bool fog);
	void SetFogColor(const RGBf &color);
	void SetFogDistance(float fMeters);
	bool GetFog() { return m_bFog; }
	void SetBgColor(const RGBf &color);
	RGBf GetBgColor() { return m_background_color; }

	// linked list of terrains
	void SetNext(vtTerrain *t) { m_pNext = t; }
	vtTerrain *GetNext() { return m_pNext; }

	// Time
	vtTime GetInitialTime();
	void TranslateToGMT(vtTime &time);
	void TranslateFromGMT(vtTime &time);
	DPoint2 GetCenterGeoLocation();

	/********************** Public Data ******************/

	// polygon containing geo corners of terrain area
	DLine2		m_Corners_geo;

	// Experimental! Completely unoptimized!
	void recreate_textures(vtTransform *pSunLight);

protected:
	/********************** Protected Methods ******************/

	// internal creation functions
	bool CreateFromTIN();
	bool CreateFromGrid();
	void create_roads(const vtString &strRoadFile);
	void _SetupVegGrid(float fLODDistance);
	void _SetupStructGrid(float fLODDistance);
	void _CreateLabels();
	void _CreateTextures(const FPoint3 &light_dir);
	bool _CreateDynamicTerrain(float fOceanDepth);
	void _CreateErrorMessage(DTErr error, vtElevationGrid *pGrid);
	void _SetErrorMessage(const vtString &msg);
	void create_artificial_horizon(bool bWater, bool bHorizon,
		bool bCenter, float fTransparency);
	void _CreateCulture();

	void _CreateChoppedTextures(int patches, int patch_size);
	void _CreateTiledMaterials(vtMaterialArray *pMat1,
							 int patches, int patch_size, float ambient,
							 float diffuse, float emmisive);
	void _ApplyPreLight(vtHeightFieldGrid3d *pLocalGrid, vtDIB *dib,
		const FPoint3 &light_dir);
	void _ComputeCenterLocation();
	void GetTerrainBounds();

	/********************** Protected Data ******************/

	// main scene graph outline
	vtGroup		*m_pTerrainGroup;

	// dynamic terrain (CLOD)
	vtDynTerrainGeom *m_pDynGeom;
	vtTransform		 *m_pDynGeomScale;

	// triangulated irregular network (TIN)
	vtTin3d		*m_pTin;

	// construction parameters used to create this terrain
	TParams		m_Params;

	// data grids
	vtElevationGrid	*m_pInputGrid;	// if non-NULL, use instead of BT
	vtHeightField3d	*m_pHeightField;
	bool			m_bPreserveInputGrid;

	// if we're switching between multiple terrains, we can remember where
	// the camera was in each one
	FMatrix4		m_CamLocation;

	// ocean and fog
	vtMovGeom		*m_pOceanGeom;
	bool			m_bFog;
	RGBf			m_fog_color;
	RGBf			m_background_color;

	// built structures, e.g. buildings and fences
	StructureSet	m_StructureSet;
	unsigned int	m_iStructSet;
	vtLodGrid		*m_pStructGrid;

	vtMaterialArray	*m_pTerrMats;	// materials for the LOD terrain

	// roads
	vtGroup			*m_pRoadGroup;
	vtRoadMap3d		*m_pRoadMap;

	// plants
	vtPlantInstanceArray3d	m_PIA;
	vtSpeciesList3d	*m_pPlantList;
	vtGroup			*m_pVegGroup;
	vtLodGrid		*m_pVegGrid;

	// routes
	vtRouteMap		m_Routes;

	// ground texture
	vtDIB			*m_pDIB;
	Array<vtImage*>	m_Images;
	Array<RGBi>		*m_pTextureColors;

	FSphere			m_bound_sphere;	// bounding sphere of terrain
									// (without surrounding ocean)
	RGBf			m_ocean_color;
	vtString		m_strParamFile;

	// keep a list of all the engines specific to this terrain
	Array<vtEngine*>  m_Engines;
	SimpleBillboardEngine	*m_pBBEngine;

	// maintain a linked list of vtTerrain objects
	vtTerrain		*m_pNext;

	Array<POIPtr>	m_PointsOfInterest;
	bool			m_bShowPOI;
	vtGroup			*m_pPOIGroup;

	// only used during initialization
	vtElevationGrid	*m_pElevGrid;

	// A useful value for computing "local time", the location of the
	//  center of the terrain in Geographic coords.
	DPoint2			m_CenterGeoLocation;;
	int m_iDifferenceFromGMT;

	// hold an informative message in case anything goes wrong
	vtString	m_strErrorMsg;

	vtProjection	m_proj;
};

#endif	// TERRAINH
