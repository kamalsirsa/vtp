//
// QuikGrid.cpp
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

#include "config_vtdata.h"
#include <stdio.h>

#if SUPPORT_QUIKGRID

#include "QuikGrid.h"

static ContourCallback s_pLineCallbackFunction = NULL;
static void *s_pContext = NULL;

//
// This globally-scoped method is found at link time by the QuikGrid
//  library, which uses it as a callback.
//
void DoLineTo( float x, float y, int drawtype )
{
	if (s_pLineCallbackFunction != NULL)
		s_pLineCallbackFunction(s_pContext, x, y, drawtype == 0);
}

void SetQuikGridCallbackFunction(ContourCallback fn, void *context)
{
	s_pLineCallbackFunction = fn;
	s_pContext = context;
}

#endif // SUPPORT_QUIKGRID
