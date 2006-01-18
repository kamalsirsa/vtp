//
// vtTin3d.h
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TIN3DH
#define TIN3DH

#include "vtdata/vtTin.h"
#include "vtdata/HeightField.h"

/** \defgroup tin TINs
 * These classes are used Triangulated Irregular Networks (TINs).
 */
/*@{*/

/**
 * This class extends vtTin with the ability to call vtlib to create 3d
 * geometry for the TIN.  It also subclasses vtHeightField so it provides
 * the ability to do height-testing and ray-picking.
 */
class vtTin3d : public vtTin
{
public:
	vtTin3d();

	bool Read(const char *fname);

	vtGeom *CreateGeometry(bool bDropShadowMesh);
	void SetTextureMaterials(vtMaterialArray *pMats);

	// implement HeightField3d virtual methods
	virtual bool FindAltitudeAtPoint(const FPoint3 &p3, float &fAltitude,
		bool bTrue = false, bool bIncludeCulture = false,
		FPoint3 *vNormal = NULL) const;
	virtual bool CastRayToSurface(const FPoint3 &point, const FPoint3 &dir,
		FPoint3 &result) const;

	FPoint3 FindVectorToClosestVertex(const FPoint3 &pos);

protected:
	Array<vtMesh*> m_Meshes;
	vtMaterialArray *m_pMats;
	vtGeom		*m_pGeom;
	vtGeom		*m_pDropGeom;
};

/*@}*/	// Group tin

#endif // TIN3DH
