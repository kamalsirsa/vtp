//
// Contours.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CONTOURSH
#define CONTOURSH

#if SUPPORT_QUIKGRID

#include "Terrain.h"
class SurfaceGrid;

/**
 * This class provides the ability to easily construct contour lines
 * on a terrain.  It does so by calling the QuikGrid library to generate
 * contour vectors, then converts those vectors into 3D line geometry
 * draped on the terrain.
 *
 * \par Here is an example of how to use it:
	\code
	ContourConverter cc;
	cc.Setup(pTerrain, RGBf(1,1,0), 10);
	cc.GenerateContours(100);
	cc.Finish();
	\endcode
 *
 * \par Or, you can generate specific contour lines:
	\code
	ContourConverter cc;
	cc.Setup(pTerrain, RGBf(1,1,0), 10);
	cc.GenerateContour(75);
	cc.GenerateContour(125);
	cc.GenerateContour(250);
	cc.Finish();
	\endcode
 *
 */
class ContourConverter
{
public:
	ContourConverter();
	~ContourConverter();

	vtTransform *Setup(vtTerrain *pTerr, const RGBf &color, float fHeight);
	void GenerateContour(float fAlt);
	void GenerateContours(float fAInterval);
	void Finish();
	void Coord(float x, float y, bool bStart);

protected:
	void NewMesh();
	void Flush();

	SurfaceGrid *m_pGrid;

	vtTerrain *m_pTerrain;
	vtHeightFieldGrid3d *m_pHF;
	DRECT m_ext;
	DPoint2 m_spacing;
	float m_fHeight;

	DLine2	m_line;
	vtTransform *m_pTrans;
	vtGeom *m_pGeom;
	vtMesh *m_pMesh;
};

#endif // SUPPORT_QUIKGRID
#endif // CONTOURSH

