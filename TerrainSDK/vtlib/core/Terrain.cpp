//
// Terrain.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"

#include "vtdata/vtLog.h"
#include "vtdata/Features.h"
#include "vtdata/StructArray.h"
#include "xmlhelper/exception.hpp"

#include "Terrain.h"
#include "Light.h"
#include "Building3d.h"
#include "Building3d.h"
#include "IntersectionEngine.h"
#include "Fence3d.h"
#include "Route.h"
#include "vtTin3d.h"
#include "TerrainScene.h"

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

//////////////////////////////////////////////////////////////////////

vtTerrain::vtTerrain()
{
	m_ocean_color.Set(40.0f/255, 75.0f/255, 124.0f/255);	// unshaded color
	m_fog_color.Set(1.0f, 1.0f, 1.0f);

	m_pTerrainGroup = (vtGroup*) NULL;
	m_pDIB = NULL;
	m_pTerrMats = NULL;
	m_pRoadMap = NULL;
	m_pInputGrid = NULL;
	m_pHeightField = NULL;
	m_bPreserveInputGrid = false;
	m_pElevGrid = NULL;
	m_pTextureColors = NULL;
	m_pDetailMats = NULL;

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

	m_CenterGeoLocation.Set(-999, -999);	// initially unknown
}

vtTerrain::~vtTerrain()
{
	VTLOG("Terrain destructing: '%s'\n", (const char *) GetName());

	// some things need to be manually deleted
	m_Content.ReleaseContents();

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
	delete m_pTextureColors;

	if (m_pDetailMats)
		m_pDetailMats->Release();
}


///////////////////////////////////////////////////////////////////////

/**
 * Tells the terrain what file contains the parameters to use.
 *
 * \param fname The name of a terrain parameters file, e.g. "Simple.xml".
 */
bool vtTerrain::SetParamFile(const char *fname)
{
	m_strParamFile = fname;
	return LoadParams();
}

bool vtTerrain::LoadParams()
{
	TParams params;
	bool success = params.LoadFrom(m_strParamFile);
	if (success)
		SetParams(params);
	return success;
}

/**
 * Set all of the parameters for this terrain.
 *
 * \param Params An object which contains all the parameters for the terrain.
 *
 * \par Note that you can set individual parameters like this:
\code
	TParams &par = pTerrain->GetParams();
	par.SetBoolValue(STR_SKY, false);
\endcode
 */
void vtTerrain::SetParams(const TParams &Params)
{
	m_Params = Params;
	
	RGBi color;
	if (m_Params.GetValueRGBi(STR_FOGCOLOR, color))
	{
		if (color.r != -1)
			m_fog_color = color;
	}
	if (m_Params.GetValueRGBi(STR_BGCOLOR, color))
	{
		if (color.r != -1)
			m_background_color = color;
	}
}

/**
 * Returns a direct reference to the parameters object for this terrain, so
 * that you can get and set the parameters.
 *
 * \par Example:
\code
	TParams &par = pTerrain->GetParams();
	par.SetBoolValue(STR_SKY, false);
\endcode
 */
TParams &vtTerrain::GetParams()
{
	return m_Params;
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
		m_Params.GetValueBool(STR_HWY),
		m_Params.GetValueBool(STR_PAVED),
		m_Params.GetValueBool(STR_DIRT));
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

	m_pRoadMap->SetHeightOffGround(m_Params.GetValueInt(STR_ROADHEIGHT));
	m_pRoadMap->DrapeOnTerrain(m_pHeightField);
	m_pRoadMap->BuildIntersections();

	m_pRoadMap->SetLodDistance(m_Params.GetValueInt(STR_ROADDISTANCE) * 1000);	// convert km to m
	m_pRoadGroup = m_pRoadMap->GenerateGeometry(m_Params.GetValueBool(STR_TEXROADS),
		vtGetDataPath());
	m_pTerrainGroup->AddChild(m_pRoadGroup);

	if (m_Params.GetValueBool(STR_ROADCULTURE))
		m_pRoadMap->GenerateSigns(m_pStructGrid);
}


///////////////////

void vtTerrain::_CreateTextures(const FPoint3 &light_dir)
{
	int iTiles = 4;		// fixed for now
	TextureEnum eTex = m_Params.GetTextureEnum();

	if (!m_pTerrMats)
		m_pTerrMats = new vtMaterialArray();
	m_pTerrMats->Empty();

	float ambient, diffuse, emmisive;
	diffuse = 1.0f;
	ambient = emmisive = 0.0f;

	if (eTex == TE_SINGLE || eTex == TE_TILED)	// load texture
	{
		vtString texname;
		if (eTex == TE_SINGLE)
		{
			texname = m_Params.GetValueString(STR_TEXTURESINGLE, true);
			VTLOG("  Single Texture: '%s'\n", (const char *) texname);
		}
		else
		{
			texname = m_Params.CookTextureFilename();
			VTLOG("  Tiled Texture: '%s'\n", (const char *) texname);
		}

		vtString texture_fname = "GeoSpecific/";
		texture_fname += texname;

		VTLOG("  Looking for: %s\n", (const char *) texture_fname);
		vtString texture_path = FindFileOnPaths(vtGetDataPath(), texture_fname);
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
				eTex = TE_NONE;
			}
			if (eTex == TE_SINGLE)
			{
				// TODO? check that DIB size is power of two, and warn if not.
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

		// This method is virtual to allow subclasses to customize the Dib,
		//  before we turn it into an vtImage
		PaintDib();
	}

	// apply pre-lighting (darkening)
	if (m_Params.GetValueBool(STR_PRELIGHT) && m_pDIB)
	{
		_ApplyPreLight(pHFGrid, m_pDIB, light_dir);
	}

	if (eTex == TE_SINGLE || eTex == TE_DERIVED)
	{
		if (m_pDIB != NULL)
		{
			// single texture
			bool b16bit = m_Params.GetValueBool(STR_REQUEST16BIT);
			vtImage *pImage = new vtImage(m_pDIB,
				(m_pDIB->GetDepth() > 8 && b16bit) ? GL_RGB5 : -1);
			m_Images.Append(pImage);
		}
	}
	if (eTex == TE_TILED && m_pDIB)
	{
		int iTileSize = m_Params.GetValueInt(STR_TILESIZE);
		_CreateChoppedTextures(iTiles, iTileSize);
		_CreateTiledMaterials(m_pTerrMats, iTiles, iTileSize, ambient,
			diffuse, emmisive);
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
			m_Params.GetValueBool(STR_MIPMAP));
	}
	// All texture images have now been passed to their materials, so don't hold on to them.
	int i, num = m_Images.GetSize();
	for (i = 0; i < num; i++)
		m_Images[i]->Release();
	m_Images.Empty();
}

//
// prepare detail texture
//
void vtTerrain::_CreateDetailTexture()
{
	vtString fname = m_Params.GetValueString(STR_DTEXTURE_NAME, true);
	vtString path = FindFileOnPaths(vtGetDataPath(), fname);
	if (path == "")
	{
		vtString prefix = "GeoTypical/";
		path = FindFileOnPaths(vtGetDataPath(), prefix+fname);
		if (path == "")
			return;
	}
	vtDIB dib;
	if (!dib.Read(path))
		return;

	m_pDetailMats = new vtMaterialArray();
	vtImage *pDetailTexture = new vtImage(&dib);

	int index = m_pDetailMats->AddTextureMaterial(pDetailTexture,
					 true,	// culling
					 false,	// lighting
					 true,	// transp: blend
					 false,	// additive
					 0.0f, 1.0f,	// ambient, diffuse
					 0.5f, 0.0f,	// alpha, emmisive
					 true, false,	// texgen, clamp
					 true);			// mipmap
	vtMaterial *pDetailMat = m_pDetailMats->GetAt(index);

	// pass ownership to the material
	pDetailTexture->Release();

	float scale = m_Params.GetValueFloat(STR_DTEXTURE_SCALE);
	float dist = m_Params.GetValueFloat(STR_DTEXTURE_DISTANCE);

	FRECT r = m_pHeightField->m_WorldExtents;
	float width_meters = r.Width();
	m_pDynGeom->SetDetailMaterial(pDetailMat, width_meters / scale, dist);
}

//
// This is the default implementation for PaintDib.  It colors from elevation.
// Developer might override it.
//
void vtTerrain::PaintDib()
{
	if (!m_pTextureColors)
	{
		// If this member hasn't been set by a subclass, then we can go ahead
		//  and use the info from the terrain parameters
		ColorMap cmap;
		vtString name = m_Params.GetValueString(STR_COLOR_MAP);
		if (name != "")
		{
			if (!cmap.Load(name))
			{
				// Look on data paths
				vtString name2 = "GeoTypical/";
				name2 += name;
				name2 = FindFileOnPaths(vtGetDataPath(), name2);
				if (name2 != "")
					cmap.Load(name2);
			}
		}
		if (cmap.Num() != 0)
			m_pTextureColors = new ColorMap(cmap);
	}
	vtHeightFieldGrid3d *pHFGrid = GetHeightFieldGrid3d();

	pHFGrid->ColorDibFromElevation(m_pDIB, m_pTextureColors, 4000);
}

/**
 * Set the array of colors to be used when automatically generating the
 * terrain texture from the elevation values.  This is the color map which
 * is used for automatic generation of texture from elevation, when the
 * terrain is built normally with the "Derived" texture option.
 * The colors brackets go from the lowest elevation value to the highest.
 *
 * \param colors A pointer to a colormap.  The terrain takes ownership of
 *		the ColorMap object so it will be deleted when the terrain is deleted.
 *
 * \par Example:
	\code
	ColorMap *colors = new ColorMap();
	colors->m_bRelative = false;
	colors->Add(100, RGBi(0,255,0));
	colors->Add(200, RGBi(255,200,150));
	pTerr->SetTextureColors(colors);
	\endcode
 */
void vtTerrain::SetTextureColors(ColorMap *colors)
{
	m_pTextureColors = colors;
}

/**
 * This method sets the terrain's color map to a series of white and black
 * bands which indicate elevation contour lines.  This is the color map
 * which is used for automatic generation of texture from elevation, when
 * the terrain is built normally with the "Derived" texture option.
 *
 * You can use this function either before the terrain is built, or
 * afterwards if you intend to re-build the textures.
 *
 * \par Example:
	\code
	vtTerrain *pTerr = new vtTerrain();
	pTerr->SetTextureColors(100, 4);
	\endcode
 *
 * \param fInterval  The vertical spacing between the contours.  For example,
 *		if the elevation range of your data is from 50 to 350 meters, then
 *		an fIterval of 100 will place contour bands at 100,200,300 meters.
 * \param fSize  The vertical thickness of each contour band, generally a
 *		few meters.  A band of this thickness will be centered on each contour
 *		line of the desired elevation.
 */
void vtTerrain::SetTextureContours(float fInterval, float fSize)
{
	// Create a color map and fill it with contour strip bands
	ColorMap *cmap = new ColorMap;
	cmap->m_bBlend = true;
	cmap->m_bRelative = false;

	RGBi white(255,255,255);
	RGBi black(0,0,0);

	float fMin, fMax;
	m_pHeightField->GetHeightExtents(fMin, fMax);
	int start = (int) (fMin / fInterval);
	int stop = (int) (fMax / fInterval);

	cmap->Add(fMin, white);
	for (int i = start; i < stop; i++)
	{
		// create a black stripe of the desired vertical thickness
		cmap->Add(i * fInterval - fSize*0.8f, white);
		cmap->Add(i * fInterval - fSize*0.5f, black);
		cmap->Add(i * fInterval + fSize*0.5f, black);
		cmap->Add(i * fInterval + fSize*0.8f, white);
	}

	// Set these as the desired color bands for the next PainDib
	m_pTextureColors = cmap;
}


/**
 * Experimental only!!!
 */
void vtTerrain::recreate_textures(vtTransform *pSunLight)
{
	_CreateTextures(pSunLight->GetDirection());
}


/////////////////////

bool vtTerrain::_CreateDynamicTerrain()
{
	int texture_patches;
	if (m_Params.GetTextureEnum() == TE_TILED)
		texture_patches = 4;	// tiled, which is always 4x4
	else
		texture_patches = 1;	// assume one texture

	LodMethodEnum method = m_Params.GetLodMethod();
	VTLOG(" LOD method %d\n", method);
//	if (m_Params.m_eLodMethod == LM_LINDSTROMKOLLER)
//	{
//		m_pDynGeom = new LKTerrain();
//		m_pDynGeom->SetName2("LK Geom");
//	}
	if (method == LM_TOPOVISTA)
	{
		m_pDynGeom = new TVTerrain();
		m_pDynGeom->SetName2("TV Geom");
	}
	else if (method == LM_MCNALLY)
	{
		m_pDynGeom = new SMTerrain();
		m_pDynGeom->SetName2("Seumas Geom");
	}
	else if (method == LM_DEMETER)
	{
#if 0	// disabled until its working
		m_pDynGeom = new DemeterTerrain();
		m_pDynGeom->SetName2("Demeter Geom");
#endif
	}
	else if (method == LM_CUSTOM)
	{
		m_pDynGeom = new CustomTerrain();
		m_pDynGeom->SetName2("CustomTerrain Geom");
	}
	else if (method == LM_BRYANQUAD)
	{
		m_pDynGeom = new BryanTerrain();
		m_pDynGeom->SetName2("BryanQuad Geom");
	}
	else if (method == LM_ROETTGER)
	{
		m_pDynGeom = new SRTerrain();
		m_pDynGeom->SetName2("Roettger Geom");
	}
	// else if (method == LM_YOURMETHOD)
	//	add your own LOD method here!
	if (!m_pDynGeom)
	{
		_SetErrorMessage("Unknown LOD method.");
		return false;
	}

	m_pDynGeom->SetOptions(m_Params.GetValueBool(STR_TRISTRIPS),
		texture_patches, m_Params.GetValueInt(STR_TILESIZE));

	m_fVerticalExag = m_Params.GetValueFloat(STR_VERTICALEXAG);
	DTErr result = m_pDynGeom->Init(m_pElevGrid, m_fVerticalExag);
	if (result != DTErr_OK)
	{
		m_pDynGeom->Release();
		m_pDynGeom = NULL;

		_CreateErrorMessage(result, m_pElevGrid);
		VTLOG(" Could not initialize CLOD: %s\n", (const char *) m_strErrorMsg);
		return false;
	}

	m_pDynGeom->SetPixelError(m_Params.GetValueFloat(STR_PIXELERROR));
	m_pDynGeom->SetPolygonCount(m_Params.GetValueInt(STR_TRICOUNT));
	m_pDynGeom->SetMaterials(m_pTerrMats);

	// build heirarchy (add terrain to scene graph)
	m_pDynGeomScale = new vtTransform();
	m_pDynGeomScale->SetName2("Dynamic Geometry Container");

	FPoint2 spacing = m_pElevGrid->GetWorldSpacing();
	m_pDynGeomScale->Scale3(spacing.x, m_fVerticalExag, -spacing.y);

	m_pDynGeomScale->AddChild(m_pDynGeom);
	m_pTerrainGroup->AddChild(m_pDynGeomScale);

	return true;
}

void vtTerrain::SetVerticalExag(float fExag)
{
	m_fVerticalExag = fExag;

	if (m_pDynGeom != NULL)
	{
		FPoint2 spacing = m_pDynGeom->GetWorldSpacing();
		m_pDynGeomScale->Identity();
		m_pDynGeomScale->Scale3(spacing.x, m_fVerticalExag, -spacing.y);

		m_pDynGeom->SetVerticalExag(m_fVerticalExag);
	}
}

void vtTerrain::_CreateErrorMessage(DTErr error, vtElevationGrid *pGrid)
{
	int x, y;
	pGrid->GetDimensions(x, y);
	switch (error)
	{
	case DTErr_OK:
		m_strErrorMsg = "No Error";
		break;
	case DTErr_EMPTY_EXTENTS:
		m_strErrorMsg.Format("The elevation has empty extents.");
		break;
	case DTErr_NOTSQUARE:
		m_strErrorMsg.Format("The elevation grid (%d x %d) is not square.", x, y);
		break;
	case DTErr_NOTPOWER2:
		m_strErrorMsg.Format("The elevation grid (%d x %d) is of an unsupported size.",
			x, y);
		break;
	case DTErr_NOMEM:
		m_strErrorMsg = "Not enough memory for CLOD.";
		break;
	default:
		m_strErrorMsg = "Unknown error.";
	}
}

void vtTerrain::_SetErrorMessage(const vtString &msg)
{
	m_strErrorMsg = msg;
	VTLOG("\t%s.\n", (const char *) msg);
}


bool vtTerrain::AddFence(vtFence3d *fen)
{
	vtStructureArray3d *structs = GetStructures();
	if (!structs)
		return false;

	structs->Append(fen);
	fen->CreateNode(this);

	// Add to LOD grid
	AddNodeToStructGrid(fen->GetGeom());
	return true;
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
	VTLOG("Route AddPoint %.1lf %.1lf\n", epos.x, epos.y);
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
		vtString fname = FindFileOnPaths(vtGetDataPath(), "GeoTypical/ocean1_256.jpg");
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
	FPoint3 down(0.0f, m_Params.GetValueFloat(STR_OCEANPLANELEVEL), 0.0f);
	m_pOceanGeom->Translate1(down);

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
	name += m_Params.GetValueString(STR_ELEVFILE, true);
	vtString grid_fname = FindFileOnPaths(vtGetDataPath(), name);
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
	vtStructureArray3d *structures = NewStructureArray();
	if (!structures->ReadXML(strFilename))
	{
		VTLOG("\tCouldn't load file.\n");
		delete structures;
		m_StructureSet.RemoveAt(m_iStructSet);
		m_iStructSet = m_StructureSet.GetSize() - 1;
		return NULL;
	}

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

int vtTerrain::GetStructureIndex()
{
	return m_iStructSet;
}

void vtTerrain::SetStructureIndex(int index)
{
	m_iStructSet = index;
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
	m_iStructSet = m_StructureSet.GetSize() - 1;
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

void vtTerrain::DeleteStructureSet(int index)
{
	vtStructureArray3d *sa = m_StructureSet[index];

	// first remove them from the terrain
	for (unsigned int i = 0; i < sa->GetSize(); i++)
	{
		vtStructure3d *str3d = sa->GetStructure3d(i);
		RemoveNodeFromStructGrid(str3d->GetContainer());
		str3d->DeleteNode();
	}
	m_StructureSet.RemoveAt(index);
	delete sa;

	// If that was the current layer, deal with it
	if (index == m_iStructSet)
		m_iStructSet = 0;
}

bool vtTerrain::FindClosestStructure(const DPoint2 &point, double epsilon,
					   int &structure, double &closest, float fMaxInstRadius)
{
	structure = -1;
	closest = 1E8;

	double dist;
	int i, index, sets = m_StructureSet.GetSize();
	for (i = 0; i < sets; i++)
	{
		vtStructureArray *sa = m_StructureSet[i];

		if (sa->FindClosestStructure(point, epsilon, index, dist, fMaxInstRadius))
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
MyTerrain::CreateCustomCulture()
{
	// model is in centimeters (cm)
	vtTransform *pFountain = LoadModel("Culture/fountain.3ds");

	pFountain->Scale3(.01f, .01f, .01f);

	PlantModelAtPoint(pFountain, DPoint2(217690, 4123475));
	AddModel(pFountain);
}
	\endcode
 */
vtTransform *vtTerrain::LoadModel(const char *filename, bool bAllowCache)
{
	vtNode *node = NULL;
	vtString path = FindFileOnPaths(vtGetDataPath(), filename);
	if (path == "")
	{
		VTLOG("Couldn't locate file '%s'\n", filename);
	}
	else
		node = vtNode::LoadModel(path, bAllowCache);

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

	m_pHeightField->m_Conversion.convert_earth_to_local_xz(pos.x, pos.y, wpos.x, wpos.z);
	m_pHeightField->FindAltitudeAtPoint(wpos, wpos.y);
	model->SetTrans(wpos);
}

void vtTerrain::_CreateCulture()
{
	// Read terrain-specific content file
	vtString con_file = m_Params.GetValueString(STR_CONTENT_FILE);
	if (con_file != "")
	{
		VTLOG(" Looking for terrain-specific content file: '%s'\n", (const char *) con_file);
		vtString fname = FindFileOnPaths(vtGetDataPath(), con_file);
		if (fname != "")
		{
			VTLOG("  Found.\n");
			try
			{
				m_Content.ReadXML(fname);
			}
			catch (xh_io_exception &ex)
			{
				// display (or a least log) error message here
				VTLOG("  XML error:");
				VTLOG(ex.getFormattedMessage().c_str());
				return;
			}
		}
		else
			VTLOG("  Not found.\n");
	}

	// The LOD distances are in meters
	_SetupStructGrid((float) m_Params.GetValueInt(STR_STRUCTDIST));
	_SetupVegGrid((float) m_Params.GetValueInt(STR_VEGDISTANCE));

	// create roads
	if (m_Params.GetValueBool(STR_ROADS))
	{
		vtString road_fname = "RoadData/";
		road_fname += m_Params.GetValueString(STR_ROADFILE, true);
		vtString road_path = FindFileOnPaths(vtGetDataPath(), road_fname);
		if (road_path != "")
			create_roads(road_path);

		if (m_pRoadMap && m_Params.GetValueBool(STR_ROADCULTURE))
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

	// In case we don't load any plants, or fail to load, we will start with
	// an empty plant array, which needs to match the projection of the rest
	// of the terrain.
	m_PIA.SetProjection(GetProjection());

	if (m_Params.GetValueBool(STR_TREES))
	{
		vtString fname = m_Params.GetValueString(STR_TREEFILE, true);

		// Read the VF file
		vtString plants_fname = "PlantData/";
		plants_fname += fname;

		VTLOG("\tLooking for plants file: %s\n", (const char *) plants_fname);

		vtString plants_path = FindFileOnPaths(vtGetDataPath(), plants_fname);
		if (plants_path == "")
		{
			VTLOG("\tNot found.\n");
		}
		else
		{
			VTLOG("\tFound: %s\n", (const char *) plants_path);

			bool success;
			if (!fname.Right(3).CompareNoCase("shp"))
				success = m_PIA.ReadSHP(plants_path);
			else
				success = m_PIA.ReadVF(plants_path);
			if (success)
			{
				// Create the 3d plants
				VTLOG("\tLoaded plants file.  Creating Plant geometry..\n");
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

	// create built structures
	vtStructure3d::InitializeMaterialArrays();

	unsigned int i, num = m_Params.m_strStructFiles.size();
	int created = 0;
	for (i = 0; i < num; i++)
	{
		vtString building_fname = "BuildingData/";
		const ParamStructLayer &psl = m_Params.m_strStructFiles[i];
		building_fname += psl.m_strStructFile;

		VTLOG("\tLooking for structures file: %s\n", (const char *) building_fname);

		vtString building_path = FindFileOnPaths(vtGetDataPath(), building_fname);
		if (building_path == "")
		{
			VTLOG("\tNot found.\n");
		}
		else
		{
			VTLOG("\tFound: %s\n", (const char *) building_path);
			vtStructureArray3d *sa = CreateStructuresFromXML(building_path);
			if (sa)
			{
				created++;

				// If the user wants it to start hidden, hide it
				if (psl.m_bVisible == false)
					sa->SetEnabled(false);
			}
		}
	}
	if (created == 0)
	{
		// No structures loaded, but the user might want to create some later,
		//  so create a default structure set, and set the projection to match
		//  the terrain.
		vtStructureArray3d *sa = NewStructureArray();
		sa->SetFilename("Untitled.vtst");
		sa->m_proj = m_proj;
	}

	// create utility structures (routes = towers and wires)
	if (m_Params.GetValueBool(STR_ROUTEENABLE))
	{
		// TODO
	}

	// Let any terrain subclasses provide their own culture
	CreateCustomCulture();
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
	fname += m_Params.GetValueString(STR_LABELFILE, true);
	vtString labels_path = FindFileOnPaths(vtGetDataPath(), fname);
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

	CreateStyledFeatures(*feat, "Fonts/Arial.ttf", m_Params.GetPointStyle());

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
	vtString font_path = FindFileOnPaths(vtGetDataPath(), fontname);
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
		float dist = m_Params.GetValueInt(STR_FOGDISTANCE) * 1000;

		if (m_fog_color.r != -1)
			m_pTerrainGroup->SetFog(true, 0, dist, m_fog_color);
		else
			m_pTerrainGroup->SetFog(true, 0, dist);
	}
	else
		m_pTerrainGroup->SetFog(false);
}

void vtTerrain::SetFogColor(const RGBf &color)
{
	m_fog_color = color;
	if (m_bFog)
		SetFog(true);
}

void vtTerrain::SetFogDistance(float fMeters)
{
	m_Params.SetValueInt(STR_FOGDISTANCE, (int) (fMeters / 1000));
	if (m_bFog)
		SetFog(true);
}

void vtTerrain::SetBgColor(const RGBf &color)
{
	m_background_color = color;
}


///////////////////////////////////////////////////////////////////////
//////////////////////////// Time Methods /////////////////////////////

/**
 * Get the time at which a terrain is set to begin.
 */
vtTime vtTerrain::GetInitialTime()
{
	const char *str = m_Params.GetValueString(STR_INITTIME);

	VTLOG("Initial time: %s\n", str);

	vtTime localtime;
	localtime.SetFromString(str);
	return localtime;
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
	if (m_CenterGeoLocation.x == -999)
		_ComputeCenterLocation();

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
	if (m_CenterGeoLocation.x == -999)
		_ComputeCenterLocation();

	time.Increment(m_iDifferenceFromGMT);
}

DPoint2 vtTerrain::GetCenterGeoLocation()
{
	if (m_CenterGeoLocation.x == -999)
		_ComputeCenterLocation();

	return m_CenterGeoLocation;
}

void vtTerrain::_ComputeCenterLocation()
{
	vtHeightField3d *pHF = GetHeightField();
	DRECT drect = pHF->GetEarthExtents();
	drect.GetCenter(m_CenterGeoLocation);

	// must convert from whatever we CRS are, to Geographic
	vtProjection Dest;
	Dest.SetWellKnownGeogCS("WGS84");

	// We won't fail on tricky Datum conversions, but we still might
	//  conceivably fail if the GDAL/PROJ files aren't found.
	OCT *trans = CreateConversionIgnoringDatum(&m_proj, &Dest);
	if (trans)
	{
		trans->Transform(1, &m_CenterGeoLocation.x, &m_CenterGeoLocation.y);
		delete trans;
	}

	// calculate offset FROM GMT
	// longitude of 180 deg = 12 hours = 720 min = 43200 sec
	m_iDifferenceFromGMT = (int) (m_CenterGeoLocation.x / 180 * 43200);
}


/**
 * First step in terrain creation: load elevation.
 * You can use these methods to build a terrain step by step, or simply
 * use the method vtTerrainScene::BuildTerrain, which calls them all.
 */
bool vtTerrain::CreateStep1()
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
		m_pElevGrid->SetupConversion(m_Params.GetValueFloat(STR_VERTICALEXAG));
		m_pHeightField = m_pElevGrid;
		m_proj = m_pElevGrid->GetProjection();
		// set global projection based on this terrain
		g_Conv = m_pElevGrid->m_Conversion;
		return true;
	}
	vtString elev_file = m_Params.GetValueString(STR_ELEVFILE, true);
	vtString fname = "Elevation/";
	fname += elev_file;
	VTLOG("\tLooking for elevation file: %s\n", (const char *) fname);

	vtString fullpath = FindFileOnPaths(vtGetDataPath(), fname);
	if (fullpath == "")
	{
		VTLOG("\t\tNot found.\n");

		vtString msg;
		msg.Format("Couldn't find elevation '%s'", (const char *) elev_file);
		_SetErrorMessage(msg);
		return false;
	}

	VTLOG("\tFound: %s\n", (const char *) fullpath);
	if (m_Params.GetValueBool(STR_TIN))
	{
		if (!m_pTin)
		{
			// if they did not provide us with a TIN, try to load it
			m_pTin = new vtTin3d;
			bool status = m_pTin->Read(fullpath);

			if (status == false)
			{
				_SetErrorMessage("TIN load failed.");
				return false;
			}
			VTLOG("\tTIN load succeeded.\n");

			m_proj = m_pTin->m_proj;
			g_Conv = m_pTin->m_Conversion;

			m_pHeightField = m_pTin;
		}
	}
	else
	{
		// Loading elevation grid...
		m_pElevGrid = new vtElevationGrid();
		bool status = m_pElevGrid->LoadFromBT(fullpath);
		if (status == false)
		{
			_SetErrorMessage("Grid load failed.");
			return false;
		}
		VTLOG("\tGrid load succeeded.\n");

		// set global projection based on this terrain
		m_proj = m_pElevGrid->GetProjection();

		int col, row;
		m_pElevGrid->GetDimensions(col, row);
		VTLOG("\t\tSize: %d x %d.\n", col, row);
		DRECT rect = m_pElevGrid->GetEarthExtents();
		VTLOG("\t\tEarth Extents LRTB: %lf %lf %lf %lf\n",
			rect.left, rect.right, rect.top, rect.bottom);

		m_pElevGrid->SetupConversion(m_Params.GetValueFloat(STR_VERTICALEXAG));
		g_Conv = m_pElevGrid->m_Conversion;

		FRECT frect = m_pElevGrid->m_WorldExtents;
		VTLOG("\t\tWorld Extents LRTB: %f %f %f %f\n",
			frect.left, frect.right, frect.top, frect.bottom);

		m_pHeightField = m_pElevGrid;

		// Apply ocean depth
		if (m_Params.GetValueBool(STR_DEPRESSOCEAN))
		{
			float fOceanDepth = m_Params.GetValueFloat(STR_DEPRESSOCEANLEVEL);
			m_pElevGrid->ReplaceValue(0, fOceanDepth);
		}
	}
	char type[10], value[2048];
	m_proj.GetTextDescription(type, value);
	VTLOG(" Projection of the terrain: %s, '%s'\n", type, value);

	return true;
}

/**
 * Next step in terrain creation: create textures.
 */
bool vtTerrain::CreateStep2(vtTransform *pSunLight)
{
	// if we aren't going to produce the terrain surface, nothing to do
	if (m_Params.GetValueBool(STR_SUPPRESS))
		return true;

	if (!m_Params.GetValueBool(STR_TIN))
		_CreateTextures(pSunLight->GetDirection());
	return true;
}

/**
 * Next step in terrain creation: create 3D geometry for the terrain.
 */
bool vtTerrain::CreateStep3()
{
	// if we aren't going to produce the terrain surface, nothing to do
	if (m_Params.GetValueBool(STR_SUPPRESS))
		return true;

	if (m_Params.GetValueBool(STR_TIN))
		return CreateFromTIN();
	else
		return CreateFromGrid();
}

bool vtTerrain::CreateFromTIN()
{
	bool bDropShadow = true;

	// build heirarchy (add terrain to scene graph)
	m_pTerrainGroup->AddChild(m_pTin->CreateGeometry(bDropShadow));

	return true;
}

// for timing how long the CLOD takes to initialize
clock_t tm1;

bool vtTerrain::CreateFromGrid()
{
	VTLOG(" CreateFromGrid\n");
	tm1 = clock();

	// create elegant dynamic LOD terrain
	if (!_CreateDynamicTerrain())
	{
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
bool vtTerrain::CreateStep4()
{
	// some algorithms need an additional stage of initialization
	if (m_pDynGeom != NULL)
	{
		m_pDynGeom->Init2();

		clock_t tm2 = clock();
		float time = ((float)tm2 - tm1)/CLOCKS_PER_SEC;
		VTLOG("CLOD construction: %.3f seconds.\n", time);
	}

	if (m_Params.GetValueBool(STR_DETAILTEXTURE))
		_CreateDetailTexture();

	return true;
}

/**
 * Next step in terrain creation: create the culture and labels.
 */
bool vtTerrain::CreateStep5()
{
	// must have a heightfield by this point
	if (!m_pHeightField)
		return false;

	_CreateCulture();

	bool bWater = m_Params.GetValueBool(STR_OCEANPLANE);
	bool bHorizon = m_Params.GetValueBool(STR_HORIZON);

	if (bWater || bHorizon)
	{
		bool bCenter = bWater;
		create_artificial_horizon(bWater, bHorizon, bCenter, 0.5f);
	}

	if (m_Params.GetValueBool(STR_LABELS))
		_CreateLabels();

	// Engines will be activated later in vtTerrainScene::SetTerrain
	ActivateEngines(false);

	return true;
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
	m_pHeightField->m_Conversion.ConvertFromEarth(p, x,  z);	// convert earth -> XZ
	return m_pHeightField->ContainsWorldPoint(x, z);
}

void vtTerrain::CreateCustomCulture()
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

vtHeightField3d *vtTerrain::GetHeightField()
{
	return m_pHeightField;
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
			if (!mono && m_Params.GetValueBool(STR_REQUEST16BIT))
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
				m_Params.GetValueBool(STR_MIPMAP));
		}
	}
}


void vtTerrain::_ApplyPreLight(vtHeightFieldGrid3d *pElevGrid, vtDIB *dib,
							  const FPoint3 &light_dir)
{
	VTLOG("Prelighting terrain texture: ");

	clock_t c1 = clock();

	float shade_factor = m_Params.GetValueFloat(STR_PRELIGHTFACTOR);
	if (m_Params.GetValueBool(STR_CAST_SHADOWS))
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
									 float fOffset, bool bCurve, bool bTrue)
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
		m_pHeightField->m_Conversion.convert_earth_to_local_xz(ext.left, ext.bottom, p1.x, p1.y);
		m_pHeightField->m_Conversion.convert_earth_to_local_xz(ext.right, ext.top, p2.x, p2.y);
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

			m_pHeightField->m_Conversion.convert_earth_to_local_xz(p.x, p.y, v.x, v.z);
			m_pHeightField->FindAltitudeAtPoint(v, v.y, bTrue);
			v.y += fOffset;
			pMesh->AddVertex(v);
			iVerts++;

			// keep a running total of approximate ground length
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
			m_pHeightField->m_Conversion.convert_earth_to_local_xz(line[i].x, line[i].y, v2.x, v2.z);
			if (i == 0)
				continue;

			// estimate how many steps to subdivide this segment into
			FPoint3 diff = v2 - v1;
			float fLen = diff.Length();
			unsigned int iSteps = (unsigned int) (fLen / fSpacing);
			if (iSteps < 1) iSteps = 1;

			FPoint3 last_v;
			for (j = (i == 1 ? 0:1); j <= iSteps; j++)
			{
				// simple linear interpolation of the ground coordinate
				v.Set(v1.x + diff.x / iSteps * j, 0.0f, v1.z + diff.z / iSteps * j);
				m_pHeightField->FindAltitudeAtPoint(v, v.y, bTrue);
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

bool vtTerrain::AddPlant(const DPoint2 &pos, int iSpecies, float fSize)
{
	int num = m_PIA.AddPlant(pos, fSize, iSpecies);
	if (num == -1)
		return false;

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
			if (pTrans != NULL)
			{
				vtGroup *pParent = pTrans->GetParent();
				if (pParent)
				{
					pParent->RemoveChild(pTrans);
					m_PIA.DeletePlant(i);
				}
			}
		}
	}
}

void vtTerrain::SetPlantList(vtSpeciesList3d *pPlantList)
{
	m_pPlantList = pPlantList;
	m_PIA.SetPlantList(m_pPlantList);
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


void vtTerrain::RemoveNodeFromStructGrid(vtNode *pNode)
{
	if (m_pStructGrid)
		m_pStructGrid->RemoveNodeFromGrid(pNode);
}
