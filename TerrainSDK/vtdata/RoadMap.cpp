//
// RoadMap.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "RoadMap.h"
#include "vtLog.h"


//
// Helpers
//
void fwriteIP2(FPoint2 &p, FILE *fp)
{
	/*  FIXME:  Ahoy, there be byte order issues here.  See below in this routine.  */
	fwrite(&p, 8, 1, fp);
}
void freadIP2(FPoint2 &p, FILE *fp)
{
	/*  FIXME:  Ahoy, there be byte order issues here.  See below in this routine.  */
	fread(&p, 8, 1, fp);
}

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
Node::Node()
{
	m_pNext = NULL;
	m_iLinks = 0;
	m_id = -1;
	m_r = NULL;
	m_IntersectTypes = NULL;
	m_Lights = NULL;
	m_fLinkAngle = NULL;
}

Node::~Node()
{
	if (m_r) delete m_r;
	if (m_fLinkAngle) delete m_fLinkAngle;
	if (m_IntersectTypes) delete m_IntersectTypes;
	if (m_Lights) delete m_Lights;
}

bool Node::operator==(Node &ref)
{
	if (m_iLinks != ref.m_iLinks)
		return false;
	for (int i = 0; i < m_iLinks; i++)
	{
		if (m_IntersectTypes[i] != ref.m_IntersectTypes[i])
			return false;
		if (m_Lights[i] != ref.m_Lights[i])
			return false;
	}
	return true;
}

void Node::Copy(Node *node)
{
	m_p = node->m_p;
	m_iLinks = node->m_iLinks;
	m_id = node->m_id;

	if (node->m_fLinkAngle)
		m_fLinkAngle = new float[m_iLinks];

	m_IntersectTypes = new IntersectionType[m_iLinks];
	m_Lights = new LightStatus[m_iLinks];
	m_r = new Link*[m_iLinks];
	for (int i = 0; i < m_iLinks; i++)
	{
		if (node->m_fLinkAngle)
			m_fLinkAngle[i] = node->m_fLinkAngle[i];

		m_IntersectTypes[i] = node->m_IntersectTypes[i];
		m_Lights[i] = node->m_Lights[i];
		m_r[i] = node->m_r[i];
	}

	//don't copy this
	m_pNext = NULL;
}

Link* Node::GetLink(int n)
{
	if (m_r && n < m_iLinks)	// safety check
		return m_r[n];
	else
		return NULL;
}

int Node::FindLink(int roadID) {
	for (int i = 0; i < m_iLinks; i++) {
		if (m_r[i]->m_id == roadID) {
			return i;
		}
	}
	return -1;
}

void Node::AddLink(class Link *pR)
{
	int i;

	m_iLinks++;

	LinkPtr *old_roads = m_r;
	IntersectionType *iTypes = m_IntersectTypes;
	LightStatus *lights = m_Lights;

	m_r = new LinkPtr[m_iLinks];
	m_IntersectTypes = new IntersectionType[m_iLinks];
	m_Lights = new LightStatus[m_iLinks];

	// copy data from previous array
	for (i = 0; i < m_iLinks-1; i++) {
		m_r[i] = old_roads[i];
		m_IntersectTypes[i] = iTypes[i];
		m_Lights[i] = lights[i];
	}

	// fill in the entry for the new road
	m_r[i] = pR;
	m_IntersectTypes[i] = IT_NONE;
	m_Lights[i] = LT_INVALID;

	if (old_roads) delete old_roads;
	delete iTypes;
	delete lights;
}

void Node::DetachLink(class Link *pR)
{
	for (int i = 0; i < m_iLinks; i++)
	{
		if (m_r[i] == pR)
		{
			for (int j = i; j < m_iLinks-1; j++) {
				m_r[j] = m_r[j+1];
				m_IntersectTypes[j] = m_IntersectTypes[j+1];
				m_Lights[j] = m_Lights[j+1];
			}
			m_iLinks--;
			return;
		}
	}
}

//angles all > 0.
void Node::DetermineLinkAngles()
{
	if (m_fLinkAngle)
		delete m_fLinkAngle;
	m_fLinkAngle = new float[m_iLinks];

	DPoint2 pn0, pn1, diff;
	for (int i = 0; i < m_iLinks; i++)
	{
		pn0 = m_p;
		pn1 = find_adjacent_roadpoint2d(m_r[i]);

		diff = pn1 - pn0;

		m_fLinkAngle[i] = atan2f((float)diff.y, (float)diff.x);
		if (m_fLinkAngle[i] < 0.0f) m_fLinkAngle[i] += PI2f;
	}
}

void Node::SortLinksByAngle()
{
	float ftmp;

	// first determine the angle of each road
	DetermineLinkAngles();

	// sort roads by radial angle (make them counter-clockwise)
	// use a bubble sort
	bool sorted = false;
	Link* tmpLink;
	IntersectionType intersectType;
	LightStatus lightStatus;
	while (!sorted)
	{
		sorted = true;
		for (int i = 0; i < m_iLinks-1; i++)
		{
			if (m_fLinkAngle[i] > m_fLinkAngle[i+1])
			{
				//save info to be replaced
				tmpLink = m_r[i];
				intersectType = m_IntersectTypes[i];
				lightStatus = m_Lights[i];
				ftmp = m_fLinkAngle[i];

				//move info over
				m_r[i] = m_r[i+1];
				m_r[i+1] = tmpLink;
				m_IntersectTypes[i] = m_IntersectTypes[i+1];
				m_IntersectTypes[i+1] = intersectType;
				m_Lights[i] = m_Lights[i+1];
				m_Lights[i+1] = lightStatus;

				m_fLinkAngle[i] = m_fLinkAngle[i+1];
				m_fLinkAngle[i+1] = ftmp;

				sorted = false;
			}
		}
	}
}

DPoint2 Node::find_adjacent_roadpoint2d(Link *pR)
{
	if (pR->GetNode(0) == this)
		return (*pR)[1];			// roads starts here
	else
		return (*pR)[pR->GetSize()-2];	// road ends here
}


//traffic control
bool Node::SetIntersectType(Link *road, IntersectionType type){
	for (int i = 0; i < m_iLinks; i++) {
		if (m_r[i] == road) {
			m_IntersectTypes[i] = type;
			return true;
		}
	}
	return false;
}

bool Node::SetIntersectType(int roadNum, IntersectionType type) {
	if (roadNum >= m_iLinks || roadNum < 0) {
		return false;
	}
	m_IntersectTypes[roadNum] = type;
	return true;
}

IntersectionType Node::GetIntersectType(Link *road)
{
#if 1
	for (int i = 0; i < m_iLinks; i++)
	{
		if (m_r[i] == road) {
			return m_IntersectTypes[i];
		}
	}
	return IT_NONE;
#else
	if (road->GetNode(0))
#endif
}

IntersectionType Node::GetIntersectType(int roadNum) {
	if (roadNum >= m_iLinks || roadNum < 0) {
		return IT_NONE;
	}
	return m_IntersectTypes[roadNum];
}

LightStatus Node::GetLightStatus(Link *road) {
	for (int i = 0; i < m_iLinks; i++) {
		if (m_r[i] == road) {
			return m_Lights[i];
		}
	}
	return LT_INVALID;
}

LightStatus Node::GetLightStatus(int roadNum) {
	if (roadNum >= m_iLinks || roadNum < 0) {
		return LT_INVALID;
	}
	return m_Lights[roadNum];
}

bool Node::SetLightStatus(Link *road, LightStatus light) {
	for (int i = 0; i < m_iLinks; i++) {
		if (m_r[i] == road) {
			m_Lights[i] = light;
			return true;
		}
	}
	return false;
}

bool Node::SetLightStatus(int roadNum, LightStatus light) {
	if (roadNum >= m_iLinks || roadNum < 0) {
		return false;
	}
	m_Lights[roadNum] = light;
	return true;
}

bool Node::HasLights()
{
	for (int i = 0; i < m_iLinks; i++)
	{
		if (GetIntersectType(i) == IT_LIGHT)
			return true;
	}
	return false;
}

bool Node::IsControlled()
{
	for (int i = 0; i < m_iLinks; i++)
	{
		if (GetIntersectType(i) != IT_NONE)
			return true;
	}
	return false;
}

void Node::AdjustForLights()
{
	if (!HasLights())
		return;

	int i;
	if (m_Lights) {
		delete m_Lights;
	}
	m_Lights = new LightStatus[m_iLinks];
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

		//go through all road pairs and see what is the difference of their angles.
		//we're shooting for difference of PI.
		for (i = 0; i < m_iLinks - 1; i++)
		{
			//since angles are sorted, angle i < angle j (not sure if that helps.)
			for (int j = i+1; j < m_iLinks; j++)
			{
				if (i != j)
				{
					newAngle  = m_fLinkAngle[j] - (m_fLinkAngle[i]+ PIf);
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
					printf("%i:%f, %i:%f, %f, %f", i,m_fLinkAngle[i], j,m_fLinkAngle[j], newAngle, bestAngle);
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
Link::Link()
{
	// Provide default values
	m_fWidth = 1.0f;
	m_iLanes = 0;
	m_Surface = SURFT_PAVED;
	m_iHwy = -1;
	m_pNext = NULL;
	m_id = 0;
	m_iFlags = (RF_FORWARD|RF_REVERSE);	// by default, road are bidirectional
	m_fHeight[0] = 0;
	m_fHeight[1] = 0;
	m_pNode[0] = NULL;
	m_pNode[1] = NULL;
}

//
// Copy constructor
//
Link::Link(Link &ref)
{
	*this = ref;
//	DLine2::DLine2(ref);	// need to do this?
}

Link::~Link()
{
}

bool Link::operator==(Link &ref)
{
	return (m_fWidth == ref.m_fWidth &&
		m_iLanes == ref.m_iLanes &&
		m_Surface == ref.m_Surface &&
		m_iHwy == ref.m_iHwy &&
		m_iFlags == ref.m_iFlags);
}

void Link::SetFlag(int flag, bool value)
{
	if (value)
		m_iFlags |= flag;
	else
		m_iFlags &= ~flag;
}

int Link::GetFlag(int flag)
{
	return (m_iFlags & flag) != 0;
}

/**
 * Find closest lateral distance from a given point to the road.
 */
double Link::DistanceToPoint(const DPoint2 &point, bool bAllowEnds)
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
 * along the road, and b is the signed lateral distance orthogonal to
 * the road at that point.
 *
 * \param p The input point.
 * \param result_a	The resulting distance along the link.
 * \param result_b  The signed lateral (or absolute) distance to the link.
 * \param closest	The closest point on the link.
 * \param roadpoint	Index into the roads points just before the closest point.
 * \param fractional Fractional distance between this road point and the next.
 * \param bAllowEnds	If true, then for cases where the the closest
 *		point is either end of the link, the distance to that point
 *		is returned.  Otherwise, only laterial distances are returned.
 */
double Link::GetLinearCoordinates(const DPoint2 &p, double &result_a,
								  double &result_b, DPoint2 &closest,
								  int &roadpoint, float &fractional,
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
			roadpoint = i;
			fractional = (float) (u/length);
		}
		traversed += length;
	}

	// 'traversed' now contains the total length of the link
	return traversed;
}

float Link::Length()
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

float Link::EstimateWidth(bool bIncludeSidewalk)
{
	float width = m_iLanes * LANE_WIDTH;
	if (GetFlag(RF_PARKING))
		width += (PARKING_WIDTH * 2);
	if (GetFlag(RF_MARGIN))
		width += (MARGIN_WIDTH * 2);
	if (bIncludeSidewalk && GetFlag(RF_SIDEWALK))
		width += (SIDEWALK_WIDTH * 2);
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
	Link *nextR;
	while (m_pFirstLink)
	{
		nextR = m_pFirstLink->m_pNext;
		delete m_pFirstLink;
		m_pFirstLink = nextR;
	}

	Node *nextN;
	while (m_pFirstNode)
	{
		nextN = m_pFirstNode->m_pNext;
		delete m_pFirstNode;
		m_pFirstNode = nextN;
	}
}

Node *vtRoadMap::FindNodeByID(int id)
{
	for (Node *pN = m_pFirstNode; pN; pN = pN->m_pNext)
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
Node *vtRoadMap::FindNodeAtPoint(const DPoint2 &point, double epsilon)
{
	Node *closest = NULL;
	double result, dist = 1E9;

	// a target rectangle, to quickly cull points too far away
	DRECT target(point.x-epsilon, point.y+epsilon, point.x+epsilon, point.y-epsilon);
	for (Node* curNode = GetFirstNode(); curNode; curNode = curNode->m_pNext)
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
	for (Link *pR = m_pFirstLink; pR; pR = pR->m_pNext)
		count++;
	return count;
}

int	vtRoadMap::NumNodes() const
{
	int count = 0;
	for (Node *pN = m_pFirstNode; pN; pN = pN->m_pNext)
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
	for (Link *pR = m_pFirstLink; pR; pR = pR->m_pNext)
	{
		// roads are a subclass of line, so we can treat them as lines
		DLine2 *dl = pR;
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

	Node *prev = NULL, *next;
	Node *pN = m_pFirstNode;
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
void vtRoadMap::RemoveNode(Node *pNode)
{
	Node *prev = NULL, *next;
	Node *pN = m_pFirstNode;

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
// Remove a road - use with caution
//
void vtRoadMap::RemoveLink(Link *pLink)
{
	Link *prev = NULL, *next;
	Link *pR = m_pFirstLink;

	while (pR)
	{
		next = pR->m_pNext;
		if (pLink == pR)
		{
			// delete it
			if (prev)
				prev->m_pNext = next;
			else
				m_pFirstLink = next;
			delete pR;
			// I assume that a node cannot be on the list more than once!!!
			break;
		}
		else
			prev = pR;
		pR = next;
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
	FILE *fp = fopen(filename, "rb");
	if (!fp)
	{
		// "Error opening file: %s",filename
		return false;
	}

	int numNodes, numLinks, i, j, nodeNum, dummy;
	Node* tmpNode;
	Link* tmpLink;

	// Is it a RMF File? and check version number
	fread(buffer,11,1,fp);
	buffer[11] = 0;

	if (strncmp(buffer, RMFVERSION_STRING, 7))
	{
		// Not an RMF file!
		// "Sorry, that file does not appear to be a valid RMF file."
		fclose(fp);
		return false;
	}
	float version = (float)atof(buffer+7);

	// work around nasty release-mode MSVC bug: values returned from atof
	// sometimes aren't immediately correct until another function is called
	version = (float)atof(buffer+7);

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
	if (version < 1.9f)
	{
		int proj_type=1, iUTMZone;
		int iDatum = EPSG_DATUM_WGS84;
		if (version >= 1.8f)
		{
			int iUTM;
			fread(&iUTM, intSize, 1, fp);
			proj_type = (iUTM != 0);
		}
		fread(&iUTMZone, intSize, 1, fp);
		if (version >= 1.8f)
		{
			fread(&iDatum, intSize, 1, fp);
		}
		m_proj.SetProjectionSimple(proj_type == 1, iUTMZone, iDatum);
	}
	else
	{
		short length;
		fread(&length, shortSize, 1, fp);
		char wkt_buf[2000], *wkt = wkt_buf;
		fread(wkt_buf, length, 1, fp);
		OGRErr err = m_proj.importFromWkt((char **) &wkt);
		if (err != OGRERR_NONE)
			return false;
	}

	// Extents
	if (version < 1.9f)
	{
		int le, ri, to, bo;
		fread(&ri, intSize, 1, fp);
		fread(&to, intSize, 1, fp);
		fread(&le, intSize, 1, fp);
		fread(&bo, intSize, 1, fp);
		m_extents.left = le;
		m_extents.right = ri;
		m_extents.top = to;
		m_extents.bottom = bo;
	}
	else
	{
		fread(&m_extents.left, doubleSize, 1, fp);
		fread(&m_extents.right, doubleSize, 1, fp);
		fread(&m_extents.bottom, doubleSize, 1, fp);
		fread(&m_extents.top, doubleSize, 1, fp);
	}
	m_bValidExtents = true;

	//get number of nodes and roads
	fread(&numNodes, intSize, 1, fp);
	fread(&numLinks, intSize, 1, fp);

	fread(buffer,7,1,fp);
	if (strcmp(buffer, "Nodes:"))
	{
		fclose(fp);
		return false;
	}

	// Use a temporary array for fast lookup
	NodePtr *pNodeLookup = new NodePtr[numNodes+1];

	// Read the nodes
	int ivalue;
	for (i = 1; i <= numNodes; i++)
	{
		tmpNode = NewNode();
		fread(&(tmpNode->m_id), intSize, 1, fp);
		if (version < 1.8f)
		{
			fread(&ivalue, intSize, 1, fp);
			tmpNode->m_p.x = ivalue;
			fread(&ivalue, intSize, 1, fp);
			tmpNode->m_p.y = ivalue;
		}
		else
		{
			fread(&tmpNode->m_p.x, doubleSize, 1, fp);
			fread(&tmpNode->m_p.y, doubleSize, 1, fp);
		}
		//add node to list
		AddNode(tmpNode);

		// and to quick lookup table
		pNodeLookup[i] = tmpNode;
	}

	fread(buffer,7,1,fp);
	if (strcmp(buffer, "Roads:"))
	{
		fclose(fp);
		return false;
	}

	// Read the roads
	float ftmp;
	int itmp;
	short stmp;
	int reject1 = 0, reject2 = 0, reject3 = 0;
	for (i = 1; i <= numLinks; i++)
	{
		tmpLink = NewLink();
		fread(&(tmpLink->m_id), intSize, 1, fp);	//id
		if (version < 1.89)
		{
			fread(&itmp, intSize, 1, fp);			//highway number
			tmpLink->m_iHwy = (short) itmp;
			fread(&(tmpLink->m_fWidth), floatSize, 1, fp);	//width
			fread(&itmp, intSize, 1, fp);			//number of lanes
			tmpLink->m_iLanes = (unsigned short) itmp;
			fread(&itmp, intSize, 1, fp);			//surface type
			tmpLink->m_Surface =  (SurfaceType) itmp;
			fread(&itmp, intSize, 1, fp);			//FLAG
			tmpLink->m_iFlags = (short) (itmp >> 16);
		}
		else
		{
			fread(&(tmpLink->m_iHwy), shortSize, 1, fp);	//highway number
			fread(&(tmpLink->m_fWidth), floatSize, 1, fp);	//width
			fread(&(tmpLink->m_iLanes), shortSize, 1, fp);	//number of lanes
			fread(&stmp, shortSize, 1, fp);					//surface type
			tmpLink->m_Surface =  (SurfaceType) stmp;
			fread(&(tmpLink->m_iFlags), shortSize, 1, fp);	//FLAG
		}

		if (version < 1.89)
		{
			fread(&ftmp, floatSize, 1, fp);		//height of road at node 0
			fread(&ftmp, floatSize, 1, fp);		//height of road at node 1
		}

		int size;
		fread(&size, intSize, 1, fp);	// number of coordinates making the road
		tmpLink->SetSize(size);

		for (j = 0; j < size; j++)
		{
			if (version < 1.8f)
			{
				fread(&ivalue, intSize, 1, fp);
				(*tmpLink)[j].x = ivalue;
				fread(&ivalue, intSize, 1, fp);
				(*tmpLink)[j].y = ivalue;
			}
			else
			{
				fread(&((*tmpLink)[j].x), doubleSize, 1, fp);
				fread(&((*tmpLink)[j].y), doubleSize, 1, fp);
			}
		}

		//set the end points
		fread(&nodeNum, intSize, 1, fp);
		tmpLink->SetNode(0, pNodeLookup[nodeNum]);
		fread(&nodeNum, intSize, 1, fp);
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
		tmpLink->GetNode(0)->AddLink(tmpLink);
		tmpLink->GetNode(1)->AddLink(tmpLink);

		// Add to list
		AddLink(tmpLink);
	}
	if (reject1 || reject2 || reject3)
	{
		VTLOG("  Ignored roads:");
		if (reject1) VTLOG(" %d for being highways, ", reject1);
		if (reject2) VTLOG(" %d for being paved, ", reject2);
		if (reject3) VTLOG(" %d for being dirt, ", reject3);
		VTLOG("done.\n");
	}

	// Read traffic control information
	fread(buffer,9, 1, fp);
	if (strcmp(buffer, "Traffic:"))
	{
		fclose(fp);
		return false;
	}

	for (i = 0; i < numNodes; i++)
	{
		int id, numLinks;

		fread(&id, intSize, 1, fp);  //node ID
		// safety check
		if (id < 1 || id > numNodes)
		{
			fclose(fp);
			return false;
		}

		tmpNode = pNodeLookup[id];
		fread(&dummy, intSize, 1, fp);
		fread(&numLinks, intSize, 1, fp);

		//get specifics for each road at the intersection:
		for (j = 0; j < numLinks; j++)
		{
			//match road number
			IntersectionType type;
			LightStatus lStatus;
			//read in data
			fread(&id, intSize, 1, fp);  //road ID
			fread(&type, intSize, 1, fp);
			fread(&lStatus, intSize, 1, fp);
			//now figure out which roads at the node get what behavior
			id = tmpNode->FindLink(id);
			if (id >= 0)
			{
				tmpNode->SetIntersectType(id, type);
				tmpNode->SetLightStatus(id, lStatus);
			}
		}
	}

	// We don't need the lookup table any more
	delete pNodeLookup;

	//are we at end of file?
	fread(buffer,8, 1, fp);
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

	Node *curNode = GetFirstNode();
	Link *curLink = GetFirstLink();
	int numNodes = NumNodes();
	int numLinks = NumLinks();

	// must have nodes, or saving will fail
	if (numNodes == 0)
		return false;

	FILE *fp = fopen(filename, "wb");
	if (!fp)
	{
		// wxString str = wxString::Format("Error opening file: %s", filename);
		return false;
	}

	i= 1;
	// go through and set id numbers for the nodes and roads
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
	FWrite(&len, shortSize);
	FWrite(wkt, len);
	OGRFree(wkt);

	// Extents
	FWrite(&m_extents.left, doubleSize);
	FWrite(&m_extents.right, doubleSize);
	FWrite(&m_extents.bottom, doubleSize);
	FWrite(&m_extents.top, doubleSize);
	FWrite(&numNodes, intSize);  // number of nodes
	FWrite(&numLinks, intSize);  // number of roads
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
	//write roads
	while (curLink)
	{
		FWrite(&(curLink->m_id), intSize);			//id
		FWrite(&(curLink->m_iHwy), shortSize);		//highway number
		FWrite(&(curLink->m_fWidth), floatSize);	//width
		FWrite(&(curLink->m_iLanes), shortSize);	//number of lanes
		FWrite(&(curLink->m_Surface), shortSize);	//surface type
		FWrite(&(curLink->m_iFlags), shortSize);	//FLAG

		int size = curLink->GetSize();
		FWrite(&size, intSize);//number of coordinates making the road
		for (i = 0; i < size; i++)
		{
			//coordinates that make the road
			FWrite(&curLink->GetAt(i).x, doubleSize);
			FWrite(&curLink->GetAt(i).y, doubleSize);
		}
		//nodes (endpoints)
		FWrite(&(curLink->GetNode(0)->m_id), intSize);	//what road is at the end point?
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
			FWrite(&(curNode->GetLink(i)->m_id), intSize);  //road ID
			FWrite(&type, intSize);  //get the intersection type associated with that road
			FWrite(&lStatus,intSize);
		}
		curNode = curNode->m_pNext;
	}

	//EOF
	FWrite("End RMF", 8);
	fclose(fp);

	return true;
}

