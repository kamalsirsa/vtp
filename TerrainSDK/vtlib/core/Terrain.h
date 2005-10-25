//
// Terrain.h
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINH
#define TERRAINH

#include "vtdata/vtTime.h"
#include "vtdata/FilePath.h"

#include "TParams.h"
#include "Trees.h"
#include "Structure3d.h"
#include "Route.h"
#include "DynTerrain.h"
#include "Content3d.h"
#include "GeomUtil.h"	// for MeshFactory
#include "Location.h"
#include "AnimPath.h"

// Try to reduce compile-time dependencies with these forward declarations
class vtDIB;
class vtTerrainGeom;
class vtTiledGeom;
class vtTextureCoverage;
class vtFence3d;
class vtRoadMap3d;
class vtLodGrid;
class vtElevationGrid;
class vtTin;
class vtTin3d;
class vtFeatureSet;
class SimpleBillboardEngine;

/** \addtogroup terrain */
/*@{*/

/** The set of all structure arrays which are on a terrain. */
class StructureSet : public Array<vtStructureArray3d *>
{
public:
	bool FindStructureFromNode(vtNode* pNode, int &iSet, int &iOffset);
};


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
	TFT_ROADS
};


/**
 * The vtTerrain class represents a terrain, which is a part of the surface
 *  of the earth.
 *
 * It is described by a set of parameters such as elevation, vegetation,
 * and time of day.  These terrain parameters are contained in the class TParams.
 *
 * To create a new terrain, first construct a vtTerrain and set its
 * parameters with SetParams() or SetParamFile().  You can then build the
 * terrain using the CreateStep methods, or add it to a vtTerrainScene
 * and use vtTerrainScene::BuildTerrain.
 */
class vtTerrain : public CultureExtension
{
public:
	vtTerrain();
	virtual ~vtTerrain();

	/********************** Public Methods ******************/

	// parameters for terrain creation
	bool SetParamFile(const char *fname);
	bool LoadParams();
	vtString GetParamFile()  { return m_strParamFile; }
	void SetParams(const TParams &Params);
	TParams &GetParams();

	// each terrain can have a long descriptive name
	void SetName(const vtString &str) { m_Params.SetValueString(STR_NAME, str); }
	vtString GetName() { return m_Params.GetValueString(STR_NAME); }

	// you can alternately give it a grid to use instead of loading a BT
	void SetLocalGrid(vtElevationGrid *pGrid, bool bPreserve);
	void SetTin(vtTin3d *pTin);
	vtTin3d *GetTin() { return m_pTin; }
	bool GetGeoExtentsFromMetadata();

	/// pass true to draw the underside of the terrain as well
	void SetBothSides(bool bFlag) { m_bBothSides = bFlag; }

	// You can use these methods to build a terrain step by step,
	// or simply use the method vtTerrainScene::BuildTerrain.
	bool CreateStep1();
	bool CreateStep2(vtTransform *pSunLight);
	bool CreateStep3();
	bool CreateStep4();
	bool CreateStep5();
	vtString GetLastError() { return m_strErrorMsg; }

	/// Set the colors to be used in a derived texture.
	void SetTextureColors(ColorMap *colors);
	ColorMap *GetTextureColors() { return m_pTextureColors; }

	/// Sets the texture colors to be a set of black contour stripes.
	void SetTextureContours(float fInterval, float fSize);

	/** Override this method to customize the Dib, before it is turned into
	 * a vtImage.  The default implementation colors from elevation. */
	virtual void PaintDib();

	/// Return true if the terrain has been created.
	bool IsCreated();

	/// Set the enabled state of the terrain (whether it is shown or not).
	void Enable(bool bVisible);

	/// Load an external geometry file.
	vtTransform *LoadModel(const char *filename, bool bAllowCache = true);

	/// Add a model (or any node) to the terrain.
	void AddNode(vtNode *pNode);
	/// Remove a node from the terrain's scene graph.
	void RemoveNode(vtNode *pNode);

	/// Place a model on the terrain.
	void PlantModel(vtTransform *model);
	/// Place a model on the terrain at a specific point.
	void PlantModelAtPoint(vtTransform *model, const DPoint2 &pos);

	/// Test whether a given point is within the current terrain.
	bool PointIsInTerrain(const DPoint2 &p);

	// set global projection based on this terrain
	void SetGlobalProjection();

	// fences
	bool AddFence(vtFence3d *f);
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
	void DeleteStructureSet(unsigned int index);
	bool FindClosestStructure(const DPoint2 &point, double epsilon,
							  int &structure, double &closest, float fMaxInstRadius);
	bool AddNodeToStructGrid(vtTransform *pTrans);
	bool AddNodeToStructGrid(vtGeom *pGeom);
	void RemoveNodeFromStructGrid(vtNode *pNode);
	vtLodGrid *GetStructureGrid() { return m_pStructGrid; }

	// roads
	vtRoadMap3d *GetRoadMap() { return m_pRoadMap; }

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
	void SetVerticalExag(float fExag);
	float GetVerticalExag() { return m_fVerticalExag; }

	// query
	vtDynTerrainGeom *GetDynTerrain() { return m_pDynGeom; }
	vtTiledGeom *GetTiledGeom() { return m_pTiledGeom; }
	vtGroup *GetTopGroup() { return m_pTerrainGroup; }
	vtHeightField3d *GetHeightField();
	vtHeightFieldGrid3d *GetHeightFieldGrid3d();
	vtProjection &GetProjection() { return m_proj; }
	virtual bool FindAltitudeOnCulture(const FPoint3 &p3, float &fAltitude) const;

	// Points of interest
	void AddPointOfInterest(double left, double bottom, double right,
					   double top, const char *name, const char *url);
	vtPointOfInterest *FindPointOfInterest(DPoint2 utm);
	void SetShowPOI( bool flag ) { m_bShowPOI = flag; }
	bool GetShowPOI() { return m_bShowPOI; }
	void ShowPOI(vtPointOfInterest *poi, bool bShow);
	void HideAllPOI();

	// symbols and labels for abstract data
	float AddSurfaceLineToMesh(vtMeshFactory *pMF, const DLine2 &line,
		float fOffset, bool bInterp = true, bool bCurve = false, bool bTrue = false);
	void CreateStyledFeatures(const vtFeatureSet &feat,  const vtTagArray &style);
	void CreateFeatureGeometry(const vtFeatureSet &feat, const vtTagArray &style);
	void CreateFeatureLabels(const vtFeatureSet &feat,   const vtTagArray &style);

	// Access the viewpoint(s) associated with this terrain
	void SetCamLocation(FMatrix4 &mat) { m_CamLocation = mat; }
	FMatrix4 &GetCamLocation() { return m_CamLocation; }
	vtLocationSaver *GetLocSaver() { return &m_LocSaver; }
	void Visited(bool bVisited) { m_bVisited = bVisited; }
	bool IsVisited() { return m_bVisited; }

	// Access the animation paths associated with this terrain
	vtAnimContainer *GetAnimContainer() { return &m_AnimContainer; }

	// Sky and Fog
	void SetFog(bool fog);
	void SetFogColor(const RGBf &color);
	void SetFogDistance(float fMeters);
	bool GetFog() { return m_bFog; }
	void SetBgColor(const RGBf &color);
	RGBf GetBgColor() { return m_background_color; }

	// Time
	vtTime GetInitialTime();
	void TranslateToGMT(vtTime &time);
	void TranslateFromGMT(vtTime &time);
	DPoint2 GetCenterGeoLocation();

	// Overlay
	vtGroup *GetOverlay() { return m_pOverlay; }

	// Scenarios
	void ActivateScenario(int iScenario);

	/********************** Public Data ******************/

	// polygon containing geo corners of terrain area
	DLine2		m_Corners_geo;

	// Experimental!
	void RecreateTextures(vtTransform *pSunLight, bool progress_callback(int) = NULL);

protected:
	/********************** Protected Methods ******************/

	// internal creation functions
	bool CreateFromTIN();
	bool CreateFromGrid();
	bool CreateFromTiles();
	void _CreateCulture();
	void _CreateVegetation();
	void _CreateStructures();
	void _CreateRoads();
	void _SetupVegGrid(float fLODDistance);
	void _SetupStructGrid(float fLODDistance);
	void _CreateAbstractLayers();
	void _CreateTextures(const FPoint3 &light_dir, bool progress_callback(int) = NULL);
	void _CreateDetailTexture();
	bool _CreateDynamicTerrain();
	void _CreateErrorMessage(DTErr error, vtElevationGrid *pGrid);
	void _SetErrorMessage(const vtString &msg);
	void create_artificial_horizon(bool bWater, bool bHorizon,
		bool bCenter, float fTransparency);

	void _CreateChoppedTextures(int patches, int patch_size, bool progress_callback(int) = NULL);
	void _CreateTiledMaterials(vtMaterialArray *pMat1,
							 int patches, int patch_size, float ambient,
							 float diffuse, float emmisive);
	void _ApplyPreLight(vtHeightFieldGrid3d *pLocalGrid, vtBitmapBase *dib,
		const FPoint3 &light_dir, bool progress_callback(int) = NULL);
	void _ComputeCenterLocation();
	void GetTerrainBounds();

	/********************** Protected Data ******************/

	// main scene graph outline
	vtGroup		*m_pTerrainGroup;

	// dynamic terrain (CLOD)
	vtDynTerrainGeom *m_pDynGeom;
	vtTransform		 *m_pDynGeomScale;
	float			m_fVerticalExag;

	// triangulated irregular network (TIN)
	vtTin3d		*m_pTin;

	// tiled geometry
	vtTiledGeom	*m_pTiledGeom;

	// construction parameters used to create this terrain
	TParams		m_Params;

	// data grids
	vtElevationGrid	*m_pInputGrid;	// if non-NULL, use instead of BT
	vtHeightField3d	*m_pHeightField;
	bool			m_bPreserveInputGrid;

	// if we're switching between multiple terrains, we can remember where
	// the camera was in each one
	FMatrix4		m_CamLocation;
	bool			m_bVisited;

	// Associate a set of saved locations and animation paths with the terrain
	vtLocationSaver	m_LocSaver;
	vtAnimContainer m_AnimContainer;

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
	vtMaterialArray *m_pDetailMats;
	bool			m_bBothSides;

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
	vtImage			*m_pImage;
	Array<vtImage*>	m_Images;
	ColorMap		*m_pTextureColors;

	FSphere			m_bound_sphere;	// bounding sphere of terrain
									// (without surrounding ocean)
	RGBf			m_ocean_color;
	vtString		m_strParamFile;

	// contain the engines specific to this terrain
	vtEngine		*m_pEngineGroup;
	SimpleBillboardEngine	*m_pBBEngine;

	Array<POIPtr>	m_PointsOfInterest;
	bool			m_bShowPOI;
	vtGroup			*m_pPOIGroup;

	// only used during initialization
	vtElevationGrid	*m_pElevGrid;

	// A useful value for computing "local time", the location of the
	//  center of the terrain in Geographic coords.
	DPoint2			m_CenterGeoLocation;
	int m_iDifferenceFromGMT;

	// hold an informative message in case anything goes wrong
	vtString	m_strErrorMsg;

	vtGroup		*m_pOverlay;

	vtProjection	m_proj;
};

/*@}*/	// Group terrain

#endif	// TERRAINH
