//
// Fence3d.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FENCE3DH
#define FENCE3DH

#include "vtdata/Fence.h"
#include "HeightField.h"

class vtFence3d : public vtFence
{
public:
	vtFence3d();
	vtFence3d(FenceType type, float fHeight, float fSpacing);

	static void SetScale(float fScale) { m_fFenceScale = fScale; }

	bool CreateShape(vtHeightField *pHeightField);
	void BuildGeometry(vtHeightField *pHeightField);
	void DestroyGeometry();
	vtGeom *GetGeom();

protected:
	void	Init();
	void	AddFenceMeshes(vtHeightField *pHeightField);
	void	AddFencepost(FPoint3 &p1, int iMatIdx);

	// all fences share the same set of materials
	static vtMaterialArray *m_pFenceMats;
	static void CreateMaterials();
	static int m_mi_woodpost;
	static int m_mi_wire;
	static int m_mi_chainlink;
	static int m_mi_metalpost;

	// fence size is exaggerated by this amount
	static float m_fFenceScale;

	vtGeom		*m_pFenceGeom;
	bool		m_bBuilt;
};

typedef class vtFence *vtFencePtr;

#endif	// FENCE3DH
