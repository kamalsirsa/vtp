//
// Contours.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CONTOURSH
#define CONTOURSH

#include "Terrain.h"
class SurfaceGrid;

class ContourConverter
{
public:
	ContourConverter();
	~ContourConverter();

	void Setup(vtTerrain *pTerr, const RGBf &color, float fHeight);
	void GenerateContour(float fAlt);
	void GenerateContours(float fAInterval);
	void NewMesh();
	void Coord(float x, float y, bool bStart);
	void Finish();

protected:
	void Flush();

	SurfaceGrid *m_pGrid;

	vtTerrain *m_pTerrain;
	vtHeightFieldGrid3d *m_pHF;
	DRECT m_ext;
	DPoint2 m_spacing;
	float m_fHeight;

	DLine2	m_line;
	vtGeom *m_pGeom;
	vtMesh *m_pMesh;
};

#endif // CONTOURSH

