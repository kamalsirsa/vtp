//
// Route.cpp
//
// Creates a route (a series of utility structures, e.g. an electrical
// transmission line), creates geometry, drapes on a terrain
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
//////////////////////////////////////////////////////////////////////

#include "vtlib/vtlib.h"
#include "Light.h"
#include "Route.h"
#include "Terrain.h"
#include "TerrainScene.h"	// for LoadUtilStructure

#define NUM_WIRE_SEGMENTS	160
#define METERS_PER_FOOT		0.3048f	// meters per foot

vtMaterialArray *vtRoute::m_pRouteMats = NULL;

///////////////////

vtUtilNode::vtUtilNode()
{
	m_struct = NULL;
	dRadAzimuth = 0.0f;
	m_pTrans = NULL;
}

void vtUtilNode::Offset(const DPoint2 &delta)
{
	m_Point += delta;
}

///////////////////

vtRoute::vtRoute(vtTerrain* pT)
{
	m_bBuilt = false;
	m_bDirty = true;

	m_pTheTerrain = pT;

	m_pWireGeom = new vtGeode;
	m_pWireGeom->setName("Route Wires");

	if (m_pRouteMats == NULL)
		_CreateMaterials();
	m_pWireGeom->SetMaterials(m_pRouteMats);
}

vtRoute::~vtRoute()
{
	uint i;
	for (i = 0; i < m_Nodes.GetSize(); i++)
		delete m_Nodes[i];
}

void vtRoute::AddPoint(const DPoint2 &epos, const char *structname)
{
	vtUtilNode *node =  new vtUtilNode;
	node->m_Point = epos;
	node->m_sStructName = structname;

	m_Nodes.Append(node);
	Dirty();

	// Load structure for the indicated node.
	// sPath identifies the path to the Route data
	vtUtilStruct *struc = vtGetTS()->LoadUtilStructure(node->m_sStructName);
	if (struc)
		node->m_struct = struc;
}

//
// Builds (or rebuilds) the geometry for a route.
//
void vtRoute::BuildGeometry(vtHeightField3d *pHeightField)
{
	if (!m_bDirty)
		return;

	if (m_bBuilt)
		DestroyGeometry();

	// create surface and shape
	_AddRouteMeshes(pHeightField);

	m_bBuilt = true;
	m_bDirty = false;
}

void vtRoute::DestroyGeometry()
{
	// Destroy the meshes so they can be re-made
	while (m_pWireGeom->GetNumMeshes())
	{
		vtMesh *pMesh = m_pWireGeom->GetMesh(0);
		m_pWireGeom->RemoveMesh(pMesh);
	}

	m_bBuilt = false;
}

void vtRoute::Dirty()
{
	_ComputeStructureRotations();
	m_bDirty = true;
}

void vtRoute::_ComputeStructureRotations()
{
	int i, nodes = m_Nodes.GetSize();
	DPoint2 curr, diff_last, diff_next, diff_use;

	if (nodes < 2)
		return;

	for (i = 0; i < nodes; i++)
	{
		curr = m_Nodes[i]->m_Point;
		if (i > 0)
		{
			diff_last = curr - m_Nodes[i-1]->m_Point;
			diff_last.Normalize();
			diff_use = diff_last;
		}
		if (i < nodes-1)
		{
			diff_next = m_Nodes[i+1]->m_Point - curr;
			diff_next.Normalize();
			diff_use = diff_next;
		}
		if (i > 0 && i < nodes-1)
		{
			// has a node before and after, average the angles
			diff_use = diff_last + diff_next;
		}
		diff_use.Normalize();
		double angle = atan2(diff_use.y, diff_use.x);
		m_Nodes[i]->dRadAzimuth = angle;
	}
}

int vtRoute::m_mi_wire;

void vtRoute::_CreateMaterials()
{
	m_pRouteMats = new vtMaterialArray;

	// add wire material (0)
	m_mi_wire = m_pRouteMats->AddRGBMaterial(RGBf(0.0f, 0.0f, 0.0f), // diffuse
//		RGBf(0.5f, 0.5f, 0.5f),	// ambient grey
		RGBf(1.5f, 1.5f, 1.5f),	// ambient bright white
		false, true, false,		// culling, lighting, wireframe
		1.0f);					// alpha
}

void vtRoute::_CreateStruct(int iNode)
{
	vtUtilNode *node =  m_Nodes[iNode];

	bool add = false;
	if (!node->m_pTrans)
	{
		vtUtilStruct *sobj = node->m_struct;
		if (!sobj)
			return;

		osg::Node *tower = sobj->m_pTower;
		if (tower)
		{
			node->m_pTrans = new vtTransform;
			vtString name;
			name.Format("RouteNode %d", iNode);
			node->m_pTrans->setName(name);
			node->m_pTrans->addChild(tower);
			add = true;
		}
	}

	// set orientation
	node->m_pTrans->Identity();
	node->m_pTrans->RotateLocal(FPoint3(0,1,0), node->dRadAzimuth);

	m_pTheTerrain->PlantModelAtPoint(node->m_pTrans, node->m_Point);

	if (add)
		m_pTheTerrain->AddNodeToStructGrid(node->m_pTrans);
}

void vtRoute::_DeleteStruct(int iNode)
{
	vtUtilNode *node =  m_Nodes[iNode];
	if (node->m_pTrans)
	{
		m_pTheTerrain->RemoveNodeFromStructGrid(node->m_pTrans);
		node->m_pTrans = NULL;
	}
}

void vtRoute::_AddRouteMeshes(vtHeightField3d *pHeightField)
{
	int i, numnodes = m_Nodes.GetSize();

	// generate the structures (poles/towers/etc.)
	for (i = 0; i < numnodes; i++)
		_CreateStruct(i);

	// and the wires
	if (numnodes > 1)
	{
		for (i=1; i<numnodes; i++)
			_StringWires(i, pHeightField);
	}
}

void vtRoute::_StringWires(long ll, vtHeightField3d *pHeightField)
{
	// pick pole numbers i and i-1 and string a wire between them
	long numiterations = NUM_WIRE_SEGMENTS;

	FPoint3 fp0, fp1;

	vtUtilNode *n0 = m_Nodes[ll-1];
	vtUtilNode *n1 = m_Nodes[ll];

	vtUtilStruct *st0 = n0->m_struct;
	vtUtilStruct *st1 = n1->m_struct;

	// safety check
	if (!st0 || !st1)
		return;

	DPoint2 p0 = n0->m_Point;
	DPoint2 p1 = n1->m_Point;

	pHeightField->ConvertEarthToSurfacePoint(p0, fp0);
	pHeightField->ConvertEarthToSurfacePoint(p1, fp1);

	FMatrix4 rot;
	rot.Identity();
	FPoint3 axisY(0, 1, 0);
	FPoint3 offset, wire0, wire1;

	vtMesh *pWireMesh;
	for (int j = 0; j < st1->m_iNumWires; j++)
	{
		pWireMesh = new vtMesh(osg::PrimitiveSet::LINE_STRIP, 0, numiterations+1);

		offset = st0->m_fpWireAtt1[j];
		rot.AxisAngle(axisY, n0->dRadAzimuth);
		rot.Transform(offset, wire0);
		FPoint3 wire_start = fp0 + wire0;

		pWireMesh->AddVertex(wire_start);

		offset = st1->m_fpWireAtt2[j];
		rot.AxisAngle(axisY, n1->dRadAzimuth);
		rot.Transform(offset, wire1);
		FPoint3 wire_end = fp1 + wire1;

		_DrawCat(wire_start, wire_end, vtGetTS()->m_fCatenaryFactor, numiterations, pWireMesh);

		pWireMesh->AddVertex(wire_end);

		pWireMesh->AddStrip2(numiterations+1, 0);
		m_pWireGeom->AddMesh(pWireMesh, m_mi_wire);
	}
}

//
// Draw catenary curve between conductor attachment points
//	from the current tower to the previous tower.
//
void vtRoute::_DrawCat(FPoint3 pt0, FPoint3 pt1, double catenary,
					  int iNumSegs, vtMesh *pWireMesh)
{
	FPoint3 diff = pt1-pt0;
	FPoint3 ptNew;
	int i;
	double xz = sqrt(diff.z*diff.z+diff.x*diff.x);	// distance in the xz plane
	double y = diff.y;
	FPoint3 step = diff / (float) iNumSegs;

	// Calculate the parabolic constants.
	double parabolicConst = (xz / 2) - (y * (catenary / xz));

	FPoint3 ptCur(0,0,0);
	double dist=0;

	vtHeightField3d *pHeightField = m_pTheTerrain->GetHeightField();
	float ground;

	// Iterate along the xz-plane
	for (i = 0; i < iNumSegs-1; i++)
	{
		ptCur.x += step.x;
		ptCur.z += step.z;
		dist = sqrt(ptCur.x*ptCur.x + ptCur.z*ptCur.z);

		ptCur.y = (float) ((dist / (2*catenary)) * (dist - (2*parabolicConst)));

		ptNew = pt0 + ptCur;
		pHeightField->FindAltitudeAtPoint(ptNew, ground);
		ground += 0.5;
		if (ptNew.y < ground)
			ptNew.y = ground;

		pWireMesh->AddVertex(ptNew);
	}
}


/** Find the util node which is closest to the given point, if it is within
 * 'error' distance.  The node and distance are returned by reference.
 */
bool vtRouteMap::FindClosestUtilNode(const DPoint2 &point, double error,
					   vtRoute* &found_route, vtUtilNode* &found_node, double &closest)
{
	found_node = NULL;
	closest = 1E8;

	if (IsEmpty())
		return false;

	DPoint2 loc;
	double dist;

	int i, j, routes = GetSize();
	for (i = 0; i < routes; i++)
	{
		vtRoute *pRoute = GetAt(i);
		int nodes = pRoute->GetSize();

		for (j = 0; j < nodes; j++)
		{
			vtUtilNode *pNode = pRoute->GetAt(j);

			loc = pNode->m_Point;

			dist = (loc - point).Length();
			if (dist > error)
				continue;
			if (dist < closest)
			{
				found_route = pRoute;
				found_node = pNode;
				closest = dist;
			}
		}
	}
	return (found_node != NULL);
}

void vtRouteMap::BuildGeometry(vtHeightField3d *pHeightField)
{
	int routes = GetSize();
	for (int i = 0; i < routes; i++)
	{
		GetAt(i)->BuildGeometry(pHeightField);
	}
}

bool vtRouteMap::FindRouteFromNode(osg::Node *pNode, int &iOffset)
{
	bool bFound = false;
	uint i;

	for (i = 0; (i < GetSize()) & !bFound; i++)
	{
		if (FindAncestor(pNode, GetAt(i)->GetGeom()))
		{
			iOffset = i;
			bFound = true;
		}
	}
	return bFound;
}

