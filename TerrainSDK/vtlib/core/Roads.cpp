//
// Roads.cpp
//
// also shorelines and rivers
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Light.h"
#include "vtdata/vtLog.h"
#include "Roads.h"

#define ROAD_HEIGHT			(vtRoadMap3d::s_fHeight)	// height about the ground
#define ROADSIDE_WIDTH		2.0f
#define ROADSIDE_DEPTH		-ROADSIDE_WIDTH

#define UV_SCALE_ROAD		(.08f)
#define UV_SCALE_SIDEWALK	(1.00f)

#define ROAD_AMBIENT 0.6	// brighter than terrain ambient
#define ROAD_DIFFUSE 0.4
#define TEXTURE_ARGS(alpha)		true, true, alpha, false, ROAD_AMBIENT, \
	ROAD_DIFFUSE, 1.0f, TERRAIN_EMISSIVE, false, false

#define ROADTEXTURE_4WD		"GeoTypical/road_4wd2.png"
#define ROADTEXTURE_TRAIL	"GeoTypical/trail2.png"
#define ROAD_FILENAME		"GeoTypical/roadset_2k.jpg"
#define ROAD_REZ 2048


////////////////////////////////////////////////////////////////////

// helper
FPoint3 find_adjacent_roadpoint(LinkGeom *pR, NodeGeom *pN)
{
	if (pR->GetNode(0) == pN)
		return pR->m_centerline[1];
	else if (pR->GetNode(1) == pN)
		return pR->m_centerline[pR->GetSize() - 2];
	else
	{
		// Adjacent road point not found!
		return FPoint3(-1, -1,-1);
	}
}

//
// return the positive difference of two angles (a - b)
// allow for wrapping around 2 PI
//
float angle_diff(float a, float b)
{
	if (a > b)
		return a-b;
	else
		return PI2f+a-b;
}

//
// helper: given two points along a road, produce a vector
// along to that road, parallel to the ground plane,
// with length corresponding to the supplied width
//
FPoint3 CreateRoadVector(FPoint3 p1, FPoint3 p2, float w)
{
	FPoint3 v = p2 - p1;
	v.y = 0;
	v.Normalize();
	v.x *= (w / 2.0f);
	v.z *= (w / 2.0f);
	return v;
}

FPoint3 CreateUnitRoadVector(FPoint3 p1, FPoint3 p2)
{
	FPoint3 v = p2 - p1;
	v.y = 0;
	v.Normalize();
	return v;
}


/////////////////////////////////////////////////////////////////////////

NodeGeom::NodeGeom()
{
	m_iVerts = 0;
	m_Lights = NULL;
}

NodeGeom::~NodeGeom()
{
}

FPoint3 NodeGeom::GetRoadVector(int i)
{
	LinkGeom *pR = GetRoad(i);
	FPoint3 pn1 = find_adjacent_roadpoint(pR, this);
	return CreateRoadVector(m_p3, pn1, pR->m_fWidth);
}

FPoint3 NodeGeom::GetUnitRoadVector(int i)
{
	FPoint3 pn1 = find_adjacent_roadpoint(GetRoad(i), this);
	return CreateUnitRoadVector(m_p3, pn1);
}


// statistics
int one = 0, two = 0, many = 0;

void NodeGeom::BuildIntersection()
{
	FPoint3 v, v_next, v_prev;
	FPoint3 pn0, pn1, upvector(0.0f, 1.0f, 0.0f);
	float w;				// road width

	SortLinksByAngle();

	// how many roads meet here?
	if (m_iLinks == 0)
	{
		; // bogus case (put a breakpoint here)
	}
	else if (m_iLinks == 1)
	{
		// dead end: only need 2 vertices for this node
		m_iVerts = 2;
		m_v.SetSize(2);

		// get info about the road
		LinkGeom *r = GetRoad(0);
		w = r->m_fWidth;

		pn1 = find_adjacent_roadpoint(r, this);
		v = CreateRoadVector(m_p3, pn1, w);

		m_v[0].Set(m_p3.x + v.z, m_p3.y + ROAD_HEIGHT, m_p3.z - v.x);
		m_v[1].Set(m_p3.x - v.z, m_p3.y + ROAD_HEIGHT, m_p3.z + v.x);

		one++;
	}
	else if (m_iLinks == 2)
	{
		// only need 2 vertices for this node; no intersection
		m_iVerts = 2;
		m_v.SetSize(2);

		// get info about the roads
		w = (GetRoad(0)->m_fWidth + GetRoad(1)->m_fWidth) / 2.0f;
		if (m_id == 8311) {
			; // (bogus case - put a breakpoint here to debug)
		}
		pn0 = find_adjacent_roadpoint(GetRoad(0), this);
		pn1 = find_adjacent_roadpoint(GetRoad(1), this);

		v = CreateRoadVector(pn0, pn1, w);

		m_v[0].Set(m_p3.x + v.z, m_p3.y + ROAD_HEIGHT, m_p3.z - v.x);
		m_v[1].Set(m_p3.x - v.z, m_p3.y + ROAD_HEIGHT, m_p3.z + v.x);

		two++;
	}
	else
	{
		// intersection: need 2 vertices for each road meeting here
		m_iVerts = 2 * m_iLinks;
		m_v.SetSize(m_iVerts);

		// for each road
		for (int i = 0; i < m_iLinks; i++)
		{
			// indices of the next and previous roads
			int i_next = (i == m_iLinks-1) ? 0 : i+1;
			int i_prev = (i == 0) ? m_iLinks-1 : i-1;

			// angle between this road and the next and previous roads
			float a_next = angle_diff(m_fLinkAngle[i_next], m_fLinkAngle[i]);
			float a_prev = angle_diff(m_fLinkAngle[i], m_fLinkAngle[i_prev]);

			// get info about the roads
			Link *pR = GetRoad(i);
			Link *pR_next = GetRoad(i_next);
			//Link *pR_prev = GetRoad(i_prev);
			w = pR->m_fWidth;
			float w_next = pR_next->m_fWidth;

			// find corner between this road and next
			v = GetUnitRoadVector(i);

			float w_avg;
			w_avg = (w + w_next) / 2.0f;
			float offset_next = w_avg / tanf(a_next / 2.0f);
			float offset_prev = w_avg / tanf(a_prev / 2.0f);
			float offset_largest = MAX(offset_next, offset_prev);
			offset_largest += 2.0f;

			// Safety check to avoid links with very close angles giving huge values
//			assert(offset_largest < 100);
			if (offset_largest > w_avg*4)
				offset_largest = w_avg*4;

			pn0 = m_p3;
			pn0 += (v * (offset_largest / 2.0f));
			v *= (w/2.0f);

			m_v[i * 2 + 0].Set(pn0.x + v.z, pn0.y + ROAD_HEIGHT, pn0.z - v.x);
			m_v[i * 2 + 1].Set(pn0.x - v.z, pn0.y + ROAD_HEIGHT, pn0.z + v.x);
		}
		many++;
	}
}


//
// Given a node and a road, return the two points that the road
// will need in order to hook up with the node.
//
void NodeGeom::FindVerticesForRoad(Link *pR, FPoint3 &p0, FPoint3 &p1)
{
	if (m_iLinks == 1)
	{
		p0 = m_v[0];
		p1 = m_v[1];
	}
	else if (m_iLinks == 2)
	{
		if (pR == m_r[0])
		{
			p0 = m_v[1];
			p1 = m_v[0];
		}
		else
		{
			p0 = m_v[0];
			p1 = m_v[1];
		}
	}
	else
	{
		int i;
		for (i = 0; i < m_iLinks; i++)
			if (m_r[i] == pR)
				break;
		if (i == m_iLinks)
		{
			// bad case!  This node does not reference the road passed
			; // (put a breakpoint here)
		}
		p0 = m_v[i*2];
		p1 = m_v[i*2+1];
	}
}


vtMesh *NodeGeom::GenerateGeometry()
{
	if (m_iLinks < 3)
		return NULL;

#if 1
	// Intersections currently look terrible, and are buggy.
	// Turn them off completely until we can implement decent ones.
	return NULL;
#else

	int j;
	FPoint3 p, upvector(0.0f, 1.0f, 0.0f);

	vtMesh *pMesh = new vtMesh(GL_TRIANGLES, VT_TexCoords | VT_Normals, (m_iLinks*2+1)*2);
	int verts = 0;

	// find the center of the junction
#if 0
	p.Set(0.0f, 0.0f, 0.0f);
	for (j = 0; j < m_iLinks*2; j++)
		p += m_v[j];
	p *= (1.0f / ((float)m_iLinks*2));
#else
	p = m_p3;
	p.y += ROAD_HEIGHT;
#endif

	for (int times = 0; times < 2; times++)
	{
		pMesh->SetVtxPUV(verts, p, 0.5, 0.5f);
		pMesh->SetVtxNormal(verts, upvector);
		verts++;

		for (j = 0; j < m_iLinks; j++)
		{
			if (times)
			{
				pMesh->SetVtxPUV(verts, m_v[j*2+1], 0.0, 0.0f);
				pMesh->SetVtxPUV(verts+1, m_v[j*2], 0.0, 1.0f);
			}
			else
			{
				pMesh->SetVtxPUV(verts, m_v[j*2+1], 0.0, 1.0f);
				pMesh->SetVtxPUV(verts+1, m_v[j*2], 1.0, 1.0f);
			}
			pMesh->SetVtxNormal(verts, upvector);
			pMesh->SetVtxNormal(verts+1, upvector);
			verts += 2;
		}
	}

	int iNVerts = m_iLinks*2+1;
	int iInCircle = m_iLinks*2;

	// create triangles
	int idx[3];
	for (j = 0; j < m_iLinks; j++)
	{
		idx[0] = 0;
		idx[1] = (j*2+1);
		idx[2] = (j*2+2);
		pMesh->AddTri(idx[0], idx[1], idx[2]);
	}
	for (j = 0; j < m_iLinks; j++)
	{
		idx[0] = iNVerts + 0;
		idx[1] = iNVerts + (j*2+1) + 1;
		idx[2] = iNVerts + ((j*2+2) % iInCircle) + 1;
		pMesh->AddTri(idx[0], idx[1], idx[2]);
	}
	return pMesh;
#endif
}


////////////////////////////////////////////////////////////////////////

#ifndef DOXYGEN_SHOULD_SKIP_THIS

RoadBuildInfo::RoadBuildInfo(int iCoords)
{
	left.SetSize(iCoords);
	right.SetSize(iCoords);
	center.SetSize(iCoords);
	crossvector.SetSize(iCoords);
	fvLength.SetSize(iCoords);
	verts = vert_index = 0;
}

#endif // DOXYGEN_SHOULD_SKIP_THIS

////////////////////////////////////////////////////////////////////////

LinkGeom::LinkGeom()
{
}

LinkGeom::~LinkGeom()
{
}

void LinkGeom::SetupBuildInfo(RoadBuildInfo &bi)
{
	FPoint3 v, pn0, pn1, pn2;
	float length = 0.0f;

	//  for each point in the road, determine coordinates
	for (unsigned int j = 0; j < GetSize(); j++)
	{
		if (j > 0)
		{
			// increment length
			v.x = m_centerline[j].x - m_centerline[j-1].x;
			v.z = m_centerline[j].z - m_centerline[j-1].z;
			length += v.Length();
		}
		bi.fvLength[j] = length;

		// we will add 2 vertices to the road mesh
		FPoint3 p0, p1;
		if (j == 0)
		{
			// add 2 vertices at this point, copied from the start node
			NodeGeom *pN = GetNode(0);
			pN->FindVerticesForRoad(this, bi.right[j], bi.left[j]);
		}
		if (j > 0 && j < GetSize()-1)
		{
			pn0 = m_centerline[j-1];
			pn1 = m_centerline[j];
			pn2 = m_centerline[j+1];

			// add 2 vertices at this point, directed at the previous and next points
			v = CreateRoadVector(pn0, pn2, m_fWidth);

			bi.right[j].Set(pn1.x + v.z, pn1.y + ROAD_HEIGHT, pn1.z - v.x);
			bi.left[j].Set(pn1.x - v.z, pn1.y + ROAD_HEIGHT, pn1.z + v.x);
		}
		if (j == GetSize()-1)
		{
			// add 2 vertices at this point, copied from the end node
			NodeGeom *pN = GetNode(1);
			pN->FindVerticesForRoad(this, bi.left[j], bi.right[j]);
		}
		bi.crossvector[j] = bi.right[j] - bi.left[j];
		bi.center[j] = bi.left[j] + (bi.crossvector[j] * 0.5f);
		bi.crossvector[j].Normalize();
	}
}

void LinkGeom::AddRoadStrip(vtMesh *pMesh, RoadBuildInfo &bi,
							float offset_left, float offset_right,
							float height_left, float height_right,
							VirtualTexture &vt,
							float u1, float u2, float uv_scale,
							normal_direction nd)
{
	FPoint3 local0, local1, normal;
	float texture_v;
	FPoint2 uv;

	for (unsigned int j = 0; j < GetSize(); j++)
	{
		texture_v = bi.fvLength[j] * uv_scale;

		local0 = bi.center[j] + (bi.crossvector[j] * offset_left);
		local1 = bi.center[j] + (bi.crossvector[j] * offset_right);
		local0.y += height_left;
		local1.y += height_right;

		if (nd == ND_UP)
			normal.Set(0.0f, 1.0f, 0.0f);	// up
		else if (nd == ND_LEFT)
			normal = (bi.crossvector[j] * -1.0f);	// left
		else
			normal = bi.crossvector[j];		// right

		vt.Adapt(FPoint2(u2, texture_v), uv);
		pMesh->AddVertexUV(local1, uv);

		vt.Adapt(FPoint2(u1, texture_v), uv);
		pMesh->AddVertexUV(local0, uv);

		pMesh->SetVtxNormal(bi.verts, normal);
		pMesh->SetVtxNormal(bi.verts+1, normal);
		bi.verts += 2;
	}
	// create tristrip
	pMesh->AddStrip2(GetSize() * 2, bi.vert_index);
	bi.vert_index += (GetSize() * 2);
}

void LinkGeom::GenerateGeometry(vtRoadMap3d *rmgeom)
{
	if (GetSize() < 2 || (m_pNode[0] == m_pNode[1]))
		return;

	bool do_roadside = true;
	switch (m_Surface)
	{
	case SURFT_NONE:
		break;
	case SURFT_GRAVEL:
	case SURFT_TRAIL:
	case SURFT_2TRACK:
	case SURFT_DIRT:
		do_roadside = false;
		break;
	case SURFT_PAVED:
	case SURFT_RAILROAD:
		break;
	}
	do_roadside = false;	// temporary override
	if (m_iHwy > 0)
		m_iFlags |= RF_MARGIN;

	// calculate total vertex count for this geometry
	int total_vertices = GetSize() * 2;	// main surface
	if (m_iFlags & RF_MARGIN)
		total_vertices += (GetSize() * 2 * 2);	// 2 margin strips
	if (m_iFlags & RF_PARKING)
		total_vertices += (GetSize() * 2 * 2);	// 2 parking strips
	if (m_iFlags & RF_SIDEWALK)
		total_vertices += (GetSize() * 2 * 4);	// 4 sidewalk strips
	if (do_roadside)
		total_vertices += (GetSize() * 2 * 2);		// 2 roadside strips

	vtMesh *pMesh = new vtMesh(vtMesh::TRIANGLE_STRIP, VT_TexCoords | VT_Normals,
		total_vertices);

	RoadBuildInfo bi(GetSize());
	SetupBuildInfo(bi);

	float offset = -m_fWidth/2;
	if (m_iFlags & RF_MARGIN)
		offset -= MARGIN_WIDTH;
	if (m_iFlags & RF_PARKING)
		offset -= PARKING_WIDTH;
	if (m_iFlags & RF_SIDEWALK)
		offset -= SIDEWALK_WIDTH;
	if (do_roadside)
		offset -= ROADSIDE_WIDTH;

#if 0
	// create left roadside strip
	if (do_roadside)
	{
		AddRoadStrip(pMesh, bi,
					offset, offset+ROADSIDE_WIDTH,
					ROADSIDE_DEPTH,
					(m_iFlags & RF_SIDEWALK) ? CURB_HEIGHT : 0.0f,
					rmgeom->m_vt[],
					0.02f, 0.98f, UV_SCALE_ROAD,
					ND_UP);
		offset += ROADSIDE_WIDTH;
	}
#endif

	// create left sidwalk
	if (m_iFlags & RF_SIDEWALK)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + SIDEWALK_WIDTH,
					CURB_HEIGHT, CURB_HEIGHT,
					rmgeom->m_vt[VTI_SIDEWALK],
					0.0f, 0.93f, UV_SCALE_SIDEWALK,
					ND_UP);
		offset += SIDEWALK_WIDTH;
		AddRoadStrip(pMesh, bi,
					offset,
					offset,
					CURB_HEIGHT, 0.0f,
					rmgeom->m_vt[VTI_SIDEWALK],
					0.93f, 1.0f, UV_SCALE_SIDEWALK,
					ND_RIGHT);
	}
	// create left parking lane
	if (m_iFlags & RF_PARKING)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + PARKING_WIDTH,
					0.0f, 0.0f,
					rmgeom->m_vt[VTI_1LANE],
					0.0f, 1.0f, UV_SCALE_ROAD,
					ND_UP);
		offset += PARKING_WIDTH;
	}
	// create left margin
	if (m_iFlags & RF_MARGIN)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + MARGIN_WIDTH,
					0.0f, 0.0f,
					rmgeom->m_vt[VTI_MARGIN],
					0.0f, 1.0f, UV_SCALE_ROAD,
					ND_UP);
		offset += MARGIN_WIDTH;
	}

	// create main road surface
	AddRoadStrip(pMesh, bi,
				-m_fWidth/2, m_fWidth/2,
				0.0f, 0.0f,
				rmgeom->m_vt[m_vti],
				0.0f, 1.0f, UV_SCALE_ROAD,
				ND_UP);
	offset = m_fWidth/2;

	// create right margin
	if (m_iFlags & RF_MARGIN)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + MARGIN_WIDTH,
					0.0f, 0.0f,
					rmgeom->m_vt[VTI_MARGIN],
					1.0f, 0.0f, UV_SCALE_ROAD,
					ND_UP);
		offset += MARGIN_WIDTH;
	}
	// create left parking lane
	if (m_iFlags & RF_PARKING)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset + PARKING_WIDTH,
					0.0f, 0.0f,
					rmgeom->m_vt[VTI_1LANE],
					0.0f, 1.0f, UV_SCALE_ROAD,
					ND_UP);
		offset += PARKING_WIDTH;
	}

	// create right sidwalk
	if (m_iFlags & RF_SIDEWALK)
	{
		AddRoadStrip(pMesh, bi,
					offset,
					offset,
					0.0f, CURB_HEIGHT,
					rmgeom->m_vt[VTI_SIDEWALK],
					1.0f, 0.93f, UV_SCALE_SIDEWALK,
					ND_LEFT);
		AddRoadStrip(pMesh, bi,
					offset,
					offset + SIDEWALK_WIDTH,
					CURB_HEIGHT, CURB_HEIGHT,
					rmgeom->m_vt[VTI_SIDEWALK],
					0.93f, 0.0f, UV_SCALE_SIDEWALK,
					ND_UP);
		offset += SIDEWALK_WIDTH;
	}

#if 0
	if (do_roadside)
	{
		// create left roadside strip
		AddRoadStrip(pMesh, bi,
					offset, offset+ROADSIDE_WIDTH,
					(m_iFlags & RF_SIDEWALK) ? CURB_HEIGHT : 0.0f,
					ROADSIDE_DEPTH,
					MATIDX_ROADSIDE,
					0.98f, 0.02f, UV_SCALE_ROAD,
					ND_UP);
	}
#endif

	// set lane coordinates
	m_Lanes.resize(m_iLanes);
	for (unsigned int i = 0; i < m_iLanes; i++)
	{
		m_Lanes.at(i).SetSize(GetSize());
	}
	for (unsigned int j = 0; j < GetSize(); j++)
	{
		for (int i = 0; i < m_iLanes; i++)
		{
			float offset = -((float)(m_iLanes-1) / 2.0f) + i;
			offset *= LANE_WIDTH;
			FPoint3 offset_diff = bi.crossvector[j] * offset;
			m_Lanes[i].SetAt(j, bi.center[j] + offset_diff);
		}
	}

	assert(total_vertices == bi.verts);
	rmgeom->AddMeshToGrid(pMesh, rmgeom->m_vt[m_vti].m_idx);
}


FPoint3 LinkGeom::FindPointAlongRoad(float fDistance)
{
	FPoint3 v;

	float length = 0.0f;

	if (fDistance <= 0) {
		static int c = 0;
		c++;
		return m_centerline[0];
	}
	// compute 2D length of this road, by adding up the 2d road segment lengths
	for (unsigned int j = 0; j < GetSize()-1; j++)
	{
		// consider length of next segment
		v.x = m_centerline[j+1].x - m_centerline[j].x;
		v.y = 0;
		v.z = m_centerline[j+1].z - m_centerline[j].z;
		length = v.Length();
		if (fDistance <= length)
		{
			float fraction = fDistance / length;
			FPoint3 p0, p1, diff;
			p0 = m_centerline[j];
			v *= fraction;
			return p0 + v;
		}
		fDistance -= length;
	}
	// if we pass the end of line, just return the last point
	return m_centerline[GetSize()-1];
}

//
// Return the 2D length of this road segment in world units
//
float LinkGeom::Length()
{
	FPoint3 v;
	v.y = 0;
	float length = 0.0f;

	// compute 2D length of this road, by adding up the 2d road segment lengths
	for (unsigned int j = 0; j < GetSize(); j++)
	{
		if (j > 0)
		{
			// increment length
			v.x = m_centerline[j].x - m_centerline[j-1].x;
			v.z = m_centerline[j].z - m_centerline[j-1].z;
			float l = v.Length();
			if (l < 0) {
				assert(false);
			}
			length += l;
		}
	}
	return length;
}

///////////////////////////////////////////////////////////////////

float vtRoadMap3d::s_fHeight = 1.0f;

vtRoadMap3d::vtRoadMap3d()
{
	m_pGroup = NULL;
	m_pMats = NULL;
}

vtRoadMap3d::~vtRoadMap3d()
{
	if (m_pMats)
		m_pMats->Release();
}

void vtRoadMap3d::BuildIntersections()
{
	int count = 0;
	for (NodeGeom *pN = GetFirstNode(); pN; pN = (NodeGeom *)pN->m_pNext)
	{
		pN->BuildIntersection();
		count++;
	}
}


int clusters_used = 0;	// for statistical purposes

void vtRoadMap3d::AddMeshToGrid(vtMesh *pMesh, int iMatIdx)
{
	// which cluster does it belong to?
	int a, b;

	FBox3 bound;
	pMesh->GetBoundBox(bound);
	FPoint3 center = bound.Center();

	a = (int)((center.x - m_extents.min.x) / m_extent_range.x * ROAD_CLUSTER);
	b = (int)((center.z - m_extents.min.z) / m_extent_range.z * ROAD_CLUSTER);

	// safety check: if the following is true, then the geometry
	// has somehow gotten mangled, so it's producing extents
	// outside of what they should be, go no further
	assert(a >= 0 && a < ROAD_CLUSTER && b >= 0 && b < ROAD_CLUSTER);

	vtGeom *pGeom;
	if (m_pRoads[a][b])
	{
		pGeom = (vtGeom *)m_pRoads[a][b]->GetChild(0);
	}
	else
	{
		float fDist[2];
		fDist[0] = 0.0f;
		fDist[1] = m_fLodDistance;

		m_pRoads[a][b] = new vtLOD();
		m_pRoads[a][b]->SetRanges(fDist, 2);
		m_pGroup->AddChild(m_pRoads[a][b]);

		FPoint3 lod_center;
		lod_center.x = m_extents.min.x + ((m_extent_range.x / ROAD_CLUSTER) * (a + 0.5f));
		lod_center.y = m_extents.min.y + (m_extent_range.y / 2.0f);
		lod_center.z = m_extents.min.z + ((m_extent_range.z / ROAD_CLUSTER) * (b + 0.5f));
		m_pRoads[a][b]->SetCenter(lod_center);

#if 0
		vtGeom *pSphere = CreateSphereGeom(m_pMats, m_mi_red, 1000.0f, 8);
		vtMovGeom *pSphere2 = new vtMovGeom(pSphere);
		m_pGroup->AddChild(pSphere2);
		pSphere2->SetTrans(lod_center);
#endif

		pGeom = new vtGeom();
		pGeom->SetName2("road");
		m_pRoads[a][b]->AddChild(pGeom);
		pGeom->SetMaterials(m_pMats);

		clusters_used++;
	}
	pGeom->AddMesh(pMesh, iMatIdx);
	pMesh->Release();	// pass ownership
}


vtGroup *vtRoadMap3d::GenerateGeometry(bool do_texture,
									   const vtStringArray &paths)
{
	VTLOG("   vtRoadMap3d::GenerateGeometry\n");
	VTLOG("   Nodes %d, Links %d\n", NumNodes(), NumLinks());
	m_pMats = new vtMaterialArray();

	// road textures
	if (do_texture)
	{
		vtString path;
		path = FindFileOnPaths(paths, "GeoTypical/roadside_32.png");
		m_mi_roadside = m_pMats->AddTextureMaterial2(path, TEXTURE_ARGS(true));
#if 0
		// 1
		m_pMats->AddTextureMaterial2("GeoTypical/margin_32.jpg",
			TEXTURE_ARGS(false));
		// 2
		m_pMats->AddTextureMaterial2("GeoTypical/sidewalk1_v2_512.jpg",
			TEXTURE_ARGS(false));
		// 3
		m_pMats->AddTextureMaterial2("GeoTypical/1lane_64.jpg",
			TEXTURE_ARGS(false));
		// 4
		m_pMats->AddTextureMaterial2("GeoTypical/2lane1way_128.jpg",
			TEXTURE_ARGS(false));
		// 5
		m_pMats->AddTextureMaterial2("GeoTypical/2lane2way_128.jpg",
			TEXTURE_ARGS(false));
		// 6
		m_pMats->AddTextureMaterial2("GeoTypical/3lane1way_256.jpg",
			TEXTURE_ARGS(false));
		// 7
		m_pMats->AddTextureMaterial2("GeoTypical/3lane2way_256.jpg",
			TEXTURE_ARGS(false));
		// 8
		m_pMats->AddTextureMaterial2("GeoTypical/4lane1way_256.jpg",
			TEXTURE_ARGS(false));
		// 9
		m_pMats->AddTextureMaterial2("GeoTypical/4lane2way_256.jpg",
			TEXTURE_ARGS(true));
		// 10
		m_pMats->AddTextureMaterial2("GeoTypical/water.jpg",
			TEXTURE_ARGS(false));
#endif
		path = FindFileOnPaths(paths, ROAD_FILENAME);
		m_mi_roads = m_pMats->AddTextureMaterial2(path, TEXTURE_ARGS(false));

		path = FindFileOnPaths(paths, ROADTEXTURE_4WD);
		m_mi_4wd = m_pMats->AddTextureMaterial2(path, TEXTURE_ARGS(true));

		path = FindFileOnPaths(paths, ROADTEXTURE_TRAIL);
		m_mi_trail = m_pMats->AddTextureMaterial2(path, TEXTURE_ARGS(true));

		m_vt[VTI_MARGIN].m_idx = m_mi_roads;
		m_vt[VTI_MARGIN].m_rect.SetRect(960.0f/ROAD_REZ, 1, 992.0f/ROAD_REZ, 0);

		m_vt[VTI_SIDEWALK].m_idx = m_mi_roads;
		m_vt[VTI_SIDEWALK].m_rect.SetRect(512.0f/ROAD_REZ, 1, 640.0f/ROAD_REZ, 0);

		m_vt[VTI_1LANE].m_idx = m_mi_roads;
		m_vt[VTI_1LANE].m_rect.SetRect(451.0f/ROAD_REZ, 1, 511.0f/ROAD_REZ, 0);

		m_vt[VTI_2LANE1WAY].m_idx = m_mi_roads;
		m_vt[VTI_2LANE1WAY].m_rect.SetRect(4.0f/ROAD_REZ, 1, 124.0f/ROAD_REZ, 0);

		m_vt[VTI_2LANE2WAY].m_idx = m_mi_roads;
		m_vt[VTI_2LANE2WAY].m_rect.SetRect(640.0f/ROAD_REZ, 1, 768.0f/ROAD_REZ, 0);

		m_vt[VTI_3LANE1WAY].m_idx = m_mi_roads;
		m_vt[VTI_3LANE1WAY].m_rect.SetRect(2.0f/ROAD_REZ, 1, 190.0f/ROAD_REZ, 0);

		m_vt[VTI_3LANE2WAY].m_idx = m_mi_roads;
		m_vt[VTI_3LANE2WAY].m_rect.SetRect(768.0f/ROAD_REZ, 1, .0f/ROAD_REZ, 0);

		m_vt[VTI_4LANE1WAY].m_idx = m_mi_roads;
		m_vt[VTI_4LANE1WAY].m_rect.SetRect(0.0f/ROAD_REZ, 1, 256.0f/ROAD_REZ, 0);

		m_vt[VTI_4LANE2WAY].m_idx = m_mi_roads;
		m_vt[VTI_4LANE2WAY].m_rect.SetRect(256.0f/ROAD_REZ, 1, 512.0f/ROAD_REZ, 0);

		m_vt[VTI_RAIL].m_idx = m_mi_roads;
		m_vt[VTI_RAIL].m_rect.SetRect(992.0f/ROAD_REZ, 1, 1248.0f/ROAD_REZ, 0);

		m_vt[VTI_STONE].m_idx = m_mi_roads;
		m_vt[VTI_STONE].m_rect.SetRect(1248.0f/ROAD_REZ, 1, 1440.0f/ROAD_REZ, 0);

		m_vt[VTI_4WD].m_idx = m_mi_4wd;
		m_vt[VTI_4WD].m_rect.SetRect(0, 0, 1, 1);

		m_vt[VTI_TRAIL].m_idx = m_mi_trail;
		m_vt[VTI_TRAIL].m_rect.SetRect(0, 0, 1, 1);
	}
	else
	{
		m_mi_roadside = m_pMats->AddRGBMaterial1(RGBf(0.8f, 0.6f, 0.4f), true, false);	// 0 brown roadside
		m_mi_roads = m_pMats->AddRGBMaterial1(RGBf(0.0f, 1.0f, 0.0f), true, false);	// 1 green
		m_mi_4wd = m_pMats->AddRGBMaterial1(RGBf(0.5f, 0.5f, 0.5f), true, false);	// 2 grey
		m_mi_trail = m_pMats->AddRGBMaterial1(RGBf(1.0f, 0.3f, 1.0f), true, false);	// 3 light purple
	}
	m_mi_red = m_pMats->AddRGBMaterial(RGBf(1.0f, 0.0f, 0.0f), RGBf(0.2f, 0.0f, 0.0f),
		true, true, false, 0.4f);	// red-translucent

	m_pGroup = new vtGroup();
	m_pGroup->SetName2("Roads");

	// wrap with an array of simple LOD nodes
	int a, b;
	for (a = 0; a < ROAD_CLUSTER; a++)
		for (b = 0; b < ROAD_CLUSTER; b++)
		{
			m_pRoads[a][b] = NULL;
		}

	_GatherExtents();

#if 0
	vtGeom *pGeom = CreateLineGridGeom(m_pMats, 0,
						   m_extents.min, m_extents.max, ROAD_CLUSTER);
	m_pGroup->AddChild(pGeom);
#endif

	vtMesh *pMesh;
	int count = 0;
	for (LinkGeom *pR = GetFirstLink(); pR; pR=(LinkGeom *)pR->m_pNext)
	{
		pR->GenerateGeometry(this);
//		if (pMesh) AddMeshToGrid(pMesh, 0);	// TODO: correct matidx
		count++;
	}
	count = 0;
	for (NodeGeom *pN = GetFirstNode(); pN; pN = (NodeGeom *)pN->m_pNext)
	{
		pMesh = pN->GenerateGeometry();
		if (pMesh)
			AddMeshToGrid(pMesh, 0);	// TODO: correct matidx
		count++;
	}

	// return top road group, ready to be added to scene graph
	return m_pGroup;
}

//
// stoplights and stopsigns
//
void vtRoadMap3d::GenerateSigns(vtLodGrid *pLodGrid)
{
	if (!pLodGrid)
		return;

#if 0
	vtString path;
	path = FindFileOnPaths(vtGetDataPath(), "Culture/stopsign4.dsm");
	vtNode *stopsign = vtLoadModel(path);
	path = FindFileOnPaths(vtGetDataPath(), "Culture/stoplight8rt.dsm");
	vtNode *stoplight = vtLoadModel(path);

	if (stopsign && stoplight)
	{
		float sc = 0.01f;	// cm
		stopsign->Scale2(sc, sc, sc);
		stoplight->Scale2(sc, sc, sc);
	}
	for (NodeGeom *pN = GetFirstNode(); pN; pN = (NodeGeom *)pN->m_pNext)
	{
		for (int r = 0; r < pN->m_iLinks; r++)
		{
			vtGeom *shape = NULL;
			if (pN->GetIntersectType(r) == IT_STOPSIGN && stopsign)
			{
				shape = (vtGeom *)stopsign->Clone();
			}
			if (pN->GetIntersectType(r) == IT_LIGHT && stoplight)
			{
				shape = (vtGeom *)stoplight->Clone();
			}
			if (!shape) continue;

			Road *road = pN->GetRoad(r);
			FPoint3 unit = pN->GetUnitRoadVector(r);
			FPoint3 perp(unit.z, unit.y, -unit.x);
			FPoint3 offset;

			shape->RotateLocal(FPoint3(0,1,0), pN->m_fRoadAngle[r] + PID2f);

			if (pN->GetIntersectType(r) == IT_STOPSIGN)
			{
				offset = pN->m_p3 + (unit * 6.0f) + (perp * (road->m_fWidth/2.0f));
			}
			if (pN->GetIntersectType(r) == IT_LIGHT)
			{
				offset = pN->m_p3 - (unit * 6.0f) + (perp * (road->m_fWidth/2.0f));
			}
			shape->Translate2(FPoint3(offset.x, offset.y + s_fHeight, offset.z));
			pLodGrid->AppendToGrid(shape);
		}
	}
#endif
}


void vtRoadMap3d::_GatherExtents()
{
	// Find extents of area covered by roads
	m_extents.InsideOut();

	// Examine the range of the roadmap area
	for (LinkGeom *pL = GetFirstLink(); pL; pL = pL->GetNext())
		m_extents.GrowToContainLine(pL->m_centerline);

	// Expand slightly for safety - in case we allow dragging road nodes
	//  interactively in the future.
	FPoint3 diff = m_extents.max - m_extents.min;
	m_extents.min -= (diff / 20.0f);
	m_extents.max += (diff / 20.0f);
	m_extent_range = m_extents.max - m_extents.min;
}


void vtRoadMap3d::DetermineSurfaceAppearance()
{
	// Pre-process some road attributes
	for (LinkGeom *pR = GetFirstLink(); pR; pR = pR->GetNext())
	{
		// set material index based on surface type, number of lanes, and direction
		bool two_way = (pR->m_iFlags & RF_FORWARD) &&
					   (pR->m_iFlags & RF_REVERSE);
		switch (pR->m_Surface)
		{
		case SURFT_NONE:
//			pR->m_vti = 3;
			pR->m_vti = 0;
			break;
		case SURFT_GRAVEL:
//			pR->m_vti = MATIDX_GRAVEL;
			pR->m_vti = 0;
			break;
		case SURFT_TRAIL:
			pR->m_vti = VTI_TRAIL;
			break;
		case SURFT_2TRACK:
		case SURFT_DIRT:
			pR->m_vti = VTI_4WD;
			break;
		case SURFT_PAVED:
			switch (pR->m_iLanes)
			{
			case 1:
				pR->m_vti = VTI_1LANE;
				break;
			case 2:
				pR->m_vti = two_way ? VTI_2LANE2WAY : VTI_2LANE1WAY;
				break;
			case 3:
				pR->m_vti = two_way ? VTI_3LANE2WAY : VTI_3LANE1WAY;
				break;
			case 4:
				pR->m_vti = two_way ? VTI_4LANE2WAY : VTI_4LANE1WAY;
				break;
			}
			break;
		case SURFT_RAILROAD:
			pR->m_vti = VTI_RAIL;
			break;
		case SURFT_STONE:
			pR->m_vti = VTI_STONE;
			break;
		}
	}
}

void vtRoadMap3d::SetLodDistance(float fDistance)
{
	m_fLodDistance = fDistance;

	if (m_pGroup)
	{
		float fDist[2];
		fDist[0] = 0.0f;
		fDist[1] = m_fLodDistance;

		int a, b;
		for (a = 0; a < ROAD_CLUSTER; a++)
			for (b = 0; b < ROAD_CLUSTER; b++)
			{
				if (m_pRoads[a][b])
					m_pRoads[a][b]->SetRanges(fDist, 2);
			}
	}
}

float vtRoadMap3d::GetLodDistance()
{
	return m_fLodDistance;
}

void vtRoadMap3d::DrapeOnTerrain(vtHeightField3d *pHeightField)
{
	FPoint3 p;
	NodeGeom *pN;

#if 0
	// This code attempts to identify cases where a node actually
	// represents something like an overpass: two roads that don't
	// actually connect.  However, it's better to take care of this
	// as a preprocess, rather than at runtime.
	float height;
	for (pN = GetFirstNode(); pN; pN = (NodeGeom *)pN->m_pNext)
	{
		bool all_same_height = true;
		height = pN->GetRoad(0)->GetHeightAt(pN);
		for (int r = 1; r < pN->m_iLinks; r++)
		{
			if (pN->GetRoad(r)->GetHeightAt(pN) != height)
			{
				all_same_height = false;
				break;
			}
		}
		if (!all_same_height)
		{
			pNew = new NodeGeom();
			for (r = 1; r < pN->m_iLinks; r++)
			{
				LinkGeom *pR = pN->GetRoad(r);
				if (pR->GetHeightAt(pN) != height)
				{
					pN->DetachRoad(pR);
					pNew->AddRoad(pR);
				}
			}
		}
	}
#endif
	for (pN = GetFirstNode(); pN; pN = (NodeGeom *)pN->m_pNext)
	{
		pHeightField->ConvertEarthToSurfacePoint(pN->m_p, pN->m_p3);
#if 0
		if (pN->m_iLinks > 0)
		{
			height = pN->GetRoad(0)->GetHeightAt(pN);
			pN->m_p3.y += height;
		}
#endif
	}
	for (LinkGeom *pR = GetFirstLink(); pR; pR = (LinkGeom *)pR->m_pNext)
	{
		pR->m_centerline.SetSize(pR->GetSize());
		for (unsigned int j = 0; j < pR->GetSize(); j++)
		{
			pHeightField->ConvertEarthToSurfacePoint(pR->GetAt(j), p);
			pR->m_centerline[j] = p;
		}
		// ignore width from file - imply from properties
		pR->m_fWidth = pR->m_iLanes * LANE_WIDTH;
		if (pR->m_fWidth == 0)
			pR->m_fWidth = 10.0f;
	}
}

