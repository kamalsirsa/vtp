//
// vtTin.h
//
// A class to represent any regular grid of points representing a height field.
// Includes functionality to find the surface height at any given point.
//
// Copyright (c) 2002 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTTINH
#define VTTINH

#include "HeightField.h"
#include "vtdata/Projections.h"

class vtTin : public vtHeightField
{
public:
	void Read(const char *fname);
	void Write(const char *fname);

	void ComputeExtents();

	vtGeom *CreateGeometry(bool bDropShadowMesh);

	// implement heightfield virtual methods
	virtual bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude, FPoint3 *vNormal = NULL);
	virtual bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result);

	FPoint3 FindVectorToClosestVertex(const FPoint3 &pos);

	vtProjection	m_proj;

protected:
	Array<vtMesh*> m_Meshes;
	vtGeom		*m_pGeom;
	vtGeom		*m_pDropGeom;

	FLine3		m_points;
};

#endif // VTTINH
