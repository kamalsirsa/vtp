//
// Terrain.h
//
// Copyright (c) 2001-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TERRAINH
#define TERRAINH

#include "vtdata/vtTime.h"
#include "vtdata/FilePath.h"

#include "AbstractLayer.h"
#include "AnimPath.h"	// for vtAnimContainer
#include "Content3d.h"
#include "DynTerrain.h"
#include "GeomUtil.h"	// for MeshFactory
#include "Location.h"
#include "Route.h"
#include "TextureUnitManager.h"
#include "TParams.h"
#include "Trees.h"	// for vtSpeciesList3d, vtPlantInstanceArray3d

// Try to reduce compile-time dependencies with these forward declarations
class vtDIB;
class vtElevationGrid;
class vtFeatureSet;
class vtFence3d;
class vtLodGrid;
class vtPagedStructureLodGrid;
class vtRoadMap3d;
class vtSimpleBillboardEngine;
class vtSimpleLodGrid;
class vtTiledGeom;
class vtTin;
class vtTin3d;

/** \addtogroup terrain */
/*@{*/


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
	TFT_HORIZON,
	TFT_OCEAN,
	TFT_VEGETATION,
	TFT_STRUCTURES,
	TFT_ROADS
};

class vtStructureExtension
{
public:
	virtual void OnCreate(vtTerrain *t, vtStructure *s) {}
	virtual void OnDelete(vtTerrain *t, vtStructure *s) {}
};

struct vtShadowOptions
{
	float fDarkness;
	bool bShadowsEveryFrame;
	bool bShadowLimit;
	float fShadowRadius;
};

typedef bool (*ProgFuncPtrType)(int);

/**
 * The vtTerrain class represents a terrain, which is a part of the surface
 *  of the earth.
 *
 * It is generally described by a set of parameters such as elevation,
 * vegetation, and time of day.  These terrain parameters are contained
 * in the class TParams.
 *
 * To create a new terrain, first construct a vtTerrain and set its
 * parameters with SetParams() or SetParamFile().
 * You can also set many properties of the terrain directly, which is useful
 * if you want to set them from memory instead of from disk.  These include:
	- Elevation grid: use SetLocalGrid().
	- Elevation TIN: use SetTin().
	- Structures: use NewStructureLayer(), then fill it with your structures.
	- Vegetation: call SetPlantList(), then GetPlantInstances().
	- Abstract layers: use GetLayers() to get the LayerSet, then create and append
	  your vtAbstractLayer objects. The features will be created
	  according to the properties you have set with vtAbstractLayer::SetProperties().
	  The properties you can set are documented with the class TParams.
	- Animation paths: use GetAnimContainer(), then add your own animpaths.
 *
 * You can then build the terrain using the CreateStep methods, or add it
 * to a vtTerrainScene and use vtTerrainScene::BuildTerrain.
 *
 * <h3>Customizing your Terrain</h3>
 *
 * To extend your terrain beyond what is possible with the terrain
 * parameters, you can create a subclass of vtTerrain and implement the
 * method CreateCustomCulture().  Here you can create anything you like,
 * and add it to the terrain.  Generally you should add your nodes with
 * AddNode(), or AddNodeToStructGrid() if it is a structure that should
 * be culled in the distance.  You can also add your nodes with
 * GetScaledFeatures()->AddChild(), if they are 'flat' like GIS features
 * or contour lines, which should be scaled up/down with the vertical
 * exaggeration of the terrain.
 *
 * <h3>How it works</h3>
 *
 * The terrain is implemented with a scene graph with the following structure.
 * - vtGroup "Terrain Group"
 *  - vtTransform "Dynamic Geometry"
 *   - A vtDynGeom for the CLOD terrain surface.
 *  - vtTransform "Scaled Features"
 *   - Abstract layers, which may need scaling in order to remain draped
 *		on the ground as the user changes the vertical exaggeration.
 *  - vtGroup "Structure LOD Grid"
 *   - many vtLOD objects, for efficient culling of large number of structures.
 *  - vtGroup "Roads"
 *   - many vtLOD objects, for efficient culling of large number of roads.
 *  - vtGroup "Vegetation"
 *   - many vtLOD objects, for efficient culling of large number of plants.
 */
class vtTerrain : public CultureExtension
{
public:
	vtTerrain();
	virtual ~vtTerrain();

	/********************** Public Methods ******************/

	// parameters for terrain creation
	void SetParamFile(const char *fname);
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
	void CreateStep0();
	bool CreateStep1();
	bool CreateStep2(vtTransform *pSunLight);
	bool CreateStep3();
	bool CreateStep4();
	bool CreateStep5();
	vtString GetLastError() { return m_strErrorMsg; }

	void SetProgressCallback(ProgFuncPtrType progress_callback = NULL);
	ProgFuncPtrType m_progress_callback;
	bool ProgressCallback(int i);

	/// Set the colors to be used in a derived texture.
	void SetTextureColors(ColorMap *colors);
	ColorMap *GetTextureColors() { return m_pTextureColors; }

	/// Sets the texture colors to be a set of black contour stripes.
	void SetTextureContours(float fInterval, float fSize);

	/** Override this method to customize the Dib, before it is turned into
	 * a vtImage.  The default implementation colors from elevation. */
	virtual void PaintDib(bool progress_callback(int) = NULL);

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

	// Layers
	/// Get at the container for all the layers
	LayerSet &GetLayers() { return m_Layers; }
	void RemoveLayer(vtLayer *lay, bool progress_callback(int) = NULL);
	vtLayer *LoadLayer(const char *fname);

	// plants
	bool AddPlant(const DPoint2 &pos, int iSpecies, float fSize);
	int DeleteSelectedPlants();
	void SetPlantList(vtSpeciesList3d *pPlantList);
	vtSpeciesList3d *GetPlantList() { return m_pPlantList; }
	/// Get the plant array for this terrain.  You can modify it directly.
	vtPlantInstanceArray3d &GetPlantInstances() { return m_PIA; }
	bool AddNodeToVegGrid(vtTransform *pTrans);

	// structures
	vtStructureLayer *GetStructureLayer();
	void SetStructureLayer(vtStructureLayer *slay);
	vtStructureLayer *NewStructureLayer();
	vtStructureLayer *LoadStructuresFromXML(const vtString &strFilename);
	void CreateStructures(vtStructureArray3d *structures);
	bool CreateStructure(vtStructureArray3d *structures, int index);
	int DeleteSelectedStructures();
	void DeleteLayer(unsigned int index);
	bool FindClosestStructure(const DPoint2 &point, double epsilon,
							  int &structure, double &closest, float fMaxInstRadius,
							  float fLinearWidthBuffer);
	bool AddNodeToStructGrid(vtTransform *pTrans);
	bool AddNodeToStructGrid(vtGeom *pGeom);
	void RemoveNodeFromStructGrid(vtNode *pNode);
	vtLodGrid *GetStructureGrid() { return m_pStructGrid; }
	int DoStructurePaging();
	vtPagedStructureLodGrid *GetStructureLodGrid() { return m_pPagedStructGrid; }
	float GetStructurePageOutDistance() { return m_fPagingStructureDist; }
	void SetStructurePageOutDistance(float f);
	int GetStructurePageMax() { return m_iPagingStructureMax; }

	void ExtendStructure(vtStructure *s);
	void SetStructureExtension(vtStructureExtension *se = NULL) { m_pStructureExtension = se; }
	vtStructureExtension *m_pStructureExtension;

	// abstract layers
	void SetAbstractLayer(vtAbstractLayer *alay);
	vtAbstractLayer *GetAbstractLayer();
	void RemoveFeatureGeometries(vtAbstractLayer *alay);
	int DeleteSelectedFeatures();
	void SetFeatureLoader(vtFeatureLoader *loader) { m_pFeatureLoader = loader; }

	/// You should add your nodes to this terrain's scaled features if
	/// they are 'flat' like GIS features or contour lines, which should
	/// be scaled up/down with the vertical exaggeration of the terrain.
	vtTransform *GetScaledFeatures() { return m_pScaledFeatures; }

	// roads
	vtRoadMap3d *GetRoadMap() { return m_pRoadMap; }

	// Terrain-specific content
	vtContentManager3d m_Content;

	// overridable by subclasses to extend culture
	virtual void CreateCustomCulture();

	// manage engines specific to this terrain
	void AddEngine(vtEngine *pE);
	void ActivateEngines(bool bActive);
	vtEngine *GetEngineGroup() { return m_pEngineGroup; }
	vtSimpleBillboardEngine	*GetBillboardEngine() { return m_pBBEngine; }

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
	float GetVerticalExag() const { return m_fVerticalExag; }

	// query
	vtDynTerrainGeom *GetDynTerrain() { return m_pDynGeom; }
	const vtDynTerrainGeom *GetDynTerrain() const { return m_pDynGeom; }
	vtTiledGeom *GetTiledGeom() { return m_pTiledGeom; }
	vtGroup *GetTopGroup() { return m_pContainerGroup; }
	vtGroup *GetTerrainGroup() { return m_pTerrainGroup; }
	vtHeightField3d *GetHeightField();
	vtHeightFieldGrid3d *GetHeightFieldGrid3d();
	vtProjection &GetProjection() { return m_proj; }
	virtual bool FindAltitudeOnCulture(const FPoint3 &p3, float &fAltitude, bool bTrue, int iCultureFlags) const;
	int GetShadowTextureUnit();


	// symbols and labels for abstract data
	float AddSurfaceLineToMesh(vtMeshFactory *pMF, const DLine2 &line,
		float fOffset, bool bInterp = true, bool bCurve = false, bool bTrue = false);

	// Access the viewpoint(s) associated with this terrain
	void SetCamLocation(FMatrix4 &mat) { m_CamLocation = mat; }
	FMatrix4 &GetCamLocation() { return m_CamLocation; }
	vtLocationSaver *GetLocSaver() { return &m_LocSaver; }
	void Visited(bool bVisited) { m_bVisited = bVisited; }
	bool IsVisited() { return m_bVisited; }

	/// Access the animation paths associated with this terrain
	vtAnimContainer *GetAnimContainer() { return &m_AnimContainer; }

	// Ocean, Sky, Fog, Shadows
	void SetWaterLevel(float fElev);
	void SetFog(bool fog);
	bool GetFog() { return m_bFog; }
	void SetFogColor(const RGBf &color);
	void SetFogDistance(float fMeters);
	void SetBgColor(const RGBf &color);
	RGBf GetBgColor() { return m_background_color; }
	void SetShadows(bool shadows);
	bool GetShadows() { return m_bShadows; }
	void SetShadowOptions(const vtShadowOptions &opt);
	void GetShadowOptions(vtShadowOptions &opt);
	void ForceShadowUpdate();

	// Time
	vtTime GetInitialTime();
	void TranslateToGMT(vtTime &time);
	void TranslateFromGMT(vtTime &time);
	DPoint2 GetCenterGeoLocation();

	// Overlay (2D image on HUD)
	vtGroup *GetOverlay() { return m_pOverlay; }

	// Scenarios
	void ActivateScenario(int iScenario);

	// Dynamic elevation
	vtElevationGrid	*GetInitialGrid() { return m_pElevGrid; }
	void UpdateElevation();
	void RedrapeCulture(const DRECT &area);

	/********************** Public Data ******************/

	// polygon containing geo corners of terrain area
	DLine2		m_Corners_geo;

	// Texture
	void RecreateTextures(vtTransform *pSunLight, bool progress_callback(int) = NULL);
	vtImage *GetTextureImage();
	vtOverlappedTiledImage	*GetOverlappedImage() { return &m_ImageTiles; }
	vtMultiTexture *AddMultiTextureOverlay(vtImage *pImage, const DRECT &extents, int TextureMode);
	vtNode *GetTerrainSurfaceNode();

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
	void _CreateImageLayers();
	void _CreateTextures(const FPoint3 &light_dir, bool progress_callback(int) = NULL);
	void _CreateDetailTexture();
	bool _CreateDynamicTerrain();
	void _CreateErrorMessage(DTErr error, vtElevationGrid *pGrid);
	void _SetErrorMessage(const vtString &msg);
	void CreateArtificialHorizon(float fAltitude, bool bWater, bool bHorizon,
		bool bCenter, float fTransparency);

	void _CreateSingleMaterial(float ambient, float diffuse, float emmisive);
	void _CreateTiledMaterials(int patches, float ambient, float diffuse, float emmisive);
	void _ApplyPreLight(vtHeightFieldGrid3d *pLocalGrid, vtBitmapBase *dib,
		const FPoint3 &light_dir, bool progress_callback(int) = NULL);
	void _ComputeCenterLocation();
	void GetTerrainBounds();
	void EnforcePageOut();
	void ConnectFogShadow(bool bFog, bool bShadow);

	/********************** Protected Data ******************/

	// main scene graph outline
	vtGroup		*m_pContainerGroup;
	vtGroup		*m_pTerrainGroup;
	vtGroup		*m_pUnshadowedGroup;

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

	// elevation data
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

	// horizon, ocean and fog
	vtFog		*m_pFog;
	vtShadow	*m_pShadow;
	vtMovGeom	*m_pHorizonGeom;
	vtMovGeom	*m_pOceanGeom;
	bool		m_bFog;
	RGBf		m_fog_color;
	RGBf		m_background_color;
	bool		m_bShadows;

	// Layers
	LayerSet		m_Layers;

	// built structures, e.g. buildings and fences
	vtStructureLayer *m_pActiveStructLayer;
	vtLodGrid		*m_pStructGrid;
	vtPagedStructureLodGrid		*m_pPagedStructGrid;
	int		m_iPagingStructureMax;
	float	m_fPagingStructureDist;

	vtMaterialArray	*m_pTerrMats;	// materials for the LOD terrain
	vtMaterialArray *m_pDetailMats;
	bool			m_bBothSides;

	// abstract layers
	vtAbstractLayer *m_pActiveAbstractLayer;
	vtTransform		*m_pScaledFeatures;
	vtFeatureLoader *m_pFeatureLoader;

	// roads
	vtGroup			*m_pRoadGroup;
	vtRoadMap3d		*m_pRoadMap;

	// plants
	vtPlantInstanceArray3d	m_PIA;
	vtSpeciesList3d	*m_pPlantList;
	vtGroup			*m_pVegGroup;
	vtSimpleLodGrid	*m_pVegGrid;

	// routes
	vtRouteMap		m_Routes;

	// ground texture
	vtImage			*m_pImageSource;
	vtImage			*m_pImage;
	vtOverlappedTiledImage	m_ImageTiles;
	ColorMap		*m_pTextureColors;
	bool			m_bTextureInitialized;
	vtTextureUnitManager m_TextureUnits;
	int				m_iShadowTextureUnit;

	FSphere			m_bound_sphere;	// bounding sphere of terrain
									// (without surrounding ocean)
	RGBf			m_ocean_color;
	vtString		m_strParamFile;

	// contain the engines specific to this terrain
	vtEngine		*m_pEngineGroup;
	vtSimpleBillboardEngine	*m_pBBEngine;

	vtArray<POIPtr>	m_PointsOfInterest;
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
	bool			m_bIsCreated;
};

/*@}*/	// Group terrain

#endif	// TERRAINH
