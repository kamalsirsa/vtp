//
// Terrain.h
//
// Copyright (c) 2001-2012 Virtual Terrain Project
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
#include "Roads.h"
#include "Route.h"
#include "TextureUnitManager.h"
#include "TiledGeom.h"
#include "TParams.h"
#include "Plants3d.h"	// for vtSpeciesList3d, vtPlantInstanceArray3d
#include "vtTin3d.h"

#include <memory>

// Try to reduce compile-time dependencies with these forward declarations
class vtDIB;
class vtElevationGrid;
class vtFeatureSet;
class vtFence3d;
class vtLodGrid;
class vtPagedStructureLodGrid;
class vtSimpleLodGrid;
class vtExternalHeightField3d;

/** \addtogroup terrain */
/*@{*/


// Terrain Feature Types
enum TFType
{
	TFT_TERRAINSURFACE,
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

/// Type of a progress callback function
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
 * GetScaledFeatures()->addChild(), if they are 'flat' like GIS features
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
	void CreateStep1();
	bool CreateStep2();
	bool CreateStep3(vtTransform *pSunLight, vtLightSource *pLightSource);
	bool CreateStep4();
	bool CreateStep5();
	void CreateStep6();
	void CreateStep7();
	void CreateStep8();
	void CreateStep9();
	vtString GetLastError() { return m_strErrorMsg; }

	void SetProgressCallback(ProgFuncPtrType progress_callback = NULL);
	ProgFuncPtrType m_progress_callback;
	bool ProgressCallback(int i);

	/// Set the colors to be used in a derived texture.
	void SetTextureColors(ColorMap *colors);
	ColorMap *GetTextureColors() { return m_pTextureColors.get(); }

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
	void addNode(osg::Node *pNode);

	/// Remove a node from the terrain's scene graph.
	void removeNode(osg::Node *pNode);

	/// Place a model on the terrain.
	void PlantModel(vtTransform *model);
	/// Place a model on the terrain at a specific point.
	void PlantModelAtPoint(vtTransform *model, const DPoint2 &pos);

	/// Test whether a given point is within the current terrain.
	bool PointIsInTerrain(const DPoint2 &p);

	// set global projection based on this terrain
	void SetGlobalProjection();

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
	void SetActiveLayer(vtLayer *lay) { m_pActiveLayer = lay; }
	vtLayer *GetActiveLayer() { return m_pActiveLayer; }

	// plants
	vtVegLayer *GetVegLayer();
	vtVegLayer *NewVegLayer();
	bool AddPlant(const DPoint2 &pos, int iSpecies, float fSize);
	int DeleteSelectedPlants();
	void SetPlantList(vtSpeciesList3d *pPlantList);
	vtSpeciesList3d *GetPlantList() { return m_pPlantList; }
	/// Get the plant array for this terrain.  You can modify it directly.
	bool AddNodeToVegGrid(osg::Node *pNode);
	int NumVegLayers() const;
	bool FindClosestPlant(const DPoint2 &point, double epsilon,
						  int &plant_index, vtVegLayer **v_layer);

	// structures
	vtStructureLayer *GetStructureLayer();
	vtStructureLayer *NewStructureLayer();
	vtStructureLayer *LoadStructuresFromXML(const vtString &strFilename);
	void CreateStructures(vtStructureArray3d *structures);
	bool CreateStructure(vtStructureArray3d *structures, int index);
	int DeleteSelectedStructures();
	bool FindClosestStructure(const DPoint2 &point, double epsilon,
							  int &structure, vtStructureLayer **st_layer,
							  double &closest, float fMaxInstRadius,
							  float fLinearWidthBuffer);
	void DeselectAllStructures();

	bool AddNodeToStructGrid(osg::Node *pNode);
	void RemoveNodeFromStructGrid(osg::Node *pNode);

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
	vtAbstractLayer *GetAbstractLayer();
	void RemoveFeatureGeometries(vtAbstractLayer *alay);
	int DeleteSelectedFeatures();
	void SetFeatureLoader(vtFeatureLoader *loader) { m_pFeatureLoader = loader; }

	/// You should add your nodes to this terrain's scaled features if
	/// they are 'flat' like GIS features or contour lines, which should
	/// be scaled up/down with the vertical exaggeration of the terrain.
	vtTransform *GetScaledFeatures() { return m_pScaledFeatures; }

	// roads
	vtRoadMap3d *GetRoadMap() { return m_pRoadMap.get(); }

	// Terrain-specific content
	vtContentManager3d m_Content;

	// overridable by subclasses to extend culture
	virtual void CreateCustomCulture();

	// manage engines specific to this terrain
	void AddEngine(vtEngine *pE);
	void ActivateEngines(bool bActive);
	vtEngine *GetEngineGroup() { return m_pEngineGroup; }

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
	vtTiledGeom *GetTiledGeom() { return m_pTiledGeom.get(); }
	vtGroup *GetTopGroup() { return m_pContainerGroup; }
	vtGroup *GetTerrainGroup() { return m_pTerrainGroup; }
	vtHeightField3d *GetHeightField();
	vtHeightFieldGrid3d *GetHeightFieldGrid3d();
	vtProjection &GetProjection() { return m_proj; }
	virtual bool FindAltitudeOnCulture(const FPoint3 &p3, float &fAltitude, bool bTrue, int iCultureFlags) const;
	int GetShadowTextureUnit();

	// symbols and labels for abstract data
	float AddSurfaceLineToMesh(vtGeomFactory *pMF, const DLine2 &line,
		float fOffset, bool bInterp = true, bool bCurve = false, bool bTrue = false);

	// Access the viewpoint(s) associated with this terrain
	void SetCamLocation(FMatrix4 &mat);
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
	vtElevationGrid	*GetInitialGrid() { return m_pElevGrid.get(); }
	void UpdateElevation();
	void RedrapeCulture(const DRECT &area);

	// Texture
	void RecreateTextures(vtTransform *pSunLight, bool progress_callback(int) = NULL);
	osg::Image *GetTextureImage();
	vtMultiTexture *AddMultiTextureOverlay(vtImage *pImage, const DRECT &extents, int TextureMode);
	osg::Node *GetTerrainSurfaceNode();

	/********************** Public Data ******************/

	// polygon containing geo corners of terrain area
	DLine2		m_Corners_geo;

protected:
	/********************** Protected Methods ******************/

	// internal creation functions
	bool CreateFromTIN();
	bool CreateFromGrid();
	bool CreateFromTiles();
	bool CreateFromExternal();
	void _CreateOtherCulture();
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

	void CreateWaterPlane();
	void MakeWaterMaterial();
	void CreateWaterHeightfield(const vtString &fname);

	void _ApplyPreLight(vtHeightFieldGrid3d *pLocalGrid, vtBitmapBase *dib,
		const FPoint3 &light_dir, bool progress_callback(int) = NULL);
	void _ComputeCenterLocation();
	void GetTerrainBounds();
	void EnforcePageOut();
	void ConnectFogShadow(bool bFog, bool bShadow);

	/********************** Protected Data ******************/

	// main scene graph outline
	vtGroupPtr	m_pContainerGroup;
	vtGroupPtr	m_pTerrainGroup;
	vtGroupPtr	m_pUnshadowedGroup;

	// dynamic terrain (CLOD)
	vtDynTerrainGeomPtr m_pDynGeom;
	vtTransformPtr	m_pDynGeomScale;
	float			m_fVerticalExag;

	// triangulated irregular network (TIN)
	vtTin3dPtr		m_pTin;

	// tiled geometry
	vtTiledGeomPtr	m_pTiledGeom;

	osg::ref_ptr<vtExternalHeightField3d> m_pExternalHeightField;

	// construction parameters used to create this terrain
	TParams		m_Params;

	// elevation data
	vtHeightField3d	*m_pHeightField;
	bool			m_bPreserveInputGrid;

	// if we're switching between multiple terrains, we can remember where
	// the camera was in each one
	FMatrix4		m_CamLocation;
	bool			m_bVisited;

	// Associate a set of saved locations and animation paths with the terrain
	vtLocationSaver	m_LocSaver;
	vtAnimContainer m_AnimContainer;

	// Ocean, shadow and fog
	vtFogPtr	m_pFog;
	vtShadowPtr	m_pShadow;
	vtMovGeodePtr m_pOceanGeom;
	bool		m_bFog;
	RGBf		m_fog_color;
	RGBf		m_background_color;
	bool		m_bShadows;
	vtTin3dPtr	m_pWaterTin3d;

	// Layers
	LayerSet		m_Layers;
	vtLayer			*m_pActiveLayer;

	// built structures, e.g. buildings and fences
	vtLodGrid		*m_pStructGrid;
	vtPagedStructureLodGrid		*m_pPagedStructGrid;
	int		m_iPagingStructureMax;
	float	m_fPagingStructureDist;

	vtMaterialArrayPtr m_pTerrMats;		// materials for the LOD terrain
	vtMaterialArrayPtr m_pDetailMats;	// and detail texture
	vtMaterialArrayPtr m_pEphemMats;	// and ephemeris
	int				m_idx_water;
	bool			m_bBothSides;	// show both sides of terrain materials

	// abstract layers
	vtTransform		*m_pScaledFeatures;
	vtFeatureLoader *m_pFeatureLoader;

	// roads
	vtGroup			*m_pRoadGroup;
	vtRoadMap3dPtr	m_pRoadMap;

	// plants
	vtSpeciesList3d	*m_pPlantList;
	vtGroup			*m_pVegGroup;
	vtSimpleLodGrid	*m_pVegGrid;

	// routes
	vtRouteMap		m_Routes;

	// ground texture and shadows
	ImagePtr		m_pUnshadedImage;
	ImagePtr		m_pSingleImage;

	auto_ptr<ColorMap>	m_pTextureColors;
	bool			m_bTextureInitialized;
	vtTextureUnitManager m_TextureUnits;
	int				m_iShadowTextureUnit;
	vtLightSource	*m_pLightSource;

	FSphere			m_bound_sphere;	// bounding sphere of terrain
									// (without surrounding ocean)
	RGBf			m_ocean_color;
	vtString		m_strParamFile;

	// contain the engines specific to this terrain
	vtEnginePtr		m_pEngineGroup;

	// only used during initialization
	auto_ptr<vtElevationGrid>	m_pElevGrid;

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
