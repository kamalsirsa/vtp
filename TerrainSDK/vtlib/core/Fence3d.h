//
// Fence3d.h
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef FENCE3DH
#define FENCE3DH

#include "vtdata/Fence.h"
#include "vtdata/HeightField.h"
#include "Structure3d.h"

/** \addtogroup struct */
/*@{*/

/**
 * This class extends vtFence with the abiliity to construct and
 * operate on a 3D model of a linear structore (parametric fence or wall.)
 */
class vtFence3d : public vtFence, public vtStructure3d
{
public:
	vtFence3d();

	// implement vtStructure3d methods
	virtual bool CreateNode(vtTerrain *pTerr);
	virtual vtGeom *GetGeom() { return m_pFenceGeom; }
	virtual vtNode *GetContained() { return m_pFenceGeom; }
	virtual void DeleteNode();

	void DestroyGeometry();

	// all fences share the same set of materials
	static vtMaterialDescriptorArray3d s_FenceMats;

protected:
	void	Init();
	void	AddFencepost(const FPoint3 &p1, vtMaterialDescriptor *desc);
	void	AddFenceMeshes(vtHeightField3d *pHeightField);
	vtMaterialDescriptor *FindDescriptor(const vtString &type);

	static void CreateMaterials();
	static int s_mi_wire;

	vtGeom		*m_pFenceGeom;
	bool		m_bBuilt;
};

typedef class vtFence *vtFencePtr;

/*@}*/	// Group struct

#endif	// FENCE3DH
