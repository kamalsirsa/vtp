//
// LocalDatabuf.h: a wrapper for the Databuf class which is found either in
//  in vtdata or libMini.
//

#ifndef LocalDatabuf_H
#define LocalDatabuf_H

#define USE_LIBMINI_DATABUF 1
#define USE_LIBMINI_DATABUF_JPEG 0
#define USE_LIBMINI_DATABUF_PNG 0
#define USE_LIBMINI_DATABUF_GREYC 0

#include "vtdata/MiniDatabuf.h"

#if USE_LIBMINI_DATABUF

#include "database.h"	// part of libMini

#ifdef _MSC_VER
  #if _MSC_VER >= 1400	// vc8
	  #pragma message( "Adding link with libMini-vc8.lib" )
	  #pragma comment( lib, "libMini-vc8.lib" )
  #else					// vc71
	  #pragma message( "Adding link with libMini-vc7.lib" )
	  #pragma comment( lib, "libMini-vc7.lib" )
  #endif
  #pragma comment( lib, "glu32.lib" )
#endif

// Directly subclass from libMini
class vtMiniDatabuf: public databuf {};

// parameters for converting external formats
struct VTP_CONVERSION_HOOK_STRUCT
   {
   float jpeg_quality;

   BOOLINT usegreycstoration;

   float greyc_p;
   float greyc_a;
   };

typedef VTP_CONVERSION_HOOK_STRUCT VTP_CONVERSION_PARAMS;

// libMini conversion hook for external formats (JPEG/PNG)
int vtb_conversionhook(int israwdata,unsigned char *srcdata,unsigned int bytes,unsigned int extformat,
                       unsigned char **newdata,unsigned int *newbytes,
                       databuf *obj,void *data);

#else	// not USE_LIBMINI_DATABUF

// Simply use the implementation in vtdata
class vtMiniDatabuf: public MiniDatabuf {};

#endif	// USE_LIBMINI_DATABUF

#endif // LocalDatabuf_H