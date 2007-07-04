//
// LodGrid.h
//
// Copyright (c) 2001-2007 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LODGRIDH
#define LODGRIDH

class vtGeom;
class vtTransform;
class vtHeightField3d;

/** \addtogroup sg */
/*@{*/

/**
 * An LOD Grid is a useful way of grouping objects in your scene graph.
 *
 * It implements a sparse 2D array of LOD nodes, which divide an area into
 * a set of Cells.  When objects (geometry, or other node types) are added
 * to the LOD Grid, they are placed in the appropriate Cell.  Each Cell has
 * a distance at which the objects in that Cell are not drawn.  This allows
 * you to have a large number of objects in your scene, yet efficiently cull
 * faraway objects.  The distance can be accessed with
 * SetDistance()/GetDistance().
 *
 * The LOD Grid is particularly designed for terrain, since the Cell division
 * is based on the horizontal (XZ) plane.
 *
 * Since the LOD Grid is a specialized kind of vtGroup, you should call
 * Release() on it rather than delete.
 */
class vtLodGrid : public vtGroup
{
public:
	virtual void Setup(const FPoint3 &origin, const FPoint3 &size,
		int iDimension, float fLODDistance, vtHeightField3d *pHF = NULL) = 0;
	void Release() = 0;

	// methods
	virtual bool AppendToGrid(vtTransform *pTrans) = 0;
	virtual bool AppendToGrid(vtGeom *pGeom) = 0;
	virtual void RemoveFromGrid(vtTransform *pTNode) = 0;
	virtual void RemoveFromGrid(vtGeom *pModel) = 0;
	virtual void RemoveNodeFromGrid(vtNode *pNode) = 0;

	virtual void SetDistance(float fLODDistance) = 0;
	virtual float GetDistance() = 0;
};

/**
 * vtSimpleLodGrid provides a very simple implementation of vtLodGrid.
 *
 * It consists of a 2D grid of LOD nodes, each covering a rectangular cell,
 * which causes that cell's children to only be shown when within a given
 * distance.
 */
class vtSimpleLodGrid : public vtLodGrid
{
public:
	vtSimpleLodGrid();
	void Setup(const FPoint3 &origin, const FPoint3 &size,
		int iDimension, float fLODDistance, vtHeightField3d *pHF = NULL);
	void Release();

	// methods
	bool AppendToGrid(vtTransform *pTrans);
	bool AppendToGrid(vtGeom *pGeom);
	void RemoveFromGrid(vtTransform *pTNode);
	void RemoveFromGrid(vtGeom *pModel);
	void RemoveNodeFromGrid(vtNode *pNode);

	void SetDistance(float fLODDistance);
	float GetDistance();

protected:
	FPoint3 m_origin;
	FPoint3 m_size;
	FPoint3 m_step;

	int m_dim;
	float m_fLODDistance;
	vtLOD **m_pCells;
	vtHeightField3d *m_pHeightField;

	vtGroup *FindCellParent(const FPoint3 &point);
	void AllocateCell(int a, int b);
	void DetermineCell(const FPoint3 &pos, int &a, int &b);
};

/*@}*/  // sg

#endif	// LODGRIDH

