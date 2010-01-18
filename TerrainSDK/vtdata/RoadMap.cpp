//
// RoadMap.cpp
//
// Copyright (c) 2001-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "RoadMap.h"
#include "vtLog.h"
#include "FilePath.h"

#define intSize 4
#define floatSize 4
#define doubleSize 8


//diff a - b.  result between PI and -PI.
float diffAngle(float a, float b)
{
	float result = a - b;
	while (result > PIf) {
		result -= PI2f;
	}
	while (result < -PIf) {
		result += PI2f;
	}
	return result;
}

//
// Nodes
//
TNode::TNode()
{
	m_pNext = NULL;
	m_iLinks = 0;
	m_id = -1;
}

TNode::~TNode()
{
}

bool TNode::operator==(TNode &ref)
{
	if (m_iLinks != ref.m_iLinks)
		return false;
	for (int i = 0; i < m_iLinks; i++)
	{
		if (m_connect[i].eIntersection != ref.m_connect[i].eIntersection)
			return false;
		if (m_connect[i].eLight != ref.m_connect[i].eLight)
			return false;
	}
	return true;
}

void TNode::Copy(TNode *node)
{
	m_p = node->m_p;
	m_iLinks = node->m_iLinks;
	m_id = node->m_id;
	m_connect = node->m_connect;
	m_pNext = NULL;	//don't copy this
}

TLink *TNode::GetLink(int n)
{
	if (n >= 0 && n < m_iLinks)	// safety check
		return m_connect[n].pLink;
	else
		return NULL;
}

int TNode::FindLink(int linkID)
{
	for (int i = 0; i < m_iLinks; i++)
	{
		if (m_connect[i].pLink->m_id == linkID)
			return i;
	}
	return -1;
}

int TNode::AddLink(TLink *pL, bool bStart)
{
	m_iLinks++;

	// fill in the entry for the new link
	LinkConnect lc;
	lc.bStart = bStart;
	lc.pLink = pL;
	lc.eIntersection = IT_NONE;
	lc.eLight = LT_INVALID;
	m_connect.push_back(lc);

	return m_connect.size() - 1;
}

void TNode::DetachLink(TLink *pL, bool bStart)
{
	for (int i = 0; i < m_iLinks; i++)
	{
		if (m_connect[i].pLink == pL && m_connect[i].bStart == bStart)
		{
			// found it
			m_connect.erase(m_connect.begin() + i);
			m_iLinks = m_connect.size();
			return;
		}
	}
}

//angles all > 0.
void TNode::DetermineLinkAngles()
{
	DPoint2 pn0, pn1, diff;
	for (int i = 0; i < m_iLinks; i++)
	{
		pn0 = m_p;
		pn1 = GetAdjacentLinkPoint2d(i);
		diff = pn1 - pn0;

		float angle = atan2f((float)diff.y, (float)diff.x);
		if (angle < 0.0f)
			angle += PI2f;
		m_connect[i].fLinkAngle = angle;
	}
}

float TNode::GetLinkAngle(int iLinkNum)
{
	return m_connect[iLinkNum].fLinkAngle;
}

void TNode::SortLinksByAngle()
{
	// first determine the angle of each link
	DetermineLinkAngles();

	// sort links by radial angle (make them counter-clockwise)
	// use a bubble sort
	bool sorted = false;
	while (!sorted)
	{
		sorted = true;
		for (int i = 0; i < m_iLinks-1; i++)
		{
			if (m_connect[i].fLinkAngle > m_connect[i+1].fLinkAngle)
			{
				// swap entries in connection array
				LinkConnect tmp1 = m_connect[i];
				m_connect[i] = m_connect[i+1];
				m_connect[i+1] = tmp1;

				sorted = false;
			}
		}
	}
}

DPoint2 TNode::GetAdjacentLinkPoint2d(int iLinkNum)
{
	LinkConnect &lc = m_connect[iLinkNum];
	if (lc.bStart)
		return lc.pLink->GetAt(1);			// link starts here
	else
		return lc.pLink->GetAt(lc.pLink->GetSize() - 2);	// link ends here
}

int TNode::GetLinkNum(TLink *link, bool bStart)
{
	for (int i = 0; i < m_iLinks; i++)
	{
		if (m_connect[i].pLink == link && m_connect[i].bStart == bStart)
			return i;
	}
	return -1;
}

//traffic control
bool TNode::SetIntersectType(int linkNum, IntersectionType type)
{
	if (linkNum < 0 || linkNum >= m_iLinks)
		return false;

	m_connect[linkNum].eIntersection = type;
	return true;
}

IntersectionType TNode::GetIntersectType(int linkNum)
{
	if (linkNum < 0 || linkNum >= m_iLinks)
		return IT_NONE;

	return m_connect[linkNum].eIntersection;
}

LightStatus TNode::GetLightStatus(int linkNum)
{
	if (linkNum >= m_iLinks || linkNum < 0)
		return LT_INVALID;

	return m_connect[linkNum].eLight;
}

bool TNode::SetLightStatus(int linkNum, LightStatus light)
{
	if (linkNum >= m_iLinks || linkNum < 0) {
		return false;
	}
	m_connect[linkNum].eLight = light;
	return true;
}

bool TNode::HasLights()
{
	for (int i = 0; i < m_iLinks; i++)
	{
		if (GetIntersectType(i) == IT_LIGHT)
			return true;
	}
	return false;
}

bool TNode::IsControlled()
{
	for (int i = 0; i < m_iLinks; i++)
	{
		if (GetIntersectType(i) != IT_NONE)
			return true;
	}
	return false;
}

void TNode::AdjustForLights()
{
	if (!HasLights())
		return;

	int i;
	for (i = 0; i< m_iLinks; i++) {
		SetLightStatus(i, LT_GREEN);
		SetIntersectType(i, IT_LIGHT);
	}
//if the intersection has signal lights, determine light relationships.
	switch (m_iLinks) {
	case 0:
	case 1:
	case 2:
		//leave colors alone
		break;
	case 3:
	case 4:
		SortLinksByAngle();
		float bestAngle = PI2f;
		int bestChoiceA = 0;
		int bestChoiceB = 0;
		float newAngle;

		//go through all link pairs and see what is the difference of their angles.
		//we're shooting for difference of PI.
		for (i = 0; i < m_iLinks - 1; i++)
		{
			//since angles are sorted, angle i < angle j (not sure if that helps.)
			for (int j = i+1; j < m_iLinks; j++)
			{
				if (i != j)
				{
					newAngle  = m_connect[j].fLinkAngle - (m_connect[i].fLinkAngle + PIf);
					//get absolute value
					if (newAngle < 0) {
						newAngle = -newAngle;
					}
					//get smallest angle
					if (newAngle > PIf) {
						newAngle = PI2f - newAngle;
					}
					//get absolute value
					if (newAngle < 0) {
						newAngle = -newAngle;
					}
					printf("%i:%f, %i:%f, %f, %f",
						i, m_connect[i].fLinkAngle,
						j, m_connect[j].fLinkAngle
						, newAngle, bestAngle);
					if (newAngle < bestAngle) {
						bestChoiceA = i;
						bestChoiceB = j;
						bestAngle = newAngle;
					}
				}
			}
		}

		SetLightStatus(bestChoiceA, LT_RED);
		SetLightStatus(bestChoiceB, LT_RED);
		break;
	}
}


//
// Links
//
TLink::TLink()
{
	// Provide default values
	m_fWidth = 1.0f;
	m_iLanes = 0;
	m_Surface = SURFT_PAVED;
	m_iHwy = -1;
	m_pNext = NULL;
	m_id = 0;
	m_iFlags = (RF_FORWARD|RF_REVERSE);	// by default, links are bidirectional
	m_fHeight[0] = 0;
	m_fHeight[1] = 0;
	m_pNode[0] = NULL;
	m_pNode[1] = NULL;
	m_fSidewalkWidth = SIDEWALK_WIDTH;
	m_fCurbHeight = CURB_HEIGHT;
	m_fMarginWidth = MARGIN_WIDTH;
	m_fLaneWidth = LANE_WIDTH;
	m_fParkingWidth = PARKING_WIDTH;
}

//
// Copy constructor
//
TLink::TLink(TLink &ref)
{
	*this = ref;
//	DLine2::DLine2(ref);	// need to do this?
}

TLink::~TLink()
{
}

bool TLink::operator==(TLink &ref)
{
	return (m_fWidth == ref.m_fWidth &&
		m_iLanes == ref.m_iLanes &&
		m_Surface == ref.m_Surface &&
		m_iHwy == ref.m_iHwy &&
		m_iFlags == ref.m_iFlags);
}

void TLink::SetFlag(int flag, bool value)
{
	if (value)
		m_iFlags |= flag;
	else
		m_iFlags &= ~flag;
}

int TLink::GetFlag(int flag)
{
	return (m_iFlags & flag) != 0;
}

/**
 * Find closest lateral distance from a given point to the link.
 */
double TLink::DistanceToPoint(const DPoint2 &point, bool bAllowEnds)
{
	double a, b;
	DPoint2 dummy1;
	int dummy2;
	float dummy3;
	GetLinearCoordinates(point, a, b, dummy1, dummy2, dummy3, bAllowEnds);
	return fabs(b);
}

/**
 * Produces the "linear coordinates" a and b, where a is the distance
 * along the link, and b is the signed lateral distance orthogonal to
 * the link at that point.
 *
 * \param p The input point.
 * \param result_a	The resulting distance along the link.
 * \param result_b  The signed lateral (or absolute) distance to the link.
 * \param closest	The closest point on the link.
 * \param linkpoint	Index into the links points just before the closest point.
 * \param fractional Fractional distance between this link point and the next.
 * \param bAllowEnds	If true, then for cases where the the closest
 *		point is either end of the link, the distance to that point
 *		is returned.  Otherwise, only laterial distances are returned.
 */
double TLink::GetLinearCoordinates(const DPoint2 &p, double &result_a,
								  double &result_b, DPoint2 &closest,
								  int &linkpoint, float &fractional,
								  bool bAllowEnds)
{
	double u, b, length, traversed = 0, min_dist = 1E10;
	DPoint2 p1, p2, diff, vec_a, vec_b, delta;

	int points = GetSize();
	for (int i=0; i < points-1; i++)
	{
		p1 = GetAt(i);
		p2 = GetAt(i+1);

		// check distance to line segment
		diff = p - p1;
		vec_a = p2 - p1;
		vec_b.Set(vec_a.y, -vec_a.x);

		length = vec_a.Length();
		vec_a /= length;
		vec_b.Normalize();
		u = diff.Dot(vec_a);

		// if u >1 or u <0, then point is not closest to line within the
		// given line segment.  use the end points.
		if ((bAllowEnds || (i > 0)) && u < 0)
//		if (u < 0)
		{
			u = 0;
			b = (p - p1).Length();
		}
		else if ((bAllowEnds || (i < points-2)) && u > length)
//		else if (u > length)
		{
			u = length;
			b = (p - p2).Length();
		}
		else
		{
			b = diff.Dot(vec_b);
		}

		if (fabs(b) < min_dist)
		{
			min_dist = fabs(b);
			result_a = traversed + u;
			result_b = b;
			closest = p1 + (vec_a * u);
			linkpoint = i;
			fractional = (float) (u/length);
		}
		traversed += length;
	}

	// 'traversed' now contains the total length of the link
	return traversed;
}

float TLink::Length()
{
	double dist = 0;
	DPoint2 tmp;
	for (unsigned int i = 1; i < GetSize(); i++)
	{
		tmp = GetAt(i-1) - GetAt(i);
		dist += tmp.Length();
	}
	return (float) dist;
}

float TLink::EstimateWidth(bool bIncludeSidewalk)
{
	float width = m_iLanes * m_fLaneWidth;
	if (GetFlag(RF_PARKING))
		width += (m_fParkingWidth * 2);
	if (GetFlag(RF_MARGIN))
		width += (m_fMarginWidth * 2);
	if (bIncludeSidewalk && GetFlag(RF_SIDEWALK))
		width += (m_fSidewalkWidth * 2);
	return width;
}


//
// RoadMap class
//

vtRoadMap::vtRoadMap()
{
	// provide inital values for extent
	m_bValidExtents = false;

	m_pFirstLink = NULL;
	m_pFirstNode = NULL;
}


vtRoadMap::~vtRoadMap()
{
	DeleteElements();
}

void vtRoadMap::DeleteElements()
{
	TLink *nextR;
	while (m_pFirstLink)
	{
		nextR = m_pFirstLink->m_pNext;
		delete m_pFirstLink;
		m_pFirstLink = nextR;
	}

	TNode *nextN;
	while (m_pFirstNode)
	{
		nextN = m_pFirstNode->m_pNext;
		delete m_pFirstNode;
		m_pFirstNode = nextN;
	}
}

TNode *vtRoadMap::FindNodeByID(int id)
{
	for (TNode *pN = m_pFirstNode; pN; pN = pN->m_pNext)
	{
		if (pN->m_id == id)
			return pN;
	}
	return NULL;
}

/**
 * Find the node closest to the indicated point.  Ignore nodes more than
 * epsilon units away from the point.
 */
TNode *vtRoadMap::FindNodeAtPoint(const DPoint2 &point, double epsilon)
{
	TNode *closest = NULL;
	double result, dist = 1E9;

	// a target rectangle, to quickly cull points too far away
	DRECT target(point.x-epsilon, point.y+epsilon, point.x+epsilon, point.y-epsilon);
	for (TNode *curNode = GetFirstNode(); curNode; curNode = curNode->m_pNext)
	{
		if (!target.ContainsPoint(curNode->m_p))
			continue;
		result = (curNode->m_p - point).Length();
		if (result < dist)
		{
			closest = curNode;
			dist = result;
		}
	}
	return closest;
}

int	vtRoadMap::NumLinks() const
{
	int count = 0;
	for (TLink *pL = m_pFirstLink; pL; pL = pL->m_pNext)
		count++;
	return count;
}

int	vtRoadMap::NumNodes() const
{
	int count = 0;
	for (TNode *pN = m_pFirstNode; pN; pN = pN->m_pNext)
		count++;
	return count;
}

DRECT vtRoadMap::GetMapExtent()
{
	if (!m_bValidExtents)
		ComputeExtents();

	if (m_bValidExtents)
		return m_extents;
	else
		return DRECT(0,0,0,0);
}

void vtRoadMap::ComputeExtents()
{
	if (NumLinks() == 0)
	{
		m_bValidExtents = false;
		return;
	}

	// iterate through all elements accumulating extents
	m_extents.SetRect(1E9, -1E9, -1E9, 1E9);
	for (TLink *pL = m_pFirstLink; pL; pL = pL->m_pNext)
	{
		// links are a subclass of line, so we can treat them as lines
		DLine2 *dl = pL;
		m_extents.GrowToContainLine(*dl);
	}
	m_bValidExtents = true;
}

//
// remove unused nodes, return the number removed
//
int vtRoadMap::RemoveUnusedNodes()
{
	VTLOG("   vtRoadMap::RemoveUnusedNodes: ");

	TNode *prev = NULL, *next;
	TNode *pN = m_pFirstNode;
	int total = 0, unused = 0;

	while (pN)
	{
		total++;
		next = pN->m_pNext;
		if (pN->m_iLinks == 0)
		{
			// delete it
			if (prev)
				prev->m_pNext = next;
			else
				m_pFirstNode = next;
			delete pN;
			unused++;
		}
		else
			prev = pN;
		pN = next;
	}
	VTLOG("   %d of %d removed\n", unused, total);
	return unused;
}

//
// Remove a node - use with caution
//
void vtRoadMap::RemoveNode(TNode *pNode)
{
	TNode *prev = NULL, *next;
	TNode *pN = m_pFirstNode;

	while (pN)
	{
		next = pN->m_pNext;
		if (pNode == pN)
		{
			// delete it
			if (prev)
				prev->m_pNext = next;
			else
				m_pFirstNode = next;
			delete pN;
			// I assume that a node cannot be on the list more than once!!!
			break;
		}
		else
			prev = pN;
		pN = next;
	}
}

//
// Remove a link - use with caution
//
void vtRoadMap::RemoveLink(TLink *pLink)
{
	TLink *prev = NULL, *next;
	TLink *pL = m_pFirstLink;

	while (pL)
	{
		next = pL->m_pNext;
		if (pLink == pL)
		{
			// delete it
			if (prev)
				prev->m_pNext = next;
			else
				m_pFirstLink = next;
			delete pL;
			// I assume that a node cannot be on the list more than once!!!
			break;
		}
		else
			prev = pL;
		pL = next;
	}
}

#define FRead(a,b) fread(a,b,1,fp)
#define FWrite(a,b) fwrite(a,b,1,fp)

//
// Read an RMF (Road Map File)
// Returns true if operation sucessful.
//
bool vtRoadMap::ReadRMF(const char *filename,
						bool bHwy, bool bPaved, bool bDirt)
{
	// Avoid trouble with '.' and ',' in Europe
	LocaleWrap normal_numbers(LC_NUMERIC, "C");

	char buffer[12];
	FILE *fp = vtFileOpen(filename, "rb");
	if (!fp)
	{
		// "Error opening file: %s",filename
		return false;
	}

	int numNodes, numLinks, i, j, nodeNum, dummy, quiet;
	TNode *tmpNode;
	TLink *tmpLink;

	// Is it a RMF File? and check version number
	if (fread(buffer,11,1,fp) != 1)
		return false;
	buffer[11] = 0;

	if (strncmp(buffer, RMFVERSION_STRING, 7))
	{
		// Not an RMF file!
		// "Sorry, that file does not appear to be a valid RMF file."
		fclose(fp);
		return false;
	}
	double version = atof(buffer+7);

	if (version < RMFVERSION_SUPPORTED)
	{
		// not recent version
		// Format("Sorry, that file appears to be a version %1.1f RMF file",
		// This program can only read version %1.1f or newer.", version, RMFVERSION_SUPPORTED);
		fclose(fp);
		return false;
	}

	// Erasing existing network
	DeleteElements();

	// Projection
	if (version < 1.9)
	{
		int proj_type=1, iUTMZone;
		int iDatum = EPSG_DATUM_WGS84;
		if (version >= 1.8f)
		{
			int iUTM;
			quiet = fread(&iUTM, intSize, 1, fp);
			proj_type = (iUTM != 0);
		}
		quiet = fread(&iUTMZone, intSize, 1, fp);
		if (version >= 1.8f)
		{
			quiet = fread(&iDatum, intSize, 1, fp);
		}
		m_proj.SetProjectionSimple(proj_type == 1, iUTMZone, iDatum);
	}
	else
	{
		quiet = fread(&dummy, 4, 1, fp);
		short length = (short) dummy;
		char wkt_buf[2000], *wkt = wkt_buf;
		quiet = fread(wkt_buf, length, 1, fp);
		OGRErr err = m_proj.importFromWkt((char **) &wkt);
		if (err != OGRERR_NONE)
			return false;
	}

	// Extents
	if (version < 1.9)
	{
		int le, ri, to, bo;
		quiet = fread(&ri, intSize, 1, fp);
		quiet = fread(&to, intSize, 1, fp);
		quiet = fread(&le, intSize, 1, fp);
		quiet = fread(&bo, intSize, 1, fp);
		m_extents.left = le;
		m_extents.right = ri;
		m_extents.top = to;
		m_extents.bottom = bo;
	}
	else
	{
		quiet = fread(&m_extents.left, doubleSize, 1, fp);
		quiet = fread(&m_extents.right, doubleSize, 1, fp);
		quiet = fread(&m_extents.bottom, doubleSize, 1, fp);
		quiet = fread(&m_extents.top, doubleSize, 1, fp);
	}
	m_bValidExtents = true;

	//get number of nodes and links
	quiet = fread(&numNodes, intSize, 1, fp);
	quiet = fread(&numLinks, intSize, 1, fp);

	quiet = fread(buffer,7,1,fp);
	if (strcmp(buffer, "Nodes:"))
	{
		fclose(fp);
		return false;
	}

	// Use a temporary array for fast lookup
	TNodePtr *pNodeLookup = new TNodePtr[numNodes+1];

	// Read the nodes
	int ivalue;
	for (i = 1; i <= numNodes; i++)
	{
		tmpNode = NewNode();
		quiet = fread(&(tmpNode->m_id), intSize, 1, fp);
		if (version < 1.8f)
		{
			quiet = fread(&ivalue, intSize, 1, fp);
			tmpNode->m_p.x = ivalue;
			quiet = fread(&ivalue, intSize, 1, fp);
			tmpNode->m_p.y = ivalue;
		}
		else
		{
			quiet = fread(&tmpNode->m_p.x, doubleSize, 1, fp);
			quiet = fread(&tmpNode->m_p.y, doubleSize, 1, fp);
		}
		//add node to list
		AddNode(tmpNode);

		// and to quick lookup table
		pNodeLookup[i] = tmpNode;
	}

	quiet = fread(buffer,7,1,fp);
	if (strcmp(buffer, "Roads:"))
	{
		fclose(fp);
		return false;
	}

	// Read the links
	float ftmp;
	int itmp;
	int reject1 = 0, reject2 = 0, reject3 = 0;
	for (i = 1; i <= numLinks; i++)
	{
		tmpLink = NewLink();
		quiet = fread(&(tmpLink->m_id), intSize, 1, fp);	//id
		if (version < 1.89)
		{
			quiet = fread(&itmp, intSize, 1, fp);			//highway number
			tmpLink->m_iHwy = (short) itmp;
			quiet = fread(&(tmpLink->m_fWidth), floatSize, 1, fp);	//width
			quiet = fread(&itmp, intSize, 1, fp);			//number of lanes
			tmpLink->m_iLanes = (unsigned short) itmp;
			quiet = fread(&itmp, intSize, 1, fp);			//surface type
			tmpLink->m_Surface =  (SurfaceType) itmp;
			quiet = fread(&itmp, intSize, 1, fp);			//FLAG
			tmpLink->m_iFlags = (short) (itmp >> 16);
		}
		else
		{
			quiet = fread(&(dummy), 4, 1, fp);			//highway number
			tmpLink->m_iHwy = (short) dummy;
			quiet = fread(&(tmpLink->m_fWidth), floatSize, 1, fp);	//width
			quiet = fread(&(dummy), 4, 1, fp);			//number of lanes
			tmpLink->m_iLanes = (short) dummy;
			quiet = fread(&dummy, 4, 1, fp);			//surface type
			tmpLink->m_Surface =  (SurfaceType) dummy;
			quiet = fread(&(dummy), 4, 1, fp);			//FLAG
			tmpLink->m_iFlags = dummy;
		}

		if (version < 1.89)
		{
			quiet = fread(&ftmp, floatSize, 1, fp);		//height of link at node 0
			quiet = fread(&ftmp, floatSize, 1, fp);		//height of link at node 1
		}

		if (version >= 2.0)
		{
			quiet = fread(&(tmpLink->m_fSidewalkWidth), floatSize, 1, fp);	// sidewalk width
			quiet = fread(&(tmpLink->m_fCurbHeight), floatSize, 1, fp);		// curb height
			quiet = fread(&(tmpLink->m_fMarginWidth), floatSize, 1, fp);	// margin width
			quiet = fread(&(tmpLink->m_fLaneWidth), floatSize, 1, fp);		// lane width
			quiet = fread(&(tmpLink->m_fParkingWidth), floatSize, 1, fp);	// parking width
		}
		int size;
		quiet = fread(&size, intSize, 1, fp);	// number of coordinates making the link
		tmpLink->SetSize(size);

		for (j = 0; j < size; j++)
		{
			if (version < 1.8f)
			{
				quiet = fread(&ivalue, intSize, 1, fp);
				(*tmpLink)[j].x = ivalue;
				quiet = fread(&ivalue, intSize, 1, fp);
				(*tmpLink)[j].y = ivalue;
			}
			else
			{
				quiet = fread(&((*tmpLink)[j].x), doubleSize, 1, fp);
				quiet = fread(&((*tmpLink)[j].y), doubleSize, 1, fp);
			}
		}

		//set the end points
		quiet = fread(&nodeNum, intSize, 1, fp);
		if (nodeNum < 1 || nodeNum > numNodes)
			return false;
		tmpLink->SetNode(0, pNodeLookup[nodeNum]);
		quiet = fread(&nodeNum, intSize, 1, fp);
		if (nodeNum < 1 || nodeNum > numNodes)
			return false;
		tmpLink->SetNode(1, pNodeLookup[nodeNum]);

		// check for inclusion
		bool include = true;
		if (!bHwy && tmpLink->m_iHwy > 0)
		{
			include = false;
			reject1++;
		}
		if (include && !bPaved && tmpLink->m_Surface == SURFT_PAVED)
		{
			include = false;
			reject2++;
		}
		if (include && !bDirt && (tmpLink->m_Surface == SURFT_TRAIL ||
			tmpLink->m_Surface == SURFT_2TRACK ||
			tmpLink->m_Surface == SURFT_DIRT ||
			tmpLink->m_Surface == SURFT_GRAVEL))
		{
			include = false;
			reject3++;
		}

		if (!include)
		{
			delete tmpLink;
			continue;
		}

		// Inform the Nodes to which it belongs
		tmpLink->GetNode(0)->AddLink(tmpLink, true);	// true: starts at this node
		tmpLink->GetNode(1)->AddLink(tmpLink, false);	// false: ends at this node

		// Add to list
		AddLink(tmpLink);
	}
	if (reject1 || reject2 || reject3)
	{
		VTLOG("  Ignored links:");
		if (reject1) VTLOG(" %d for being highways, ", reject1);
		if (reject2) VTLOG(" %d for being paved, ", reject2);
		if (reject3) VTLOG(" %d for being dirt, ", reject3);
		VTLOG("done.\n");
	}

	// Read traffic control information
	quiet = fread(buffer,9, 1, fp);
	if (strcmp(buffer, "Traffic:"))
	{
		fclose(fp);
		return false;
	}

	for (i = 0; i < numNodes; i++)
	{
		int id, numLinks;

		quiet = fread(&id, intSize, 1, fp);  //node ID
		// safety check
		if (id < 1 || id > numNodes)
		{
			fclose(fp);
			return false;
		}

		tmpNode = pNodeLookup[id];
		quiet = fread(&dummy, intSize, 1, fp);
		quiet = fread(&numLinks, intSize, 1, fp);

		//get specifics for each link at the intersection:
		for (j = 0; j < numLinks; j++)
		{
			//match link number
			IntersectionType type;
			LightStatus lStatus;
			//read in data
			quiet = fread(&id, intSize, 1, fp);  //link ID
			quiet = fread(&type, intSize, 1, fp);
			quiet = fread(&lStatus, intSize, 1, fp);
			//now figure out which links at the node get what behavior
			id = tmpNode->FindLink(id);
			if (id >= 0)
			{
				tmpNode->SetIntersectType(id, type);
				tmpNode->SetLightStatus(id, lStatus);
			}
		}
	}

	// We don't need the lookup table any more
	delete [] pNodeLookup;

	//are we at end of file?
	quiet = fread(buffer,8, 1, fp);
	fclose(fp);
	if (strcmp(buffer, "End RMF"))
		return false;

	return true;
}

//
// returns true if operation sucessful.
//
bool vtRoadMap::WriteRMF(const char *filename)
{
	int i;

	TNode *curNode = GetFirstNode();
	TLink *curLink = GetFirstLink();
	int numNodes = NumNodes();
	int numLinks = NumLinks();

	// must have nodes, or saving will fail
	if (numNodes == 0)
		return false;

	FILE *fp = vtFileOpen(filename, "wb");
	if (!fp)
	{
		// wxString str = wxString::Format("Error opening file: %s", filename);
		return false;
	}

	i= 1;
	// go through and set id numbers for the nodes and links
	while (curNode) {
		curNode->m_id = i++;
		curNode = curNode->m_pNext;
	}
	i=1;
	while (curLink) {
		curLink->m_id = i++;
		curLink = curLink->m_pNext;
	}

	curNode = GetFirstNode();
	curLink = GetFirstLink();

	FWrite(RMFVERSION_STRING, 11);

	// Projection
	char *wkt;
	OGRErr err = m_proj.exportToWkt(&wkt);
	if (err != OGRERR_NONE)
		return false;
	short len = (short) strlen(wkt);
	FWrite(&len, 4);
	FWrite(wkt, len);
	OGRFree(wkt);

	// Extents
	FWrite(&m_extents.left, doubleSize);
	FWrite(&m_extents.right, doubleSize);
	FWrite(&m_extents.bottom, doubleSize);
	FWrite(&m_extents.top, doubleSize);
	FWrite(&numNodes, intSize);  // number of nodes
	FWrite(&numLinks, intSize);  // number of links
	FWrite("Nodes:",7);
	//write nodes
	while (curNode)
	{
		FWrite(&(curNode->m_id), intSize);		// id
		FWrite(&curNode->m_p.x, doubleSize);	// coordinate
		FWrite(&curNode->m_p.y, doubleSize);
		curNode = curNode->m_pNext;
	}
	FWrite("Roads:",7);
	//write links
	while (curLink)
	{
		FWrite(&(curLink->m_id), intSize);			//id
		FWrite(&(curLink->m_iHwy), 4);				//highway number
		FWrite(&(curLink->m_fWidth), floatSize);	//width
		FWrite(&(curLink->m_iLanes), 4);			//number of lanes
		FWrite(&(curLink->m_Surface), 4);			//surface type
		FWrite(&(curLink->m_iFlags), 4);			//FLAG
		FWrite(&(curLink->m_fSidewalkWidth), floatSize);	// sidewalk width
		FWrite(&(curLink->m_fCurbHeight), floatSize);	// curb height
		FWrite(&(curLink->m_fMarginWidth), floatSize);	// margin width
		FWrite(&(curLink->m_fLaneWidth), floatSize);	// lane width width
		FWrite(&(curLink->m_fParkingWidth), floatSize);	// parking width

		int size = curLink->GetSize();
		FWrite(&size, intSize);//number of coordinates making the link
		for (i = 0; i < size; i++)
		{
			//coordinates that make the link
			FWrite(&curLink->GetAt(i).x, doubleSize);
			FWrite(&curLink->GetAt(i).y, doubleSize);
		}
		//nodes (endpoints)
		FWrite(&(curLink->GetNode(0)->m_id), intSize);	//what link is at the end point?
		FWrite(&(curLink->GetNode(1)->m_id), intSize);

		curLink = curLink->m_pNext;
	}

	int dummy = 0;

	//write traffic control information
	FWrite("Traffic:",9);
	curNode = GetFirstNode();
	while (curNode)
	{
		// safety check
		if (curNode->m_id < 1 || curNode->m_id > numNodes)
			return false;

		FWrite(&(curNode->m_id), intSize);	//node ID
		FWrite(&(dummy), intSize); //node traffic behavior
		FWrite(&(curNode->m_iLinks), intSize); //node traffic behavior
		for (i = 0; i < curNode->m_iLinks; i++) {
			IntersectionType type = curNode->GetIntersectType(i);
			LightStatus lStatus = curNode->GetLightStatus(i);
			FWrite(&(curNode->GetLink(i)->m_id), intSize);  //link ID
			FWrite(&type, intSize);  //get the intersection type associated with that link
			FWrite(&lStatus,intSize);
		}
		curNode = curNode->m_pNext;
	}

	//EOF
	FWrite("End RMF", 8);
	fclose(fp);

	return true;
}

