//
// QuikGrid.h
//
// This code provides an interface to the QuikGrid LGPL library
//  (http://www.perspectiveedge.com/source.html)
//
// In particular, it solves the problem that QuikGrid can only link to
//  a single callback function for contour output.
//
// Copyright (c) 2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#if SUPPORT_QUIKGRID

// QuikGrid headers
#include "surfgrid.h"
#include "contour.h"

typedef void (*ContourCallback)(void *context, float x, float y, bool bStart);

void SetQuikGridCallbackFunction(ContourCallback fn, void *context);

#endif // SUPPORT_QUIKGRID
