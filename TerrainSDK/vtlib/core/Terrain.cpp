//
// Terrain.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include "Terrain.h"
#include "Light.h"
#include "Building3d.h"
#include "Building3d.h"
#include "vtdata/StructArray.h"
#include "IntersectionEngine.h"
#include "TerrainSurface.h"
#include "Fence3d.h"
#include "Route.h"
#include "vtTin3d.h"

//#include "LKTerrain.h"
#include "TVTerrain.h"
#include "SMTerrain.h"
#include "CustomTerrain.h"
#include "BryanTerrain.h"
#include "SRTerrain.h"
// add your own LOD method header here!

#define LARGEST_BLOCK_SIZE	16

// use a grid of LOD cells of size LOD_GRIDSIZE x LOD_GRIDSIZE
#define LOD_GRIDSIZE		192

// static data path
StringArray vtTerrain::m_DataPaths;

//////////////////////////////////////////////////////////////////////

vtTerrain::vtTerrain()
{
	m_ocean_color.Set(40.0f/255, 75.0f/255, 124.0f/255);	// unshaded color

	m_pTerrainGroup = NULL;
	m_pDIB = NULL;
	m_pCoverage = NULL;
	m_pTerrApps1 = NULL;
	m_pTerrApps2 = NULL;
	m_pRoadMap = NULL;
	m_pInputGrid = NULL;
	m_pHeightField = NULL;
	m_bPreserveInputGrid = false;
	m_pImage = NULL;
	m_pLocalGrid = NULL;
	m_pLodGrid = NULL;

	m_pOceanGeom = NULL;
	m_pRoadGroup = NULL;
	m_pTreeGroup = NULL;

	m_bShowPOI = true;
	m_pPlantList = NULL;

	m_pDynGeom = NULL;
	m_pDynGeomScale = NULL;
	m_pTerrainGeom = NULL;
	m_pTin = NULL;
	m_pNext = NULL;

	m_CamLocation.Identity();
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

	// Do not delete the PlantList, the application may be sharing the same
	// list with several different terrains.
//	delete m_pPlantList;

	if (!m_bPreserveInputGrid)
	{
		delete m_pLocalGrid;
	}
	delete m_pCoverage;
	delete m_pImage;
	delete m_pDIB;
//	delete m_pTreeGroup;	// don't delete, same as m_pLodGrid
	delete m_pRoadMap;
	delete m_pRoadGroup;
	delete m_pTerrApps1;
	delete m_pTerrApps2;
	delete m_pOceanGeom;
	delete m_pLodGrid;
//	delete m_pInputGrid;	// don't delete, copied to m_pLocalGrid
	delete m_pTin;
	delete m_pDynGeom;
	delete m_pTerrainGeom;
	delete m_pTerrainGroup;
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
void vtTerrain::SetLocalGrid(vtLocalGrid *pGrid, bool bPreserve)
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

void vtTerrain::create_roads(vtString strRoadFile)
{
	m_pRoadMap = new vtRoadMap3d();

	bool success = m_pRoadMap->ReadRMF(strRoadFile,
		m_Params.m_bHwy != 0, m_Params.m_bPaved != 0, m_Params.m_bDirt != 0);
	if (!success)
	{
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
		m_DataPaths);
	m_pTerrainGroup->AddChild(m_pRoadGroup);
	if (m_Params.m_bRoadCulture)
		m_pRoadMap->GenerateSigns(m_pLodGrid);
}


///////////////////

void vtTerrain::create_textures()
{
	int iTiles = 4;		// fixed for now
	TextureEnum eTex = m_Params.m_eTexture;

	m_pTerrApps1 = new vtMaterialArray();
	m_pTerrApps2 = new vtMaterialArray();

	float ambient, diffuse, emmisive;
	if (m_Params.m_bPreLit)
	{
		diffuse = 1.0f;
		ambient = emmisive = 0.0f;
	}
	else
	{
		ambient = TERRAIN_AMBIENT;
		diffuse = TERRAIN_DIFFUSE;
		emmisive = TERRAIN_EMISSIVE;
	}

	if (eTex == TE_SINGLE || eTex == TE_TILED)	// load texture
	{
		vtString texture_fname = "GeoSpecific/";
		if (m_Params.m_eTexture == TE_SINGLE)
			texture_fname += m_Params.m_strTextureSingle;	// single texture
		else
			texture_fname += m_Params.m_strTextureFilename;
		vtString texture_path = FindFileOnPaths(m_DataPaths, texture_fname);

		if (texture_path == "")
		{
			// failed to find texture
		}
		else
		{
			// Load a DIB of the whole, large texture
			m_pDIB = new vtDIB(texture_path);
			if (! m_pDIB->m_bLoadedSuccessfully)
			{
				m_pTerrApps1->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
											 RGBf(0.2f, 0.2f, 0.2f),
											 true, !m_Params.m_bPreLit);	// for shaded white
				m_pTerrApps2->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
											 RGBf(0.2f, 0.2f, 0.2f),
											 true, !m_Params.m_bPreLit);	// for shaded white
				m_Params.m_eTexture = TE_NONE;
			}
		}
	}
	if (eTex == TE_DERIVED)
	{
		// Determine the correct size for the derived texture: ideally
		// as large as the input grid, but not larger than the hardware
		// texture size limit.
		GLint tmax;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tmax);

		int cols, rows;
		m_pLocalGrid->GetDimensions(cols, rows);

		int tsize = cols-1;
		if (tsize > tmax) tsize = tmax;

		// derive color from elevation
		m_pDIB = new vtDIB(tsize, tsize, 24, false);
		m_pLocalGrid->ColorDibFromElevation(m_pDIB, RGBi(m_ocean_color));
	}

	// apply pre-lighting (darkening)
	if (m_Params.m_bPreLight && m_pDIB)
		ApplyPreLight(m_pLocalGrid, m_pDIB);

	if (eTex == TE_SINGLE || eTex == TE_DERIVED)
	{
		// single texture
		m_pDIB->LeaveInternalDIB(true);
		m_pImage = new vtImage(m_pDIB,
			(m_pDIB->GetDepth() > 8 && m_Params.m_b16bit) ? GL_RGB5 : -1);
	}
	if (eTex == TE_TILED && m_pDIB)
	{
		m_pDIB->LeaveInternalDIB(false);

		CreateChoppedTextures(m_pLocalGrid, m_pDIB, iTiles, m_Params.m_iTilesize);
		if (m_Params.m_bRegular)
			_CreateTiledMaterials1(m_pTerrApps1,
							 iTiles, m_Params.m_iTilesize, ambient, diffuse,
							 emmisive);
		if (m_Params.m_bDynamic)
			_CreateTiledMaterials2(m_pTerrApps2,
							 iTiles, m_Params.m_iTilesize, ambient, diffuse,
							 emmisive);
	}
	if (eTex == TE_NONE || m_pDIB == NULL)	// none or failed to find texture
	{
		// no texture: create plain white material
		m_pTerrApps1->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
									 RGBf(0.2f, 0.2f, 0.2f),
									 true, !m_Params.m_bPreLit);
		m_pTerrApps2->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
									 RGBf(0.2f, 0.2f, 0.2f),
									 true, !m_Params.m_bPreLit);
	}
	// We're not going to use it anymore, so we're done with the DIB
	if (m_pDIB != NULL)
	{
		delete m_pDIB;
		m_pDIB = NULL;
	}
	if (eTex == TE_SINGLE || eTex == TE_DERIVED)
	{
		m_pTerrApps1->AddTextureMaterial(m_pImage,
			true,		// culling
			!m_Params.m_bPreLit,	// lighting
			false,		// transparent
			false,		// additive
			ambient, diffuse,
			1.0f,		// alpha
			emmisive,
			false,		// texgen
			false,		// clamp
			m_Params.m_bMipmap);
		m_pTerrApps2->AddTextureMaterial(m_pImage,
			true,		// culling
			!m_Params.m_bPreLit,	// lighting
			false,		// transparent
			false,		// additive
			ambient, diffuse,
			1.0f,		// alpha
			emmisive,
			true,		// texgen
			false,		// clamp
			m_Params.m_bMipmap);
	}
}


/////////////////////

bool  vtTerrain::create_regular_terrain(float fOceanDepth)
{
	int texture_patches;
	if (m_Params.m_eTexture == TE_TILED)
		texture_patches = 4;	// tiled, which is always 4x4
	else
		texture_patches = 1;	// assume one texture

	bool bLighting, bTextured;
	if (m_Params.m_eTexture == TE_NONE)
	{
		bLighting = true;
		bTextured = false;
	}
	else
	{
		bLighting = !m_Params.m_bPreLit;
		bTextured = true;
	}

	// determine geometry flags
	int VtxType = 0;
	if (bLighting)
		VtxType |= VT_Normals;
	if (bTextured)
		VtxType |= VT_TexCoords;
	if (m_Params.m_bVertexColors)
		VtxType |= VT_Colors;

	// create terrain surface
	m_pTerrainGeom = new vtTerrainGeom();

	m_pTerrainGeom->SetName2("RegularTerrain");
	m_pTerrainGeom->SetMaterials(m_pTerrApps1);

	m_pTerrainGeom->CreateFromLocalGrid(m_pLocalGrid, VtxType,
							m_Params.m_iSubsample, m_Params.m_iSubsample,
							LARGEST_BLOCK_SIZE,	texture_patches,
							m_Params.m_bSuppressLand,
							fOceanDepth, bLighting);

	switch (m_Params.m_eTexture)
	{
		case TE_SINGLE:
		case TE_DERIVED:
			m_pTerrainGeom->DrapeTextureUV();
			break;
		case TE_TILED:
			m_pTerrainGeom->DrapeTextureUVTiled(m_pCoverage);
			break;
	}

	// build heirarchy (add terrain to scene graph)
	m_pTerrainGroup->AddChild(m_pTerrainGeom);

	return true;
}

bool vtTerrain::create_dynamic_terrain(float fOceanDepth, int &iError)
{
	int texture_patches;
	if (m_Params.m_eTexture == TE_TILED)
		texture_patches = 4;	// tiled, which is always 4x4
	else
		texture_patches = 1;	// assume one texture

	bool bLighting, bTextured;
	if (m_Params.m_eTexture == TE_NONE)
	{
		bLighting = true;
		bTextured = false;
	}
	else
	{
		bLighting = !m_Params.m_bPreLit;
		bTextured = true;
	}

//	if (m_Params.m_eLodMethod == LM_LINDSTROMKOLLER)
//	{
//		m_pDynGeom = new LKTerrain();
//		m_pDynGeom->SetName2("LK Shape");
//	}
	if (m_Params.m_eLodMethod == LM_TOPOVISTA)
	{
		m_pDynGeom = new TVTerrain();
		m_pDynGeom->SetName2("TV Shape");
	}
	else if (m_Params.m_eLodMethod == LM_MCNALLY)
	{
		m_pDynGeom = new SMTerrain();
		m_pDynGeom->SetName2("Seumas Shape");
	}
	else if (m_Params.m_eLodMethod == LM_CUSTOM)
	{
		m_pDynGeom = new CustomTerrain();
		m_pDynGeom->SetName2("CustomTerrain Shape");
	}
	else if (m_Params.m_eLodMethod == LM_BRYANQUAD)
	{
		m_pDynGeom = new BryanTerrain();
		m_pDynGeom->SetName2("BryanQuad Shape");
	}
	else if (m_Params.m_eLodMethod == LM_ROETTGER)
	{
		m_pDynGeom = new SRTerrain();
		m_pDynGeom->SetName2("Roettger Shape");
	}
	// add your own LOD method here!

	m_pDynGeom->SetOptions(m_Params.m_bTriStrips != 0,
		texture_patches, m_Params.m_iTilesize);

	bool result = m_pDynGeom->Init(m_pLocalGrid,
				   m_Params.m_fVerticalExag, fOceanDepth, iError);
	if (result == false)
	{
		m_pDynGeom = NULL;
		return false;
	}

	m_pDynGeom->SetPixelError(m_Params.m_fPixelError);
	m_pDynGeom->SetPolygonCount(m_Params.m_iTriCount);
	m_pDynGeom->SetMaterials(m_pTerrApps2);

	// build heirarchy (add terrain to scene graph)
	m_pDynGeomScale = new vtTransform();

	DPoint2 spacing = m_pLocalGrid->GetWorldSpacing();
	m_pDynGeomScale->Scale3(spacing.x, m_Params.m_fVerticalExag, -spacing.y);

	m_pDynGeomScale->AddChild(m_pDynGeom);
	m_pTerrainGroup->AddChild(m_pDynGeomScale);

	return true;
}


void vtTerrain::AddFence(vtFence3d *f)
{
	vtStructure3d *str = (vtStructure3d *) m_Structures.NewStructure();
	str->SetFence(f);
	m_Structures.Append(str);
	str->CreateNode(m_pHeightField);

	// Add to LOD grid
	AddNodeToLodGrid(str->GetGeom());
}

void vtTerrain::AddFencepoint(vtFence3d *f, const DPoint2 &epos)
{
	// Adding a fence point might change the fence extents such that it moves
	// to a new LOD cell.  So, remove it from the LOD grid, add the point,
	// then add it back.
	m_pLodGrid->RemoveFromGrid(f->GetGeom());

	f->AddPoint(epos);
	f->BuildGeometry(m_pHeightField);

	AddNodeToLodGrid(f->GetGeom());
}

void vtTerrain::RedrawFence(vtFence3d *f)
{
	f->BuildGeometry(m_pHeightField);
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

void vtTerrain::add_routepoint_earth(vtRoute *f, const DPoint2 &epos,
									 const char *structname)
{
	f->AddPoint(epos, structname);
	f->BuildGeometry(m_pHeightField);
}

void vtTerrain::RedrawRoute(vtRoute *f)
{
	f->BuildGeometry(m_pHeightField);
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

	vtMaterialArray *pApp_Ocean = new vtMaterialArray();

	if (bWater)
	{
		// create ocean material: texture waves
		vtString fname = FindFileOnPaths(m_DataPaths, "GeoTypical/ocean1_256.bmp");
		pApp_Ocean->AddTextureMaterial2(fname,
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
		pApp_Ocean->AddRGBMaterial1(RGBf(1.0f, 0.8f, 0.6f),	// tan ground
			false, true, false);		// cull, light, wire
		VtxType = VT_Normals;
	}

	vtGeom *pGeom = new vtGeom();
	pGeom->SetMaterials(pApp_Ocean);

	TerrainPatch *geo;
	float width, depth;
	int i, j;

	FRECT world_extents = m_pHeightField->m_Conversion.m_WorldExtents;
	FPoint2 world_size(world_extents.Width(), world_extents.Height());

	width = (float) world_size.x;
	depth = (float) world_size.y;
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

			geo = new TerrainPatch(VtxType, 4);
			geo->MakeGrid(1, 1, width/1, depth/1,
				world_extents.left + (i * width),
				world_extents.bottom - (j * depth), 5.0f, 5.0f);

			pGeom->AddMesh(geo, 0);	// actually add
		}
	}

	pGeom->SetName2("Horizon Plane");

	m_pOceanGeom = new vtMovGeom(pGeom);

	// fudge ocean downward, to reduce z-buffer collision with near-sea-level
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
	vtString grid_fname = FindFileOnPaths(m_DataPaths, name);
	if (grid_fname == "")
		return false;
	else
		return grid.LoadBTHeader(grid_fname);
}


///////////////////////////////////////////////
// Built Structures

void vtTerrain::CreateStructuresFromXML(vtString strFilename)
{
	if (!m_Structures.ReadXML(strFilename))
	{
		// Format("Couldn't load file %s", strFilename);
		return;
	}
	int num_structs = m_Structures.GetSize();

	m_Structures.SetHeightField(m_pHeightField);
	for (int i = 0; i < num_structs; i++)
	{
		vtStructure3d *str = (vtStructure3d *) m_Structures.GetAt(i);

		const char *options = NULL;
		if (str->GetType() == ST_BUILDING)
			options = "roof walls detail";

		// Construct
		bool bSuccess = m_Structures.ConstructStructure(str, options);
		if (!bSuccess)
			continue;

		vtTransform *pTrans = str->GetTransform();
		if (pTrans)
		{
			AddNodeToLodGrid(pTrans);
		}
		else
		{
			vtGeom *pGeom = str->GetGeom();
			if (pGeom)
				AddNodeToLodGrid(pGeom);
		}
	}
}

/**
 * Loads an external 3d model as a movable node.  The file will be looked for
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
	vtNode *node = vtLoadModel(filename);
	if (!node)
	{
		vtString path = FindFileOnPaths(m_DataPaths, filename);
		if (path != "")
		{
			vtNode *node = vtLoadModel(path);
		}
	}
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
	FPoint3 pos;

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


void vtTerrain::create_culture(bool bSound)
{
	// m_iTreeDistance is in kilometers, so multiply to get meters
	setup_LodGrid(m_Params.m_iTreeDistance * 1000.0f);

	// create roads
	if (m_Params.m_bRoads)
	{
		vtString road_fname = "RoadData/";
		road_fname += m_Params.m_strRoadFile;
		vtString road_path = FindFileOnPaths(m_DataPaths, road_fname);
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

	// create trees
	m_PIA.SetHeightField(m_pHeightField);
	m_PIA.SetPlantList(m_pPlantList);
	if (m_Params.m_bTrees)
	{
		// Read the VF file
		vtString plants_fname = "PlantData/";
		plants_fname += m_Params.m_strTreeFile;
		vtString plants_path = FindFileOnPaths(m_DataPaths, plants_fname);
		bool success = m_PIA.ReadVF(plants_path);
		if (success)
		{
			// Create the 3d plants
			m_PIA.CreatePlantNodes();

			int i, size = m_PIA.GetSize();
			for (i = 0; i < size; i++)
			{
				vtTransform *pTrans = m_PIA.GetPlantNode(i);

				// add tree to scene graph
				if (pTrans)
					AddNodeToLodGrid(pTrans);
			}
		}
	}

	// create built structures
	if (m_Params.m_bBuildings)
	{
		vtString building_fname = "BuildingData/";
		building_fname += m_Params.m_strBuildingFile;
		vtString building_path = FindFileOnPaths(m_DataPaths, building_fname);
		CreateStructuresFromXML(building_path);
	}
	else
	{
		// No structures loaded, but the might create some later, so set
		// the projection to match the terrain.
		m_Structures.m_proj = m_proj;
	}

	// create utility structures (routes = towers and wires)
	if (m_Params.m_bRouteEnable && m_Params.m_strRouteFile[0] != '\0')
	{
		// TODO
	}

	CreateCustomCulture(bSound);
}


//
// Create an LOD grid to contain and efficiently hide the culture
//  (trees, buildings) far away from the camera
//
void vtTerrain::setup_LodGrid(float fLODDistance)
{
	// must have a terrain with some size
	if (!m_pHeightField)
		return;

	FRECT world_extents;
	world_extents = m_pHeightField->m_Conversion.m_WorldExtents;

	FPoint3 org(world_extents.left, 0.0f, world_extents.bottom);
	FPoint3 size(world_extents.right, 0.0f, world_extents.top);

	m_pLodGrid = new vtLodGrid(org, size, LOD_GRIDSIZE, fLODDistance, m_pHeightField);
	m_pLodGrid->SetName2("LOD Grid");
	m_pTerrainGroup->AddChild(m_pLodGrid);
	m_pTreeGroup = m_pLodGrid;
}


/////////////////////////

void vtTerrain::create_floating_labels(const char *filename)
{
	// create container group
	vtGroup *pPlaceNames = new vtGroup();
	pPlaceNames->SetName2("Place Names");
	m_pTerrainGroup->AddChild(pPlaceNames);

	FILE *fp = fopen(filename, "r");
	char string[80];
	float utm_x, utm_y;
	int meter_height;
	FPoint3 p3;

	vtString font_path = FindFileOnPaths(m_DataPaths, "Fonts/Arial.ttf");
	if (font_path == "")
		return;

	vtMaterialArray *pMats = new vtMaterialArray();
	int index = pMats->AddRGBMaterial1(RGBf(1,1,1), false, false);
	vtFont *font = new vtFont;
	bool success = font->LoadFont(font_path);

	while( !feof(fp) )
	{
		int ret = fscanf(fp, "%f %f %d %s\n", &utm_x, &utm_y, &meter_height, string);
		if (!ret) break;

		vtTransform *bb = new vtTransform();
#if 0
		vtBillBoard *bb = new vtBillBoard();
		bb->SetName2(string);
		bb->SetText(string);
		bb->SetForeColor(RGBf(1.0f, 1.0f, 0.0f));
		bb->SetTextFont("Data/Fonts/default.txf");
		bb->SetKind(BB_Axial);
		FPoint3 yvec(0.0f, 1.0f, 0.0f);
		bb->SetAxis(yvec);
#endif
		vtTextMesh *text = new vtTextMesh(font, true);	// center
		text->SetText(string);
		vtGeom *geom = new vtGeom();
		geom->SetName2(string);
		geom->SetMaterials(pMats);
		geom->AddText(text, index);

		bb->AddChild(geom);

		float width = 250.0f;
		float height = 250.0f;
//		float scale_x = width;
		bb->Scale3(width/20, height/20, 1.0f);

		m_pHeightField->ConvertEarthToSurfacePoint(utm_x, utm_y, p3);
		p3.y += (200.0f + meter_height);
		bb->SetTrans(p3);
		pPlaceNames->AddChild(bb);
	}
	fclose(fp);
}


bool vtTerrain::CreateStep1(int &iError)
{
	// create terrain group - this holds all surfaces for the terrain
	m_pTerrainGroup = new vtGroup();
	m_pTerrainGroup->SetName2("Terrain Group");

	if (m_pInputGrid)
	{
		m_pLocalGrid = m_pInputGrid;
		return true;
	}
	vtString fname = "Elevation/";
	fname += m_Params.m_strElevFile;
	vtString fullpath = FindFileOnPaths(m_DataPaths, fname);
	if (fullpath == "")
	{
		iError = TERRAIN_ERROR_NOTFOUND;
		return false;
	}
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
		m_pLocalGrid = new vtLocalGrid();
		bool status = m_pLocalGrid->LoadFromBT(fullpath);
		if (status == false)
		{
			iError = TERRAIN_ERROR_NOTFOUND;
			return false;
		}
		m_pLocalGrid->SetupConversion(m_Params.m_fVerticalExag);
	}
	return true;
}

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
		m_proj = m_pLocalGrid->GetProjection();
		g_Conv = m_pLocalGrid->m_Conversion;

		create_textures();
	}
	return true;
}

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
	float fOceanDepth;
	if (m_Params.m_bDepressOcean)
		fOceanDepth = m_Params.m_fDepressOceanLevel;
	else
		fOceanDepth = 0.0f;

	if (m_Params.m_bRegular)
	{
		// first, create brute-force regular terrain
		if (!create_regular_terrain(fOceanDepth))
		{
			//AfxMessageBox("Couldn't create regular terrain");
			iError = TERRAIN_ERROR_NOREGULAR;
			return false;
		}
		else
		{
			m_pTerrainGeom->SetEnabled(true);
			m_pHeightField = m_pTerrainGeom;
		}
	}

	tm = clock();
	//
	if (m_Params.m_bDynamic)
	{
		// then, the somewhat more elegant dynamic LOD terrain
		if (!create_dynamic_terrain(fOceanDepth, iError))
		{
			//AfxMessageBox("Couldn't create dynamic LOD terrain");
			return false;
		}
		else
		{
			m_pDynGeom->SetEnabled(!m_Params.m_bRegular);
			m_pHeightField = m_pDynGeom;
		}
	}

	if (!m_bPreserveInputGrid)
	{
		// we don't need the original grid any more
		delete m_pLocalGrid;
		m_pLocalGrid = NULL;
	}
	return true;
}

bool vtTerrain::CreateStep4(int &iError)
{
	// some algorithms need an additional stage of initialization
	if (m_pDynGeom != NULL)
		m_pDynGeom->Init2();

	clock_t tm2 = clock();
	float time = ((float)tm2 - tm)/CLOCKS_PER_SEC;
//	FILE *fp = fopen("time.txt", "wb");
//	fprintf(fp, "time %.2f\n", time);
//	fclose(fp);
	return true;
}

bool vtTerrain::CreateStep5(bool bSound, int &iError)
{
	// must have a heightfield by this point
	if (!m_pHeightField)
		return false;

	create_culture(bSound);

	if (m_Params.m_bOceanPlane || m_Params.m_bHorizon)
	{
		bool bWater = m_Params.m_bOceanPlane;
		bool bHorizon = m_Params.m_bHorizon;
		bool bCenter = m_Params.m_bOceanPlane;
		create_artificial_horizon(bWater, bHorizon, bCenter, 0.5f);
	}

	if (m_Params.m_bLabels)
	{
		vtString labels_path = FindFileOnPaths(m_DataPaths, "PointData/places.txt");
		if (labels_path != "")
			create_floating_labels(labels_path);
	}

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
	else if (m_pTerrainGeom != NULL)
		m_pTerrainGeom->GetBoundSphere(m_bound_sphere);
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
	return m_pHeightField->PointIsInTerrain(x, z);
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
	for (int k = 0; k < m_Engines.GetSize(); k++)
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
	case TFT_REGULAR:
		if (m_pTerrainGeom)
			m_pTerrainGeom->SetEnabled(bOn);
		break;
	case TFT_OCEAN:
		if (m_pOceanGeom)
			m_pOceanGeom->SetEnabled(bOn);
		break;
	case TFT_VEGETATION:
		if (m_pTreeGroup)
			m_pTreeGroup->SetEnabled(bOn);
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
	case TFT_REGULAR:
		if (m_pTerrainGeom)
			return m_pTerrainGeom->GetEnabled();
		break;
	case TFT_OCEAN:
		if (m_pOceanGeom)
			return m_pOceanGeom->GetEnabled();
		break;
	case TFT_VEGETATION:
		if (m_pTreeGroup)
			return m_pTreeGroup->GetEnabled();
		break;
	case TFT_ROADS:
		if (m_pRoadGroup)
			return m_pRoadGroup->GetEnabled();
		break;
	}
	return false;
}


void vtTerrain::CreateChoppedTextures(vtLocalGrid *pLocalGrid, vtDIB *dib1,
									  int patches, int patch_size)
{
	int size = patch_size;
	bool mono = (dib1->GetDepth() == 8);

	int x_off, y_off, x, y, i, j;

	for (i = 0; i < patches; i++)
	{
		x_off = i * (size - 1);
		for (j = 0; j < patches; j++)
		{
			y_off = j * (size - 1);

			// make a tile
			vtDIB *dib2 = new vtDIB(size, size, dib1->GetDepth(), mono);

			unsigned long pixel;
			if (mono)
			{
				for (x = 0; x < size; x++)
					for (y = 0; y < size; y++)
					{
						pixel = dib1->GetPixel8(x_off + x, y_off + y);
						dib2->SetPixel8(x, y, pixel);
					}
			}
			else
			{
				for (x = 0; x < size; x++)
					for (y = 0; y < size; y++)
					{
						pixel = dib1->GetPixel24(x_off + x, y_off + y);
						dib2->SetPixel24(x, y, pixel);
					}
			}

			vtImage *pImage = new vtImage(dib2, (!mono && m_Params.m_b16bit) ? GL_RGB5 : -1);

			// Can't delete the internals DIBs here because the scene graph
			//   needs the data?  Actually no, the scene graph gets a copy of it.
//			dib2->LeaveInternalDIB(true);
			delete dib2;

			m_Images.SetAt(i*patches+j, pImage);
		}
	}
}


/*
 * Creates an array of materials for the brute force terrain geometry.
 */
void vtTerrain::_CreateTiledMaterials1(vtMaterialArray *pApp1,
							 int patches, int patch_size, float ambient,
							 float diffuse, float emmisive)
{
	int size = patch_size;

	int i, j;

	m_pCoverage = new vtTextureCoverage[patches*patches];
	for (i = 0; i < patches*patches-1; i++)
		m_pCoverage[i].m_pNext = m_pCoverage + (i+1);
	m_pCoverage[i].m_pNext = NULL;

	for (i = 0; i < patches; i++)
	{
		for (j = 0; j < patches; j++)
		{
			vtImage *image = m_Images.GetAt(i*patches+j);
			int idx = pApp1->AddTextureMaterial(image,
				true,		// culling
				!m_Params.m_bPreLit, // lighting
				false,		// transparency
				false,		// additive
				ambient, diffuse,
				1.0f,		// alpha
				emmisive,
				false,			// texgen
				false,			// clamp
				m_Params.m_bMipmap);
			m_pCoverage[i*patches+j].m_appidx = idx;
		}
	}

	for (i = 0; i < patches; i++)
	{
		for (int j = 0; j < patches; j++)
		{
			m_pCoverage[i*patches+(patches-1-j)].m_xmin = (1.0f / patches) * i;
			m_pCoverage[i*patches+(patches-1-j)].m_xmax = (1.0f / patches) * (i+1);
			m_pCoverage[i*patches+(patches-1-j)].m_zmin = -(1.0f / patches) * j;
			m_pCoverage[i*patches+(patches-1-j)].m_zmax = -(1.0f / patches) * (j+1);
		}
	}
}


/*
 * Creates an array of materials for the dynamic LOD terrain geometry.
 */
void vtTerrain::_CreateTiledMaterials2(vtMaterialArray *pApp1,
							 int patches, int patch_size, float ambient,
							 float diffuse, float emmisive)
{
	int i, j;

	for (i = 0; i < patches; i++)
	{
		for (j = 0; j < patches; j++)
		{
			vtImage *image = m_Images.GetAt(i*patches+j);
			pApp1->AddTextureMaterial(image,
				true, 		// culling
				!m_Params.m_bPreLit, // lighting
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


void vtTerrain::ApplyPreLight(vtLocalGrid *pLocalGrid, vtDIB *dib)
{
	FPoint3 light_dir;
	light_dir.Set(-1.0f, -1.0f, 0.0f);
	light_dir.Normalize();

	clock_t c1 = clock();
	pLocalGrid->ShadeDibFromElevation(dib, light_dir, m_Params.m_fPreLightFactor);
	clock_t c2 = clock();

	clock_t c3 = c2 - c1;
	int foo = 1;

	// must also adjust ocean color to match
//	FPoint3 up(0.0f, -1.0f, 0.0f);
//	float shade = up.Dot(light_dir);	// shading 0 (dark) to 1 (light)
//	m_ocean_color *= shade;
}

void vtTerrain::AddPointOfInterest(double ulx, double uly, double brx, double bry,
								  const char *name, const char *url)
{
	POIPtr p = new vtPointOfInterest();
	p->m_rect.SetRect(ulx, uly, brx, bry);
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

	vtMesh *pGeom = new vtMesh(GL_LINES, 0, STEPS*4);

	FPoint3 v1, v2, v3, v4, v;
	g_Conv.convert_earth_to_local_xz(poi->m_rect.left, poi->m_rect.top, v1.x, v1.z);
	g_Conv.convert_earth_to_local_xz(poi->m_rect.right, poi->m_rect.top, v2.x, v2.z);
	g_Conv.convert_earth_to_local_xz(poi->m_rect.right, poi->m_rect.bottom, v3.x, v3.z);
	g_Conv.convert_earth_to_local_xz(poi->m_rect.left, poi->m_rect.bottom, v4.x, v4.z);

	int i;
	for (i = 0; i < STEPS; i++)
	{
		v.Set(v1.x + (v2.x - v1.x) / STEPS * i, 0.0f, v1.z + (v2.z - v1.z) / STEPS * i);
		m_pHeightField->FindAltitudeAtPoint(v, v.y);
		v.y += 10.0f;
		pGeom->AddVertex(v);
	}
	for (i = 0; i < STEPS; i++)
	{
		v.Set(v2.x + (v3.x - v2.x) / STEPS * i, 0.0f, v2.z + (v3.z - v2.z) / STEPS * i);
		m_pHeightField->FindAltitudeAtPoint(v, v.y);
		v.y += 10.0f;
		pGeom->AddVertex(v);
	}
	for (i = 0; i < STEPS; i++)
	{
		v.Set(v3.x + (v4.x - v3.x) / STEPS * i, 0.0f, v3.z + (v4.z - v3.z) / STEPS * i);
		m_pHeightField->FindAltitudeAtPoint(v, v.y);
		v.y += 10.0f;
		pGeom->AddVertex(v);
	}
	for (i = 0; i < STEPS; i++)
	{
		v.Set(v4.x + (v1.x - v4.x) / STEPS * i, 0.0f, v4.z + (v1.z - v4.z) / STEPS * i);
		m_pHeightField->FindAltitudeAtPoint(v, v.y);
		v.y += 10.0f;
		pGeom->AddVertex(v);
	}


	poi->m_pGeom = new vtGeom();

	vtMaterialArray *pApp = new vtMaterialArray();
	pApp->AddRGBMaterial(RGBf(1.0f, 0.0f, 0.0f),
		RGBf(0.2f, 0.0f, 0.0f), true, false); // red

	poi->m_pGeom->SetMaterials(pApp);
	poi->m_pGeom->SetName2("POI group");
	poi->m_pGeom->AddMesh(pGeom, 0);

	m_pTerrainGroup->AddChild(poi->m_pGeom);
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

const char *vtTerrain::DesribeError(int iError)
{
	switch (iError)
	{
	case TERRAIN_ERROR_NOTFOUND: return "The terrain data file was not found.";
	case TERRAIN_ERROR_NOREGULAR: return "The regular grid terrain could not be constructed.";
	case TERRAIN_ERROR_NOTPOWER2: return "The elevation data is of an unsupported size.\n\
The continuous LOD algorithms require that the data is\n\
square and the dimensions are a power of 2 plus 1.\n\
For example, 513x513 and 1025x105 are supported sizes.";
	case TERRAIN_ERROR_NOMEM: return "Not enough memory.";
	}
	return "No error.";
}


void vtTerrain::AddPlant(const DPoint2 &pos, int iSpecies, float fSize)
{
	vtPlantInstance pi;
	pi.m_p = pos;
	pi.species_id = iSpecies;
	pi.size = fSize;

	int num = m_PIA.Append(pi);
	m_PIA.CreatePlantNode(num);

	vtTransform *pTrans = m_PIA.GetPlantNode(num);

	// add tree to scene graph
	AddNodeToLodGrid(pTrans);
}


/**
 * Adds a node to the terrain.
 * The node will be added directly, so it is always in the scene whenever
 * the terrain is visible.
 *
 * \sa AddNodeToLodGrid
 */
void vtTerrain::AddNode(vtNode *pNode)
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
void vtTerrain::AddNodeToLodGrid(vtTransform *pTrans)
{
	if (m_pLodGrid)
		m_pLodGrid->AppendToGrid(pTrans);
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
void vtTerrain::AddNodeToLodGrid(vtGeom *pGeom)
{
	if (m_pLodGrid)
		m_pLodGrid->AppendToGrid(pGeom);
}


