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

#if SWITCH
//LaneIO
LaneIO::LaneIO()
{
	m_iChoices = 0;
	m_pRoads = NULL;
	m_iLane = NULL;
}

LaneIO::~LaneIO()
{
	delete m_iLane;
	delete m_pRoads;
}

void LaneIO::AddChoice(Road *road, int lane)
{
	Road **roads = m_pRoads;
	int *lanes = m_iLane;
	m_iChoices++;
	m_pRoads = new Road*[m_iChoices];
	m_iLane = new int[m_iChoices];
	for (int i = 0; i < m_iChoices -1; i++) {
		m_pRoads[i] = roads[i];
		m_iLane[i] = lanes[i];
	}
	m_pRoads[i] = road;
	m_iLane[i] = lane;
	delete lanes;
	delete roads;
}

int LaneIO::GetRoadNum(Road *road)
{
	for (int i = 0; i < m_iChoices; i++) {
		if (m_pRoads[i] == road) {
			return i;
		}
	}
	return -1;
}

//RoadIO
RoadIO::RoadIO(int lanes)
{
	m_iLanes = lanes;
	m_pLaneIO = new LaneIO*[lanes];
}

RoadIO::~RoadIO()
{
	for (int i = 0; i < m_iLanes; i++) {
		delete m_pLaneIO[i];
	}
	delete m_pLaneIO;
}

//Switch
Switch::Switch(int r, class Road **roads) {
	m_iRoads= r;
	m_pRoads = roads;
	m_pRoadIO = new RoadIO*[r];
}

Switch::~Switch()
{
	for (int i = 0; i < m_iRoads; i++) {
		delete m_pRoadIO[i];
	}
	delete m_pRoadIO;
}

//return possible road choices given current road and lane.
Road** Switch::RoadChoices(Road *curRoad, int curLane)
{
	for (int i = 0; i < m_iRoads; i++) {
		if (m_pRoads[i] == curRoad) {
			return m_pRoadIO[i]->m_pLaneIO[curLane]->m_pRoads;
		}
	}
	return NULL;
}

//return possible lane choices given destination road and current road and lane
int Switch::LaneChoice(Road *curRoad, int curLane, Road* destRoad)
{
	for (int i = 0; i < m_iRoads; i++) {
		if (m_pRoads[i] == curRoad) {
			int roadnum = m_pRoadIO[i]->m_pLaneIO[curLane]->GetRoadNum(destRoad);
			return m_pRoadIO[i]->m_pLaneIO[curLane]->m_iLane[roadnum];
		}
	}
	return -1;
}

//return possible lane choices given destination road and current road and lane
int Switch::LaneChoice(Road *curRoad, int curLane, int destRoad)
{
	for (int i = 0; i < m_iRoads; i++) {
		if (m_pRoads[i] == curRoad) {
			return m_pRoadIO[i]->m_pLaneIO[curLane]->m_iLane[destRoad];
		}
	}
	return -1;
}
#endif // SWITCH

//
// Nodes
//
Node::Node()
{
	m_pNext = NULL;
	m_iRoads = 0;
	m_r = NULL;
	m_IntersectTypes = NULL;
	m_Lights = NULL;
	m_fRoadAngle = NULL;
#if SWITCH
	m_pSwitch = NULL;
#endif
}

Node::~Node()
{
	if (m_r) delete m_r;
	if (m_fRoadAngle) delete m_fRoadAngle;
	if (m_IntersectTypes) delete m_IntersectTypes;
	if (m_Lights) delete m_Lights;
}

bool Node::operator==(Node &ref)
{
	if (m_iRoads != ref.m_iRoads)
		return false;
	for (int i = 0; i < m_iRoads; i++)
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
	m_iRoads = node->m_iRoads;
	m_id = node->m_id;

	if (node->m_fRoadAngle)
		m_fRoadAngle = new float[m_iRoads];

	m_IntersectTypes = new IntersectionType[m_iRoads];
	m_Lights = new LightStatus[m_iRoads];
	m_r = new Road*[m_iRoads];
	for (int i = 0; i < m_iRoads; i++)
	{
		if (node->m_fRoadAngle)
			m_fRoadAngle[i] = node->m_fRoadAngle[i];

		m_IntersectTypes[i] = node->m_IntersectTypes[i];
		m_Lights[i] = node->m_Lights[i];
		m_r[i] = node->m_r[i];
	}

	//don't copy this
	m_pNext = NULL;
}

Road* Node::GetRoad(int n)
{
	if (m_r && n <= m_iRoads)	// safety check
		return m_r[n];
	else
		return NULL;
}

int Node::FindRoad(int roadID) {
	for (int i = 0; i < m_iRoads; i++) {
		if (m_r[i]->m_id == roadID) {
			return i;
		}
	}
	return -1;
}

void Node::AddRoad(class Road *pR)
{
	int i;

	m_iRoads++;

	RoadPtr *old_roads = m_r;
	IntersectionType *iTypes = m_IntersectTypes;
	LightStatus *lights = m_Lights;

	m_r = new RoadPtr[m_iRoads];
	m_IntersectTypes = new IntersectionType[m_iRoads];
	m_Lights = new LightStatus[m_iRoads];

	// copy data from previous array
	for (i = 0; i < m_iRoads-1; i++) {
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

void Node::DetachRoad(class Road *pR)
{
	for (int i = 0; i < m_iRoads; i++)
	{
		if (m_r[i] == pR)
		{
			for (int j = i; j < m_iRoads-1; j++) {
				m_r[j] = m_r[j+1];
				m_IntersectTypes[j] = m_IntersectTypes[j+1];
				m_Lights[j] = m_Lights[j+1];
			}
			m_iRoads--;
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
void Node::DetermineRoadAngles()
{
	if (m_fRoadAngle)
		delete m_fRoadAngle;
	m_fRoadAngle = new float[m_iRoads];

	DPoint2 pn0, pn1, diff;
	for (int i = 0; i < m_iRoads; i++)
	{
		pn0 = m_p;
		pn1 = find_adjacent_roadpoint2d(m_r[i]);

		diff = pn1 - pn0;

		m_fRoadAngle[i] = atan2f((float)diff.y, (float)diff.x);
		if (m_fRoadAngle[i] < 0.0f) m_fRoadAngle[i] += PI2f;
	}
}

void Node::SortRoadsByAngle()
{
	float ftmp;

	// first determine the angle of each road
	DetermineRoadAngles();

	// sort roads by radial angle (make them counter-clockwise)
	// use a bubble sort
	bool sorted = false;
	Road* tmpRoad;
	IntersectionType intersectType;
	LightStatus lightStatus;
	while (!sorted)
	{
		sorted = true;
		for (int i = 0; i < m_iRoads-1; i++)
		{
			if (m_fRoadAngle[i] > m_fRoadAngle[i+1])
			{
				//save info to be replaced
				tmpRoad = m_r[i];
				intersectType = m_IntersectTypes[i];
				lightStatus = m_Lights[i];
				ftmp = m_fRoadAngle[i];

				//move info over
				m_r[i] = m_r[i+1];
				m_r[i+1] = tmpRoad;
				m_IntersectTypes[i] = m_IntersectTypes[i+1];
				m_IntersectTypes[i+1] = intersectType;
				m_Lights[i] = m_Lights[i+1];
				m_Lights[i+1] = lightStatus;

				m_fRoadAngle[i] = m_fRoadAngle[i+1];
				m_fRoadAngle[i+1] = ftmp;

				sorted = false;
			}
		}
	}
}

DPoint2 Node::find_adjacent_roadpoint2d(Road *pR)
{
	if (pR->GetNode(0) == this)
		return (*pR)[1];			// roads starts here
	else
		return (*pR)[pR->GetSize()-2];	// road ends here
}


//traffic control
bool Node::SetIntersectType(Road *road, IntersectionType type){
	for (int i = 0; i < m_iRoads; i++) {
		if (m_r[i] == road) {
			m_IntersectTypes[i] = type;
			return true;
		}
	}
	return false;
}

bool Node::SetIntersectType(int roadNum, IntersectionType type) {
	if (roadNum >= m_iRoads || roadNum < 0) {
		return false;
	}
	m_IntersectTypes[roadNum] = type;
	return true;
}

IntersectionType Node::GetIntersectType(Road *road)
{
#if 1
	for (int i = 0; i < m_iRoads; i++)
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
	if (roadNum >= m_iRoads || roadNum < 0) {
		return IT_NONE;
	}
	return m_IntersectTypes[roadNum];
}

LightStatus Node::GetLightStatus(Road *road) {
	for (int i = 0; i < m_iRoads; i++) {
		if (m_r[i] == road) {
			return m_Lights[i];
		}
	}
	return LT_INVALID;
}

LightStatus Node::GetLightStatus(int roadNum) {
	if (roadNum >= m_iRoads || roadNum < 0) {
		return LT_INVALID;
	}
	return m_Lights[roadNum];
}

bool Node::SetLightStatus(Road *road, LightStatus light) {
	for (int i = 0; i < m_iRoads; i++) {
		if (m_r[i] == road) {
			m_Lights[i] = light;
			return true;
		}
	}
	return false;
}

bool Node::SetLightStatus(int roadNum, LightStatus light) {
	if (roadNum >= m_iRoads || roadNum < 0) {
		return false;
	}
	m_Lights[roadNum] = light;
	return true;
}

bool Node::HasLights()
{
	for (int i = 0; i < m_iRoads; i++)
	{
		if (GetIntersectType(i) == IT_LIGHT)
			return true;
	}
	return false;
}

bool Node::IsControlled()
{
	for (int i = 0; i < m_iRoads; i++)
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
	m_Lights = new LightStatus[m_iRoads];
	for (i = 0; i< m_iRoads; i++) {
		SetLightStatus(i, LT_GREEN);
		SetIntersectType(i, IT_LIGHT);
	}
//if the intersection has signal lights, determine light relationships.
	switch (m_iRoads) {
	case 0:
	case 1:
	case 2:
		//leave colors alone
		break;
	case 3:
	case 4:
		SortRoadsByAngle();
		float bestAngle = PI2f;
		int bestChoiceA = 0;
		int bestChoiceB = 0;
		float newAngle;

		//go through all road pairs and see what is the difference of their angles.
		//we're shooting for difference of PI.
		for (i = 0; i < m_iRoads - 1; i++)
		{
			//since angles are sorted, angle i < angle j (not sure if that helps.)
			for (int j = i+1; j < m_iRoads; j++)
			{
				if (i != j)
				{
					newAngle  = m_fRoadAngle[j] - (m_fRoadAngle[i]+ PIf);
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
					printf("%i:%f, %i:%f, %f, %f", i,m_fRoadAngle[i], j,m_fRoadAngle[j], newAngle, bestAngle);
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

#if SWITCH
//sets up the internal works of the switch.  connects the lanes.
void Node::SetupSwitch() {
	m_pSwitch = new Switch(m_iRoads, m_r);
	//sort the road angles.
	SortRoadsByAngle();
	int i;
	bool in;
	bool out;
	int *in_rightlane = new int[m_iRoads];  //the rightmost lane on the road (going in)
	int *in_leftlane = new int[m_iRoads];	//the leftmost lane on the road (going in)
	int *out_rightlane = new int[m_iRoads];  //the rightmost lane on the road (going out)
	int *out_leftlane = new int[m_iRoads];	//the leftmost lane on the road (going out)
	int *num_inlanes = new int[m_iRoads];	//number on lanes going into the road from this node
	int *num_outlanes = new int[m_iRoads];//number on lanes coming out of the road from this node
	int *direction = new int[m_iRoads];  //direction to change lane value from rightmost to leftmost lane. (going in)
	//determine I/O of the roads
	for (i = 0; i < m_iRoads; i++) {
		if (m_r[i]->GetNode(0) == this) {
			in = m_r[i]->m_iFlags & RF_FORWARD;
			out = m_r[i]->m_iFlags & RF_REVERSE;
			//two way street
			if (in && out) {
				//higher lane number to the left.
				in_rightlane[i] = 0;
				in_leftlane[i] = (m_r[i]->m_iLanes)/2 - 1;
				//higher lane number to the right.
				out_rightlane[i] = m_r[i]->m_iLanes - 1;
				out_leftlane[i] = in_leftlane[i] + 1;
				if (out_leftlane[i] == m_r[i]->m_iLanes) {
					//only one lane on the road.
					out_leftlane[i] = 0;
				}
				num_inlanes[i] = in_leftlane[i] - in_rightlane[i] + 1;
			} else if (in) {
				//one way street.  traffic goes in to road
				in_rightlane[i] = 0;
				in_leftlane[i] = m_r[i]->m_iLanes - 1;
				out_rightlane[i] = -1;
				out_leftlane[i] = -1;
				num_inlanes[i] = m_r[i]->m_iLanes;
			} else if (out) {
				//one way street.  traffic comes out of road
				//WRONG WAY!
				in_rightlane[i] = -1;
				in_leftlane[i] = -1;
				out_rightlane[i] = m_r[i]->m_iLanes - 1;
				out_leftlane[i] = 0;
				num_inlanes[i] = 0;
			} else {
				//road must have a direction.
				assert(false);
			}
			direction[i] = 1;
			num_outlanes[i] = m_r[i]->m_iLanes - num_inlanes[i];
		}
		if (m_r[i]->GetNode(1) == this) {
			in = m_r[i]->m_iFlags & RF_REVERSE;
			out = m_r[i]->m_iFlags & RF_FORWARD;
			//two way street
			if (in && out) {
				//higher lane number to the right.
				in_rightlane[i] = m_r[i]->m_iLanes-1;
				in_leftlane[i] = (m_r[i]->m_iLanes+1)/2;
				//higher lane number to the left.
				out_rightlane[i] = 0;
				out_leftlane[i] = in_leftlane[i] -1;
				if (out_leftlane[i] < 0) {
					//only one lane on the road.
					out_leftlane[i] = 0;
				}
				num_inlanes[i] = in_rightlane[i] - in_leftlane[i] + 1;
			} else if (in) {
				//one way street.  traffic goes in to road
				in_rightlane[i] = m_r[i]->m_iLanes-1;
				in_leftlane[i] = 0;
				out_rightlane[i] = -1;
				out_leftlane[i] = -1;
				num_inlanes[i] = m_r[i]->m_iLanes;
			} else if (out) {
				//one way street.  traffic comes out of road
				//WRONG WAY!
				in_rightlane[i] = -1;
				in_leftlane[i] = -1;
				out_rightlane[i] = 0;
				out_leftlane[i] = m_r[i]->m_iLanes - 1;
				num_inlanes[i] = 0;
			} else {
				//road must have a direction.
				assert(false);
			}
			direction[i] = -1;
			num_outlanes[i] = m_r[i]->m_iLanes - num_inlanes[i];
		}
	}

	int left, straight, right, uturn;
	//handle road by road
	for (i = 0; i < m_iRoads; i++) {
		//handle only if traffic comes out of this road.at this node
		if (num_outlanes[i] > 0) {
			left = -1;
			straight = -1;
			right = -1;
			uturn = -1;

			//calculate left, right, straight, uturn options
			DetermineOptions(i, num_inlanes, right, straight, left, uturn);

			m_pSwitch->m_pRoadIO[i] = new RoadIO(m_r[i]->m_iLanes);

			if (num_outlanes[i] == 0) {
				//error
				assert(false);
			}

			int leftTurnLanes = 0;  //how many lanes total in current road makes a left turn
			int leftLostLanes = 0;	//how many lanes are left turn only (doesn't go straight)
			int rightTurnLanes = 0;  //how many lanes total in current road makes a right turn
			int rightLostLanes = 0;	//how many lanes are right turn only (doesn't go straight)
			int uTurnLanes = 0;	//how many lanes are allowed to make a uturn
			int straightLanes = 0; //how many lanes go straight into the next road.
			//ideal case.  one to one correspondence.
			if (num_outlanes[i] == num_inlanes[straight]) {
				if (left >= 0) {
					leftTurnLanes = 1;
				}
				if (right >= 0) {
					rightTurnLanes = 1;
				}
				if (uturn >= 0) {
					uTurnLanes = 1;
				}
				if (straight >= 0) {
					straightLanes = num_inlanes[straight];
				}
			}
			//lanes expanded.
			if (num_outlanes[i] > num_inlanes[straight]) {

				//ignore for now.

			}
			//squeezed
			if (num_outlanes[i] < num_inlanes[straight]) {

				//ignore for now.

			}

			//traverse through the lanes and determine where they can go.
			int lane = out_leftlane[i];
			bool loop = true;
			int iter = 0;
			int straightIndex = 0;
			if (straightLanes) {
				straightIndex = in_leftlane[straight];
			}
			while (loop) {
				iter++;
				m_pSwitch->m_pRoadIO[i]->m_pLaneIO[lane] = new LaneIO();
				if (uTurnLanes) {
					uTurnLanes--;
					m_pSwitch->m_pRoadIO[i]->m_pLaneIO[lane]->AddChoice(m_r[uturn], in_leftlane[uturn]);
				}
				if (leftTurnLanes) {
					leftTurnLanes--;
					leftLostLanes--;
					m_pSwitch->m_pRoadIO[i]->m_pLaneIO[lane]->AddChoice(m_r[left], in_leftlane[left]);
				}
				if (num_outlanes[i] - iter < rightTurnLanes) {
					rightTurnLanes--;
					rightLostLanes--;
					m_pSwitch->m_pRoadIO[i]->m_pLaneIO[lane]->AddChoice(m_r[right], in_rightlane[right]);
				}
				if (straightLanes && leftLostLanes <= 0) {
					straightLanes--;
					m_pSwitch->m_pRoadIO[i]->m_pLaneIO[lane]->AddChoice(m_r[straight], straightIndex);
					straightIndex -= direction[straight];
				}

				if (lane == out_rightlane[i]) {
					loop = false;
				}
				lane += direction[i];
			}

		} else {
			m_pSwitch->m_pRoadIO[i] = NULL;
		}
	}
	delete in_rightlane;
	delete in_leftlane;
	delete out_rightlane;
	delete out_leftlane;
	delete num_inlanes;
}

//i is index to current road
//determine which roads are left, straight, right of the current road.
void Node::DetermineOptions(int i, int *in, int &right, int &straight, int &left, int &uturn) {
	int j;
	float angle;
	switch (m_iRoads) {
	case 0:
		break;
	case 1:
		uturn = i;
		break;
	case 2:
		straight = !i;
		break;
	case 3:
	case 4:
		for (j = 0; j < m_iRoads; j++) {
			//only process it if we can go into the road.
			if (in[i] > 0) {
				if (i == j) {
					uturn = i;
				} else {
					angle = diffAngle(m_fRoadAngle[j], m_fRoadAngle[i]);
					if (angle > PI2f/3) {
						left = j;
					} else if (angle < -PI2f/3) {
						right = j;
					} else {
						straight = j;
					}
				}
			}
		}

		break;
	case 5:
		//unsupported.
		break;
	}
}
#endif // SWITCH


//
// Roads
//

Road::Road()
{
	// Provide default values
	m_fWidth = 1.0f;
	m_iLanes = 0;
	m_Surface = ST_PAVED;
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
Road::Road(Road &ref)
{
	*this = ref;
//	DLine2::DLine2(ref);	// need to do this?
}

Road::~Road()
{
}

bool Road::operator==(Road &ref)
{
	return (m_fWidth == ref.m_fWidth &&
		m_iLanes == ref.m_iLanes &&
		m_Surface == ref.m_Surface &&
		m_iHwy == ref.m_iHwy &&
		m_iFlags == ref.m_iFlags);
}

void Road::SetFlag(int flag, bool value)
{
	if (value)
		m_iFlags |= flag;
	else
		m_iFlags &= ~flag;
}

bool Road::GetFlag(int flag)
{
	return (m_iFlags & flag) != 0;
}

//closet distance from target to the road
double Road::DistanceToPoint(DPoint2 target)
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

float Road::GetHeightAt(int i)
{
	return m_fHeight[i];
}

float Road::GetHeightAt(Node *node)
{
	if (m_pNode[0] == node)
		return m_fHeight[0];

	if (m_pNode[1] == node)
		return m_fHeight[1];

	//error.
	assert(false);
	return 0;
}

void Road::SetHeightAt(int i, float height)
{
	m_fHeight[i] = height;
}

void Road::SetHeightAt(Node *node, float height)
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

float Road::Length()
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

float Road::EstimateWidth(bool bIncludeSidewalk)
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

	m_pFirstRoad = NULL;
	m_pFirstNode = NULL;
}


vtRoadMap::~vtRoadMap()
{
	DeleteElements();
}

void vtRoadMap::DeleteElements()
{
	Road *nextR;
	while (m_pFirstRoad)
	{
		nextR = m_pFirstRoad->m_pNext;
		delete m_pFirstRoad;
		m_pFirstRoad = nextR;
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


int	vtRoadMap::NumRoads() const
{
	int count = 0;
	for (Road *pR = m_pFirstRoad; pR; pR = pR->m_pNext)
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
	if (NumRoads() == 0 || NumNodes() == 0)
	{
		m_bValidExtents = false;
		return;
	}

	// iterate through all elements accumulating extents
	m_extents.SetRect(1E9, -1E9, -1E9, 1E9);
	for (Road *pR = m_pFirstRoad; pR; pR = pR->m_pNext)
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
		if (pN->m_iRoads == 0)
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

	int numNodes, numRoads, i, j, nodeNum, dummy;
	Node* tmpNode;
	Road* tmpRoad;

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
	fread(&numRoads, intSize, 1, fp);

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
	for (i = 1; i <= numRoads; i++)
	{
		tmpRoad = NewRoad();
		float tmp;
		fread(&(tmpRoad->m_id), intSize, 1, fp);		//id
		fread(&(tmpRoad->m_iHwy), intSize, 1, fp);		//highway number
		fread(&(tmpRoad->m_fWidth), floatSize, 1, fp);	//width
		fread(&(tmpRoad->m_iLanes), intSize, 1, fp);	//number of lanes
		fread(&(tmpRoad->m_Surface), intSize, 1, fp);	//surface type
		fread(&(tmpRoad->m_iFlags), intSize, 1, fp);	//FLAG
		fread(&tmp, floatSize, 1, fp);		//height of road at node 0
		tmpRoad->SetHeightAt(0, tmp);
		fread(&tmp, floatSize, 1, fp);		//height of road at node 1
		tmpRoad->SetHeightAt(1, tmp);

		int size;
		fread(&size, intSize, 1, fp);    //number of coordinates making the road
		tmpRoad->SetSize(size);

		for (j = 0; j < tmpRoad->GetSize(); j++)
		{
			if (version < 1.8f)
			{
				fread(&ivalue, intSize, 1, fp);
				(*tmpRoad)[j].x = ivalue;
				fread(&ivalue, intSize, 1, fp);
				(*tmpRoad)[j].y = ivalue;
			}
			else
			{
				fread(&((*tmpRoad)[j].x), doubleSize, 1, fp);
				fread(&((*tmpRoad)[j].y), doubleSize, 1, fp);
			}
		}

		//set the end points
		fread(&nodeNum, intSize, 1, fp);
		tmpRoad->SetNode(0, pNodeLookup[nodeNum]);
		fread(&nodeNum, intSize, 1, fp);
		tmpRoad->SetNode(1, pNodeLookup[nodeNum]);

		// check for inclusion
		bool include = false;
		if (bHwy && tmpRoad->m_iHwy > 0) include = true;
		if (bPaved && tmpRoad->m_Surface == ST_PAVED) include = true;
		if (bDirt && (tmpRoad->m_Surface == ST_TRAIL ||
			tmpRoad->m_Surface == ST_2TRACK ||
			tmpRoad->m_Surface == ST_DIRT ||
			tmpRoad->m_Surface == ST_GRAVEL)) include = true;

		if (!include)
		{
			delete tmpRoad;
			continue;
		}

		// Inform the Nodes to which it belongs
		tmpRoad->GetNode(0)->AddRoad(tmpRoad);
		tmpRoad->GetNode(1)->AddRoad(tmpRoad);

		// Add to list
		AddRoad(tmpRoad);
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
		int id, numRoads;

		fread(&id, intSize, 1, fp);  //node ID
		// safety check
		if (id < 1 || id > numNodes)
		{
			fclose(fp);
			return false;
		}

		tmpNode = pNodeLookup[id];
		fread(&dummy, intSize, 1, fp);
		fread(&numRoads, intSize, 1, fp);

		//get specifics for each road at the intersection:
		for (j = 0; j < numRoads; j++)
		{
			//match road number
			IntersectionType type;
			LightStatus lStatus;
			//read in data
			fread(&id, intSize, 1, fp);  //road ID
			fread(&type, intSize, 1, fp);
			fread(&lStatus, intSize, 1, fp);
			//now figure out which roads at the node get what behavior
			id = tmpNode->FindRoad(id);
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
	Road *curRoad = GetFirstRoad();
	int numNodes = NumNodes();
	int numRoads = NumRoads();

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
	while (curRoad) {
		curRoad->m_id = i++;
		curRoad = curRoad->m_pNext;
	}
	
	curNode = GetFirstNode();
	curRoad = GetFirstRoad();

	FWrite(RMFVERSION_STRING, 11);

	// Projection
	int iUTM = m_proj.IsUTM();
	int iZone = m_proj.GetUTMZone();
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
	FWrite(&numRoads, intSize);  // number of roads
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
	while (curRoad)
	{
		float tmp;
		FWrite(&(curRoad->m_id), intSize);		//id
		FWrite(&(curRoad->m_iHwy), intSize);		//highway number
		FWrite(&(curRoad->m_fWidth), floatSize);	//width
		FWrite(&(curRoad->m_iLanes), intSize);	//number of lanes
		FWrite(&(curRoad->m_Surface), intSize);	//surface type
		FWrite(&(curRoad->m_iFlags), intSize);	//FLAG
		tmp = curRoad->GetHeightAt(0);
		FWrite(&tmp, floatSize); //height of the road at node 0
		tmp = curRoad->GetHeightAt(1);
		FWrite(&tmp, floatSize); //height of the road at node 1

		int size = curRoad->GetSize();
		FWrite(&size, intSize);//number of coordinates making the road
		for (i = 0; i < size; i++)
		{
			//coordinates that make the road
			FWrite(&curRoad->GetAt(i).x, doubleSize);
			FWrite(&curRoad->GetAt(i).y, doubleSize);
		}
		//nodes (endpoints)
		FWrite(&(curRoad->GetNode(0)->m_id), intSize);	//what road is at the end point?
		FWrite(&(curRoad->GetNode(1)->m_id), intSize);
		
		curRoad = curRoad->m_pNext;
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
		FWrite(&(curNode->m_iRoads), intSize); //node traffic behavior
		for (i = 0; i < curNode->m_iRoads; i++) {
			IntersectionType type = curNode->GetIntersectType(i);
			LightStatus lStatus = curNode->GetLightStatus(i);
			FWrite(&(curNode->GetRoad(i)->m_id), intSize);  //road ID	
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

