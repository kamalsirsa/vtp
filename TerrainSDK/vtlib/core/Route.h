//
// Route.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
//////////////////////////////////////////////////////////////////////

#ifndef ROUTEH
#define ROUTEH

#define NUM_STRUCT_NAMES 14

class vtHeightField3d;
class vtTerrain;
class vtMesh;

/**
 * This class represents a single type of utility structure, such as a
 * telephone power or transmission tower.
 */
class vtUtilStruct
{
public:
	vtUtilStruct();

	vtTransform*	m_pTower;	// The station may have a tower placed on it
	vtString m_sStructName;

	// The points at which the wires attach
	FPoint3 m_fpWireAtt1[7];
	FPoint3 m_fpWireAtt2[7];

	int m_iNumWires;
};

/**
 * This class represents a 'node' in a network of utility structures.
 * Generally this will have a physical structure associated with it,
 * the type of which is described by the class vtUtilStruct.
 */
class vtUtilNode
{
public:
	vtUtilNode();
	void Offset(const DPoint2 &delta);

	DPoint2 m_dpOffsetElevations;	// x=Left, y=Right
	DPoint2 m_dpOffsetDistances;	// x=Left, y=Right
	double dRadAzimuth;				// azimuth (orientation to next node)
	vtUtilStruct *m_struct;			// Object to draw.
	vtString m_sStructName;			// Structure Family

	DPoint2 m_Point;

	vtTransform *m_pTrans;
};

/**
 * A vtRoute is a set of nodes (vtUtilNode) describing the path of
 * utility components (such as a power line).
 */
class vtRoute
{
public:
	vtRoute(vtTerrain* pT);
	~vtRoute();

	void AddPoint(const DPoint2 &epos, const char *structname);

	void BuildGeometry(vtHeightField3d *pHeightField);
	void DestroyGeometry();
	void Dirty();
	static void ReleaseMaterials();

	vtGeom *GetGeom() { return m_pWireGeom; }

	// Station set readers.
	vtUtilNode *GetAt(unsigned int iter)
	{
		return (iter < m_Nodes.GetSize() ? m_Nodes[iter] : NULL);
	}
	long GetSize() { return m_Nodes.GetSize(); }

protected:
	bool _LoadStructure(vtUtilNode *node);
	bool _WireReader(const char *filename, vtUtilStruct *st);
	void _ComputeStructureRotations();
	void _CreateStruct(int iNode);
	void _AddRouteMeshes(vtHeightField3d *pHeightField);
	void _StringWires(long lTowerIndex, vtHeightField3d *pHeightField);
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

/**
 * vtRouteMap is a container for a set of vtRoute objects.
 */
class vtRouteMap : public Array<vtRoute *>
{
public:
	virtual ~vtRouteMap() { Empty(); free(m_Data); m_Data = NULL; m_MaxSize = 0; }
	void DestructItems(unsigned int first, unsigned int last)
	{
		for (unsigned int i = first; i <= last; i++)
			delete GetAt(i);
	}

	bool FindClosestUtilNode(const DPoint2 &point, double error,
					   vtRoute* &route, vtUtilNode* &node, double &closest);
	void BuildGeometry(vtHeightField3d *pHeightField);
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
