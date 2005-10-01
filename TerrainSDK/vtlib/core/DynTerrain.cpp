//
// Dynamic Terrain class
//
// This is the parent class for terrain which can redefine it's
// surface at render time.
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include "vtdata/HeightField.h"
#include "vtdata/vtLog.h"
#include "DynTerrain.h"

vtDynTerrainGeom::vtDynTerrainGeom() : vtDynGeom(), vtHeightFieldGrid3d()
{
	m_fPixelError = 2.0f;
	m_iPolygonTarget = 10000;

	m_bCulleveryframe = true;
	m_bCullonce = false;
	m_bDetailTexture = false;
	m_pDetailMat = NULL;

	m_fXLookup = m_fZLookup = NULL;
}

vtDynTerrainGeom::~vtDynTerrainGeom()
{
	delete m_fXLookup;
	delete m_fZLookup;
}

void vtDynTerrainGeom::Init2()
{
}

DTErr vtDynTerrainGeom::BasicInit(const vtElevationGrid *pGrid)
{
	// initialize the HeightFieldGrid3D
	const LinearUnits units = pGrid->GetProjection().GetUnits();

	float fMinHeight, fMaxHeight;
	pGrid->GetHeightExtents(fMinHeight, fMaxHeight);

	int cols, rows;
	pGrid->GetDimensions(cols, rows);

	// Set up HeightFieldGrid
	Initialize(units, pGrid->GetEarthExtents(), fMinHeight, fMaxHeight, cols, rows);

	if (fabs(m_WorldExtents.Width()) < 0.000001 ||
		fabs(m_WorldExtents.Height()) < 0.000001)
		return DTErr_EMPTY_EXTENTS;

	// Allocate and set the xz lookup tables
	m_fXLookup = new float[m_iColumns];
	m_fZLookup = new float[m_iRows];
	int i;
	for (i = 0; i < m_iColumns; i++)
		m_fXLookup[i] = m_WorldExtents.left + i * m_fXStep;
	for (i = 0; i < m_iRows; i++)
		m_fZLookup[i] = m_WorldExtents.bottom - i * m_fZStep;

	m_iTotalTriangles = m_iColumns * m_iRows * 2;

	return DTErr_OK;
}


void vtDynTerrainGeom::SetOptions(bool bUseTriStrips, int iTPatchDim, int iTPatchSize)
{
	m_bUseTriStrips = bUseTriStrips;
	m_iTPatchDim = iTPatchDim;
	m_iTPatchSize = iTPatchSize;
}

// overrides for HeightField
bool vtDynTerrainGeom::FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude, bool bTrue) const
{
	DPoint2 spacing = GetSpacing();
	int iX = (int)((p.x - m_EarthExtents.left) / spacing.x);
	int iY = (int)((p.y - m_EarthExtents.bottom) / spacing.y);

	// safety check
	if (iX < 0 || iX >= m_iColumns-1 || iY < 0 || iY >= m_iRows-1)
	{
		fAltitude = 0.0f;
		return false;
	}

	float alt0, alt1, alt2, alt3;
	alt0 = GetElevation(iX, iY, bTrue);
	alt1 = GetElevation(iX+1, iY, bTrue);
	alt2 = GetElevation(iX+1, iY+1, bTrue);
	alt3 = GetElevation(iX, iY+1, bTrue);

	// find fractional amount (0..1 across quad)
	double fX = (p.x - (m_EarthExtents.left + iX * spacing.x)) / spacing.x;
	double fY = (p.y - (m_EarthExtents.bottom + iY * spacing.y)) / spacing.y;

	// which of the two triangles in the quad is it?
	if (fX + fY < 1)
		fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt3 - alt0));
	else
		fAltitude = (float) (alt2 + (1.0-fX) * (alt3 - alt2) + (1.0-fY) * (alt1 - alt2));

	return true;
}

bool vtDynTerrainGeom::FindAltitudeAtPoint(const FPoint3 &p, float &fAltitude,
									bool bTrue, bool bIncludeCulture, FPoint3 *vNormal) const
{
	// Look on culture first
	if (bIncludeCulture && m_pCulture != NULL)
	{
		if (m_pCulture->FindAltitudeOnCulture(p, fAltitude))
			return true;
	}

	int iX = (int)((p.x - m_WorldExtents.left) / m_fXStep);
	int iZ = (int)(-(p.z - m_WorldExtents.bottom) / m_fZStep);

	// safety check
	bool bogus = false;
	if (iX < 0 || iX > m_iColumns-1 || iZ < 0 || iZ > m_iRows-1)
		bogus = true;
	else if (iX == m_iColumns-1 || iZ == m_iRows-1)
	{
		if (p.x == m_WorldExtents.right || p.z == m_WorldExtents.top)
		{
			// right on the edge: allow this point, but don't interpolate
			fAltitude = GetElevation(iX, iZ, bTrue);
			if (vNormal) vNormal->Set(0,1,0);
			return true;
		}
		else
			bogus = true;
	}
	if (bogus)
	{
		fAltitude = 0.0f;
		if (vNormal) vNormal->Set(0, 1, 0);
		return false;
	}

	if (vNormal != NULL)
	{
		FPoint3 p0, p1, p2, p3;
		GetWorldLocation(iX, iZ, p0, bTrue);
		GetWorldLocation(iX+1, iZ, p1, bTrue);
		GetWorldLocation(iX+1, iZ+1, p2, bTrue);
		GetWorldLocation(iX, iZ+1, p3, bTrue);

		// find fractional amount (0..1 across quad)
		float fX = (float)  (p.x - p0.x) / m_fXStep;
		float fZ = (float) -(p.z - p0.z) / m_fZStep;

		// which way is this quad split?
		if ((iX + iZ) & 1)
		{
			// which of the two triangles in the quad is it?
			if (fX + fZ < 1)
			{
				fAltitude = p0.y + fX * (p1.y - p0.y) + fZ * (p3.y - p0.y);
				vNormal->UnitNormal(p0, p1, p3);
			}
			else
			{
				fAltitude = p2.y + (1.0f-fX) * (p3.y - p2.y) + (1.0f-fZ) * (p1.y - p2.y);
				vNormal->UnitNormal(p2, p3, p1);
			}
		}
		else
		{
			if (fX > fZ)
			{
				fAltitude = p0.y + fX * (p1.y - p0.y) + fZ * (p2.y - p1.y);
				vNormal->UnitNormal(p1, p2, p0);
			}
			else
			{
				fAltitude = p0.y + fX * (p2.y - p3.y) + fZ * (p3.y - p0.y);
				vNormal->UnitNormal(p3, p0, p2);
			}
		}
	}
	else
	{
		// It's faster to simpler to operate only the elevations, if we don't
		//  need to compute a normal vector.
		float alt0 = GetElevation(iX, iZ, bTrue);
		float alt1 = GetElevation(iX+1, iZ, bTrue);
		float alt2 = GetElevation(iX+1, iZ+1, bTrue);
		float alt3 = GetElevation(iX, iZ+1, bTrue);

		// find fractional amount (0..1 across quad)
		float fX = (p.x - (m_WorldExtents.left + iX * m_fXStep)) / m_fXStep;
		float fY = (p.z - (m_WorldExtents.bottom - iZ * m_fZStep)) / -m_fZStep;

		// which way is this quad split?
		if ((iX + iZ) & 1)
		{
			// which of the two triangles in the quad is it?
			if (fX + fY < 1)
				fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt3 - alt0));
			else
				fAltitude = (float) (alt2 + (1.0f-fX) * (alt3 - alt2) + (1.0f-fY) * (alt1 - alt2));
		}
		else
		{
			if (fX > fY)
				fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt2 - alt1));
			else
				fAltitude = (float) (alt0 + fX * (alt2 - alt3) + fY * (alt3 - alt0));
		}
	}
	return true;
}


void vtDynTerrainGeom::SetCull(bool bOnOff)
{
	m_bCulleveryframe = bOnOff;
}

void vtDynTerrainGeom::CullOnce()
{
	m_bCullonce = true;
}

void vtDynTerrainGeom::SetPixelError(float fPixelError)
{
	m_fPixelError = fPixelError;
}

float vtDynTerrainGeom::GetPixelError()
{
	return m_fPixelError;
}

void vtDynTerrainGeom::SetPolygonCount(int iPolygonCount)
{
	m_iPolygonTarget = iPolygonCount;
}

int vtDynTerrainGeom::GetPolygonCount()
{
	return m_iPolygonTarget;
}

void vtDynTerrainGeom::SetDetailMaterial(vtMaterial *pMat, float fTiling, float fDistance)
{
	m_pDetailMat = pMat;
	m_bDetailTexture = (m_pDetailMat != NULL);
	m_fDetailTiling = fTiling;
	m_fDetailDistance = fDistance;
}

void vtDynTerrainGeom::EnableDetail(bool bOn)
{
	m_bDetailTexture = (m_pDetailMat != NULL && bOn);
}

int vtDynTerrainGeom::GetNumDrawnTriangles()
{
	return m_iDrawnTriangles;
}

///////////////////////////////////////////////////////////////////////
//
// Overrides for vtDynGeom
//
void vtDynTerrainGeom::DoCalcBoundBox(FBox3 &box)
{
	// derive bounding box from known terrain origin and size

	// units are those of the coordinate space below the transform
	box.Set(0,				   m_fMinHeight, 0,
			(float)m_iColumns, m_fMaxHeight, (float)m_iRows);
}

void vtDynTerrainGeom::DoCull(const vtCamera *pCam)
{
	// make sure we cull at least every 300 ms
	bool bCullThisFrame = false;
#if 0
	static float last_time = 0.0f;
	float this_time = vtGetTime();
	if ((this_time - last_time) > 0.3f)
	{
		bCullThisFrame = true;
		last_time = this_time;
	}
#endif
	if (m_bCulleveryframe || m_bCullonce || bCullThisFrame)
	{
		DoCulling(pCam);
		m_bCullonce = false;
	}
}

//////////////////////////////////////////////////////////////////////

void vtDynTerrainGeom::SetupTexGen(float fTiling)
{
#if !VTLIB_NI
	GLfloat sPlane[4] = { fTiling * 1.0f / (m_iColumns-1), 0.0f, 0.0f, 0.0f };
	GLfloat tPlane[4] = { 0.0f, 0.0f, fTiling * 1.0f / (m_iRows-1), 0.0f };

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, sPlane);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, tPlane);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_2D);
#endif
}

void vtDynTerrainGeom::SetupBlockTexGen(int a, int b)
{
#if !VTLIB_NI
	// carefully determine the right u,v offset, leaving a
	// half-texel of buffer at the edge of each patch
	float uv_offset = 1.0f / m_iTPatchSize / 2.0f;

	float grid_offset_x = uv_offset * ((m_iColumns-1)/4.0f) * 2.0f;
	float grid_offset_y = uv_offset * (-(m_iRows-1)/4.0f) * 2.0f;

	float factor_x = (float) (1.0 / ((m_iColumns-1)/4.00+grid_offset_x));
	float factor_y = (float) (1.0 / (-(m_iRows-1)/4.00+grid_offset_y));

	GLfloat sPlane[4] = { factor_x, 0.00, 0.0, (a*2+1) * uv_offset };
	GLfloat tPlane[4] = { 0.0, 0.00, -factor_y, (b*2+1) * uv_offset };

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, sPlane);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, tPlane);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_2D);
#endif
}

void vtDynTerrainGeom::DisableTexGen()
{
#if !VTLIB_NI
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
#endif
}

//GLboolean ca, va, na, ia, ta;

void vtDynTerrainGeom::PreRender() const
{
#if VTLIB_DSM || 0
	// preserve
	ca = glIsEnabled(GL_COLOR_ARRAY);
	va = glIsEnabled(GL_VERTEX_ARRAY);
	na = glIsEnabled(GL_NORMAL_ARRAY);
	ia = glIsEnabled(GL_INDEX_ARRAY);
	ta = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_INDEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	// get ready to count drawn triangles
	vtDynTerrainGeom *pHack = (vtDynTerrainGeom *)this;
	pHack->m_iDrawnTriangles = 0;
}

void vtDynTerrainGeom::PostRender() const
{
#if VTLIB_DSM || 0
	// restore
	if (ca) glEnableClientState(GL_COLOR_ARRAY);
	if (va) glEnableClientState(GL_VERTEX_ARRAY);
	if (na) glEnableClientState(GL_NORMAL_ARRAY);
	if (ia) glEnableClientState(GL_INDEX_ARRAY);
	if (ta) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
}


///////////////////////////////////////////////////////////////////////

#include "pnmbase.h"

// preloader statistics
static float kbytes1=0.0f,kbytes2=0.0f;
static double secs=0.0;

void request_callback(unsigned char *mapfile,unsigned char *texfile,
					  unsigned char *fogfile,void *data,
					  void **hfield,void **texture,void **fogmap)
   {
   int bytes;

   if ((*hfield=readfile((char *)mapfile,&bytes))!=NULL) kbytes1+=bytes/1024.0f;
   if ((*texture=readfile((char *)texfile,&bytes))!=NULL) kbytes1+=bytes/1024.0f;

   if (fogfile==NULL) *fogmap=NULL;
   else if ((*fogmap=readfile((char *)fogfile,&bytes))!=NULL) kbytes1+=bytes/1024.0f;
   }

int COL,ROW;
void *HFIELD=NULL,*TEXTURE=NULL,*FOGMAP=NULL;
int HLOD,TLOD;

void preload_callback(int col,int row,unsigned char *mapfile,int hlod,
					  unsigned char *texfile,int tlod,unsigned char *fogfile,
					  void *data)
   {
   int bytes;

   if (HFIELD!=NULL || TEXTURE!=NULL || FOGMAP!=NULL) return;

   COL=col;
   ROW=row;

   if ((HFIELD=readfile((char *)mapfile,&bytes))!=NULL) kbytes2+=bytes/1024.0f;
   if ((TEXTURE=readfile((char *)texfile,&bytes))!=NULL) kbytes2+=bytes/1024.0f;

   if (fogfile==NULL) FOGMAP=NULL;
   else if ((FOGMAP=readfile((char *)fogfile,&bytes))!=NULL) kbytes2+=bytes/1024.0f;

   HLOD=hlod;
   TLOD=tlod;
   }

void deliver_callback(int *col,int *row,void **hfield,int *hlod,void **texture,
					  int *tlod,void **fogmap,void *data)
   {
   *col=COL;
   *row=ROW;

   *hfield=HFIELD;
   *texture=TEXTURE;
   *fogmap=FOGMAP;

   *hlod=HLOD;
   *tlod=TLOD;

   HFIELD=TEXTURE=FOGMAP=NULL;
   }

void mini_error_handler(char *file,int line,int fatal)
{
	VTLOG("libMini error: file '%s', line %d, fatal %d\n", file, line, fatal);
}

vtTiledGeom::vtTiledGeom()
{
	m_pMiniLoad = NULL;
}

vtTiledGeom::~vtTiledGeom()
{
	delete m_pMiniLoad;
}

class TiledDatasetDescription
{
public:
	bool Read(const char *ini_fname);

	int cols, rows;
	int lod0size;
	DRECT earthextents;
	vtProjection proj;
};

bool TiledDatasetDescription::Read(const char *dataset_fname)
{
	FILE *fp = fopen(dataset_fname, "rb");
	if (!fp) return false;
	fscanf(fp, "[TilesetDescription]\n");
	fscanf(fp, "Columns=%d\n", &cols);
	fscanf(fp, "Rows=%d\n", &rows);
	fscanf(fp, "LOD0_Size=%d\n", &lod0size);
	fscanf(fp, "Extent_Left=%lf\n", &earthextents.left);
	fscanf(fp, "Extent_Right=%lf\n", &earthextents.right);
	fscanf(fp, "Extent_Bottom=%lf\n", &earthextents.bottom);
	fscanf(fp, "Extent_Top=%lf\n", &earthextents.top);
	// read CRS from WKT
	char wkt[4096];
//	fscanf(fp, "CRS=%s\n", wkt);
	fgets(wkt, 4096, fp);
	char *wktp = wkt + 4;	// skip "CRS="
	proj.importFromWkt(&wktp);
	fclose(fp);
	return true;
}

bool vtTiledGeom::ReadTileList(const char *dataset_fname_elev, const char *dataset_fname_image)
{
#if 0
	// for now, hardcode instead of reading from file
	cols = 9;
	rows = 10;
	coldim = 15360;		// 512 * 30m
	rowdim = 15360;		// 512 * 30m
#endif

	TiledDatasetDescription elev, image;

	if (!elev.Read(dataset_fname_elev))
		return false;
	if (!image.Read(dataset_fname_image))
		return false;

	// We assume that the projection and extents of the two datasets are the same,
	//  so simply take them from the elevation dataset.

	cols = elev.cols;
	rows = elev.rows;
	coldim = elev.earthextents.Width() / cols;	// TODO: watchout, earth vs. world?
	rowdim = elev.earthextents.Height() / rows;
	lod0size = elev.lod0size;
	m_proj = elev.proj;
	center.x = coldim * cols / 2.0f;
	center.y = 0;
	center.z = -rowdim * rows / 2.0f;

	// Set up earth->world heightfield properties
	Initialize(m_proj.GetUnits(), elev.earthextents, 0, 4000);	// TODO min/maxheight?

	// folder names are same as the .ini files, without the .ini
	vtString folder_elev = dataset_fname_elev;
	RemoveFileExtensions(folder_elev);
	vtString folder_image = dataset_fname_image;
	RemoveFileExtensions(folder_image);

#if 0
	// Inform VTP objects of heightfield attributes
	m_proj.SetWellKnownGeogCS("WGS84");
	m_proj.SetUTMZone(5);
	earthextents.left  = 176136;
	earthextents.right = 314376;
	earthextents.bottom= 2092670;
	earthextents.top   = 2246270;
#endif

	exaggeration=2.0f; // exaggeration=200%

	// global resolution
	res=5.0E6f;
	minres=100.0f;

	hfields = new ucharptr[cols*rows];
	textures = new ucharptr[cols*rows];
	vtString str, str2;
	int i, j;
	for (i = 0; i < cols; i++)
	{
		for (j = 0; j < rows; j++)
		{
			str2.Format("/tile.%d-%d", i, j);

			str = folder_elev;
			str += str2;
			str += ".pgm";

			hfields[i+cols*j] = new byte[str.GetLength()+1];
			strcpy((char *) hfields[i+cols*j], (const char *) str);

			str = folder_image;
			str += str2;
			str += ".ppm";

			textures[i+cols*j] = new byte[str.GetLength()+1];
			strcpy((char *) textures[i+cols*j], (const char *) str);
		}
	}

	setminierrorhandler(mini_error_handler);

	SetupMiniLoad();

	// The miniload constructor has copied all the strings we passed to it,
	//  so we should delete the original copy of them
	for (i = 0; i < cols; i++)
	{
		for (j = 0; j < rows; j++)
		{
			delete [] hfields[i+cols*j];
			delete [] textures[i+cols*j];
		}
	}

	return true;
}

#define USE_VERTEX_CACHE	1

void vtTiledGeom::SetupMiniLoad()
{
	VTLOG("Calling miniload constructor(%d,%d,..)\n", cols, rows);
	m_pMiniLoad = new miniload(hfields, textures,
		cols, rows,
		coldim, rowdim,
		1.0f, center.x, center.y, center.z);
	m_pMiniTile = m_pMiniLoad->getminitile();

#if USE_VERTEX_CACHE
	// use tile caching with vertex arrays
	cache.setcallbacks(m_pMiniTile, // the minitile object to be cached
						cols,rows, // number of tile columns and rows
						coldim,rowdim, // overall extent
						center.x,center.y,center.z); // origin with negative Z
#endif

	// Set (pre)loader parameters
	const float farp = 400000;	// 400 km

	//pfarp: controls the radius of the preloading area
	//	- should be greater than the distance to the far plane farp
	float pfarp = 1.25f*farp;

	//prange: controls the enabling distance of the first texture LOD
	//	- a value of zero disables the use of the texture pyramid
	//	- the range can be calculated easily from a given screen space
	//		error threshold using the miniload::calcrange method
//	float prange = farp/10.0f;
	//int texdim = 512, winheight = 600, fovy = m_fFOVY;
	//float prange = m_pMiniLoad->calcrange(texdim,winheight,fovy);
	float prange = 20000;

	//pbasesize: specifies the maximum texture size that is paged in
	//	- a value of zero means that texture size is not limited
	//	- for non-zero values LOD 0 is redefined to be the level
	//		with the specified base texture size so that larger
	//		textures have negative LOD and are not paged in
	//	- the dimension of a texel from LOD 0 can be fed into
	//		the calcrange method in order to control texture paging
	//		by using a screen space error instead of an enabling range
	//	- the base size should be set to the size of the largest
	//		texture tile so that the maximum texture detail is shown
	//	- but the base size should not exceed the maximum texture
	//		size supported by the hardware
	int pbasesize = 512;

	//paging: enables/disables paging from the LOD pyramid
	//	- a value of zero means that only LOD 0 is used
	//	- for a value of one the next larger level is paged in
	//		after a LOD difference of one level is reached
	//	- for a value of s>=1 the next larger level is preloaded
	//		after a LOD difference of s-1 levels is reached
	//	recommended: 1
	int paging = 1;

	//plazyness: controls the lazyness of paging
	//	- a value of zero means that the LODs are updated instantly
	//	- for a value of l>=0 the next smaller level is paged in
	//		after a LOD difference of l+1 levels is reached
	//	- for a value of l>=1 the next smaller level is preloaded
	//		after a LOD difference of l levels is reached
	//	- for maximum memory utilization set l to 0
	//	- to reduce data traffic increase l
	//	  recommended: 1
	int plazyness = 0;

	//pupdate: update time
	//	- determines the number of frames after which a complete
	//		update of the tiles (both visible and preloaded) is finished
	//	- should be not much smaller than the frame rate
	//	- then for each frame only a small fraction of the tiles is
	//		updated in order to limit the update latencies
	//	- a value of zero means that one tile is updated per frame
	int pupdate = 1000;

	//expire: expiration time
	//	- determines the number of frames after which invisible tiles
	//		are removed from the tile cache
	//	- should be much larger than the frame rate
	//	- a value of zero disables expiration
	int pexpire = 100000;

	m_pMiniLoad->setloader(request_callback,
		NULL,	// data
		NULL/*preload_callback*/,
		deliver_callback,
		paging,
		pfarp, prange, pbasesize,
		plazyness, pupdate, pexpire);

	// define resolution reduction of invisible tiles
	m_pMiniTile->setreduction(2.0f,3.0f);
}

void vtTiledGeom::DoRender()
{
	float ex = m_eyepos_ogl.x;
	float ey = m_eyepos_ogl.y;
	float ez = m_eyepos_ogl.z;

	float ux = eye_up.x;
	float uy = eye_up.y;
	float uz = eye_up.z;

	float dx = eye_forward.x;
	float dy = eye_forward.y;
	float dz = eye_forward.z;

	// Convert the eye location to the unusual coordinate scheme of libMini.
//	ex -= (cols/2)*m_fXStep;
//	ez += (rows/2)*m_fZStep;

	const int fpu=0;

#if 1
	// update vertex arrays
	m_pMiniLoad->draw(res,
				ex,ey,ez,
				dx,dy,dz,
				ux, uy, uz,
				m_fFOVY,m_fAspect,
				m_fNear,m_fFar,
				1.0f,
				fpu);

  #if USE_VERTEX_CACHE
	// render vertex arrays
	int vtx=cache.rendercache();
  #endif
#elif 1
	m_pMiniTile->draw(res,
		ex, ey, ez,
		dx, dy, dz,
		ux, uy, uz,
		m_fFOVY, m_fAspect,
		m_fNear, m_fFar,
		1.0f,
		fpu);
#else
	glBegin(GL_QUADS);
	glVertex3f(0, 0, 0);
	glVertex3f(130000, 0, 0);
	glVertex3f(130000, 0, -150000);
	glVertex3f(0, 0, -150000);
	glEnd();
#endif
}

void vtTiledGeom::DoCalcBoundBox(FBox3 &box)
{
#if 0
	FPoint3 center(m_pMiniLoad->CENTERX,
		m_pMiniLoad->CENTERY,
		m_pMiniLoad->CENTERZ);
	FPoint3 size(m_pMiniLoad->COLS * m_pMiniLoad->COLDIM,
		0,
		m_pMiniLoad->ROWS*m_pMiniLoad->ROWDIM);
	box.min = center - size/2;
	box.max = center + size/2;
#else
	// TEMP OVERRIDE for testing
	box.min.Set(0, 0, -150000);
	box.max.Set(130000, 0, 0);
#endif
}

void vtTiledGeom::DoCull(const vtCamera *pCam)
{
	// Grab necessary values from the VTP Scene framework, store for later
	m_eyepos_ogl = pCam->GetTrans();
	m_window_size = vtGetScene()->GetWindowSize();
	m_fAspect = (float)m_window_size.x / m_window_size.y;
	m_fNear = pCam->GetHither();
	m_fFar = pCam->GetYon();

	// Get up vector and direction vector from camera matrix
	FMatrix4 mat;
	pCam->GetTransform1(mat);
	FPoint3 up(0.0f, 1.0f, 0.0f);
	mat.TransformVector(up, eye_up);

	FPoint3 forward(0.0f, 0.0f, -1.0f);
	mat.TransformVector(forward, eye_forward);

	if (pCam->IsOrtho())
	{
		// libMini supports orthographic viewing as of libMini 5.0.
		// A negative FOV value indicates to the library that the FOV is
		//  actually the orthographic height of the camera.
		m_fFOVY = pCam->GetWidth() / m_fAspect;
		m_fFOVY = -m_fFOVY;
	}
	else
	{
		float fov = pCam->GetFOV();
		float fov_y2 = atan(tan (fov/2) / m_fAspect);
		m_fFOVY = fov_y2 * 2.0f * 180 / PIf;
	}
}

bool vtTiledGeom::FindAltitudeOnEarth(const DPoint2 &p, float &fAltitude,
									  bool bTrue) const
{
	// TODO: support other arguments
	float x, z;
	g_Conv.ConvertFromEarth(p, x, z);

	float alt = m_pMiniTile->getheight(x, z);
	if (alt == -FLT_MAX)
		return false;
	fAltitude = alt;

#if 0
	int col = (int)(x / coldim);
	int row = (int)(-z / rowdim);

	// safety check
	if (col < 0 || col >= cols || row < 0 || row >= rows)
	{
		fAltitude = 0.0f;
		return false;
	}

	// Now determine height on the tile; find offset within the tile
	//double ox=coldim*(bi-(COLS-1)/2.0f)+CENTERX;
	//double oz=rowdim*(bj-(ROWS-1)/2.0f)+CENTERZ;

	float alt0, alt1, alt2, alt3;
	alt0 = GetElevation(iX, iY, bTrue);
	alt1 = GetElevation(iX+1, iY, bTrue);
	alt2 = GetElevation(iX+1, iY+1, bTrue);
	alt3 = GetElevation(iX, iY+1, bTrue);

	// find fractional amount (0..1 across quad)
	double fX = (p.x - (m_EarthExtents.left + iX * spacing.x)) / spacing.x;
	double fY = (p.y - (m_EarthExtents.bottom + iY * spacing.y)) / spacing.y;

	// which of the two triangles in the quad is it?
	if (fX + fY < 1)
		fAltitude = (float) (alt0 + fX * (alt1 - alt0) + fY * (alt3 - alt0));
	else
		fAltitude = (float) (alt2 + (1.0-fX) * (alt3 - alt2) + (1.0-fY) * (alt1 - alt2));

#endif
	return true;
}

bool vtTiledGeom::FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
	bool bTrue, bool bIncludeCulture, FPoint3 *vNormal) const
{
	// TODO: support other arguments
	float alt = m_pMiniTile->getheight(p3.x, p3.z);

	// This is what libMini does if the point isn't on the terrain
	if (alt == -FLT_MAX)
		return false;

	fAltitude = alt;
	return true;
}

bool vtTiledGeom::CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
	FPoint3 &result) const
{
	float alt;
	bool bOn = FindAltitudeAtPoint(point, alt);

	// special case: straight up or down
	float mag2 = sqrt(dir.x*dir.x+dir.z*dir.z);
	if (fabs(mag2) < .000001)
	{
		result = point;
		result.y = alt;
		if (!bOn)
			return false;
		if (dir.y > 0)	// points up
			return (point.y < alt);
		else
			return (point.y > alt);
	}

	if (bOn && point.y < alt)
		return false;	// already firmly underground

	// adjust magnitude of dir until 2D component has a good magnitude
	// TODO: better estimate than 0,0 tile!
	int size = m_pMiniTile->getsize(0,0);
	float fXStep = m_pMiniTile->getcoldim() / size;
	float fZStep = m_pMiniTile->getrowdim() / size;
	float smallest = std::min(fXStep, fZStep);
	float adjust = smallest / mag2;
	FPoint3 dir2 = dir * adjust;

	bool found_above = false;
	FPoint3 p = point, lastp = point;
	while (true)
	{
		// are we out of bounds and moving away?
		if (p.x < m_WorldExtents.left && dir2.x < 0)
			return false;
		if (p.x > m_WorldExtents.right && dir2.x > 0)
			return false;
		if (p.z < m_WorldExtents.top && dir2.z < 0)
			return false;
		if (p.z > m_WorldExtents.bottom && dir2.z > 0)
			return false;

		bOn = FindAltitudeAtPoint(p, alt);
		if (bOn)
		{
			if (p.y > alt)
				found_above = true;
			else
				break;
		}
		lastp = p;
		p += dir2;
	}
	if (!found_above)
		return false;

	// now, do a binary search to refine the result
	FPoint3 p0 = lastp, p1 = p, p2;
	for (int i = 0; i < 10; i++)
	{
		p2 = (p0 + p1) / 2.0f;
		int above = PointIsAboveTerrain(p2);
		if (above == 1)	// above
			p0 = p2;
		else if (above == 0)	// below
			p1 = p2;
	}
	p2 = (p0 + p1) / 2.0f;
	// make sure it's precisely on the ground
	FindAltitudeAtPoint(p2, p2.y);
	result = p2;
	return true;
}
