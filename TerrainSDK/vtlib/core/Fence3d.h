//
// Fence3d.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FENCE3DH
#define FENCE3DH

#include "vtdata/Fence.h"
#include "vtdata/HeightField.h"
#include "Structure3d.h"

/**
 * This class extends vtFence with the abiliity to construct and
 * operate on a 3D model of a parametric fence or wall.
 */
class vtFence3d : public vtFence, public vtStructure3d
{
public:
	vtFence3d();
	vtFence3d(FenceType type, float fHeight, float fSpacing);

	// implement vtStructure3d methods
	virtual bool CreateNode(vtTerrain *pTerr);
	virtual vtGeom *GetGeom() { return m_pFenceGeom; }
	virtual vtNodeBase *GetContained() { return m_pFenceGeom; }
	virtual void DeleteNode();

	static void SetScale(float fScale) { s_fFenceScale = fScale; }

	void DestroyGeometry();

protected:
	void	Init();
	void	AddFenceMeshes(vtHeightField3d *pHeightField);
	void	AddFencepost(FPoint3 &p1, int iMatIdx);
	void	CreateMeshesWithPosts(vtHeightField3d *pHeightField);
	void	CreateMeshesWithoutPosts(vtHeightField3d *pHeightField);

	// all fences share the same set of materials
	static vtMaterialArray *s_pFenceMats;
	static void CreateMaterials();
	static int m_mi_woodpost;
	static int m_mi_wire;
	static int m_mi_chainlink;
	static int m_mi_metalpost;
	static int m_mi_hedgerow;
	static int m_mi_drystone;
	static int m_mi_privet;
	static int m_mi_stone;
	static int m_mi_beech;

	// fence size is exaggerated by this amount
	static float s_fFenceScale;

	vtGeom		*m_pFenceGeom;
	bool		m_bBuilt;
};

typedef class vtFence *vtFencePtr;

#endif	// FENCE3DH
