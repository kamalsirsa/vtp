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
vtString  vtTerrain::m_strDataPath;

//////////////////////////////////////////////////////////////////////

vtTerrain::vtTerrain()
{
//	m_ocean_color.Set(8.0f/255, 0.0f/255, 74.0f/255);
//	m_ocean_color.Set(8.0f/255, 0.0f/255, 99.0f/255);
//	m_ocean_color.Set(194.0f/255, 0.0f/255, 112.0f/255);	// unshaded color
	m_ocean_color.Set(40.0f/255, 75.0f/255, 124.0f/255);	// unshaded color

	m_pTerrainGroup = NULL;
	m_pDIB = NULL;
	m_pCoverage = NULL;
	m_pTerrApps1 = NULL;
	m_pTerrApps2 = NULL;
	m_pRoadMap = NULL;
	m_pHeightField = NULL;
	m_pImage = NULL;
	m_pLocalGrid = NULL;
	m_pLodGrid = NULL;

	m_pOceanGeom = NULL;
	m_pRoadGroup = NULL;
	m_pTreeGroup = NULL;

	m_bShowPOI = true;
	m_pPlantList = NULL;

	m_pDynGeom = NULL;
	m_pTerrainGeom = NULL;
	m_pNext = NULL;

	m_CamLocation.Identity();
}

vtTerrain::~vtTerrain()
{
	// some things need to be manually deleted
	if (m_pCoverage) delete m_pCoverage;

	if (m_pRoadMap) delete m_pRoadMap;

	int i, size = m_PointsOfInterest.GetSize();
	for (i = 0; i < size; i++)
	{
		POIPtr p = m_PointsOfInterest.GetAt(i);
		delete p;
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
		m_strDataPath);
	m_pTerrainGroup->AddChild(m_pRoadGroup);
	if (m_Params.m_bRoadCulture)
		m_pRoadMap->GenerateSigns(m_pLodGrid);
}


///////////////////

void vtTerrain::create_textures(int iTiles,
								const char *szTextureFile)
{
	TextureEnum eTex = m_Params.m_eTexture;

	m_pTerrApps1 = new vtMaterialArray();
	m_pTerrApps2 = new vtMaterialArray();

//	int minfilter = m_Params.m_bMipmap ? APP_TriLinear : APP_Linear;

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

	if (eTex == TE_NONE)	// none
	{
		// no texture
		m_pTerrApps1->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
									 RGBf(0.2f, 0.2f, 0.2f),
									 true, !m_Params.m_bPreLit);	// for shaded white
		m_pTerrApps2->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
									 RGBf(0.2f, 0.2f, 0.2f),
									 true, !m_Params.m_bPreLit);	// for shaded white
//		AddRGBMaterial(m_pTerrApps, 1.0f, 1.0f, 1.0f, false, true);	// for shaded white, bothsides
//		AddRGBMaterial(m_pTerrApps, 1.0f, 1.0f, 1.0f, true, false);	// for vertex color
	}
	if (eTex == TE_SINGLE || eTex == TE_TILED)	// load texture
	{
		// Load a DIB of the whole, large texture
		m_pDIB = new vtDIB(szTextureFile);
		if (! m_pDIB->m_bLoadedSuccessfully)
		{
			m_pTerrApps1->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
										 RGBf(0.2f, 0.2f, 0.2f),
										 true, !m_Params.m_bPreLit);	// for shaded white
			m_pTerrApps2->AddRGBMaterial(RGBf(1.0f, 1.0f, 1.0f),
										 RGBf(0.2f, 0.2f, 0.2f),
										 true, !m_Params.m_bPreLit);	// for shaded white
			m_Params.m_eTexture = TE_NONE;
			return;
		}
	}
	if (eTex == TE_DERIVED)
	{
		// derive color from elevation
		m_pDIB = new vtDIB(1024, 1024, 24, false);
		m_pLocalGrid->ColorDibFromElevation(m_pDIB, RGBi(m_ocean_color));
	}

	// apply pre-lighting (darkening)
	if (m_Params.m_bPreLight)
		ApplyPreLight(m_pLocalGrid, m_pDIB);

	if (eTex == TE_SINGLE || eTex == TE_DERIVED)
	{
		// single texture
		m_pDIB->LeaveInternalDIB(true);
		m_pImage = new vtImage(m_pDIB,
			(m_pDIB->GetDepth() > 8 && m_Params.m_b16bit) ? GL_RGB5 : -1);
	}
	if (eTex == TE_TILED)
	{
		m_pDIB->LeaveInternalDIB(false);

		CreateChoppedTextures(m_pLocalGrid, m_pDIB, iTiles, m_Params.m_iTilesize);
		if (m_Params.m_bRegular)
			CreateChoppedAppearances1(m_pTerrApps1,
							 iTiles, m_Params.m_iTilesize, ambient, diffuse,
							 emmisive);
		if (m_Params.m_bDynamic)
			CreateChoppedAppearances2(m_pTerrApps2,
							 iTiles, m_Params.m_iTilesize, ambient, diffuse,
							 emmisive);
	}
	// We're not going to use it for tree planting, we're done with the DIB
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
							fOceanDepth * WORLD_SCALE, bLighting);

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

#if 0
	/*
     * Set the debug-heap flag to keep freed blocks in the
     * heap's linked list - This will allow us to catch any
     * inadvertent use of freed memory
	 */
	int tmpDbgFlag;
	tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);
#endif

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
				   WORLD_SCALE * m_Params.m_fVerticalExag, fOceanDepth, iError);
	if (result == false)
	{
		m_pDynGeom = NULL;
		return false;
	}

	m_pDynGeom->SetPixelError(m_Params.m_fPixelError);
	m_pDynGeom->SetPolygonCount(m_Params.m_iTriCount);
	m_pDynGeom->SetMaterials(m_pTerrApps2);

	// build heirarchy (add terrain to scene graph)
	vtTransform *trans = new vtTransform();

	DPoint2 spacing = m_pLocalGrid->GetWorldSpacing();
	trans->Scale3(spacing.x, WORLD_SCALE * m_Params.m_fVerticalExag, -spacing.y);

	trans->AddChild(m_pDynGeom);
	m_pTerrainGroup->AddChild(trans);

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
	m_pRoutes.Append(f);

	// Add directly
	m_pTerrainGroup->AddChild(f->GetGeom());

	// Or add to LOD grid (much more efficient)
//	AddNodeToLodGrid(f->GetGeom());
}

void vtTerrain::add_routepoint_earth(vtRoute *f, const DPoint2 &epos)
{
	f->add_point(epos);
	f->BuildGeometry(m_pHeightField);
}

void vtTerrain::RedrawRoute(vtRoute *f)
{
	f->BuildGeometry(m_pHeightField);
}

void vtTerrain::LoadRoute(float fRouteOffL, float fRouteOffR,
						  float fRouteStInc, vtString sRouteName)
{
	vtRoute *pRoute;
	bool stop = false;

	//parse the parameter string passed in
	char  *end;
	char holder[40];
	strcpy(holder, m_Params.m_strRouteFile);
	if ((end = strstr(holder, ".p3D"))!=0)
		holder[end-holder]='\0';
	else
		return;
	vtString filename = m_strDataPath + "RouteData/";
	filename += holder;
	vtString logFile = filename+".xyz";
	vtString p3DFile = filename+".p3D";

	FILE *fplog = fopen(logFile, "r"); // m_strDatapath +
	if (!fplog)
		return;
	FILE *fpp3D = fopen(p3DFile, "r"); // m_strDatapath +
	if (!fpp3D)
		return;

	pRoute = new vtRoute(fRouteOffL, fRouteOffR, fRouteStInc, sRouteName,
		this);

	if (pRoute->logReader(fplog))
	{
		if (pRoute->p3DReader(fpp3D))
		{
			// Get the needed tower images
			if (!pRoute->StructureReader(m_strDataPath + "RouteData/"))
				return;	//error loading structures.  don't display these routes
			AddRoute(pRoute);
			pRoute->BuildGeometry(m_pHeightField);
			m_pRoutes.Append(pRoute);

			// Add directly
			m_pTerrainGroup->AddChild(pRoute->GetGeom());
		}
		else
			delete pRoute;
	}
	else //no route read
		delete pRoute;

//	while (!stop)  // not EOF
//	{
//		if (strncmp(buf, "Begin", 5) == 0) // begin building route
//		{
//			// default for now? (TODO)
//			pRoute = new vtRoute(fRouteOffL, fRouteOffR, fRouteStInc, sRouteName);
//			stop = pRoute->load(fp);
//			AddRoute(pRoute);
//			pRoute->BuildGeometry(m_pHeightField);
//		}
//	}
	fclose(fplog);
	fclose(fpp3D);
}

void vtTerrain::SaveRoute()
{
	vtString route_fname = m_strDataPath;
	route_fname += "RouteData/testsave.fmf"; // fix this

	int numroutes = m_pRoutes.GetSize();

	vtRoute *temproute;

	FILE *fp = fopen(route_fname, "w");
	if (fp)
	{
//		fwrite("FMF1.0\n", 7, 1, fp);  // header

		for (int i = 0; i < numroutes; i++)  // save each route
		{
			temproute = m_pRoutes.GetAt(i);
			temproute->save(fp);
		}

//		fwrite("EOF", 3, 1, fp);
		fclose(fp);
	}
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
		vtString str = m_strDataPath + "GeoTypical/ocean1_256.bmp";
		pApp_Ocean->AddTextureMaterial2(str,
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

	FPoint2 world_size;
	FRECT world_extents;
	m_pHeightField->GetWorldSize(world_size);
	m_pHeightField->GetWorldExtents(world_extents);

	width = (float) world_size.x;
	depth = (float) world_size.y;
	for (i = -3; i < 4; i++)
	{
		for (j = -3; j < 4; j++)
		{
			// don't need to do far corners
			if ((i == -3 || i == 3) && (j == -3 || j == 3)) continue;
			if (i == 0 && j == 1) {
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
	float level = m_Params.m_fOceanPlaneLevel * WORLD_SCALE;
	m_pOceanGeom->Translate1(FPoint3(0.0f, level, 0.0f));

	m_pTerrainGroup->AddChild(m_pOceanGeom);
}


//
// set global projection based on this terrain's elevation grid
//
void vtTerrain::SetGlobalProjection()
{
	if (!m_pHeightField)
		return;

	// set the base coordinate at the lower-left corner of the terrain
	DRECT area;
	m_pHeightField->GetEarthExtents(area);
	vtProjection &proj = m_pHeightField->GetProjection();

	SetLocalProjection(proj, DPoint2(area.left, area.bottom));

	g_Proj.SetVerticalScale(WORLD_SCALE * m_Params.m_fVerticalExag);
}

bool vtTerrain::LoadHeaderIntoGrid(vtElevationGrid &grid)
{
	vtString grid_fname = m_strDataPath;
	grid_fname += "Elevation/";
	grid_fname += m_Params.m_strElevFile;

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
		if (str->GetType() == ST_INSTANCE)
			options = m_strDataPath;

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
 * scale.  If the units are of the model are meters, you should scale by
 * <b>WORLD_SCALE</b> to that it matches the units of the Terrain:
 *
 * \par Example:
	\code
MyTerrain::CreateCustomCulture(bool bSound)
{
	vtTransform *pFountain = LoadModel("Culture/fountain.3ds");

	pFountain->Scale3(WORLD_SCALE, WORLD_SCALE, WORLD_SCALE);

	PlantModelAtPoint(pFountain, DPoint2(217690, 4123475));
	AddModel(pFountain);
}
	\endcode
 */
vtTransform *vtTerrain::LoadModel(const char *filename)
{
	vtNode *node = vtLoadModel(m_strDataPath + filename);
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
 * \param bGeo true if the position is given in geographic coordinates
 * (longitude, latitude), otherwise it is assumed to be in the meters-based
 * projection that the Terrain is using.
 *
 * Note: this function does not add the model to the terrain's scene
 * graph.  Use <b>AddNode</b> for that operation.
 */
void vtTerrain::PlantModelAtPoint(vtTransform *model, const DPoint2 &pos, bool bGeo)
{
	FPoint3 wpos;

	if (bGeo)
		g_Proj.convert_latlon_to_local_xz(pos.y, pos.x, wpos.x, wpos.z);
	else
		g_Proj.convert_meters_to_local_xz(pos.x, pos.y, wpos.x, wpos.z);
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
		vtString road_fname = m_strDataPath;
		road_fname += "RoadData/";
		road_fname += m_Params.m_strRoadFile;
		create_roads(road_fname);

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
	if (m_Params.m_bTrees)
	{
		// Read the VF file
		vtString plants_fname = m_strDataPath;
		plants_fname += "PlantData/";
		plants_fname += m_Params.m_strTreeFile;
		bool success = m_PIA.ReadVF(plants_fname);
		if (success)
		{
			// Create the 3d plants
			CreatePlantsFromPIA();
		}
	}

	// create built structures
	if (m_Params.m_bBuildings)
	{
		vtString building_fname = m_strDataPath;
		building_fname += "BuildingData/";
		building_fname += m_Params.m_strBuildingFile;
		CreateStructuresFromXML(building_fname);
	}
	else
	{
		// No structures loaded, but the might create some later, so set
		// the projection to match the terrain.
		m_Structures.m_proj = m_pHeightField->GetProjection();
	}

	// create utility structures (routes = towers and wires)
	if (m_Params.m_bRouteEnable && m_Params.m_strRouteFile[0] != '\0')
	{
//		LoadRoute(m_fRouteOffL, m_fRouteOffR, m_fRouteStInc, m_sRouteName);
		LoadRoute(0.0f, 0.0f, 0.0f, "route1");
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

	FPoint2 world_size;
	FRECT world_extents;
	m_pHeightField->GetWorldSize(world_size);
	m_pHeightField->GetWorldExtents(world_extents);

	FPoint3 org(world_extents.left, 0.0f, world_extents.bottom);
	FPoint3 size(world_size.x, 0.0f, -world_size.y);

	m_pLodGrid = new vtLodGrid(org, size, LOD_GRIDSIZE, fLODDistance * WORLD_SCALE, m_pHeightField);
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

	while( !feof(fp) )
	{
		int ret = fscanf(fp, "%f %f %d %s\n", &utm_x, &utm_y, &meter_height, string);
		if (!ret) break;

#if 0
		vtBillBoard *bb = new vtBillBoard();
		bb->SetName2(string);
#if 0
		bb->SetText(string);
		bb->SetForeColor(RGBf(1.0f, 1.0f, 0.0f));
		bb->SetTextFont("Data/Fonts/default.txf");
		bb->SetKind(BB_Axial);
#endif
		FPoint3 yvec(0.0f, 1.0f, 0.0f);
		bb->SetAxis(yvec);

		// For some reason, Z scale must be set the same as X scale?
		float width = 1000.0f * WORLD_SCALE;
		float height = 250.0f * WORLD_SCALE;
		float scale_x = width;
		bb->Scale2(scale_x, height, scale_x);

		m_pHeightField->ConvertEarthToSurfacePoint(utm_x, utm_y, p3);
		p3.y += ((200.0f + meter_height) * WORLD_SCALE);
		bb->SetTrans(p3);
		pPlaceNames->AddChild(bb);
#endif
	}
	fclose(fp);
}


bool vtTerrain::CreateStep1(int &iError)
{
//	ISMSurface::DoOptimize = false;

	// create terrain group - this holds all surfaces for the terrain
	m_pTerrainGroup = new vtGroup();
	m_pTerrainGroup->SetName2("Terrain Group");

	// Loading elevation grid...
	vtString grid_fname = m_strDataPath;
	grid_fname += "Elevation/";
	grid_fname += m_Params.m_strElevFile;

	m_pLocalGrid = new vtLocalGrid();
	bool status = m_pLocalGrid->LoadFromBT(grid_fname);
	if (status != true)
	{
		iError = TERRAIN_ERROR_NOTFOUND;
		return false;
	}
	return true;
}

bool vtTerrain::CreateStep2(int &iError)
{
	// set global projection based on this terrain
	m_pLocalGrid->SetGlobalProjection();
	g_Proj.SetVerticalScale(WORLD_SCALE * m_Params.m_fVerticalExag);

	// set up ability to do terrain following on source array
	m_pLocalGrid->Setup(m_Params.m_fVerticalExag);

	vtString texture_fname = m_strDataPath;
	texture_fname += "GeoSpecific/";
	if (m_Params.m_eTexture == TE_SINGLE)
		texture_fname += m_Params.m_strTextureSingle;	// single texture
	else
		texture_fname += m_Params.m_strTextureFilename;

	create_textures(4, texture_fname);
	return true;
}

bool vtTerrain::CreateStep3(int &iError)
{
	float fOceanDepth;
	if (m_Params.m_bDepressOcean)
		fOceanDepth = m_Params.m_fDepressOceanLevel;
	else
		fOceanDepth = 0.0f;

	// first, create brute-force regular terrain
	//
	if (m_Params.m_bRegular)
	{
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

	// then, the somewhat more elegant dynamic LOD terrain
	//
	if (m_Params.m_bDynamic)
	{
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

#if 0
	// create a box the size of the local grid
	FRECT rect;
	FPoint3 size;
	m_pLocalGrid->GetWorldExtents(rect);
	size.x = rect.Width();
	size.z = fabs(rect.Height());
	m_pLocalGrid->FindHeightExtents();
	size.y = (m_pLocalGrid->m_fMaxHeight - m_pLocalGrid->m_fMinHeight)* WORLD_SCALE;
	vtMesh *pMesh = new vtMesh(GL_TRIANGLE_FAN, 0, 4);
//	pMesh->CreateBlock(size);
	pMesh->CreateRectangle(size.x, size.z);
	vtGeom *pGeom = new vtGeom();
	pGeom->AddMesh(pMesh);
	pGeom->SetName2("TerrainBoxGeom");
	// add box to scene graph
	m_pTerrainGroup->AddChild(pGeom);
#endif

	// we don't need the original grid any more
	delete m_pLocalGrid;
	m_pLocalGrid = NULL;
	return true;
}

bool vtTerrain::CreateStep4(int &iError)
{
	// some algorithms need an additional stage of initialization
	if (m_pDynGeom != NULL)
		m_pDynGeom->Init2();
	return true;
}

bool vtTerrain::CreateStep5(bool bSound, int &iError)
{
	if (m_pHeightField)
		create_culture(bSound);

	if (m_Params.m_bOceanPlane || m_Params.m_bHorizon)
	{
		bool bWater = m_Params.m_bOceanPlane;
		bool bHorizon = m_Params.m_bHorizon;
		bool bCenter = m_Params.m_bOceanPlane;
		create_artificial_horizon(bWater, bHorizon, bCenter, 0.5f);
	}

	if (m_Params.m_bLabels)
		create_floating_labels(m_strDataPath + "places.txt");

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
	if (m_pDynGeom != NULL)
		m_pDynGeom->GetBoundSphere(m_bound_sphere);
	else if (m_pTerrainGeom != NULL)
		m_pTerrainGeom->GetBoundSphere(m_bound_sphere);
	else
		m_bound_sphere.Empty();
}

//
// test whether a given point is within the current terrain
//
bool vtTerrain::PointIsInTerrainUTM(int utm_zone, float utm_x, float utm_y)
{
	DPoint2 p(utm_x, utm_y);

	if (!g_Proj.IsUTM() ||	// terrain is not UTM, must convert
		(g_Proj.IsUTM() && g_Proj.GetUTMZone() != utm_zone))
	{
		vtProjection utmproj;
		utmproj.SetUTMZone(utm_zone);
		OCT *trans = OGRCreateCoordinateTransformation(&utmproj, &g_Proj);
		if (!trans)
			return false;
		trans->Transform(1, &p.x, &p.y);
		delete trans;
	}

	float x, z;
	g_Proj.ConvertFromEarth(p, x,  z);	// convert earth -> XZ
	return m_pHeightField->PointIsInTerrain(x, z);
}

//
// test whether a given point is within the current terrain
//
bool vtTerrain::PointIsInTerrainLL(float lat, float lon)
{
	DPoint2 p(lon, lat);

	if (!g_Proj.IsGeographic())	// terrain is in UTM, must convert
	{
		vtProjection geoproj;
		CreateSimilarGeographicProjection(g_Proj, geoproj);
		OCT *trans = OGRCreateCoordinateTransformation(&geoproj, &g_Proj);
		if (!trans)
			return false;
		trans->Transform(1, &p.x, &p.y);
		delete trans;
	}

	float x, z;
	g_Proj.ConvertFromEarth(p, x,  z);	// convert earth -> XZ
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
	case OCEAN:
		if (m_pOceanGeom)
			m_pOceanGeom->SetEnabled(bOn);
		break;
	case VEGETATION:
		if (m_pTreeGroup)
			m_pTreeGroup->SetEnabled(bOn);
		break;
	case ROADS:
		if (m_pRoadGroup)
			m_pRoadGroup->SetEnabled(bOn);
		break;
	}
}

bool vtTerrain::GetFeatureVisible(TFType ftype)
{
	switch (ftype)
	{
	case OCEAN:
		if (m_pOceanGeom)
			return m_pOceanGeom->GetEnabled();
		break;
	case VEGETATION:
		if (m_pTreeGroup)
			return m_pTreeGroup->GetEnabled();
		break;
	case ROADS:
		if (m_pRoadGroup)
			return m_pRoadGroup->GetEnabled();
		break;
	}
	return false;
}

/////////////////////
// helpers

vtGeom *CreatePlaneGeom(int iMatIdx, FPoint2 org, FPoint2 size,
						   float xTiling, float zTiling, int steps)
{
	vtGeom *pGeom = new vtGeom();
	TerrainPatch *geo = new TerrainPatch(VT_Normals | VT_TexCoords, (steps+1)*(steps+1));
	geo->MakeGrid(steps, steps,
		size.x/steps, size.y/steps,
		org.x, org.y,
		xTiling, zTiling);		// tiling
	pGeom->AddMesh(geo, iMatIdx);
	return pGeom;
}

vtGeom *CreateSphereGeom(vtMaterialArray *pMats, int iMatIdx, float fRadius, int res)
{
	vtGeom *pGeom = new vtGeom();
	vtMesh *geo = new vtMesh(GL_TRIANGLE_STRIP, VT_Normals | VT_TexCoords, res*res*2);
	geo->CreateEllipsoid(FPoint3(fRadius, fRadius, fRadius), res);
	pGeom->SetMaterials(pMats);
	pGeom->AddMesh(geo, iMatIdx);
	return pGeom;
}

vtGeom *CreateLineGridGeom(vtMaterialArray *pMats, int iMatIdx,
						   FPoint3 min1, FPoint3 max1, int steps)
{
	vtGeom *pGeom = new vtGeom();
	vtMesh *geo = new vtMesh(GL_LINES, 0, (steps+1)*4);

	FPoint3 p, diff = max1 - min1, step = diff/steps;
	p.y = diff.y;
	int i, idx = 0;
	for (i = 0; i < steps+1; i++)
	{
		p.x = min1.x + step.x * i;
		p.z = min1.z;
		geo->AddVertex(p);
		p.z = max1.z;
		geo->AddVertex(p);
		geo->AddLine(idx, idx+1);
		idx += 2;
	}
	for (i = 0; i < steps+1; i++)
	{
		p.z = min1.z + step.z * i;
		p.x = min1.x;
		geo->AddVertex(p);
		p.x = max1.x;
		geo->AddVertex(p);
		geo->AddLine(idx, idx+1);
		idx += 2;
	}
	pGeom->SetMaterials(pMats);
	pGeom->AddMesh(geo, iMatIdx);
	return pGeom;
}


/////////////////////

#if 0
	// what sizes can we support?
	int d, size, n, tsize, mem, mem_mm;
	for (d = 8; d <= 10; d++)
	{
		for (n = 1; n <= 8; n++)
		{
			size = 1 << d;
			tsize = n * (size-1) + 1;
			mem = n * n * size * size * 4;
			mem_mm = mem * 4 / 3;

			// print potential sizes
			if (mem <= 64 * 1024 * 1024)
//				TRACE("n = %d  size = %4d  tsize = %4d  mem = %5dK  MM = %5dK\n",
				TRACE("<tr> <td>%d</td>  <td>%d</td>  <td>%d</td>  <td>%d</td>  <td>%d</td> </tr>\n",
					n, size, tsize, mem/1024, mem_mm/1024);
		}
	}
#endif


///////////////////

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

			// Can't delete the internals DIBs here because DISM holds on
			//   to its pointer, that we passed with SetBitmap
			dib2->LeaveInternalDIB(true);
			delete dib2;

			m_Images.SetAt(i*patches+j, pImage);
		}
	}
}



void vtTerrain::CreateChoppedAppearances1(vtMaterialArray *pApp1,
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


void vtTerrain::CreateChoppedAppearances2(vtMaterialArray *pApp1,
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



#if 0
	// what sizes can we support?
	int d, size, n, tsize, mem, mem_mm;
	for (d = 8; d <= 10; d++)
	{
		for (n = 1; n <= 8; n++)
		{
			size = 1 << d;
			tsize = n * (size-1) + 1;
			mem = n * n * size * size * 4;
			mem_mm = mem * 4 / 3;

			// print potential sizes
			if (mem <= 64 * 1024 * 1024)
//				TRACE("n = %d  size = %4d  tsize = %4d  mem = %5dK  MM = %5dK\n",
				TRACE("<tr> <td>%d</td>  <td>%d</td>  <td>%d</td>  <td>%d</td>  <td>%d</td> </tr>\n",
					n, size, tsize, mem/1024, mem_mm/1024);
		}
	}
#endif

void vtTerrain::ApplyPreLight(vtLocalGrid *pLocalGrid, vtDIB *dib,
							 int xPatch, int yPatch, int nPatches)
{
	FPoint3 light_dir;
	light_dir.Set(-1.0f, -1.0f, 0.0f);
	light_dir.Normalize();

	clock_t c1 = clock();
	pLocalGrid->ShadeDibFromElevation(dib, light_dir, m_Params.m_fPreLightFactor,
		 xPatch, yPatch, nPatches);
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

#define STEPS 20

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
	g_Proj.convert_meters_to_local_xz(poi->m_rect.left, poi->m_rect.top, v1.x, v1.z);
	g_Proj.convert_meters_to_local_xz(poi->m_rect.right, poi->m_rect.top, v2.x, v2.z);
	g_Proj.convert_meters_to_local_xz(poi->m_rect.right, poi->m_rect.bottom, v3.x, v3.z);
	g_Proj.convert_meters_to_local_xz(poi->m_rect.left, poi->m_rect.bottom, v4.x, v4.z);

	int i;
	for (i = 0; i < STEPS; i++)
	{
		v.Set(v1.x + (v2.x - v1.x) / STEPS * i, 0.0f, v1.z + (v2.z - v1.z) / STEPS * i);
		m_pHeightField->FindAltitudeAtPoint(v, v.y);
		v.y += (10.0f * WORLD_SCALE);
		pGeom->AddVertex(v);
	}
	for (i = 0; i < STEPS; i++)
	{
		v.Set(v2.x + (v3.x - v2.x) / STEPS * i, 0.0f, v2.z + (v3.z - v2.z) / STEPS * i);
		m_pHeightField->FindAltitudeAtPoint(v, v.y);
		v.y += (10.0f * WORLD_SCALE);
		pGeom->AddVertex(v);
	}
	for (i = 0; i < STEPS; i++)
	{
		v.Set(v3.x + (v4.x - v3.x) / STEPS * i, 0.0f, v3.z + (v4.z - v3.z) / STEPS * i);
		m_pHeightField->FindAltitudeAtPoint(v, v.y);
		v.y += (10.0f * WORLD_SCALE);
		pGeom->AddVertex(v);
	}
	for (i = 0; i < STEPS; i++)
	{
		v.Set(v4.x + (v1.x - v4.x) / STEPS * i, 0.0f, v4.z + (v1.z - v4.z) / STEPS * i);
		m_pHeightField->FindAltitudeAtPoint(v, v.y);
		v.y += (10.0f * WORLD_SCALE);
		pGeom->AddVertex(v);
	}

#if 0
	float vert1[3], vert2[3], vert3[3], vert4[3];

	// top-left
	convert_meters_to_local_xz(poi->m_rect.left, poi->m_rect.top, x, z);
	v1.x = x;
	v1.z = z;
	m_pHeightField->FindAltitudeAtPoint(v1, v1.y);
	vert1[0] = v1.x;
	vert1[1] = v1.y;
	vert1[2] = v1.z;

	// top-right
	convert_meters_to_local_xz(poi->m_rect.right, poi->m_rect.top, x, z);
	v2.x = x;
	v2.z = z;
	m_pHeightField->FindAltitudeAtPoint(v2, v2.y);
	vert2[0] = v2.x;
	vert2[1] = v2.y;
	vert2[2] = v2.z;

	// bottom-right
	convert_meters_to_local_xz(poi->m_rect.right, poi->m_rect.bottom, x, z);
	v3.x = x;
	v3.z = z;
	m_pHeightField->FindAltitudeAtPoint(v2, v2.y);
	vert3[0] = v3.x;
	vert3[1] = v3.y;
	vert3[2] = v3.z;

	// bottom-left
	convert_meters_to_local_xz(poi->m_rect.left, poi->m_rect.bottom, x, z);
	v4.x = x;
	v4.z = z;
	m_pHeightField->FindAltitudeAtPoint(v2, v2.y);
	vert4[0] = v4.x;
	vert4[1] = v4.y;
	vert4[2] = v4.z;

	pGeom->AddVertices(vert1, 1);
	pGeom->AddVertices(vert2, 1);
	pGeom->AddVertices(vert3, 1);
	pGeom->AddVertices(vert4, 1);
#endif

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
	CreatePlantInstance(num);
}

void vtTerrain::CreatePlantsFromPIA()
{
	int size = m_PIA.GetSize();

	for (int i = 0; i < size; i++)
		CreatePlantInstance(i);
}

void vtTerrain::CreatePlantInstance(int i)
{
	if (!m_pPlantList)
		return;

	float size_variability = 0.3f;
	FPoint3 p3;

	vtPlantInstance pi = m_PIA.GetAt(i);

	vtPlantSpecies3d *ps = m_pPlantList->GetSpecies(pi.species_id);
	if (!ps)
		return;

	vtPlantAppearance3d *pApp = ps->GetAppearanceByHeight(pi.size);
	if (!pApp)
		return;

	vtTransform *pTrans = pApp->GenerateGeom();
	m_pHeightField->ConvertEarthToSurfacePoint(pi.m_p, p3);

	pTrans->SetTrans(p3);
#if 1
	float random_scale = 1.0f + random_offset(size_variability);
	pTrans->Scale3(random_scale, random_scale, random_scale);
	float random_rotation = random(PI2f);
	pTrans->RotateLocal(FPoint3(0,1,0), random_rotation);
#endif
	// add tree to scene graph
	AddNodeToLodGrid(pTrans);

	m_PlantGeoms.SetAt(i, pTrans);
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

