//
// LodGrid.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LODGRIDH
#define LODGRIDH

class vtGeom;
class vtTransform;
class vtHeightField;

class vtLodGrid : public vtGroup
{
public:
	vtLodGrid(const FPoint3 &origin, const FPoint3 &size,
		int iDimension, float fLODDistance, vtHeightField3d *pHF = NULL);
	void Destroy();

	// methods
	bool AppendToGrid(vtTransform *pTrans);
	bool AppendToGrid(vtGeom *pGeom);
	void RemoveFromGrid(vtTransform *pTNode);
	void RemoveFromGrid(vtGeom *pModel);
	void RemoveNodeFromGrid(vtNodeBase *pNode);

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

#endif
