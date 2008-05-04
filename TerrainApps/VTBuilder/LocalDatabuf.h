//
// LocalDatabuf.h: a wrapper for the Databuf class which is found either in
//  in vtdata or libMini.
//

#ifndef LocalDatabuf_H
#define LocalDatabuf_H

#define USE_LIBMINI_DATABUF 1

#include "vtdata/MiniDatabuf.h"

#if USE_LIBMINI_DATABUF

/*
#include <mini/database.h> // part of libMini
*/

#include <database.h> // part of libMini

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

// Directly subclass from libMini
class vtMiniDatabuf: public databuf
{
public:
	// A useful method to set the extents (in local CRS) and the corners
	//  (in Geo WGS84) at the same time.
	bool SetBounds(const vtProjection &proj, const DRECT &extents);

   //! get rgb[a] color
   void getrgb(const unsigned int i,const unsigned int j,const unsigned int k, float *value);
   void getrgba(const unsigned int i,const unsigned int j,const unsigned int k, float *value);
};

#else	// not USE_LIBMINI_DATABUF

// Simply use the implementation in vtdata
class vtMiniDatabuf: public MiniDatabuf {};

#endif	// USE_LIBMINI_DATABUF

#endif // LocalDatabuf_H
