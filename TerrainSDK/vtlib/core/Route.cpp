//
// Route.cpp
//
// Creates a route (a series of utility structures, e.g. an electrical
// transmission line), creates geometry, drapes on a terrain
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//
//////////////////////////////////////////////////////////////////////

#include "vtlib/vtlib.h"
#include "Light.h"
#include "Route.h"
#include "Terrain.h"

#define NUM_WIRE_SEGMENTS	160
#define METERS_PER_FOOT		0.3048f	// meters per foot
#define CATENARY_FACTOR		140.0

vtMaterialArray *vtRoute::m_pRouteMats = NULL;

///////////////////

vtUtilStruct::vtUtilStruct()
{
	m_pTower = NULL;
	m_iNumWires = 0;
}

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

	m_pWireGeom = new vtGeom;
	m_pWireGeom->SetName2("Route Wires");

	if (m_pRouteMats == NULL)
		_CreateMaterials();
	m_pWireGeom->SetMaterials(m_pRouteMats);
}

vtRoute::~vtRoute()
{
	unsigned int i;
	for (i = 0; i < m_Nodes.GetSize(); i++)
		delete m_Nodes[i];
	for (i = 0; i < m_StructObjs.GetSize(); i++)
		delete m_StructObjs[i];
}

void vtRoute::AddPoint(const DPoint2 &epos, const char *structname)
{
	vtUtilNode *st =  new vtUtilNode;
	st->m_Point = epos;
	st->m_sStructName = structname;

	m_Nodes.Append(st);
	Dirty();

	_LoadStructure(st);
}

//
// Builds (or rebuilds) the geometry for a fence.
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
	m_pRouteMats = new vtMaterialArray();

	// add wire material (0)
	m_mi_wire = m_pRouteMats->AddRGBMaterial(RGBf(0.0f, 0.0f, 0.0f), // diffuse
//		RGBf(0.5f, 0.5f, 0.5f),	// ambient grey
		RGBf(1.5f, 1.5f, 1.5f),	// ambient bright white
		false, true, false,		// culling, lighting, wireframe
		1.0f);					// alpha
}

void vtRoute::ReleaseMaterials()
{
	if (m_pRouteMats)
	{
		m_pRouteMats->Release();
		m_pRouteMats = NULL;
	}
}

void vtRoute::_CreateStruct(int iNode)
{
	vtUtilNode *node =  m_Nodes.GetAt(iNode);

	bool add = false;
	if (!node->m_pTrans)
	{
		vtUtilStruct *sobj = node->m_struct;
		if (!sobj)
			return;

		vtNodeBase* tower = sobj->m_pTower;
		if (tower)
		{
			node->m_pTrans = new vtTransform;
			vtString name;
			name.Format("RouteNode %d", iNode);
			node->m_pTrans->SetName2(name);
			node->m_pTrans->AddChild(tower);
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


void vtRoute::_AddRouteMeshes(vtHeightField3d *pHeightField)
{
	int i, numnodes = m_Nodes.GetSize();

	// generate the structures (poles/towers/etc.)
	for (i = 0; i < numnodes; i++)
	{
		_CreateStruct(i);
	}

	// and the wires
	if (numnodes > 1)
	{
		for (i=1; i<numnodes; i++)
		{
			_StringWires(i, pHeightField);
		}
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
	for (int j = 0; j< 7; j++)	//7== max number of wires per structure.
	{
		if (j >= st1->m_iNumWires)
			continue;	// skip if no wire.

		pWireMesh = new vtMesh(GL_LINE_STRIP, 0, numiterations+1);

		offset = st0->m_fpWireAtt1[j];
		rot.AxisAngle(axisY, n0->dRadAzimuth - PID2f);
		rot.Transform(offset, wire0);
		FPoint3 wire_start = fp0 + wire0;

		pWireMesh->AddVertex(wire_start);

		offset = st1->m_fpWireAtt2[j];
		rot.AxisAngle(axisY, n1->dRadAzimuth - PID2f);
		rot.Transform(offset, wire1);
		FPoint3 wire_end = fp1 + wire1;

		_DrawCat(wire_start, wire_end, CATENARY_FACTOR, numiterations, pWireMesh);

		pWireMesh->AddVertex(wire_end);

		pWireMesh->AddStrip2(numiterations+1, 0);
		m_pWireGeom->AddMesh(pWireMesh, m_mi_wire);
		pWireMesh->Release();		// pass ownership to geometry
	}
}

UtilStructName s_Names[NUM_STRUCT_NAMES] =
{
	{ "A1", "Steel Poles Tangent", "a1" },
	{ "A2", "Guyed Tangent", "a2" },
	{ "A3", "Light Angle", "a3" },
	{ "A4", "Medium Angle", "a4" },

	{ "A5", "Heavy Angle", "a5" },
	{ "A6", "Large Angle Deadend", "a6" },
	{ "A7", "Light Angle Deadend", "a7" },
	{ "L1", "Lattice Tower", "l1" },
	{ "L2", "Lattice Deadend", "l2" },

	{ "H1", "H-Frame Tangent", "h1" },
	{ "H2", "H-Frame Guyed Tangent", "h2" },
	{ "H3", "H-Frame Hold Down", "h3" },
	{ "H4", "H-Frame Large Angle Deadend", "h4" },
	{ "H5", "H-Frame Light Angle Deadend", "h5" }
};

bool vtRoute::_LoadStructure(vtUtilNode *node)
{
	// Load structure for the indicated node.
	// sPath identifies the path to the Route data
	vtString sname = node->m_sStructName;

	vtString sStructureDataPath = "Culture/UtilityStructures/";
	vtString sStructure = sStructureDataPath;
	vtString sWires;

	// Check to see if it's already loaded
	unsigned int i;
	for (i = 0; i < m_StructObjs.GetSize(); i++)
	{
		if (m_StructObjs[i]->m_sStructName == sname)
		{
			node->m_struct = m_StructObjs[i];
			return true;
		}
	}

	// If not, look for it in the list of known structure types
	for (i = 0; i < NUM_STRUCT_NAMES; i++)
	{
		if (sname.CompareNoCase(s_Names[i].brief) == 0)
		{
			sStructure = s_Names[i].filename;
			sStructure += ".obj";
			sWires = s_Names[i].filename;
			sWires += ".wire";
			break;
		}
	}
	if (i == NUM_STRUCT_NAMES)
	{
		// No such structure
//		return false;
		sStructure = "test.obj";
		sWires = "test.wire";
	}
	vtString struct_file = FindFileOnPaths(vtTerrain::s_DataPaths,
		sStructureDataPath + sStructure);

	if (struct_file == "")
		return false;

	vtUtilStruct *stnew = new vtUtilStruct;
	stnew->m_pTower = m_pTheTerrain->LoadModel(struct_file);
	if (!stnew->m_pTower)
	{
		delete stnew;
		return false;
	}
	stnew->m_pTower->SetName2(sname);

	// scale to match world units
	float sc = (float) METERS_PER_FOOT;
	stnew->m_pTower->Scale3(sc, sc, sc);

	// rotate it until it's upright
	stnew->m_pTower->RotateLocal(FPoint3(1,0,0), -PIf);

	stnew->m_sStructName = sname;
	int j = m_StructObjs.Append(stnew);
	node->m_struct = m_StructObjs[j];

	vtString wire_file = FindFileOnPaths(vtTerrain::s_DataPaths,
		sStructureDataPath + sWires);
	if (wire_file != "")
		_WireReader(wire_file, stnew);

	return true;
}


bool vtRoute::_WireReader(const char *filename, vtUtilStruct *st)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	// Read wire locations from a .wire file into the station record
	int icount = 0;
	FILE* f = fopen(filename, "r");
	if (!f)
		return false;

	float height=0;
	int inumwires=0;
	if (!feof(f))
		fscanf(f, "%f", &height);
//	node->m_fTowerHeight = height;
	if (!feof(f))
		fscanf(f, "%d", &inumwires);
	st->m_iNumWires = inumwires;

	while (!feof(f))
	{	// read wire positions relative to the tower
		float a1,a2,a3;
		FPoint3 fpTemp;
		fscanf(f, "%f", &a1);
		fscanf(f, "%f", &a2);
		fscanf(f, "%f", &a3);
		fpTemp.Set(-a2, a3, a1);
		fpTemp *= METERS_PER_FOOT;
		st->m_fpWireAtt1[icount] = fpTemp;
		fscanf(f, "%f", &a1);
		fscanf(f, "%f", &a2);
		fscanf(f, "%f", &a3);
		fpTemp.Set(-a2, a3, a1);
		fpTemp *= METERS_PER_FOOT;
		st->m_fpWireAtt2[icount++] = fpTemp;
	}

	fclose(f);
	return true;
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

		ptCur.y = (dist / (2*catenary)) * (dist - (2*parabolicConst));

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


