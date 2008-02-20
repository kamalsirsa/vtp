//
// LocalDatabuf.h: a wrapper for the Databuf class which is found either in
//  in vtdata or libMini.
//

#ifndef LocalDatabuf_H
#define LocalDatabuf_H

#define USE_LIBMINI_DATABUF 1

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

#else	// not USE_LIBMINI_DATABUF

// Simply use the implementation in vtdata
class vtMiniDatabuf: public MiniDatabuf {};

#endif	// USE_LIBMINI_DATABUF

#endif // LocalDatabuf_H
