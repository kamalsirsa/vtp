//
// Terrain.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINH
#define TERRAINH

#include "TParams.h"
#include "Trees.h"
#include "Structure3d.h"
#include "vtdata/FilePath.h"
#include "Route.h"

class vtTerrainGeom;
class vtTextureCoverage;
class vtFence3d;
class vtRoadMap3d;
class vtLodGrid;
class vtDynTerrainGeom;
class vtElevationGrid;
class vtTin;

typedef vtImage *vtImagePtr;

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
	TFT_REGULAR,
	TFT_OCEAN,
	TFT_VEGETATION,
	TFT_ROADS,
	TFT_TOWERS
};


/**  The vtTerrain class represents a rectangular area of terrain.
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
	~vtTerrain();

	/********************** Public Methods ******************/

	// parameters for terrain creation
	bool SetParamFile(const char *fname);
	bool LoadParams();
	vtString GetParamFile()  { return m_strParamFile; }
	void SetParams(TParams &pParams) { m_Params = pParams; }
	TParams &GetParams() { return m_Params; }

	// each terrain can have a long descriptive name
	void SetName(vtString str) { m_Params.m_strName = str; }
	vtString GetName() { return m_Params.m_strName; }

	// you can alternately give it a grid to use instead of loading a BT
	void SetLocalGrid(vtLocalGrid *pGrid, bool bPreserve);
	void SetTin(vtTin3d *pTin);
	vtTin3d *GetTin() { return m_pTin; }

	/// primary creation function
	vtGroup *CreateScene(bool bSound, int &iError);
	bool CreateStep1(int &iError);
	bool CreateStep2(int &iError);
	bool CreateStep3(int &iError);
	bool CreateStep4(int &iError);
	bool CreateStep5(bool bSound, int &iError);
	const char *DesribeError(int iError);

	/// return true if the terrain has been created
	bool IsCreated();

	/// set the enabled state of the terrain (whether it is shown or not)
	void Enable(bool bVisible);

	vtGroup *GetTopGroup() { return m_pTerrainGroup; }

	/// load an external geometry file
	vtTransform *LoadModel(const char *filename);

	/// add a model (or any node) to the terrain
	void AddNode(vtNode *pNode);

	/// add a model (or any node) to the LOD grid of the terrain 
	void AddNodeToLodGrid(vtTransform *pTrans);
	void AddNodeToLodGrid(vtGeom *pGeom);

	/// place a model on the terrain
	void PlantModel(vtTransform *model);
	/// place a model on the terrain at a specific point
	void PlantModelAtPoint(vtTransform *model, const DPoint2 &p);

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
	void AddPlant(const DPoint2 &pos, int iSpecies, float fSize);
	void SetPlantList(vtPlantList3d *pPlantList) { m_pPlantList = pPlantList; }
	vtPlantInstanceArray3d &GetPlantInstances() { return m_PIA; }

	// structures
	vtStructureArray3d &GetStructures() { return m_Structures; }

	// overridable by subclasses to extend culture
	virtual void CreateCustomCulture(bool bDoSound);

	// manage engines specific to this terrain
	void AddEngine(vtEngine *pE);
	void ActivateEngines(bool bActive);

	// reports world coordinates
	FPoint3 GetCenter();
	float GetRadius();
	void GetTerrainBounds();

	// turn various features on/off
	void SetFeatureVisible(TFType ftype, bool bOn);
	bool GetFeatureVisible(TFType ftype);

	// query
	RGBf GetOceanColor() { return m_ocean_color; }
	vtDynTerrainGeom *GetDynTerrain() { return m_pDynGeom; }

	// Points of interest
	void AddPointOfInterest(double ulx, double uly, double brx, double bry,
							  const char *name, const char *url);
	vtPointOfInterest *FindPointOfInterest(DPoint2 utm);
	void SetShowPOI( bool flag ) { m_bShowPOI = flag; }
	bool GetShowPOI() { return m_bShowPOI; }
	void ShowPOI(vtPointOfInterest *poi, bool bShow);
	void HideAllPOI();

	// Access the viewpoint associated with this terrain
	void SetCamLocation(FMatrix4 &mat) { m_CamLocation = mat; }
	FMatrix4 &GetCamLocation() { return m_CamLocation; }

	vtHeightField *GetHeightField() { return m_pHeightField; }

	// linked list of terrains
	void SetNext(vtTerrain *t) { m_pNext = t; }
	vtTerrain *GetNext() { return m_pNext; }

	/********************** Public Data ******************/

	// polygon containing geo corners of terrain area
	DLine2		m_Corners_geo;

	/********************** Statics ******************/

	// during creation, all data will be looked for on the data path
	static void SetDataPath(const StringArray &paths) { m_DataPaths = paths; }
	static StringArray m_DataPaths;

protected:
	/********************** Protected Methods ******************/

	// internal creation functions
	bool CreateFromTIN(int &iError);
	bool CreateFromGrid(int &iError);
	void create_roads(vtString strRoadFile);
	void setup_LodGrid(float fLODDistance);
	void create_textures();
	bool create_regular_terrain(float fOceanDepth);
	bool create_dynamic_terrain(float fOceanDepth, int &iError);
	void create_artificial_horizon(bool bWater, bool bHorizon,
		bool bCenter, float fTransparency);
	void CreateStructuresFromXML(vtString strFilename);
	void create_culture(bool bSound);
	void create_floating_labels(const char *filename);

	void CreateChoppedTextures(vtLocalGrid *pLocalGrid, vtDIB *dib1,
								int patches, int patch_size);
	void CreateChoppedAppearances1(vtMaterialArray *pApp1,
							 int patches, int patch_size, float ambient,
							 float diffuse, float emmisive);
	void CreateChoppedAppearances2(vtMaterialArray *pApp1,
							 int patches, int patch_size, float ambient,
							 float diffuse, float emmisive);
	void ApplyPreLight(vtLocalGrid *pLocalGrid, vtDIB *dib);

	/********************** Protected Data ******************/

	// main scene graph outline
	vtGroup		*m_pTerrainGroup;

	// regular terrain (brute-force)
	vtTerrainGeom	*m_pTerrainGeom;

	// dynamic terrain (CLOD)
	vtDynTerrainGeom *m_pDynGeom;
	vtTransform		 *m_pDynGeomScale;

	// triangulated irregular network (TIN)
	vtTin3d		*m_pTin;

	// construction parameters used to create this terrain
	TParams		m_Params;

	// data grids
	vtLocalGrid		*m_pInputGrid;	// if non-NULL, use instead of BT
	vtHeightField	*m_pHeightField;
	vtLodGrid		*m_pLodGrid;
	bool			m_bPreserveInputGrid;

	// if we're switching between multiple terrains, we can remember where
	// the camera was in each one
	FMatrix4		m_CamLocation;

	// ocean
	vtMovGeom		*m_pOceanGeom;

	// built structures, e.g. buildings and fences
	vtStructureArray3d	m_Structures;

	vtMaterialArray		*m_pTerrApps1;	// for 'regular' terrain
	vtMaterialArray		*m_pTerrApps2;	// for dynamic LOD terrain

	// roads
	vtGroup			*m_pRoadGroup;
	vtRoadMap3d		*m_pRoadMap;

	// plants
	vtPlantInstanceArray3d	m_PIA;
	vtGroup				*m_pTreeGroup;
	vtPlantList3d		*m_pPlantList;

	// routes
	vtRouteMap		m_Routes;

	// ground texture
	vtDIB				*m_pDIB;
	Array<vtImagePtr>	m_Images;
	vtImage				*m_pImage;
	vtTextureCoverage	*m_pCoverage;

	FSphere			m_bound_sphere;		// bounding sphere of terrain
									// (without surrounding ocean)
	RGBf			m_ocean_color;
	vtString		m_strParamFile;

	// keep a list of all the engines specific to this terrain
	Array<vtEngine*>  m_Engines;

	// maintain a linked list
	vtTerrain		*m_pNext;

	Array<POIPtr>	m_PointsOfInterest;
	bool			m_bShowPOI;

	vtGroup			*m_pPOIGroup;

	// only used during initialization
	vtLocalGrid		*m_pLocalGrid;

	vtProjection	m_proj;
};

//helpers
vtGeom *CreatePlaneGeom(FPoint2 org, FPoint2 size,
						   float xTiling, float zTiling, int steps);
vtGeom *CreateSphereGeom(vtMaterialArray *pMats, int iMatIdx, float fRadius, int res);
vtGeom *CreateLineGridGeom(vtMaterialArray *pMats, int iMatIdx,
					   FPoint3 min1, FPoint3 max1, int steps);

#endif
