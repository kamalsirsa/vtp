//
// Terrain.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include "vtdata/vtLog.h"
#include "vtdata/Features.h"

#include "Terrain.h"
#include "Light.h"
#include "Building3d.h"
#include "Building3d.h"
#include "vtdata/StructArray.h"
#include "IntersectionEngine.h"
#include "Fence3d.h"
#include "Route.h"
#include "vtTin3d.h"

//#include "LKTerrain.h"
#include "TVTerrain.h"
#include "SMTerrain.h"
#include "CustomTerrain.h"
#include "BryanTerrain.h"
#include "SRTerrain.h"
#include "DemeterTerrain.h"
// add your own LOD method header here!

#define LARGEST_BLOCK_SIZE	16

// use a grid of LOD cells of size LOD_GRIDSIZE x LOD_GRIDSIZE
#define LOD_GRIDSIZE		192

///////////////////////////////////////////////////////////////////////

// All terrains share a static data path and content manager
vtStringArray vtTerrain::s_DataPaths;
vtContentManager3d vtTerrain::s_Content;

//////////////////////////////////////////////////////////////////////

vtTerrain::vtTerrain()
{
	m_ocean_color.Set(40.0f/255, 75.0f/255, 124.0f/255);	// unshaded color
	m_fog_color.Set(-1.0f, -1.0f, -1.0f);

	m_pTerrainGroup = (vtGroup*) NULL;
	m_pDIB = NULL;
	m_pTerrMats = NULL;
	m_pRoadMap = NULL;
	m_pInputGrid = NULL;
	m_pHeightField = NULL;
	m_bPreserveInputGrid = false;
	m_pElevGrid = NULL;

	m_pOceanGeom = NULL;
	m_pRoadGroup = NULL;

	// vegetation
	m_pVegGrid = NULL;
	m_pPlantList = NULL;
	m_pBBEngine = NULL;

	m_bShowPOI = true;
	m_pPOIGroup = NULL;

	m_pDynGeom = NULL;
	m_pDynGeomScale = NULL;
	m_pTin = NULL;
	m_pNext = NULL;

	// structures
	m_iStructSet = 0;
	m_pStructGrid = NULL;

	m_CamLocation.Identity();

	m_fCenterLongitude = -999;	// initially unknown
}

vtTerrain::~vtTerrain()
{
	// some things need to be manually deleted
	int i, size = m_PointsOfInterest.GetSize();
	for (i = 0; i < size; i++)
	{
		POIPtr p = m_PointsOfInterest.GetAt(i);
		delete p;
	}

	size = m_StructureSet.GetSize();
	for (i = 0; i < size; i++)
	{
		vtStructureArray3d *sa3d = m_StructureSet[i];
		delete sa3d;
	}

	// Do not delete the PlantList, the application may be sharing the same
	// list with several different terrains.
//	delete m_pPlantList;

	if (!m_bPreserveInputGrid)
	{
		delete m_pElevGrid;
	}
	delete m_pDIB;
	delete m_pRoadMap;
	if (m_pRoadGroup)
	{
		m_pTerrainGroup->RemoveChild(m_pRoadGroup);
		m_pRoadGroup->Release();
	}
	if (m_pOceanGeom)
	{
		m_pTerrainGroup->RemoveChild(m_pOceanGeom);
		m_pOceanGeom->Release();
	}
	if (m_pStructGrid)
	{
		m_pTerrainGroup->RemoveChild(m_pStructGrid);
		m_pStructGrid->Release();
	}
	if (m_pVegGrid)
	{
		m_pTerrainGroup->RemoveChild(m_pVegGrid);
		m_pVegGrid->Release();
	}
//	delete m_pInputGrid;	// don't delete, copied to m_pElevGrid
	if (m_pDynGeom)
	{
		m_pDynGeomScale->RemoveChild(m_pDynGeom);
		m_pDynGeom->Release();
	}
	if (m_pDynGeomScale)
	{
		m_pTerrainGroup->RemoveChild(m_pDynGeomScale);
		m_pDynGeomScale->Release();
	}

	delete m_pTin;

	if (m_pTerrainGroup != (vtGroup*) NULL)
		m_pTerrainGroup->Release();

	if (m_pTerrMats)
		m_pTerrMats->Release();

	if (m_pBBEngine)
	{
		vtGetScene()->RemoveEngine(m_pBBEngine);
		delete m_pBBEngine;
	}
}


///////////////////////////////////////////////////////////////////////

bool vtTerrain::SetParamFile(const char *fname)
{
	m_strParamFile = fname;
	return LoadParams();
}

bool vtTerrain::LoadParams()
{
	TParams params;
	bool success = params.LoadFromFile(m_strParamFile);
	if (success)
		SetParams(params);
	return success;
}

/**
 * This method allows you to give the terrain a grid to use directly
 * instead of loading the BT file specified in the TParams.
 *
 * You must allocate this grid dynamically with 'new', since vtTerrain
 * will 'delete' it after using it during initialization.  If you don't
 * want the memory to be deleted, pass 'true' for bPreserve.
 *
 * \param pGrid The grid object which the terrain should use.
 * \param bPreserve True if the terrain should not delete the grid object,
 * otherwise false.
 *
 */
void vtTerrain::SetLocalGrid(vtElevationGrid *pGrid, bool bPreserve)
{
	m_pInputGrid = pGrid;
	m_bPreserveInputGrid = bPreserve;
}

/**
 * This method allows you to give the terrain a TIN to use directly
 * instead of loading a .tin file as specified in the TParams.
 */
void vtTerrain::SetTin(vtTin3d *pTin)
{
	m_pTin = pTin;
}


///////////////////////////////////////////////////////////////////////

void vtTerrain::create_roads(const vtString &strRoadFile)
{
	VTLOG("Creating Roads: ");
	m_pRoadMap = new vtRoadMap3d();

	VTLOG("  Reading from file '%s'\n", (const char *) strRoadFile);
	bool success = m_pRoadMap->ReadRMF(strRoadFile,
		m_Params.m_bHwy != 0, m_Params.m_bPaved != 0, m_Params.m_bDirt != 0);
	if (!success)
	{
		VTLOG("	read failed.\n");
		delete m_pRoadMap;
		m_pRoadMap = NULL;
		return;
	}

	//some nodes may not have any roads attached to them.  delete them.
	m_pRoadMap->RemoveUnusedNodes();

	m_pRoadMap->DetermineSurfaceAppearance();

	m_pRoadMap->SetHeightOffGround(m_Params.m_fRoadHeight);
	m_pRoadMap->DrapeOnTerrain(m_pHeightField);
	m_pRoadMap->BuildIntersections();

	m_pRoadMap->SetLodDistance(m_Params.m_fRoadDistance * 1000);	// convert km to m
	m_pRoadGroup = m_pRoadMap->GenerateGeometry(m_Params.m_bTexRoads != 0,
		s_DataPaths);
	m_pTerrainGroup->AddChild(m_pRoadGroup);

	if (m_Params.m_bRoadCulture)
		m_pRoadMap->GenerateSigns(m_pStructGrid);
}


///////////////////

void vtTerrain::_CreateTextures(const vtTime &time)
{
	int iTiles = 4;		// fixed for now
	TextureEnum eTex = m_Params.m_eTexture;

	if (!m_pTerrMats)
		m_pTerrMats = new vtMaterialArray();
	m_pTerrMats->Empty();

	float ambient, diffuse, emmisive;
	diffuse = 1.0f;
	ambient = emmisive = 0.0f;

	if (eTex == TE_SINGLE || eTex == TE_TILED)	// load texture
	{
		vtString texture_fname = "GeoSpecific/";
		if (m_Params.m_eTexture == TE_SINGLE)
			texture_fname += m_Params.m_strTextureSingle;	// single texture
		else
			texture_fname += m_Params.m_strTextureFilename;

		VTLOG("  Texture: %s\n", (const char *) texture_fname);
		vtString texture_path = FindFileOnPaths(s_DataPaths, texture_fname);
		if (texture_path == "")
		{
			// failed to find texture
			VTLOG("  Failed to find texture.\n");
		}
		else
		{
			VTLOG("  Found texture, path is: %s\n", (const char *) texture_path);
			// Load a DIB of the whole, large texture
			m_pDIB = new vtDIB();
			bool result = m_pDIB->Read(texture_path);
			if (! result)
			{
				VTLOG("  Failed to load texture.\n");
				m_pTerrMats->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
											RGBf(0.2f, 0.2f, 0.2f),
											true, false);
				m_Params.m_eTexture = TE_NONE;
			}
			if (eTex == TE_SINGLE)
			{
				// TODO: check that DIB size is power of two, and warn if not
			}
		}
	}

	vtHeightFieldGrid3d *pHFGrid = GetHeightFieldGrid3d();

	if (eTex == TE_DERIVED)
	{
		// Determine the correct size for the derived texture: ideally
		// as large as the input grid, but not larger than the hardware
		// texture size limit.
		GLint tmax = 0;	// TODO: cannot make direct GL calls in threaded environment
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tmax);

		int cols, rows;
		pHFGrid->GetDimensions(cols, rows);

		int tsize = cols-1;
		if ((tmax > 0) && (tsize > tmax))
			tsize = tmax;
		VTLOG("\t grid width is %d, texture max is %d, creating artificial texture of dimension %d\n",
			cols, tmax, tsize);

		// derive color from elevation
		m_pDIB = new vtDIB();
		m_pDIB->Create(tsize, tsize, 24, false);
		pHFGrid->ColorDibFromElevation(m_pDIB, RGBi(m_ocean_color));
	}

	// apply pre-lighting (darkening)
	if (m_Params.m_bPreLight && m_pDIB)
	{
		_ApplyPreLight(pHFGrid, m_pDIB, time);
	}

	if (eTex == TE_SINGLE || eTex == TE_DERIVED)
	{
		if (m_pDIB != NULL)
		{
			// single texture
			vtImage *pImage = new vtImage(m_pDIB,
				(m_pDIB->GetDepth() > 8 && m_Params.m_b16bit) ? GL_RGB5 : -1);
			m_Images.Append(pImage);
		}
	}
	if (eTex == TE_TILED && m_pDIB)
	{
		_CreateChoppedTextures(iTiles, m_Params.m_iTilesize);
		_CreateTiledMaterials(m_pTerrMats,
						 iTiles, m_Params.m_iTilesize, ambient, diffuse,
						 emmisive);
	}
	if (eTex == TE_NONE || m_pDIB == NULL)	// none or failed to find texture
	{
		// no texture: create plain white material
		m_pTerrMats->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
									 RGBf(0.2f, 0.2f, 0.2f),
									 true, false);
		return;
	}
	// We're not going to use it anymore, so we're done with the DIB
	if (m_pDIB != NULL)
	{
		delete m_pDIB;
		m_pDIB = NULL;
	}
	if (eTex == TE_SINGLE || eTex == TE_DERIVED)
	{
		vtImage *pImage = m_Images[0];
		m_pTerrMats->AddTextureMaterial(pImage,
			true,		// culling
			false,		// lighting
			false,		// transparent
			false,		// additive
			ambient, diffuse,
			1.0f,		// alpha
			emmisive,
			true,		// texgen
			false,		// clamp
			m_Params.m_bMipmap);
	}
	// All texture images have now been passed to their materials, so don't hold on to them.
	int i, num = m_Images.GetSize();
	for (i = 0; i < num; i++)
		m_Images[i]->Release();
	m_Images.Empty();
}

/**
 * Experimental only!!!
 */
void vtTerrain::recreate_textures(const vtTime &time)
{
	_CreateTextures(time);
}


/////////////////////

bool vtTerrain::create_dynamic_terrain(float fOceanDepth, int &iError)
{
	int texture_patches;
	if (m_Params.m_eTexture == TE_TILED)
		texture_patches = 4;	// tiled, which is always 4x4
	else
		texture_patches = 1;	// assume one texture

	VTLOG(" LOD method %d\n", m_Params.m_eLodMethod);
//	if (m_Params.m_eLodMethod == LM_LINDSTROMKOLLER)
//	{
//		m_pDynGeom = new LKTerrain();
//		m_pDynGeom->SetName2("LK Geom");
//	}
	if (m_Params.m_eLodMethod == LM_TOPOVISTA)
	{
		m_pDynGeom = new TVTerrain();
		m_pDynGeom->SetName2("TV Geom");
	}
	else if (m_Params.m_eLodMethod == LM_MCNALLY)
	{
		m_pDynGeom = new SMTerrain();
		m_pDynGeom->SetName2("Seumas Geom");
	}
	else if (m_Params.m_eLodMethod == LM_DEMETER)
	{
#if 0	// disabled until its working
		m_pDynGeom = new DemeterTerrain();
		m_pDynGeom->SetName2("Demeter Geom");
#endif
	}
	else if (m_Params.m_eLodMethod == LM_CUSTOM)
	{
		m_pDynGeom = new CustomTerrain();
		m_pDynGeom->SetName2("CustomTerrain Geom");
	}
	else if (m_Params.m_eLodMethod == LM_BRYANQUAD)
	{
		m_pDynGeom = new BryanTerrain();
		m_pDynGeom->SetName2("BryanQuad Geom");
	}
	else if (m_Params.m_eLodMethod == LM_ROETTGER)
	{
		m_pDynGeom = new SRTerrain();
		m_pDynGeom->SetName2("Roettger Geom");
	}
	if (!m_pDynGeom)
	{
		VTLOG(" Could not construct CLOD\n");
		return false;
	}

	// add your own LOD method here!

	m_pDynGeom->SetOptions(m_Params.m_bTriStrips != 0,
		texture_patches, m_Params.m_iTilesize);

	bool result = m_pDynGeom->Init(m_pElevGrid,
				   m_Params.m_fVerticalExag, fOceanDepth, iError);
	if (result == false)
	{
		m_pDynGeom->Release();
		m_pDynGeom = NULL;
		VTLOG(" Could not initialize CLOD\n");
		return false;
	}

	m_pDynGeom->SetPixelError(m_Params.m_fPixelError);
	m_pDynGeom->SetPolygonCount(m_Params.m_iTriCount);
	m_pDynGeom->SetMaterials(m_pTerrMats);

	// build heirarchy (add terrain to scene graph)
	m_pDynGeomScale = new vtTransform();
	m_pDynGeomScale->SetName2("Dynamic Geometry Container");

	FPoint2 spacing = m_pElevGrid->GetWorldSpacing();
	m_pDynGeomScale->Scale3(spacing.x, m_Params.m_fVerticalExag, -spacing.y);

	m_pDynGeomScale->AddChild(m_pDynGeom);
	m_pTerrainGroup->AddChild(m_pDynGeomScale);

	return true;
}


void vtTerrain::AddFence(vtFence3d *fen)
{
	GetStructures()->Append(fen);
	fen->CreateNode(this);

	// Add to LOD grid
	AddNodeToStructGrid(fen->GetGeom());
}

void vtTerrain::AddFencepoint(vtFence3d *f, const DPoint2 &epos)
{
	// Adding a fence point might change the fence extents such that it moves
	// to a new LOD cell.  So, remove it from the LOD grid, add the point,
	// then add it back.
	m_pStructGrid->RemoveFromGrid(f->GetGeom());

	f->AddPoint(epos);

	f->CreateNode(this);

	AddNodeToStructGrid(f->GetGeom());
}

void vtTerrain::RedrawFence(vtFence3d *f)
{
	f->CreateNode(this);
}

// routes
void vtTerrain::AddRoute(vtRoute *f)
{
	m_Routes.Append(f);

	// Add directly
	m_pTerrainGroup->AddChild(f->GetGeom());

	// Or add to LOD grid (much more efficient)
//	AddNodeToLodGrid(f->GetGeom());
}

void vtTerrain::add_routepoint_earth(vtRoute *route, const DPoint2 &epos,
									 const char *structname)
{
	route->AddPoint(epos, structname);
	route->BuildGeometry(m_pHeightField);
}

void vtTerrain::RedrawRoute(vtRoute *route)
{
	route->BuildGeometry(m_pHeightField);
}

void vtTerrain::SaveRoute()
{
}

/**
 * Create a horizontal plane at sea level.
 *
 * If the terrain has a large body of water on 1 or more sides, this method
 * is useful for extending the water to the horizon by creating additional
 * ocean plane geometry.
 *
 * \param bWater : true for a watery material, false for a land material
 * \param bHorizon : if true, create tiles extending from the terrain extents
 *		to the horizon.
 * \param bCenter : if true, create a tile in the center (covering the terrain
 *		extents).
 * \param fTransparency : a value from 0 (tranparent) to 1 (opaque)
 */
void vtTerrain::create_artificial_horizon(bool bWater, bool bHorizon,
										  bool bCenter, float fTransparency)
{
	int VtxType;

	vtMaterialArray *pMat_Ocean = new vtMaterialArray();

	if (bWater)
	{
		// create ocean material: texture waves
		vtString fname = FindFileOnPaths(s_DataPaths, "GeoTypical/ocean1_256.jpg");
		pMat_Ocean->AddTextureMaterial2(fname,
			false, false,		// culling, lighting
			false,				// the texture itself has no alpha
			false,				// additive
			TERRAIN_AMBIENT,	// ambient
			1.0f,				// diffuse
			fTransparency,		// alpha
			TERRAIN_EMISSIVE,	// emissive
			false,				// texgen
			false,				// clamp
			false);				// don't mipmap: allowing texture aliasing to
								// occur, it actually looks more water-like
		VtxType = VT_Normals | VT_TexCoords;
	}
	else
	{
		pMat_Ocean->AddRGBMaterial1(RGBf(1.0f, 0.8f, 0.6f),	// tan ground
			false, true, false);		// cull, light, wire
		VtxType = VT_Normals;
	}

	vtGeom *pGeom = new vtGeom();
	pGeom->SetMaterials(pMat_Ocean);
	pMat_Ocean->Release();

	float width, depth;
	int i, j;

	FRECT world_extents = m_pHeightField->m_WorldExtents;
	FPoint2 world_size(world_extents.Width(), world_extents.Height());

	width = (float) world_size.x;
	depth = (float) world_size.y;
	FPoint3 base, size(width, 0, depth);

	for (i = -3; i < 4; i++)
	{
		for (j = -3; j < 4; j++)
		{
			// don't need to do far corners
			if ((i == -3 || i == 3) && (j == -3 || j == 3)) continue;
			if (i == 0 && j == 0) {
				// center tile
				if (!bCenter) continue;
			}
			else {
				if (!bHorizon) continue;
			}

			base.x = world_extents.left + (i * width);
			base.y = 0;
			base.z = world_extents.bottom - (j * depth);

			vtMesh *mesh = new vtMesh(GL_TRIANGLE_STRIP, VtxType, 4);
			mesh->CreateRectangle(1, 1, base, size, 5.0f);

			pGeom->AddMesh(mesh, 0);	// actually add
			mesh->Release();	// pass ownership to the Geometry
		}
	}

	pGeom->SetName2("Horizon Plane");

	m_pOceanGeom = new vtMovGeom(pGeom);
	m_pOceanGeom->SetName2("Horizon");

	// offset the ocean/horizon plane, to reduce z-buffer collision with near-sea-level
	// areas of land near the ocean
	m_pOceanGeom->Translate1(FPoint3(0.0f, m_Params.m_fOceanPlaneLevel, 0.0f));

	m_pTerrainGroup->AddChild(m_pOceanGeom);
}


//
// set global projection based on this terrain's heightfield
//
void vtTerrain::SetGlobalProjection()
{
	if (m_pHeightField)
		g_Conv = m_pHeightField->m_Conversion;
}

bool vtTerrain::LoadHeaderIntoGrid(vtElevationGrid &grid)
{
	vtString name = "Elevation/";
	name += m_Params.m_strElevFile;
	vtString grid_fname = FindFileOnPaths(s_DataPaths, name);
	if (grid_fname == "")
	{
		VTLOG("\t'%s' not found on data paths.", (const char *)name);
		return false;
	}
	else
		return grid.LoadBTHeader(grid_fname);
}


///////////////////////////////////////////////
// Built Structures

/**
 * Attempt to load structures from a VTST file.  If successful, the structures
 * will be added to the Terrain's set of structure arrays.
 */
vtStructureArray3d *vtTerrain::CreateStructuresFromXML(const vtString &strFilename)
{
	VTLOG("CreateStructuresFromXML '%s'\n", (const char *) strFilename);
	vtStructureArray3d *structures = new vtStructureArray3d;
	if (!structures->ReadXML(strFilename))
	{
		VTLOG("\tCouldn't load file.\n");
		delete structures;
		return NULL;
	}
	structures->SetTerrain(this);
	m_StructureSet.Append(structures);

	CreateStructures(structures);
	return structures;
}

void vtTerrain::CreateStructures(vtStructureArray3d *structures)
{
	int num_structs = structures->GetSize();
	int suceeded = 0;

	for (int i = 0; i < num_structs; i++)
	{
		bool bSuccess = CreateStructure(structures, i);
		if (bSuccess)
			suceeded++;
	}
	VTLOG("\tSuccessfully created and added %d of %d structures.\n",
		suceeded, num_structs);
}

bool vtTerrain::CreateStructure(vtStructureArray3d *structures, int index)
{
	vtStructure *str = (vtStructure *) structures->GetAt(index);
	vtStructure3d *str3d = structures->GetStructure3d(index);

	// Construct
	bool bSuccess = structures->ConstructStructure(str3d);
	if (!bSuccess)
	{
		VTLOG("\tFailed to create stucture %d\n", index);
		return false;
	}

	bSuccess = false;
	vtTransform *pTrans = str3d->GetContainer();
	if (pTrans)
		bSuccess = AddNodeToStructGrid(pTrans);
	else
	{
		vtGeom *pGeom = str3d->GetGeom();
		if (pGeom)
			bSuccess = AddNodeToStructGrid(pGeom);
	}
	if (!bSuccess)
		VTLOG("\tWarning: Structure %d apparently not within bounds of terrain grid.\n", index);

	return bSuccess;
}

/**
 * Get the currently active structure array for this terrain.
 */
vtStructureArray3d *vtTerrain::GetStructures()
{
	if (m_iStructSet < m_StructureSet.GetSize())
		return m_StructureSet[m_iStructSet];
	else
		return NULL;
}

/**
 * Create a new structure array for this terrain, and return it.
 */
vtStructureArray3d *vtTerrain::NewStructureArray()
{
	vtStructureArray3d *sa = new vtStructureArray3d;

	// these structures will use the heightfield and projection of this terrain
	sa->SetTerrain(this);
	sa->m_proj = m_proj;

	m_StructureSet.Append(sa);
	return sa;
}

/**
 * Delete all the selected structures in the terrain's active structure array.
 */
void vtTerrain::DeleteSelectedStructures()
{
	vtStructureArray3d *structures = GetStructures();

	// first remove them from the terrain
	for (unsigned int i = 0; i < structures->GetSize(); i++)
	{
		vtStructure *str = structures->GetAt(i);
		if (str->IsSelected())
		{
			vtStructure3d *str3d = structures->GetStructure3d(i);
			RemoveNodeFromStructGrid(str3d->GetContainer());
		}
	}

	// then do a normal delete-selected
	structures->DeleteSelected();
}

bool vtTerrain::FindClosestStructure(const DPoint2 &point, double epsilon,
					   int &structure, double &closest)
{
	structure = -1;
	closest = 1E8;

	double dist;
	int i, index, sets = m_StructureSet.GetSize();
	for (i = 0; i < sets; i++)
	{
		vtStructureArray *sa = m_StructureSet[i];

		if (sa->FindClosestStructure(point, epsilon, index, dist))
		{
			if (dist < closest)
			{
				structure = index;
				closest = dist;
				m_iStructSet = i;
			}
		}
	}
	return (structure != -1);
}

/**
 * Loads an external 3D model as a movable node.  The file will be looked for
 * on the Terrain's data path, and wrapped with a vtTransform so that it can
 * be moved.
 *
 * To add the model to the Terrain's scene graph, use <b>AddModel</b> or
 * <b>AddModelToLodGrid</b>.  To plant the model on the terrain, use
 * <b>PlantModel</b> or <b>PlantModelAtPoint</b>.
 *
 * You should also make sure that your model is displayed at the correct
 * scale.  If the units are of the model are not meters, you should scale
 * the correct factor so that it matches the units of the Terrain:
 *
 * \par Example:
	\code
MyTerrain::CreateCustomCulture(bool bSound)
{
	// model is in centimeters (cm)
	vtTransform *pFountain = LoadModel("Culture/fountain.3ds");

	pFountain->Scale3(.01f, .01f, .01f);

	PlantModelAtPoint(pFountain, DPoint2(217690, 4123475));
	AddModel(pFountain);
}
	\endcode
 */
vtTransform *vtTerrain::LoadModel(const char *filename)
{
	vtNodeBase *node = NULL;
	vtString path = FindFileOnPaths(s_DataPaths, filename);
	if (path == "")
	{
		VTLOG("Couldn't locate file '%s'\n", filename);
	}
	else
		node = vtLoadModel(path);

	if (node)
	{
		vtTransform *trans = new vtTransform();
		trans->AddChild(node);
		return trans;
	}
	else
		return NULL;
}


/**
 * "Plants" a model on the ground.  This is done by moving the model directly
 * up or down such that its local origin is at the height of the terrain.
 *
 * Note: this function does not add the model to the terrain's scene
 * graph.  Use <b>AddNode</b> for that operation.
 */
void vtTerrain::PlantModel(vtTransform *model)
{
	FPoint3 pos = model->GetTrans();
	m_pHeightField->FindAltitudeAtPoint(pos, pos.y);
	model->SetTrans(pos);
}


/**
 * "Plants" a model on the ground.  This is done by moving the node to the
 * indicated earth coordinate, then moving it directly up or down such that
 * its local origin is at the height of the terrain.
 *
 * \param model The model to be placed on the terrain.
 * \param pos The position (in earth coordinates) at which to place it.
 * This position is assumed to be in the same coordinate system that
 * the Terrain is using.
 *
 * Note: this function does not add the model to the terrain's scene
 * graph.  Use <b>AddNode</b> for that operation.
 */
void vtTerrain::PlantModelAtPoint(vtTransform *model, const DPoint2 &pos)
{
	FPoint3 wpos;

	g_Conv.convert_earth_to_local_xz(pos.x, pos.y, wpos.x, wpos.z);
	m_pHeightField->FindAltitudeAtPoint(wpos, wpos.y);
	model->SetTrans(wpos);
}


void vtTerrain::_CreateCulture(bool bSound)
{
	// The LOD distances are in meters
	_SetupStructGrid((float) m_Params.m_iStructDistance);
	_SetupVegGrid((float) m_Params.m_iVegDistance);

	// create roads
	if (m_Params.m_bRoads)
	{
		vtString road_fname = "RoadData/";
		road_fname += m_Params.m_strRoadFile;
		vtString road_path = FindFileOnPaths(s_DataPaths, road_fname);
		create_roads(road_path);

		if (m_pRoadMap && m_Params.m_bRoadCulture)
		{
			NodeGeom* node = m_pRoadMap->GetFirstNode();
			IntersectionEngine* lightEngine;
			char string[50];
			while (node)
			{
				if (node->HasLights())
				{
					// add an traffic control engine
					lightEngine = new IntersectionEngine(node);
					sprintf(string, "Traffic Control: Node %i", node->m_id);
					lightEngine->SetName2(string);
					AddEngine(lightEngine);
				}
				node = (NodeGeom*)node->m_pNext;
			}
		}
	}

	m_pBBEngine = new SimpleBillboardEngine(PID2f);
	m_pBBEngine->SetName2("Billboard Engine");
	AddEngine(m_pBBEngine);

	// create trees
	m_PIA.SetHeightField(m_pHeightField);
	m_PIA.SetPlantList(m_pPlantList);

	// In case we don't load any plants, or fail to load, we will start with
	// an empty plant array, which needs to match the projection of the rest
	// of the terrain.
	m_PIA.SetProjection(GetProjection());

	if (m_Params.m_bPlants)
	{
		// Read the VF file
		vtString plants_fname = "PlantData/";
		plants_fname += m_Params.m_strVegFile;

		VTLOG("\tLooking for plants file: %s\n", (const char *) plants_fname);

		vtString plants_path = FindFileOnPaths(s_DataPaths, plants_fname);
		if (plants_path == "")
		{
			VTLOG("\tNot found.\n");
		}
		else
		{
			VTLOG("\tFound: %s\n", (const char *) plants_path);

			bool success;
			if (!m_Params.m_strVegFile.Right(3).CompareNoCase("shp"))
				success = m_PIA.ReadSHP(plants_path);
			else
				success = m_PIA.ReadVF(plants_path);
			if (success)
			{
				// Create the 3d plants
				VTLOG("\tLoaded plants file.\n");
				int created = m_PIA.CreatePlantNodes();
				VTLOG("\tCreated: %d of %d plants\n", created, m_PIA.GetNumEntities());

				int i, size = m_PIA.GetNumEntities();
				for (i = 0; i < size; i++)
				{
					vtTransform *pTrans = m_PIA.GetPlantNode(i);

					// add tree to scene graph
					if (pTrans)
						AddNodeToVegGrid(pTrans);
				}
			}
			else
				VTLOG("\tCouldn't load VF file.\n");
		}
	}

	// create built structures - there is always at least one structure array
	vtStructure3d::InitializeMaterialArrays();
	vtStructureArray3d *sa = NewStructureArray();
	sa->SetFilename("default.vtst");

	unsigned int i, num = m_Params.m_strStructFiles.size();
	for (i = 0; i < num; i++)
	{
		vtString building_fname = "BuildingData/";
		building_fname += m_Params.m_strStructFiles[i];

		VTLOG("\tLooking for structures file: %s\n", (const char *) building_fname);

		vtString building_path = FindFileOnPaths(s_DataPaths, building_fname);
		if (building_path == "")
		{
			VTLOG("\tNot found.\n");
		}
		else
		{
			VTLOG("\tFound: %s\n", (const char *) building_path);
			CreateStructuresFromXML(building_path);
		}
	}
	if (num == 0)
	{
		// No structures loaded, but the might create some later, so set
		// the projection to match the terrain.
		GetStructures()->m_proj = m_proj;
	}

	// create utility structures (routes = towers and wires)
	if (m_Params.m_bRouteEnable && m_Params.m_strRouteFile[0] != '\0')
	{
		// TODO
	}

	CreateCustomCulture(bSound);
}


//
// Create an LOD grid to contain and efficiently hide stuff that's far away
//
void vtTerrain::_SetupVegGrid(float fLODDistance)
{
	// must have a terrain with some size
	if (!m_pHeightField)
		return;

	FRECT world_extents;
	world_extents = m_pHeightField->m_WorldExtents;

	FPoint3 org(world_extents.left, 0.0f, world_extents.bottom);
	FPoint3 size(world_extents.right, 0.0f, world_extents.top);

	m_pVegGrid = new vtLodGrid(org, size, LOD_GRIDSIZE, fLODDistance, m_pHeightField);
	m_pVegGrid->SetName2("Vegetation LOD Grid");
	m_pTerrainGroup->AddChild(m_pVegGrid);
}

//
// Create an LOD grid to contain and efficiently hide stuff that's far away
//
void vtTerrain::_SetupStructGrid(float fLODDistance)
{
	// must have a terrain with some size
	if (!m_pHeightField)
		return;

	FRECT world_extents;
	world_extents = m_pHeightField->m_WorldExtents;

	FPoint3 org(world_extents.left, 0.0f, world_extents.bottom);
	FPoint3 size(world_extents.right, 0.0f, world_extents.top);

	m_pStructGrid = new vtLodGrid(org, size, LOD_GRIDSIZE, fLODDistance, m_pHeightField);
	m_pStructGrid->SetName2("Structures LOD Grid");
	m_pTerrainGroup->AddChild(m_pStructGrid);
}

/////////////////////////

void vtTerrain::_CreateLabels()
{
	vtString fname = "PointData/";
	fname += m_Params.m_strLabelFile;
	vtString labels_path = FindFileOnPaths(s_DataPaths, fname);
	if (labels_path == "")
	{
		VTLOG("Couldn't find features file '%s'\n", (const char *) fname);
		return;
	}

	vtFeatureLoader loader;

	vtFeatureSet *feat = loader.LoadFrom(labels_path);
	if (!feat)
	{
		VTLOG("Couldn't read features from file '%s'\n", (const char *) labels_path);
		return;
	}
	VTLOG("Read features from file '%s'\n", (const char *) labels_path);

	CreateStyledFeatures(*feat, "Fonts/Arial.ttf", m_Params.m_Style);

	delete feat;
}


// Helper for CreateStyledFeatures
bool GetColorField(const vtFeatureSet &feat, int iRecord, int iField, RGBAf &rgba)
{
	vtString str;
	float r, g, b;
	feat.GetValueAsString(iRecord, iField, str);
	if (sscanf((const char *)str, "%f %f %f", &r, &g, &b) != 3)
		return false;
	rgba.Set(r, g, b, 1);
	return true;
}

void vtTerrain::CreateStyledFeatures(const vtFeatureSet &feat, const char *fontname,
									 const PointStyle &style)
{
	unsigned int features = feat.GetNumEntities();
	if (features == 0)
		return;

	// create container group
	vtGroup *pPlaceNames = new vtGroup();
	pPlaceNames->SetName2("Place Names");
	m_pTerrainGroup->AddChild(pPlaceNames);

	// Create materials.
	vtMaterialArray *pLabelMats = new vtMaterialArray();

	int field_index_color = feat.GetFieldIndex("color");

	// default case: common label color
	int common_material_index = pLabelMats->AddRGBMaterial1(style.m_label_color, false, false);

#if 0
	// It turns out that we don't have to do this, because OSG lets us
	//  specify text color directly, rather than using materials.
	if (field_index_color != -1)
	{
		// go through all the features collecting unique colors
		for (i = 0; i < features; i++)
		{
			// if we have a unique color, add it
			if (GetColorField(feat, i, field_index_color, rgba))
			{
				if (pLabelMats->FindByDiffuse(rgba) == -1)
				{
					RGBi rgb = (RGBi) (RGBf) rgba;
					pLabelMats->AddRGBMaterial1(rgb, false, false);
				}
			}
		}
	}
#endif

	// Find and load the font.
	vtString font_path = FindFileOnPaths(s_DataPaths, fontname);
	if (font_path == "")
	{
		VTLOG("Couldn't find font file '%s'\n", fontname);
		return;
	}
	vtFont *font = new vtFont;
	bool success = font->LoadFont(font_path);
	if (success)
		VTLOG("Read font from file '%s'\n", fontname);
	else
	{
		VTLOG("Couldn't read font from file '%s', not creating labels.\n", fontname);
		return;
	}

	// We support both 2D and 3D points
	const vtFeatureSetPoint2D *pSetP2 = dynamic_cast<const vtFeatureSetPoint2D*>(&feat);
	const vtFeatureSetPoint3D *pSetP3 = dynamic_cast<const vtFeatureSetPoint3D*>(&feat);

	unsigned int i;
	DPoint2 p2;
	DPoint3 p3;
	FPoint3 fp3;
	vtString str;
	RGBAf rgba;
	for (i = 0; i < features; i++)
	{
		// Get the earth location of the label
		if (pSetP2)
			p2 = pSetP2->GetPoint(i);
		else if (pSetP3)
		{
			p3 = pSetP3->GetPoint(i);
			p2.Set(p3.x, p3.y);
		}

		if (!m_pHeightField->ConvertEarthToSurfacePoint(p2.x, p2.y, fp3))
			continue;

		// Elevate the location by the desired vertical offset
		fp3.y += style.m_label_elevation;

		// If we have a 3D point, we can use the Z component of the point
		//  to further affect the elevation.
		if (pSetP3)
			fp3.y += p3.z;

		// Create the vtTextMesh
		vtTextMesh *text = new vtTextMesh(font, style.m_label_size, true);	// center

		feat.GetValueAsString(i, style.m_field_index, str);
#if SUPPORT_WSTRING
		// Text might be UTF-8
		wstring2 wide_string;
		wide_string.from_utf8(str);
		text->SetText(wide_string);
#else
		// Hope that it isn't
		text->SetText(str);
#endif

		// Create the vtGeom object to contain the vtTextMesh
		vtGeom *geom = new vtGeom();
		geom->SetName2(str);
		geom->SetMaterials(pLabelMats);

#if 0
		// This is the material code that we don't (apparently) need.
		int material_index;
		if (field_index_color == -1)
			material_index = common_material_index;
		else
		{
			if (GetColorField(feat, i, field_index_color, rgba))
				material_index = pLabelMats->FindByDiffuse(rgba);
			else
				material_index = common_material_index;
		}
		geom->AddTextMesh(text, material_index);
#else
		if (field_index_color != -1)
		{
			if (GetColorField(feat, i, field_index_color, rgba))
			{
				text->SetColor(rgba);
			}
		}
		geom->AddTextMesh(text, common_material_index);
#endif

		// Add to a billboarding transform so that the labels turn
		// toward the viewer
		vtTransform *bb = new vtTransform();
		bb->AddChild(geom);
		m_pBBEngine->AddTarget(bb);

		bb->SetTrans(fp3);
		pPlaceNames->AddChild(bb);
	}
	delete font;
	pLabelMats->Release();

	VTLOG("Created %d text labels\n", features);
}

void vtTerrain::SetFog(bool fog)
{
	m_bFog = fog;
	if (m_bFog)
	{
		float dist = m_Params.m_fFogDistance * 1000;

		if (m_fog_color.r != -1)
			m_pTerrainGroup->SetFog(true, 0, dist, m_fog_color);
		else
			m_pTerrainGroup->SetFog(true, 0, dist);
	}
	else
	{
		m_pTerrainGroup->SetFog(false);
		vtGetScene()->SetBgColor(m_ocean_color);
	}
}

void vtTerrain::SetFogColor(const RGBf &color)
{
	m_fog_color = color;
	if (m_bFog) SetFog(true);
}

void vtTerrain::SetFogDistance(float fMeters)
{
	m_Params.m_fFogDistance = fMeters / 1000;
	if (m_bFog) SetFog(true);
}

/**
 * Given a time value, convert it from the LT (local time) of the center of
 * this terrain to GMT.  Local time is defined precisely by longitude,
 * e.g. at noon local time, the sun is exactly halfway across the sky.
 *
 * Note that this is different that the "standard time" of a given place,
 * which involves finding out what time zone is in effect (complicated!)
 */
void vtTerrain::TranslateToGMT(vtTime &time)
{
	if (m_fCenterLongitude == -999)
		_ComputeCenterLongitude();

	time.Increment(-m_iDifferenceFromGMT);
}

/**
 * Given a time value, convert it to the LT (local time) of the center of
 * this terrain from GMT.  Local time is defined precisely by longitude,
 * e.g. at noon local time, the sun is exactly halfway across the sky.
 *
 * Note that this is different that the "standard time" of a given place,
 * which involves finding out what time zone is in effect (complicated!)
 */
void vtTerrain::TranslateFromGMT(vtTime &time)
{
	if (m_fCenterLongitude == -999)
		_ComputeCenterLongitude();

	time.Increment(m_iDifferenceFromGMT);
}

void vtTerrain::_ComputeCenterLongitude()
{
	vtHeightFieldGrid3d *pHFGrid = GetHeightFieldGrid3d();
	DRECT drect = pHFGrid->GetEarthExtents();
	DPoint2 center;
	drect.GetCenter(center);

	// must convert from whatever we are, to geo
	vtProjection Dest;
	Dest.SetWellKnownGeogCS("WGS84");

	// safe (won't fail on tricky Datum conversions)
	OCT *trans = CreateConversionIgnoringDatum(&m_proj, &Dest);
	trans->Transform(1, &center.x, &center.y);
	delete trans;

	m_fCenterLongitude = center.x;

	// calculate offset FROM GMT
	// longitude of 180 deg = 12 hours = 720 min = 43200 sec
	m_iDifferenceFromGMT = (int) (m_fCenterLongitude / 180 * 43200);
}


/**
 * First step in terrain creation: load elevation.
 */
bool vtTerrain::CreateStep1(int &iError)
{
	// create terrain group - this holds all surfaces for the terrain
	m_pTerrainGroup = new vtGroup();
	m_pTerrainGroup->SetName2("Terrain Group");
#ifdef VTLIB_PSM
	m_pTerrainGroup->IncUse();
#endif

	if (m_pInputGrid)
	{
		m_pElevGrid = m_pInputGrid;
		return true;
	}
	vtString fname = "Elevation/";
	fname += m_Params.m_strElevFile;
	VTLOG("\tLooking for elevation file: %s\n", (const char *) fname);

	vtString fullpath = FindFileOnPaths(s_DataPaths, fname);
	if (fullpath == "")
	{
		iError = TERRAIN_ERROR_NOTFOUND;
		VTLOG("\t\tNot found.\n");
		return false;
	}

	VTLOG("\tFound: %s\n", (const char *) fullpath);
	if (m_Params.m_bTin)
	{
		if (!m_pTin)
		{
			// if they did not provide us with a TIN, try to load it
			m_pTin = new vtTin3d;
			m_pTin->Read(fullpath);
		}
	}
	else
	{
		// Loading elevation grid...
		m_pElevGrid = new vtElevationGrid();
		bool status = m_pElevGrid->LoadFromBT(fullpath);
		if (status == false)
		{
			iError = TERRAIN_ERROR_NOTFOUND;
			VTLOG("\tGrid load failed.\n");
			return false;
		}
		else
		{
			int col, row;
			m_pElevGrid->GetDimensions(col, row);
			VTLOG("\tGrid load succeeded, size %d x %d.\n", col, row);
		}
		m_pElevGrid->SetupConversion(m_Params.m_fVerticalExag);
	}
	return true;
}

/**
 * Next step in terrain creation: create textures.
 */
bool vtTerrain::CreateStep2(int &iError)
{
	if (m_Params.m_bTin)
	{
		m_proj = m_pTin->m_proj;
		g_Conv = m_pTin->m_Conversion;
	}
	else
	{
		// set global projection based on this terrain
		m_proj = m_pElevGrid->GetProjection();
		g_Conv = m_pElevGrid->m_Conversion;

		vtTime terraintime;
		terraintime.SetTimeOfDay(m_Params.m_iInitTime, 0, 0);
		_CreateTextures(terraintime);
	}
	char type[10], value[2048];
	m_proj.GetTextDescription(type, value);
	VTLOG(" Projection of the terrain: %s, '%s'\n", type, value);
	return true;
}

/**
 * Next step in terrain creation: create 3D geometry for the terrain.
 */
bool vtTerrain::CreateStep3(int &iError)
{
	if (m_Params.m_bTin)
		return CreateFromTIN(iError);
	else
		return CreateFromGrid(iError);
}

bool vtTerrain::CreateFromTIN(int &iError)
{
	m_pHeightField = m_pTin;

	bool bDropShadow = true;

	// build heirarchy (add terrain to scene graph)
	m_pTerrainGroup->AddChild(m_pTin->CreateGeometry(bDropShadow));

	return true;
}

// for timing how long the CLOD takes to initialize
clock_t tm;

bool vtTerrain::CreateFromGrid(int &iError)
{
	VTLOG(" CreateFromGrid\n");
	float fOceanDepth;
	if (m_Params.m_bDepressOcean)
		fOceanDepth = m_Params.m_fDepressOceanLevel;
	else
		fOceanDepth = 0.0f;

	tm = clock();

	// create elegant dynamic LOD terrain
	iError = 0;
	if (!create_dynamic_terrain(fOceanDepth, iError))
	{
		if (iError == 0)
			iError = TERRAIN_ERROR_LODFAILED;
		return false;
	}
	else
	{
		m_pDynGeom->SetEnabled(true);
		m_pHeightField = m_pDynGeom;
	}

	if (!m_bPreserveInputGrid)
	{
		// we don't need the original grid any more
		delete m_pElevGrid;
		m_pElevGrid = NULL;
	}
	return true;
}

/**
 * Next step in terrain creation: additional CLOD construction.
 */
bool vtTerrain::CreateStep4(int &iError)
{
	// some algorithms need an additional stage of initialization
	if (m_pDynGeom != NULL)
	{
		m_pDynGeom->Init2();

		clock_t tm2 = clock();
		float time = ((float)tm2 - tm)/CLOCKS_PER_SEC;
		VTLOG("CLOD construction: %.3f seconds.\n", time);
	}
	return true;
}

/**
 * Next step in terrain creation: create the culture and labels.
 */
bool vtTerrain::CreateStep5(bool bSound, int &iError)
{
	// must have a heightfield by this point
	if (!m_pHeightField)
		return false;

	_CreateCulture(bSound);

	if (m_Params.m_bOceanPlane || m_Params.m_bHorizon)
	{
		bool bWater = m_Params.m_bOceanPlane;
		bool bHorizon = m_Params.m_bHorizon;
		bool bCenter = m_Params.m_bOceanPlane;
		create_artificial_horizon(bWater, bHorizon, bCenter, 0.5f);
	}

	if (m_Params.m_bLabels)
		_CreateLabels();

	// Engines will be activated later in vtTerrainScene::SetTerrain
	ActivateEngines(false);

	return true;
}

/**
 * CreateScene constructs all geometry, textures and objects for a given terrain.
 *
 * \param bSound : Allow sounds to be created.
 * \param iError : Returns by reference an error value, or 0 for no error.
 * \returns A vtGroup which is the top of the terrain scene graph.
 */
vtGroup *vtTerrain::CreateScene(bool bSound, int &iError)
{
	if (!CreateStep1(iError))
		return NULL;

	if (!CreateStep2(iError))
		return NULL;

	if (!CreateStep3(iError))
		return NULL;

	if (!CreateStep4(iError))
		return NULL;

	if (!CreateStep5(bSound, iError))
		return NULL;

	return m_pTerrainGroup;
}

bool vtTerrain::IsCreated()
{
	return m_pTerrainGroup != NULL;
}

void vtTerrain::Enable(bool bVisible)
{
	m_pTerrainGroup->SetEnabled(bVisible);
}

FPoint3 vtTerrain::GetCenter()
{
	GetTerrainBounds();
	return m_bound_sphere.center;
}

float vtTerrain::GetRadius()
{
	GetTerrainBounds();
	return m_bound_sphere.radius;
}

void vtTerrain::GetTerrainBounds()
{
	if (m_pDynGeomScale != NULL)
		m_pDynGeomScale->GetBoundSphere(m_bound_sphere);
	else
		m_bound_sphere.Empty();
}

/**
 * Tests whether a given point (in Earth coordinates) is within the current
 * extents of the terrain.
 */
bool vtTerrain::PointIsInTerrain(const DPoint2 &p)
{
	float x, z;
	g_Conv.ConvertFromEarth(p, x,  z);	// convert earth -> XZ
	return m_pHeightField->ContainsWorldPoint(x, z);
}

void vtTerrain::CreateCustomCulture(bool bDoSound)
{
}


void vtTerrain::AddEngine(vtEngine *pE)
{
	// if the user didn't indicate which scene, default
	vtGetScene()->AddEngine(pE);

	// add to this Terrain's engine list
	m_Engines.Append(pE);
}

void vtTerrain::ActivateEngines(bool bActive)
{
	// turn off the engine specific to the previous terrain
	for (unsigned int k = 0; k < m_Engines.GetSize(); k++)
	{
		vtEngine *pE = m_Engines.GetAt(k);
		pE->SetEnabled(bActive);
	}
}

//////////////////////////

void vtTerrain::SetFeatureVisible(TFType ftype, bool bOn)
{
	switch (ftype)
	{
	case TFT_TERRAINSURFACE:
		if (m_pDynGeom)
			m_pDynGeom->SetEnabled(bOn);
		break;
	case TFT_OCEAN:
		if (m_pOceanGeom)
			m_pOceanGeom->SetEnabled(bOn);
		break;
	case TFT_VEGETATION:
		if (m_pVegGrid)
			m_pVegGrid->SetEnabled(bOn);
		break;
	case TFT_STRUCTURES:
		if (m_pStructGrid)
			m_pStructGrid->SetEnabled(bOn);
		break;
	case TFT_ROADS:
		if (m_pRoadGroup)
			m_pRoadGroup->SetEnabled(bOn);
		break;
	}
}

bool vtTerrain::GetFeatureVisible(TFType ftype)
{
	switch (ftype)
	{
	case TFT_TERRAINSURFACE:
		if (m_pDynGeom)
			return m_pDynGeom->GetEnabled();
		break;
	case TFT_OCEAN:
		if (m_pOceanGeom)
			return m_pOceanGeom->GetEnabled();
		break;
	case TFT_VEGETATION:
		if (m_pVegGrid)
			return m_pVegGrid->GetEnabled();
		break;
	case TFT_STRUCTURES:
		if (m_pStructGrid)
			return m_pStructGrid->GetEnabled();
		break;
	case TFT_ROADS:
		if (m_pRoadGroup)
			return m_pRoadGroup->GetEnabled();
		break;
	}
	return false;
}

void vtTerrain::SetLODDistance(TFType ftype, float fDistance)
{
	switch (ftype)
	{
	case TFT_VEGETATION:
		if (m_pVegGrid)
			m_pVegGrid->SetDistance(fDistance);
		break;
	case TFT_STRUCTURES:
		if (m_pStructGrid)
			m_pStructGrid->SetDistance(fDistance);
		break;
	case TFT_ROADS:
		if (m_pRoadMap)
			m_pRoadMap->SetLodDistance(fDistance);
		break;
	}
}

float vtTerrain::GetLODDistance(TFType ftype)
{
	switch (ftype)
	{
	case TFT_VEGETATION:
		if (m_pVegGrid)
			return m_pVegGrid->GetDistance();
		break;
	case TFT_STRUCTURES:
		if (m_pStructGrid)
			return m_pStructGrid->GetDistance();
		break;
	case TFT_ROADS:
		if (m_pRoadMap)
			return m_pRoadMap->GetLodDistance();
		break;
	}
	return 0.0f;
}

vtHeightFieldGrid3d *vtTerrain::GetHeightFieldGrid3d()
{
	// if we still have the source elevation, use it
	if (m_pElevGrid)
		return m_pElevGrid;

	// otherwise, later on, we might only have the runtime (CLOD) grid
	else if (m_pDynGeom)
		return m_pDynGeom;

	return NULL;	// no grid to return, possible because it's a TIN
}

void vtTerrain::_CreateChoppedTextures(int patches, int patch_size)
{
	int size = patch_size;
	bool mono = (m_pDIB->GetDepth() == 8);

	int x_off, y_off, x, y, i, j;

	for (i = 0; i < patches; i++)
	{
		x_off = i * (size - 1);
		for (j = 0; j < patches; j++)
		{
			y_off = j * (size - 1);

			// make a tile
			vtDIB *dib2 = new vtDIB();
			dib2->Create(size, size, m_pDIB->GetDepth(), mono);

			unsigned long pixel;
			if (mono)
			{
				for (x = 0; x < size; x++)
					for (y = 0; y < size; y++)
					{
						pixel = m_pDIB->GetPixel8(x_off + x, y_off + y);
						dib2->SetPixel8(x, y, pixel);
					}
			}
			else
			{
				for (x = 0; x < size; x++)
					for (y = 0; y < size; y++)
					{
						pixel = m_pDIB->GetPixel24(x_off + x, y_off + y);
						dib2->SetPixel24(x, y, pixel);
					}
			}

			int internalformat;
			if (!mono && m_Params.m_b16bit)
				internalformat = GL_RGB5;
			else
				internalformat = -1;
			vtImage *pImage = new vtImage(dib2, internalformat);

			// Can we delete the internals DIBs here, or does the scene graph
			//   needs the data?  Actually no, the scene graph gets a copy of it.
//			dib2->LeaveInternalDIB(true);
			delete dib2;

			m_Images.SetAt(i*patches+j, pImage);
		}
	}
}


/*
 * Creates an array of materials for the dynamic LOD terrain geometry.
 */
void vtTerrain::_CreateTiledMaterials(vtMaterialArray *pMat1,
							 int patches, int patch_size, float ambient,
							 float diffuse, float emmisive)
{
	int i, j;

	for (i = 0; i < patches; i++)
	{
		for (j = 0; j < patches; j++)
		{
			vtImage *image = m_Images.GetAt(i*patches+j);
			pMat1->AddTextureMaterial(image,
				true, 		// culling
				false,		// lighting
				false,		// transparency
				false,		// additive
				ambient, diffuse,
				1.0f,		// alpha
				emmisive,
				true,		// texgen
				false,		// clamp
				m_Params.m_bMipmap);
		}
	}
}


void vtTerrain::_ApplyPreLight(vtHeightFieldGrid3d *pElevGrid, vtDIB *dib,
							  const vtTime &time)
{
	VTLOG("Prelighting terrain texture: ");
	FPoint3 light_dir;

#if 0
	// This is a cool scientifically accurate way (which we're not using yet..)
	float altDEG, aziDEG;

	// Compute angle of sun based on location, date and time  (#include "spa.h")
	SunAltAzi(altDEG,aziDEG,44.875,-110.875,2004,8,1,8,20,0,-7.0,1000.0);  

	// Convert sun azimuth and elevation angles to radians
	float alt=(altDEG/180.0f*PIf);
	float azi=(aziDEG/180.0f*PIf);

	light_dir.x=(-sin(azi)*cos(alt));
	light_dir.z=(-cos(azi)*cos(alt));
	light_dir.y=-sin(alt);
#endif

	// at midnight: below the earth, pointing up
	light_dir.Set(0,1,0);

	float fraction = (float) time.GetSecondOfDay() / (24*60*60);
	float angle = PID2f + fraction * PI2f;
	light_dir.Set(cosf(angle), sinf(angle), 0);

	clock_t c1 = clock();

	float shade_factor = m_Params.m_fPreLightFactor;
	if (m_Params.m_bCastShadows)
	{
		// A more accurate shading, still a little experimental
		pElevGrid->ShadowCastDib(dib, light_dir, shade_factor);
	}
	else
        pElevGrid->ShadeDibFromElevation(dib, light_dir, shade_factor);

	clock_t c2 = clock();

	clock_t c3 = c2 - c1;
	VTLOG("%.3f seconds.\n", (float)c3 / CLOCKS_PER_SEC);
}

void vtTerrain::AddPointOfInterest(double left, double bottom, double right,
					   double top, const char *name, const char *url)
{
	POIPtr p = new vtPointOfInterest();
	p->m_rect.SetRect(left, top, right, bottom);
	p->m_name = name;
	p->m_url = url;

	m_PointsOfInterest.Append(p);
}

vtPointOfInterest *vtTerrain::FindPointOfInterest(DPoint2 utm)
{
	int points = m_PointsOfInterest.GetSize();
	for (int i = 0; i < points; i++)
	{
		vtPointOfInterest *p = m_PointsOfInterest.GetAt(i);
		if (p->m_rect.ContainsPoint(utm))
			return p;
	}
	return NULL;
}

#define STEPS 40

void vtTerrain::ShowPOI(vtPointOfInterest *poi, bool bShow)
{
	if (!bShow)
	{
		if (poi->m_pGeom) poi->m_pGeom->SetEnabled(false);
		return;
	}

	// now we must show it - check if the shape is already built
	if (poi->m_pGeom)
	{
		poi->m_pGeom->SetEnabled(true);
		return;
	}

	vtMesh *pMesh = new vtMesh(GL_LINE_STRIP, 0, STEPS*4);

	DLine2 dline;
	dline.Append(DPoint2(poi->m_rect.left, poi->m_rect.top));
	dline.Append(DPoint2(poi->m_rect.right, poi->m_rect.top));
	dline.Append(DPoint2(poi->m_rect.right, poi->m_rect.bottom));
	dline.Append(DPoint2(poi->m_rect.left, poi->m_rect.bottom));
	dline.Append(DPoint2(poi->m_rect.left, poi->m_rect.top));
	AddSurfaceLineToMesh(pMesh, dline, 10.0f);

	poi->m_pGeom = new vtGeom();

	vtMaterialArray *pMat = new vtMaterialArray();
	pMat->AddRGBMaterial1(RGBf(1.0f, 0.0f, 0.0f), false, false); // red

	poi->m_pGeom->SetMaterials(pMat);
	poi->m_pGeom->SetName2("POI Geom");
	poi->m_pGeom->AddMesh(pMesh, 0);
	pMesh->Release();	// pass ownership to the Geometry
	pMat->Release();

	m_pTerrainGroup->AddChild(poi->m_pGeom);
}

#include "vtdata/CubicSpline.h"

float vtTerrain::AddSurfaceLineToMesh(vtMesh *pMesh, const DLine2 &line,
									 float fOffset, bool bCurve)
{
	unsigned int i, j;
	FPoint3 v1, v2, v;

	// try to guess how finely to tesselate our line
	float fSpacing;
	if (m_pDynGeom)
	{
		FPoint2 spacing = m_pDynGeom->GetWorldSpacing();
		fSpacing = std::min(spacing.x, spacing.y) / 2;
	}
	else if (m_pTin)
	{
		// TINs don't have a grid spacing.  In lieu of using a completely
		//  different (more correct) algorithm for draping, just estimate.
		DRECT ext = m_pTin->GetEarthExtents();
		FPoint2 p1, p2;
		g_Conv.convert_earth_to_local_xz(ext.left, ext.bottom, p1.x, p1.y);
		g_Conv.convert_earth_to_local_xz(ext.right, ext.top, p2.x, p2.y);
		fSpacing = (p2 - p1).Length() / 1000.0f;
	}

	float fTotalLength = 0.0f;
	int iStart = pMesh->GetNumVertices();
	int iVerts = 0;
	unsigned int points = line.GetSize();
	if (bCurve)
	{
		DPoint3 p;

		CubicSpline spline;
		for (i = 0; i < points; i++)
		{
			p.Set(line[i].x, line[i].y, 0);
			spline.AddPoint(p);
		}
		bool result = spline.Generate();

		// estimate how many steps to subdivide this line into
		double dLinearLength = line.Length();
		double full = (double) (points-1);
		int iSteps = (unsigned int) (dLinearLength / fSpacing);
		if (iSteps < 3)
			iSteps = 3;
		double dStep = full / iSteps;

		FPoint3 last_v;
		double f;
		for (f = 0; f <= full; f += dStep)
		{
			spline.Interpolate(f, &p);

			g_Conv.convert_earth_to_local_xz(p.x, p.y, v.x, v.z);
			m_pHeightField->FindAltitudeAtPoint(v, v.y);
			v.y += fOffset;
			pMesh->AddVertex(v);
			iVerts++;

			// keep a running toal of approximate ground length
			if (f > 0)
				fTotalLength += (v - last_v).Length();
			last_v = v;
		}

		spline.Cleanup();
	}
	else
	{
		for (i = 0; i < points; i++)
		{
			v1 = v2;
			g_Conv.convert_earth_to_local_xz(line[i].x, line[i].y, v2.x, v2.z);
			if (i == 0)
				continue;

			// estimate how many steps to subdivide this segment into
			FPoint3 diff = v2 - v1;
			float fLen = diff.Length();
			unsigned int iSteps = (unsigned int) (fLen / fSpacing);
			if (iSteps < 3) iSteps = 3;

			FPoint3 last_v;
			for (j = 0; j <= iSteps; j++)
			{
				// simple linear interpolation of the ground coordinate
				v.Set(v1.x + diff.x / iSteps * j, 0.0f, v1.z + diff.z / iSteps * j);
				m_pHeightField->FindAltitudeAtPoint(v, v.y);
				v.y += fOffset;
				pMesh->AddVertex(v);
				iVerts++;

				// keep a running toal of approximate ground length
				if (j > 0)
					fTotalLength += (v - last_v).Length();
				last_v = v;
			}
		}
	}
	pMesh->AddStrip2(iVerts, iStart);
	return fTotalLength;
}

void vtTerrain::HideAllPOI()
{
	int points = m_PointsOfInterest.GetSize();
	for (int i = 0; i < points; i++)
	{
		vtPointOfInterest *p = m_PointsOfInterest.GetAt(i);
		ShowPOI(p, false);
	}
}

const char *vtTerrain::DescribeError(int iError)
{
	VTLOG("DescribeError %d\n", iError);
	switch (iError)
	{
	case TERRAIN_ERROR_NOTFOUND: return "The terrain data file was not found.";
	case TERRAIN_ERROR_NOREGULAR: return "The regular grid terrain could not be constructed.";
	case TERRAIN_ERROR_NOTPOWER2: return "The elevation data is of an unsupported size.\n\
The continuous LOD algorithms require that the data is\n\
square and the dimensions are a power of 2 plus 1.\n\
For example, 513x513 and 1025x105 are supported sizes.";
	case TERRAIN_ERROR_NOMEM: return "Not enough memory.";
	case TERRAIN_ERROR_LODFAILED: return "Couldn't create CLOD terrain surface.";
	}
	return "No error.";
}


bool vtTerrain::AddPlant(const DPoint2 &pos, int iSpecies, float fSize)
{
	int num = m_PIA.AddPlant(pos, fSize, iSpecies);
	if (!m_PIA.CreatePlantNode(num))
		return false;

	vtTransform *pTrans = m_PIA.GetPlantNode(num);

	// add tree to scene graph
	AddNodeToVegGrid(pTrans);
	return true;
}

/**
 * Delete all the selected plants in the terrain's plant array.
 */
void vtTerrain::DeleteSelectedPlants()
{
	// first remove them from the terrain
	for (unsigned int i = 0; i < m_PIA.GetNumEntities(); i++)
	{
		if (m_PIA.IsSelected(i))
		{
			vtTransform *pTrans = m_PIA.GetPlantNode(i);
			m_pVegGrid->RemoveFromGrid(pTrans);
			m_PIA.DeletePlant(i);
		}
	}
}


/**
 * Adds a node to the terrain.
 * The node will be added directly, so it is always in the scene whenever
 * the terrain is visible.
 *
 * \sa AddNodeToLodGrid
 */
void vtTerrain::AddNode(vtNodeBase *pNode)
{
	m_pTerrainGroup->AddChild(pNode);
}

/**
 * Adds a node to the terrain.
 * The node will be added to the LOD Grid of the terrain, so it will be
 * culled when it is far from the viewer.  This is usually desirable when
 * the models are complicated or there are lot of them.
 *
 * There is another form of this method which takes a vtGeom node instead.
 *
 * \sa AddNode
 */
bool vtTerrain::AddNodeToVegGrid(vtTransform *pTrans)
{
	if (!m_pVegGrid)
		return false;
	return m_pVegGrid->AppendToGrid(pTrans);
}

/**
 * Adds a node to the terrain.
 * The node will be added to the LOD Grid of the terrain, so it will be
 * culled when it is far from the viewer.  This is usually desirable when
 * the models are complicated or there are lot of them.
 *
 * There is another form of this method which takes a vtGeom node instead.
 *
 * \sa AddNode
 */
bool vtTerrain::AddNodeToStructGrid(vtTransform *pTrans)
{
	if (!m_pStructGrid)
		return false;
	return m_pStructGrid->AppendToGrid(pTrans);
}

/**
 * Adds a node to the terrain.
 * The node will be added to the LOD Grid of the terrain, so it will be
 * culled when it is far from the viewer.  This is usually desirable when
 * the models are complicated or there are lot of them.
 *
 * There is another form of this method which takes a vtTransform node instead.
 *
 * \sa AddNode
 */
bool vtTerrain::AddNodeToStructGrid(vtGeom *pGeom)
{
	if (!m_pStructGrid)
		return false;
	return m_pStructGrid->AppendToGrid(pGeom);
}


void vtTerrain::RemoveNodeFromStructGrid(vtNodeBase *pNode)
{
	if (m_pStructGrid)
		m_pStructGrid->RemoveNodeFromGrid(pNode);
}
