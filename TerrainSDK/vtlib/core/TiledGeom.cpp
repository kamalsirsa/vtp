//
// vtTiledGeom: Renders tiled heightfields using Roettger's libMini library
//
// Copyright (c) 2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"
#include "TiledGeom.h"

#include "mini.h"
#include "miniload.hpp"
#include "minicache.hpp"
#include "pnmbase.h"
#include "miniOGL.h"

// Set this to use the 'minicache' OpenGL primitive cache.
//  Actually, we depend on it for adaptive resolution, so leave it at 1.
#define USE_VERTEX_CACHE	1

#define ENABLE_TILE_CACHE	1
#define WE_OWN_BUFFERS		ENABLE_TILE_CACHE

/////////////
// singleton; TODO: get rid of this to allow multiple instances
static vtTiledGeom *s_pTiledGeom = NULL;


#ifndef DOXYGEN_SHOULD_SKIP_THIS
///////////////////////////////////////////////////////////////////////
// class TiledDatasetDescription implementation

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

bool TiledDatasetDescription::GetCorners(DLine2 &line, bool bGeo) const
{
	line.SetSize(4);
	line[0].x = earthextents.left;
	line[0].y = earthextents.bottom;
	line[1].x = earthextents.left;
	line[1].y = earthextents.top;
	line[2].x = earthextents.right;
	line[2].y = earthextents.top;
	line[3].x = earthextents.right;
	line[3].y = earthextents.bottom;

	if (bGeo && !proj.IsGeographic())
	{
		// must convert from whatever we are, to geo
		vtProjection Dest;
		Dest.SetWellKnownGeogCS("WGS84");

		// This is safe (won't fail on tricky Datum conversions) but might
		//  be slightly inaccurate
		OCT *trans = CreateConversionIgnoringDatum(&proj, &Dest);

		if (!trans)
		{
			// inconvertible projections
			return false;
		}
		for (int i = 0; i < 4; i++)
		{
			DPoint2 p = line[i];
			trans->Transform(1, &p.x, &p.y);
			line.SetAt(i, p);
		}
		delete trans;
	}
	return true;
}

#endif	// DOXYGEN_SHOULD_SKIP_THIS


///////////////////////////////////////////////////////////////////////

void request_callback(unsigned char *mapfile, unsigned char *texfile,
					  unsigned char *fogfile, void *data,
					  databuf *hfield, databuf *texture, databuf *fogmap)
{
#if 0
	vtString str1 = "NULL", str2 = "NULL";
	if (mapfile)
		str1 = StartOfFilename((char *)mapfile);
	if (texfile)
		str2 = StartOfFilename((char *)texfile);
	VTLOG("request_callback(%s, %s", (const char *)str1, (const char *)str2);
	if (hfield != NULL)
		VTLOG1(", hfield");
	if (texture != NULL)
		VTLOG1(", texture");
	VTLOG1(")\n");
#endif

	// FetchTile
	// FetchAndCacheTile
	// we need to load (or get from cache) one or both: hfield and texture
	if (mapfile!=NULL)
		*hfield = s_pTiledGeom->FetchAndCacheTile((char *)mapfile);;

	if (texfile!=NULL)
		*texture = s_pTiledGeom->FetchAndCacheTile((char *)texfile);

	if (fogfile!=NULL)
		*fogmap = s_pTiledGeom->FetchAndCacheTile((char *)fogfile);
}

void mini_error_handler(char *file,int line,int fatal)
{
	VTLOG("libMini error: file '%s', line %d, fatal %d\n", file, line, fatal);
}


///////////////////////////////////////////////////////////////////////
// class vtTiledGeom implementation

vtTiledGeom::vtTiledGeom()
{
	s_pTiledGeom = this;

	m_pMiniLoad = NULL;
	m_pMiniTile = NULL;
	m_pMiniCache = NULL;

	// This maxiumum scale is a reasonable tradeoff between the exaggeration
	//  that the user is likely to need, and numerical precision issues.
	m_fMaximumScale = 10.0f;
	m_fHeightScale = 1.0f;
	m_fDrawScale = m_fHeightScale / m_fMaximumScale;

	// defaults
	SetVertexTarget(30000);
	m_fResolution = TILEDGEOM_RESOLUTION_MIN+1;
	m_fHResolution = 2 * m_fResolution;
	m_fLResolution = TILEDGEOM_RESOLUTION_MIN;
	m_bNeedResolutionAdjust = false;

	m_iFrame = 0;
	m_iCacheSize = 0;
	m_iMaxCacheSize = 60 * 1024 * 1024;	// 40 MB cache max
	m_iTileLoads = 0;
	m_iCacheHits = 0;
}

vtTiledGeom::~vtTiledGeom()
{
	EmptyCache();
	delete m_pMiniCache;
	delete m_pMiniLoad;
}

bool vtTiledGeom::ReadTileList(const char *dataset_fname_elev, const char *dataset_fname_image)
{
	TiledDatasetDescription elev, image;

	if (!elev.Read(dataset_fname_elev))
		return false;
	if (!image.Read(dataset_fname_image))
		return false;

	// We assume that the projection and extents of the two datasets are the same,
	//  so simply take them from the elevation dataset.

	// Set up earth->world heightfield properties
	m_proj = elev.proj;
	Initialize(m_proj.GetUnits(), elev.earthextents, 0, 4000);	// TODO min/maxheight?

	cols = elev.cols;
	rows = elev.rows;
	//coldim = elev.earthextents.Width() / cols;	// TODO: watchout, earth vs. world?
	//rowdim = elev.earthextents.Height() / rows;
	coldim = m_WorldExtents.Width() / cols;
	rowdim = -m_WorldExtents.Height() / rows;
	image_lod0size = image.lod0size;
	center.x = coldim * cols / 2.0f;
	center.y = 0;
	center.z = -rowdim * rows / 2.0f;

	// folder names are same as the .ini files, without the .ini
	vtString folder_elev = dataset_fname_elev;
	RemoveFileExtensions(folder_elev);
	vtString folder_image = dataset_fname_image;
	RemoveFileExtensions(folder_image);

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

	//miniOGL::glext_tc = FALSE;

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
	delete [] hfields;
	delete [] textures;

	return true;
}

void vtTiledGeom::SetVerticalExag(float fExag)
{
	m_fHeightScale = fExag;

	// safety check
	if (m_fHeightScale > m_fMaximumScale)
		m_fHeightScale = m_fMaximumScale;

	m_fDrawScale = m_fHeightScale / m_fMaximumScale;
	if (m_pMiniLoad)
		m_pMiniLoad->setrelscale(m_fDrawScale);
}

void vtTiledGeom::SetVertexTarget(int iVertices)
{
	m_iVertexTarget = iVertices;
	m_bNeedResolutionAdjust = true;
}

void vtTiledGeom::SetupMiniLoad()
{
	VTLOG("Calling miniload constructor(%d,%d,..)\n", cols, rows);
	m_pMiniLoad = new miniload(hfields, textures,
		cols, rows,
		coldim, rowdim,
		m_fMaximumScale, center.x, center.y, center.z);
	m_pMiniLoad->setrelscale(m_fDrawScale);
	m_pMiniTile = m_pMiniLoad->getminitile();

#if USE_VERTEX_CACHE
	// use primitive caching with vertex arrays
	m_pMiniCache = new minicache;
	m_pMiniCache->setcallbacks(m_pMiniTile, // the minitile object to be cached
						cols,rows, // number of tile columns and rows
						coldim,rowdim, // overall extent
						center.x,center.y,center.z); // origin with negative Z
#endif

	// Set (pre)loader parameters
	const float farp = 200000;	// 100 km

	//pfarp: controls the radius of the preloading area
	//	- should be greater than the distance to the far plane farp
	float pfarp = 1.25f*farp;
//	float pfarp = farp;

	//prange: controls the enabling distance of the first texture LOD
	//	- a value of zero disables the use of the texture pyramid
	//	- the range can be calculated easily from a given screen space
	//		error threshold using the miniload::calcrange method
//	float prange = m_pMiniLoad->calcrange(texdim,winheight,fovy);
	float prange = sqrt(coldim*coldim+rowdim*rowdim)/2;

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
	int pbasesize = image_lod0size;

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
	int pupdate = 300;

	//expire: expiration time
	//	- determines the number of frames after which invisible tiles
	//		are removed from the tile cache
	//	- should be much larger than the frame rate
	//	- a value of zero disables expiration
	int pexpire = 0;

	m_pMiniLoad->setloader(NULL,	// 'file exists' callback
		request_callback,
		NULL,	// data
		NULL,	// preload_callback
		NULL,	// deliver_callback
		paging,
		pfarp, prange, pbasesize,
		plazyness, pupdate, pexpire);

	// define resolution reduction of invisible tiles
	m_pMiniTile->setreduction(2.0f,3.0f);

	// Stefan says: The lazyness is always >= 0.5 for textures to avoid
	// the following: Suppose you move 1m forward and the LOD is increased.
	// If you move back again 1m then the LOD would be changed (decreased)
	// again. If you then go another 1m forward you would need to reload
	// the same LOD again, but you only moved one meter. This is definitely
	// a behavior which we have to avoid. So there is a minimum lazyness
	// of 0.5 for textures and 1.0 for DEMs.
	// However, lazyness >= 0.5 may be just too conservative.
	// Minimum texture lazyness is configurable:
	m_pMiniLoad->configure_minlazy(0.25);

    // Tell the library not to free the buffers we pass to it, so that our
	// cache can own them and pass them again when needed, without copying.
	m_pMiniLoad->configure_dontfree(WE_OWN_BUFFERS);

	miniOGL::configure_compression(0);
}

databuf vtTiledGeom::FetchAndCacheTile(const char *fname)
{
#if !ENABLE_TILE_CACHE
	m_iMaxCacheSize = 0;	// disable it
#endif
	databuf result;
	result.comment = NULL;

	std::string name = fname;
	TileCache::iterator it = m_Cache.find(name);
	if (it == m_Cache.end())
	{
		// not found in cache; load it
#if 1
		vtString str = StartOfFilename((char *)fname);
		VTLOG1(" disk load: ");
		VTLOG1(str);
		VTLOG1("\n");
#endif
		m_iTileLoads++;

		// Convert PNM to data buffer
		int width, height, components, pnmtype;
		unsigned char *pnmdata = readPNMimage((unsigned char *)fname, &width, &height,
			&components, &pnmtype, 0, NULL, NULL);
		if (!pnmdata)
		{
			result.data = NULL;
			return result;
		}

		result.xsize = width;
		result.ysize = height;
		result.components = components;

		// convert internal data to desired form
		if (pnmtype == 5)	// elevation, 1 or 2 bytes
		{
			int k;
			result.length = width*height*sizeof(short);
			short *hfield=(short *)malloc(result.length);
			if (components==2)
			{
				result.type = 1;	// short
				for (k=0; k<width*height; k++)
				{
					int v=256*pnmdata[2*k]+pnmdata[2*k+1];
					if (v<32768) hfield[k]=v;
					else hfield[k]=v-65536;
				}
			}
			result.data = hfield;

			// We're done with the pnm data
			free(pnmdata);
		}
		else if (pnmtype == 6)	// texture, 3 byte RGB
		{
			result.type = 0;	// bytes
			result.length = width * height * components;
			result.data = pnmdata;
		}

		// max cache size of 0 disables caching
		if (m_iMaxCacheSize == 0)
			return result;

		// Add it to the cache
		// First, make room for it
		while (m_iCacheSize > m_iMaxCacheSize)
		{
			// look for oldest (LRU) tile
			TileCache::iterator oldest;
			int oldest_frame = 1<<30;
			for (it = m_Cache.begin(); it != m_Cache.end(); it++)
			{
				if (it->second.framestamp < oldest_frame)
				{
					oldest_frame = it->second.framestamp;
					oldest = it;
				}
			}
			// remove it from the cache
			CacheEntry &entry = oldest->second;
			m_iCacheSize -= entry.buf->length;
			free(entry.buf->data);
			delete entry.buf;
			m_Cache.erase(oldest);
		}
		// add new entry to the cache
		CacheEntry entry;
		entry.buf = new databuf;
		*entry.buf = result;
		entry.framestamp = m_iFrame;
		m_Cache[name] = entry;
		m_iCacheSize += entry.buf->length;

#if 1
		VTLOG(" cache size (K): %d\n", m_iCacheSize / 1024);
#endif
		// return the original
		return result;
	}
	else
	{
#if 1
		vtString str = StartOfFilename((char *)fname);
		VTLOG1("from cache: ");
		VTLOG1(str);
		VTLOG1("\n");
#endif
		// found in cache
		m_iCacheHits++;
		CacheEntry &entry = it->second;

		// return the original
		return *entry.buf;
	}
	return result;		// just suppress compiler warning, it won't get there
}

void vtTiledGeom::EmptyCache()
{
	int count = 0;
	for (TileCache::iterator it = m_Cache.begin(); it != m_Cache.end(); it++)
	{
		free(it->second.buf->data);
		delete it->second.buf;
		count++;
	}
	m_Cache.clear();
}

void vtTiledGeom::DoRender()
{
	clock_t c1 = clock();

	// count frames
	m_iFrame++;

	// One update every 5 frames is a good approximation
	int fpu=5;
	static float last_res = 0;
	if (last_res != m_fResolution)
		fpu=1;
	last_res = m_fResolution;

	// update vertex arrays
	m_pMiniLoad->draw(m_fResolution,
				m_eyepos_ogl.x, m_eyepos_ogl.y, m_eyepos_ogl.z,
				eye_forward.x, eye_forward.y, eye_forward.z,
				eye_up.x, eye_up.y, eye_up.z,
				m_fFOVY, m_fAspect,
				m_fNear, m_fFar,
				fpu);

#if USE_VERTEX_CACHE
	// render vertex arrays
	static int last_vtx = 0;
	// int vtx=m_pMiniCache->rendercache();
	m_iVertexCount = m_pMiniCache->rendercache();
//	m_iVertexCount = m_pMiniCache->getvtxcnt();
#endif

	static first = true;
	if (first)
	{
		VTLOG("  First Render: %.3f seconds.\n", (float)(clock() - c1) / CLOCKS_PER_SEC);
		first = false;
	}

	// When vertex count changes, we know a full update occurred
	if (m_iVertexCount != last_vtx || m_bNeedResolutionAdjust)
	{
		m_bNeedResolutionAdjust = false;

		// adaptively adjust resolution threshold up or down to attain
		// the desired polygon (vertex) count target
		int diff = m_iVertexCount - m_iVertexTarget;
		int iRange = m_iVertexTarget / 10;		// ensure within 10%

		// If we aren't within the triangle count range adjust the input
		//  resolution, like a binary search
		if (diff < -iRange || diff > iRange)
		{
			//VTLOG("(%d/%d) diff %d, ", m_iVertexCount, m_iVertexTarget, diff);
			if (diff < -iRange)
			{
				m_fLResolution = m_fResolution;
				
				// if the high end isn't high enough, double it
				if (m_fLResolution + 5 >= m_fHResolution)
				{
					//VTLOG1("increase HRes, ");
					m_fHResolution *= 2;
				}
			}
			else
			{
				m_fHResolution = m_fResolution;
				if (m_fLResolution + 5 >= m_fHResolution)
				{
					//VTLOG1("decrease LRes, ");
					m_fLResolution = m_fLResolution/2;
				}
			}

			m_fResolution = m_fLResolution + (m_fHResolution - m_fLResolution) / 2;
			//VTLOG("rez: [%.1f, %.1f, %.1f]\n",
			//	m_fLResolution, m_fResolution, m_fHResolution);

			// keep the resolution within reasonable bounds
			if (m_fResolution < TILEDGEOM_RESOLUTION_MIN)
				m_fResolution = TILEDGEOM_RESOLUTION_MIN;
			if (m_fResolution > TILEDGEOM_RESOLUTION_MAX)
				m_fResolution = TILEDGEOM_RESOLUTION_MAX;

			m_bNeedResolutionAdjust = true;
		}
	}
	last_vtx = m_iVertexCount;
}

void vtTiledGeom::DoCalcBoundBox(FBox3 &box)
{
	FPoint3 center(m_pMiniLoad->CENTERX,
		0,	// We don't know, so use +/-8000 meters
		m_pMiniLoad->CENTERZ);
	FPoint3 size(m_pMiniLoad->COLS * m_pMiniLoad->COLDIM,
		16000,
		m_pMiniLoad->ROWS*m_pMiniLoad->ROWDIM);
	box.min = center - size/2;
	box.max = center + size/2;	// TODO? use height extents, if they are known?
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
	// TODO: support other arguments?
	float x, z;
	m_Conversion.ConvertFromEarth(p, x, z);
	float alt = m_pMiniLoad->getheight(x, z);

	// This is what libMini returns if the point isn't on the terrain
	if (alt == -FLT_MAX)
		return false;

	if (bTrue)
		// convert stored value to true value
		fAltitude = alt / m_fDrawScale / m_fMaximumScale;
	else
		// convert stored value to drawn value
		fAltitude = alt;

	return true;
}

bool vtTiledGeom::FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
	bool bTrue, bool bIncludeCulture, FPoint3 *vNormal) const
{
	// TODO: support other arguments?
	float alt = m_pMiniLoad->getheight(p3.x, p3.z);

	// This is what libMini returns if the point isn't on the terrain
	if (alt == -FLT_MAX)
		return false;

	if (bTrue)
		// convert stored value to true value
		fAltitude = alt / m_fDrawScale / m_fMaximumScale;
	else
		// convert stored value to drawn value
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
	if (size == 0)	// there might be no tile loaded there
		return false;
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

FPoint2 vtTiledGeom::GetWorldSpacingAtPoint(const DPoint2 &p)
{
	float x, z;
	m_Conversion.ConvertFromEarth(p, x, z);

	float dimx, dimz;
	m_pMiniLoad->getdim(x, z, &dimx, &dimz);
	return FPoint2(dimx, dimz);
}
