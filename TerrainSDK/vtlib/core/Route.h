//
// Route.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
//////////////////////////////////////////////////////////////////////

#ifndef ROUTEH
#define ROUTEH

class vtHeightField;
class vtTerrain;

class vtUtilStruct
{
public:
	vtUtilStruct();

	vtTransform*	m_pTower;		// The station may have a tower placed on it
	vtString m_sStructName;

	FPoint3 m_fpWireAtt1[7];	 // where are the wires?
	FPoint3 m_fpWireAtt2[7];	 // where are the wires?

	int m_iNumWires;
};

class vtUtilNode
{
public:
	vtUtilNode();
	void Offset(const DPoint2 &delta);

	DPoint2 m_dpOffsetElevations;	// x=Left, y=Right
	DPoint2 m_dpOffsetDistances;	// x=Left, y=Right
	double dRadAzimuth;				// azimuth.
	vtUtilStruct *m_struct;		// Object to draw.
	vtString m_sStructName;			// Structure Family

	DPoint2 m_Point;

	vtTransform *m_pTrans;
};


class vtRoute
{
public:
	vtRoute(vtTerrain* pT);

	void AddPoint(const DPoint2 &epos, const char *structname);

	void BuildGeometry(vtHeightField *pHeightField);
	void DestroyGeometry();
	void Dirty();

	vtGeom *GetGeom() { return m_pWireGeom; }

	// Station set readers.
	vtUtilNode *GetAt(int iter) 
	{
		return (iter < m_Nodes.GetSize() ? m_Nodes[iter] : m_Nodes[0]);
	}
	long GetSize() { return m_Nodes.GetSize(); }

protected:
	bool _LoadStructure(vtUtilNode *node);
	bool _WireReader(const char *filename, vtUtilStruct *st);
	void _ComputeStructureRotations();
	void _CreateStruct(int iNode);
	void _AddRouteMeshes(vtHeightField *pHeightField);
	void _StringWires(long lTowerIndex, vtHeightField *pHeightField);	
	void _DrawCat(FPoint3 p0, FPoint3 p1,
		double catenary, int iNumSegs, vtMesh *pWireMesh);

	// all routes share the same set of materials
	static vtMaterialArray *m_pRouteMats;
	static void _CreateMaterials();
	static int m_mi_wire;

	vtGeom		*m_pWireGeom;
	vtTerrain	*m_pTheTerrain;
	Array<vtUtilNode*>		m_Nodes;
	Array<vtUtilStruct*>	m_StructObjs;

	bool m_bBuilt;
	bool m_bDirty;
};

class vtRouteMap : public Array<vtRoute *>
{
public:
	bool FindClosestUtilNode(const DPoint2 &point, double error,
					   vtRoute* &route, vtUtilNode* &node, double &closest);
	void BuildGeometry(vtHeightField *pHeightField);
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

struct UtilStructName
{
	char *brief;
	char *full;
	char *filename;
};

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#endif //ROUTEH
