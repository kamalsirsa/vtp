//
// LocalDatabuf.cpp: a wrapper for the Databuf class which is found either in
//  in vtdata or libMini.
//
// Copyright (c) 2008 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "LocalDatabuf.h"

/////////////////////////////////////////////////////

#ifdef _MSC_VER
  #if _MSC_VER >= 1400 // vc8
	  #pragma message( "Adding link with libMini-vc8.lib" )
	  #pragma comment( lib, "libMini-vc8.lib" )
  #else // vc71
	  #pragma message( "Adding link with libMini-vc7.lib" )
	  #pragma comment( lib, "libMini-vc7.lib" )
  #endif
  #pragma comment( lib, "glu32.lib" )
#endif

/////////////////////////////////////////////////////


#if USE_LIBMINI_DATABUF

// A useful method to set the extents (in local CRS) and the corners
//  (in Geo WGS84) at the same time.
bool vtMiniDatabuf::SetBounds(const vtProjection &proj, const DRECT &extents)
{
	// First, set the extent rectangle
	set_extents((float)extents.left, (float)extents.right, (float)extents.bottom, (float)extents.top);

	// Create transform from local to Geo-WGS84
	vtProjection geo;
	geo.SetWellKnownGeogCS("WGS84");
	OCT *trans = CreateCoordTransform(&proj, &geo);

	if (!trans)
		return false;

	// Convert each corner as a point
	DPoint2 sw_corner, se_corner, nw_corner, ne_corner;

	sw_corner.Set(extents.left, extents.bottom);
	trans->Transform(1, &sw_corner.x, &sw_corner.y);

	se_corner.Set(extents.right, extents.bottom);
	trans->Transform(1, &se_corner.x, &se_corner.y);

	nw_corner.Set(extents.left, extents.top);
	trans->Transform(1, &nw_corner.x, &nw_corner.y);

	ne_corner.Set(extents.right, extents.top);
	trans->Transform(1, &ne_corner.x, &ne_corner.y);

	set_LLWGS84corners((float)sw_corner.x, (float)sw_corner.y,
                       (float)se_corner.x, (float)se_corner.y,
                       (float)nw_corner.x, (float)nw_corner.y,
                       (float)ne_corner.x, (float)ne_corner.y);
	delete trans;
	return true;
}

//! get rgb[a] color
void vtMiniDatabuf::getrgb(const unsigned int i,const unsigned int j,const unsigned int k, float *value)
{
	if (type==3)
	{
		unsigned char *ptr=&((unsigned char *)data)[3*(i+(j+k*ysize)*xsize)];

		value[0] = ptr[0];
		value[1] = ptr[1];
		value[2] = ptr[2];
	}
}

void vtMiniDatabuf::getrgba(const unsigned int i,const unsigned int j,const unsigned int k, float *value)
{
	if (type==4)
	{
		unsigned char *ptr=&((unsigned char *)data)[4*(i+(j+k*ysize)*xsize)];

		value[0] = ptr[0];
		value[1] = ptr[1];
		value[2] = ptr[2];
		value[3] = ptr[3];
	}
}

#endif
