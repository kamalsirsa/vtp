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

double Node::DistanceToPoint(DPoint2 target)
{
	DPoint2 point = m_p - target;
	return point.Length();
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

//closet distance from target to the road
double Link::DistanceToPoint(DPoint2 target)
{
	DPoint2 delta;
	double bestDistance = -1;
	double dist;

	double u, ux, uy,un, ud;
	DPoint2 p1, p2, p3 = target;

	for (int i=0; i < GetSize()-1; i++)
	{
		p1 = GetAt(i);
		p2 = GetAt(i+1);

		u = .5;

		//check distance to line segment
		ux = p2.x - p1.x;
		uy = p2.y - p1.y;
		un = (p3.x - p1.x)*(p2.x - p1.x) + (p3.y-p1.y)*(p2.y-p1.y);
		ud = ux*ux + uy*uy;
		u = un/ud;

		delta.x = (p1.x + u*ux) - p3.x;
		delta.y = (p1.y + u*uy) - p3.y;

		dist = delta.Length();

		//if u >1 or u <0, then point is not closest to line within the
		//given line segment.  use the end points.
		if (u > 1 || u < 0)
		{
			delta = target - GetAt(i);
			dist = delta.Length();
		}

		if (dist < bestDistance || bestDistance == -1) {
			bestDistance = dist;
			//TRACE ("%i, %.3f, %.3f:\t%.3f\n",m_id, x,y, dist);
		}
	}
	return bestDistance;
}

float Link::GetHeightAt(int i)
{
	return m_fHeight[i];
}

float Link::GetHeightAt(Node *node)
{
	if (m_pNode[0] == node)
		return m_fHeight[0];

	if (m_pNode[1] == node)
		return m_fHeight[1];

	//error.
	assert(false);
	return 0;
}

void Link::SetHeightAt(int i, float height)
{
	m_fHeight[i] = height;
}

void Link::SetHeightAt(Node *node, float height)
{
	if (m_pNode[0] == node)
	{
		m_fHeight[0] = height;
	} else if (m_pNode[1] == node)
	{
		m_fHeight[1] = height;
	} else {
		//error!!!!
		assert(false);
	}
}

float Link::Length()
{
	double dist = 0;
	DPoint2 tmp;
	for (int i = 1; i < GetSize(); i++)
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
	// aren't immediately correct until another function is called
	float version2 = (float)atof(buffer+7);

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
	int proj_type, iUTMZone;
	DATUM eDatum;
	if (version >= 1.8f)
	{
		int iUTM;
		fread(&iUTM, intSize, 1, fp);
		proj_type = (iUTM != 0);
	}
	fread(&iUTMZone, intSize, 1, fp);
	if (version >= 1.8f)
	{
		int iDatum;
		fread(&iDatum, intSize, 1, fp);
		eDatum = (DATUM) iDatum;
	}
	m_proj.SetProjectionSimple(proj_type == 1, iUTMZone, eDatum);

	// Extents
	int le, ri, to, bo;
	fread(&ri, intSize, 1, fp);
	fread(&to, intSize, 1, fp);
	fread(&le, intSize, 1, fp);
	fread(&bo, intSize, 1, fp);
	m_extents.left = le;
	m_extents.right = ri;
	m_extents.top = to;
	m_extents.bottom = bo;
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
	for (i = 1; i <= numLinks; i++)
	{
		tmpLink = NewLink();
		float tmp;
		fread(&(tmpLink->m_id), intSize, 1, fp);		//id
		fread(&(tmpLink->m_iHwy), intSize, 1, fp);		//highway number
		fread(&(tmpLink->m_fWidth), floatSize, 1, fp);	//width
		fread(&(tmpLink->m_iLanes), intSize, 1, fp);	//number of lanes
		fread(&(tmpLink->m_Surface), intSize, 1, fp);	//surface type
		fread(&(tmpLink->m_iFlags), intSize, 1, fp);	//FLAG
		fread(&tmp, floatSize, 1, fp);		//height of road at node 0
		tmpLink->SetHeightAt(0, tmp);
		fread(&tmp, floatSize, 1, fp);		//height of road at node 1
		tmpLink->SetHeightAt(1, tmp);

		int size;
		fread(&size, intSize, 1, fp);	// number of coordinates making the road
		tmpLink->SetSize(size);

		for (j = 0; j < tmpLink->GetSize(); j++)
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
		bool include = false;
		if (bHwy && tmpLink->m_iHwy > 0) include = true;
		if (bPaved && tmpLink->m_Surface == SURFT_PAVED) include = true;
		if (bDirt && (tmpLink->m_Surface == SURFT_TRAIL ||
			tmpLink->m_Surface == SURFT_2TRACK ||
			tmpLink->m_Surface == SURFT_DIRT ||
			tmpLink->m_Surface == SURFT_GRAVEL)) include = true;

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
	int iZone = m_proj.GetUTMZone();
	int iUTM = (iZone != 0);
	int iDatum = (int) m_proj.GetDatum();
	FWrite(&iUTM, intSize);
	FWrite(&iZone, intSize);
	FWrite(&iDatum, intSize);

	// Extents
	int le, ri, to, bo;
	le = (int) m_extents.left;
	ri = (int) m_extents.right;
	to = (int) m_extents.top;
	bo = (int) m_extents.bottom;
	FWrite(&ri, intSize);
	FWrite(&to, intSize);
	FWrite(&le, intSize);
	FWrite(&bo, intSize);
	FWrite(&numNodes, intSize);  // number of nodes
	FWrite(&numLinks, intSize);  // number of roads
	FWrite("Nodes:",7);	
	//write nodes
	while (curNode) 
	{
		FWrite(&(curNode->m_id), intSize);		// id
		FWrite(&curNode->m_p.x, doubleSize);		// coordinate
		FWrite(&curNode->m_p.y, doubleSize);
		curNode = curNode->m_pNext;
	}
	FWrite("Roads:",7);	
	//write roads
	while (curLink)
	{
		float tmp;
		FWrite(&(curLink->m_id), intSize);		//id
		FWrite(&(curLink->m_iHwy), intSize);		//highway number
		FWrite(&(curLink->m_fWidth), floatSize);	//width
		FWrite(&(curLink->m_iLanes), intSize);	//number of lanes
		FWrite(&(curLink->m_Surface), intSize);	//surface type
		FWrite(&(curLink->m_iFlags), intSize);	//FLAG
		tmp = curLink->GetHeightAt(0);
		FWrite(&tmp, floatSize); //height of the road at node 0
		tmp = curLink->GetHeightAt(1);
		FWrite(&tmp, floatSize); //height of the road at node 1

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

