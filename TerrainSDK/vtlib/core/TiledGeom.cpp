//
// vtTiledGeom: Renders tiled heightfields using Roettger's libMini library
//
// Copyright (c) 2005-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtdata/FilePath.h"
#include "vtdata/vtLog.h"
#include "TiledGeom.h"

#include "mini.h"
#include "miniload.hpp"
#include "minicache.hpp"
#include "datacloud.hpp"
#include "miniOGL.h"

// If we use the pthreads library, we can support multithreading
#define SUPPORT_PTHREADING	1

#if SUPPORT_PTHREADING
  #include <pthread.h>
  #ifdef _MSC_VER
	#pragma message( "Adding link with pthreadVC2.lib" )
	#pragma comment( lib, "pthreadVC2.lib" )
  #endif
#endif

#define USE_OPENTHREADS	0
#if USE_OPENTHREADS
  #include "OpenThreads/Thread"
#endif

// Set this to use the 'minicache' OpenGL primitive cache.
//  Actually, we depend on it for adaptive resolution, so leave it at 1.
#define USE_VERTEX_CACHE	1

#define WE_OWN_BUFFERS		0

#define LOG_TILE_LOADS		0

/////////////
// singleton; TODO: get rid of this to allow multiple instances
static vtTiledGeom *s_pTiledGeom = NULL;

#if SUPPORT_PTHREADING
   const int numthreads = 1;
   pthread_t pthread[numthreads];
   pthread_mutex_t mutex,iomutex;
   pthread_attr_t attr;

   void threadinit()
      {
      pthread_mutex_init(&mutex,NULL);
	  pthread_mutex_init(&iomutex,NULL);

      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
      }

   void threadexit()
      {
      pthread_mutex_destroy(&mutex);
	  pthread_mutex_destroy(&iomutex);
      pthread_attr_destroy(&attr);
      }

   void startthread(void *(*thread)(void *background),backarrayelem *background,void *data)
      {pthread_create(&pthread[background->background-1],&attr,thread,background);}

   void jointhread(backarrayelem *background,void *data)
      {
      void *status;
      pthread_join(pthread[background->background-1],&status);
      }

   void lock_cs(void *data)
      {pthread_mutex_lock(&mutex);}

   void unlock_cs(void *data)
      {pthread_mutex_unlock(&mutex);}

	void lock_io(void *data)
	  {pthread_mutex_lock(&iomutex);}

	void unlock_io(void *data)
	  {pthread_mutex_unlock(&iomutex);}
#endif
#if USE_OPENTHREADS
   const int numthreads = 1;
   OpenThread::Thread pthread[numthreads];
   pthread_mutex_t mutex;
   pthread_attr_t attr;

   void threadinit()
      {
      pthread_mutex_init(&mutex,NULL);

      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
      }

   void threadexit()
      {
      pthread_mutex_destroy(&mutex);
      pthread_attr_destroy(&attr);
      }

   void startthread(void *(*thread)(void *background),backarrayelem *background,void *data)
      {pthread_create(&pthread[background->background-1],&attr,thread,background);}

   void jointhread(backarrayelem *background,void *data)
      {
      void *status;
      pthread_join(pthread[background->background-1],&status);
      }

   void lock_cs(void *data)
      {pthread_mutex_lock(&mutex);}

   void unlock_cs(void *data)
      {pthread_mutex_unlock(&mutex);}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
///////////////////////////////////////////////////////////////////////
// class TiledDatasetDescription implementation

TiledDatasetDescription::TiledDatasetDescription()
{
	cols = rows = lod0size = 0;
	earthextents.Empty();
	minheight = maxheight = INVALID_ELEVATION;
}

bool TiledDatasetDescription::Read(const char *dataset_fname)
{
	FILE *fp = vtFileOpen(dataset_fname, "rb");
	if (!fp) return false;
	fscanf(fp, "[TilesetDescription]\n");
	char buf[4096];
	while (fgets(buf, 4096, fp))
	{
		if (!strncmp(buf, "Columns", 7))
			sscanf(buf, "Columns=%d\n", &cols);
		if (!strncmp(buf, "Rows", 4))
			sscanf(buf, "Rows=%d\n", &rows);
		if (!strncmp(buf, "LOD0_Size", 9))
			sscanf(buf, "LOD0_Size=%d\n", &lod0size);
		if (!strncmp(buf, "Extent_Left", 11))
			sscanf(buf, "Extent_Left=%lf\n", &earthextents.left);
		if (!strncmp(buf, "Extent_Right", 12))
			sscanf(buf, "Extent_Right=%lf\n", &earthextents.right);
		if (!strncmp(buf, "Extent_Bottom", 13))
			sscanf(buf, "Extent_Bottom=%lf\n", &earthextents.bottom);
		if (!strncmp(buf, "Extent_Top", 10))
			sscanf(buf, "Extent_Top=%lf\n", &earthextents.top);
		if (!strncmp(buf, "Elevation_Min", 13))
			sscanf(buf, "Elevation_Min=%f\n", &minheight);
		if (!strncmp(buf, "Elevation_Max", 13))
			sscanf(buf, "Elevation_Max=%f\n", &maxheight);
		if (!strncmp(buf, "CRS", 3))
		{
			// read CRS from WKT
			char *wktp = buf + 4;	// skip "CRS="
			proj.importFromWkt(&wktp);
		}
		if (!strncmp(buf, "RowLODs", 7))
		{
			int rownum;
			sscanf(buf+7, "%d:", &rownum);

			// safety check
			if (rownum < 0 || rownum >= rows)
				return false;

			if (rownum == 0)
				lodmap.alloc(cols, rows);

			char *c = buf + 11;
			for (int i = 0; i < cols; i++)
			{
				int mmin, mmax;
				while (*c == ' ') c++;
				sscanf(c, "%d/%d", &mmin, &mmax);
				lodmap.set(i, rownum, mmin, mmax);
				while (*c != 0 && *c != ' ' && *c != '\n') c++;
			}
		}
	}
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

// check a file
int file_exists(const char *filename)
{
	FILE *file;
	if ((file=vtFileOpen(filename,"rb"))==NULL)
		return(0);
	fclose(file);
	return(1);
}

///////////////////////////////////////////////////////////////////////

int request_callback(int col,int row,unsigned char *mapfile,int hlod,
					 unsigned char *texfile,int tlod,
					  unsigned char *fogfile, void *data,
					  databuf *hfield, databuf *texture, databuf *fogmap)
{
	vtTiledGeom *tg = (vtTiledGeom *) data;
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

	if (!hfield && !texture && !fogmap)
	{
		// just checking for file existence
		int present=1;

		if (mapfile!=NULL)
		{
#if LOG_TILE_LOADS
			VTLOG1(" exist check: ");
			VTLOG1((char *)mapfile);
#endif
//			present&=file_exists((char *)mapfile);
			present &= (tg->CheckMapFile((char *)mapfile, false) ? 1 : 0);
		}
		if (texfile!=NULL)
		{
#if LOG_TILE_LOADS
			VTLOG1(" exist check: ");
			VTLOG1((char *)texfile);
#endif
//			present&=file_exists((char *)texfile);
			present &= (tg->CheckMapFile((char *)texfile, true) ? 1 : 0);
		}
#if LOG_TILE_LOADS
		if (present)
			VTLOG1(" YES\n");
		else
			VTLOG1(" NO\n");
#endif

		return(present);
	}

	// libMini's databuf::read depends on American punctuation
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// we need to load (or get from cache) one or both: hfield and texture
	if (mapfile!=NULL)
		*hfield = s_pTiledGeom->FetchTile((char *)mapfile);

	if (texfile!=NULL)
		*texture = s_pTiledGeom->FetchTile((char *)texfile);

	if (fogfile!=NULL)
		*fogmap = s_pTiledGeom->FetchTile((char *)fogfile);

	return 1;
}

void mini_error_handler(char *file, int line, int fatal)
{
	VTLOG("libMini error: file '%s', line %d, fatal %d\n", file, line, fatal);
}

void request_callback_async(unsigned char *mapfile, databuf *map,
							int istexture, int background, void *data)
{
	map->loaddata((char *)mapfile);
}

int check_callback(unsigned char *mapfile, int istexture, void *data)
{
	vtTiledGeom *tg = (vtTiledGeom*) data;
	return tg->CheckMapFile((char *)mapfile, istexture != 0);
}

int inquiry_callback(int col, int row, unsigned char *mapfile, int hlod,
					  void *data, float *minvalue, float *maxvalue)
{
	vtTiledGeom *tg = (vtTiledGeom*) data;
	*minvalue = tg->m_elev_info.minheight;
	*maxvalue = tg->m_elev_info.maxheight;
	return 1;
}

void query_callback(int col, int row, unsigned char *texfile, int tlod,
					void *data, int *tsizex, int *tsizey)
{
	vtTiledGeom *tg = (vtTiledGeom*) data;

	// Find the actual size of this lod of this tile
	LODMap &map = tg->m_image_info.lodmap;
	if (map.exists())
	{
		int mmin, mmax;
		map.get(col, row, mmin, mmax);
		int lodsize = 1 << (mmin - tlod);
		*tsizex = *tsizey = lodsize;
	}
	else
	{
		// we must assume that all tiles have the same base LOD size
		int tbasesize = tg->image_lod0size; // size of texture LOD 0
		while (tlod-->0)
			tbasesize/=2;
		*tsizex = *tsizey = tbasesize;
	}
}

///////////////////////////////////////////////////////////////////////
// class vtTiledGeom implementation

vtTiledGeom::vtTiledGeom()
{
	s_pTiledGeom = this;

	m_pMiniLoad = NULL;
	m_pMiniTile = NULL;
	m_pMiniCache = NULL;
	m_pDataCloud = NULL;

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
	m_iTileLoads = 0;

	// The terrain surface is not lit by diffuse light (since there are no normals
	//  for per-vertex lighting).  However, it does respond to ambient light level
	//  (so that the terrain is dark at night).
	m_pPlainMaterial = new vtMaterial;
	m_pPlainMaterial->SetDiffuse(0,0,0);
	m_pPlainMaterial->SetAmbient(1,1,1);
	m_pPlainMaterial->SetLighting(true);
}

vtTiledGeom::~vtTiledGeom()
{
#if SUPPORT_PTHREADING
	delete m_pDataCloud;
#endif
	delete m_pMiniCache;
	delete m_pMiniLoad;

	delete m_pPlainMaterial;
}

bool vtTiledGeom::ReadTileList(const char *dataset_fname_elev,
							   const char *dataset_fname_image,
							   bool bThreading, bool bGradual)
{
	if (!m_elev_info.Read(dataset_fname_elev))
		return false;
	if (!m_image_info.Read(dataset_fname_image))
		return false;

	// If it's an older elevation dataset, we won't know elevation extents, so
	//  conservatively guess +/-8kmeters
	if (m_elev_info.minheight == INVALID_ELEVATION)
	{
		m_elev_info.minheight = -8192;
		m_elev_info.maxheight = 8192;
	}

	// We assume that the projection and extents of the two datasets are the same,
	//  so simply take them from the elevation dataset.

	// Set up earth->world heightfield properties
	m_proj = m_elev_info.proj;
	Initialize(m_proj.GetUnits(), m_elev_info.earthextents, m_elev_info.minheight, m_elev_info.maxheight);

	cols = m_elev_info.cols;
	rows = m_elev_info.rows;
	//coldim = elev.earthextents.Width() / cols;	// TODO: watchout, earth vs. world?
	//rowdim = elev.earthextents.Height() / rows;
	coldim = m_WorldExtents.Width() / cols;
	rowdim = -m_WorldExtents.Height() / rows;
	image_lod0size = m_image_info.lod0size;
	center.x = coldim * cols / 2.0f;
	center.y = 0;
	center.z = -rowdim * rows / 2.0f;

	// folder names are same as the .ini files, without the .ini
	m_folder_elev = dataset_fname_elev;
	RemoveFileExtensions(m_folder_elev);
	m_folder_image = dataset_fname_image;
	RemoveFileExtensions(m_folder_image);

	hfields = new ucharptr[cols*rows];
	textures = new ucharptr[cols*rows];
	vtString str, str2;
	int i, j, mmin, mmax;
	for (i = 0; i < cols; i++)
	{
		for (j = 0; j < rows; j++)
		{
			// Set up elevation LOD0 filename
			str2.Format("/tile.%d-%d", i, j);
			str = m_folder_elev;
			str += str2;
			str += ".db";

			bool elev_exists = false, image_exists = false;
			if (m_elev_info.lodmap.exists())
			{
				m_elev_info.lodmap.get(i, j, mmin, mmax);
				if (mmin > 0)
					elev_exists = true;
			}
			else
			{
				// we don't already know, so we must test file existence
				elev_exists = (file_exists(str) != 0);
			}
			if (elev_exists)
			{
				hfields[i+cols*j] = new byte[str.GetLength()+1];
				strcpy((char *) hfields[i+cols*j], str);
			}
			else
				hfields[i+cols*j] = NULL;

			// Set up image LOD0 filename
			str = m_folder_image;
			str += str2;
			str += ".db";
			if (m_image_info.lodmap.exists())
			{
				m_image_info.lodmap.get(i, j, mmin, mmax);
				if (mmin > 0)
					image_exists = true;
			}
			else
			{
				// we don't already know, so we must test file existence
				image_exists = (file_exists(str) != 0);
			}
			if (image_exists)
			{
				textures[i+cols*j] = new byte[str.GetLength()+1];
				strcpy((char *) textures[i+cols*j], str);
			}
			else
				textures[i+cols*j] = NULL;
		}
	}

	setminierrorhandler(mini_error_handler);

	SetupMiniLoad(bThreading, bGradual);

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

void vtTiledGeom::SetupMiniLoad(bool bThreading, bool bGradual)
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
//	float pfarp = farp;
//	float pfarp = 1.25f*farp;
	// Doc says: a value of zero disables preloading of invisible tiles
	//  beyond the far plane.
	float pfarp = 0.0f;

	//prange: controls the enabling distance of the first texture LOD
	//	- a value of zero disables the use of the texture pyramid
	//	- the range can be calculated easily from a given screen space
	//		error threshold using the miniload::calcrange method
//	float prange = m_pMiniLoad->calcrange(texdim,winheight,fovy);
	prange = sqrt(coldim*coldim+rowdim*rowdim)/8;
	prange_min = prange / 3;
	prange_max = prange * 2;

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
	int pupdate = 120;

	//expire: expiration time
	//	- determines the number of frames after which invisible tiles
	//		are removed from the tile cache
	//	- should be much larger than the frame rate
	//	- a value of zero disables expiration
	int pexpire = 0;

	m_pMiniLoad->setloader(
		request_callback,
		this,	// data
		NULL,	// preload_callback
		NULL,	// deliver_callback
		paging,
		pfarp, prange, pbasesize,
		plazyness, pupdate, pexpire);

	// Texture paging range calculates: distance to a tile.
	// To be on the safe side libMini has to take the distance to the next
	// corner and then subtract at least x times the actual tile edge length.
	// x is 0.5 by default but you can configure it with configure_tsafety
	m_pMiniLoad->configure_tsafety(0.0f);

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

	// New feature of libMini 8.3.x for faster paging
	m_pMiniLoad->setfastinit(1);

	miniOGL::configure_compression(0);

#if SUPPORT_PTHREADING
	if (bThreading)
	{
		// Now set up MiniCloud
		m_pDataCloud = new datacloud(m_pMiniLoad);
		m_pDataCloud->setloader(request_callback_async, this, check_callback,
		   paging, pfarp, prange, pbasesize, plazyness, pupdate, pexpire);

		// optional callback for better paging performance
		m_pDataCloud->setinquiry(inquiry_callback, this);

		// optional callback for better paging performance
		m_pDataCloud->setquery(query_callback, this);

		// upload for 10ms and keep for 18 seconds (0.3 minutes)
		m_pDataCloud->setschedule(0.01, 0.3);

		// allow 512 MB tile cache size?
	//	m_pDataCloud->setmaxsize(512.0);
		m_pDataCloud->setmaxsize(0);

		m_pDataCloud->setthread(startthread, NULL, jointhread,
			lock_cs, unlock_cs,
			lock_io, unlock_io);
		m_pDataCloud->setmulti(numthreads);

		threadinit();

		// If the user wants, start with minimal tileset and load first tiles gradually
		if (bGradual)
		{
			float rx = center.x;
			float rz = center.z;
			//float rrad = prange;
			float rrad = 1.0f;
			// This will start with a _very_ minimal tileset of 2x2 tiles
			//m_pMiniLoad->restrictroi(rx, rz, rrad);

			float res = TILEDGEOM_RESOLUTION_MIN;
			float ex = center.x;
			float ey = 10*farp;
			float ez = center.z;
			// This ensures that lowest detail is loaded first
			m_pMiniLoad->updateroi(res, ex, ey, ez, rx, rz, rrad);
		}
	}
#endif // THREADED
}

void vtTiledGeom::SetPagingRange(float val)
{
	prange = val;
	m_pMiniLoad->setrange(prange);
}

float vtTiledGeom::GetPagingRange()
{
	prange = m_pMiniLoad->getrange();
	return prange;
}

databuf vtTiledGeom::FetchTile(const char *fname)
{
	databuf result;

	// load it
#if LOG_TILE_LOADS
	vtString str = StartOfFilename((char *)fname);
	VTLOG1(" disk load: ");
	VTLOG1(str);
	VTLOG1("\n");
#endif
	m_iTileLoads++;

	// Load data buffer directly
	result.loaddata(fname);

	return result;
}

bool vtTiledGeom::CheckMapFile(const char *mapfile, bool bIsTexture)
{
	// we don't need to check file existence if we already know which LODs exist
	if (m_elev_info.lodmap.exists())
	{
		int col, row, lod = 0;
		int mmin, mmax;
		if (bIsTexture)
		{
			// checking an image tile
			sscanf((char *)mapfile + m_folder_image.GetLength(), "/tile.%d-%d.db%d", &col, &row, &lod);
			m_image_info.lodmap.get(col, row, mmin, mmax);
			int num_lods = mmin-mmax+1;
			return (lod < num_lods);
		}
		else
		{
			// checking an elevation tile
			sscanf((char *)mapfile + m_folder_elev.GetLength(), "/tile.%d-%d.db%d", &col, &row, &lod);
			m_elev_info.lodmap.get(col, row, mmin, mmax);
			int num_lods = mmin-mmax+1;
			return (lod < num_lods);
		}
	}
	else
	{
		// no lod info, must check file
		return (file_exists((char *)mapfile) != 0);
	}
	return false;
}

void vtTiledGeom::DoRender()
{
	clock_t c1 = clock();

	// This vtlib material is just a placeholder, since libMini applies its own
	//  textured materials directly using OpenGL.
	ApplyMaterial(m_pPlainMaterial);

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

	static bool first = true;
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
	bool bTrue, int iCultureFlags, FPoint3 *vNormal) const
{
	// Look on culture first
	if (iCultureFlags != 0 && m_pCulture != NULL)
	{
		if (m_pCulture->FindAltitudeOnCulture(p3, fAltitude, bTrue, iCultureFlags))
			return true;
	}

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
	//int size = m_pMiniTile->getsize(0,0);
	//if (size == 0)	// there might be no tile loaded there
	//	return false;
	int size = 1024;	// A reasonable default

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
