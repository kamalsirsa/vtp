//
// RoadMap.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ROADMAPH
#define ROADMAPH

#include "DLG.h"

#define RMFVERSION_STRING "RMFFile1.8"
#define RMFVERSION_CURRENT 1.8f
#define RMFVERSION_SUPPORTED 1.7f	// oldest supported version

#define SWITCH 0

enum SurfaceType {
	ST_NONE,
	ST_GRAVEL,
	ST_TRAIL,
	ST_2TRACK,
	ST_DIRT,
	ST_PAVED,
	ST_RAILROAD
};

//WARNING!!!! Do NOT change the order of the items in the enumeration.
enum IntersectionType {
	IT_NONE,	// uncontrolled
	IT_LIGHT,	// controlled intersection with at least one traffic light
	IT_STOPSIGN,  // controlled intersection with at least one stop sign
};

enum LightStatus {
	LT_INVALID,
	LT_RED,
	LT_YELLOW,
	LT_GREEN
};

// road flags
#define RF_SIDEWALK	0x8000000
#define RF_PARKING	0x4000000
#define RF_MARGIN	0x2000000
#define RF_FORWARD	0x0800000	// true if traffic flows from node 0 to 1
#define RF_REVERSE	0x0400000	// true if traffic flows from node 1 to 0
// the following are for temporary, runtime use
#define RF_HIT		0x0000001

// Typical, default values for widths, in meters
#define SIDEWALK_WIDTH		1.5f
#define CURB_HEIGHT			0.15f
#define MARGIN_WIDTH		1.6f
#define LANE_WIDTH			3.3f
#define PARKING_WIDTH		LANE_WIDTH

//incomplete code for a node switch that tells which lanes connect to other
//lanes on other roads.
#if SWITCH
// Info in a node that directs traffic flow.
// Tells which lanes can turn into which lanes.
class LaneIO {
public:
	LaneIO();
	~LaneIO();
	void AddChoice(class Road *road, int lane);
	int GetRoadNum(class Road *road);
	//number of choices to choose from reaching end of this lane
	int m_iChoices;
	//possible roads
	class Road **m_pRoads;
	//corresponding lane to road choice
	int *m_iLane;
};

class RoadIO {
public:
	RoadIO(int lanes);
	~RoadIO();
	int m_iLanes;
	LaneIO **m_pLaneIO;
};

class Switch
{
public:
	Switch(int r, class Road **roads);
	~Switch();
	//return possible road choices given current road and lane.
	Road** RoadChoices(Road *curRoad, int curLane);
	//return possible lane to follow given destination road and current road and lane
	int LaneChoice(Road *curRoad, int curLane, Road* destRoad);
	int LaneChoice(Road *curRoad, int curLane, int destRoad);

	int m_iRoads;
	//roads
	class Road **m_pRoads;  //points to node's roads
	//structure to hold lanes associated with the road
	RoadIO **m_pRoadIO;
};
#endif //SWITCH

//
// A place where 2 or more roads meet
//
class Node
{
public:
	Node();
	~Node();

	// comparison
	bool operator==(Node &ref);

	//copies internal variables from given node.
	void Copy(Node* node);

	class Road *GetRoad(int n);
	int FindRoad(int roadID);			//returns internal number of road with given ID.  -1 if not found.
	void AddRoad(class Road *pR);		//adds a road to the node
	void DetachRoad(class Road *pR);	//detaches the road from the node.
	double DistanceToPoint(DPoint2 target);  //distance from a point to the node
	void DetermineRoadAngles();			//resulting angles > 0
	void SortRoadsByAngle();			//sorts the internal roads by angle.
	DPoint2 find_adjacent_roadpoint2d(Road *pR);  //returns the 2nd point on the road from the node.

	//sets intersection type for node.  returns false if road not found
	bool SetIntersectType(Road *road, IntersectionType type);
	bool SetIntersectType(int roadNum, IntersectionType type);  //roadNum is internal index within the node
	IntersectionType GetIntersectType(Road *road);	//returns the intersection type of given road
	IntersectionType GetIntersectType(int roadNum); //returns the intersection type of given road index (not ID)
	LightStatus GetLightStatus(Road *road);			//returns the light status of given road
	LightStatus GetLightStatus(int roadNum);		//returns the light status of given road index (not ID)
	bool SetLightStatus(Road *road, LightStatus light); //sets the light status of given road
	bool SetLightStatus(int roadNum, LightStatus light); //sets the light status of given road index (not ID)

	bool HasLights();
	bool IsControlled();	// true if any stopsigns or stoplights

	//adjust the light relationship of the roads at the node (if the intersection is has a signal light.)
	void AdjustForLights();

#if SWITCH
	void SetupSwitch();
	//i is index to current road
	void DetermineOptions(int i, int *in,
						int &right, int &straight, int &left, int &uturn);
	Switch *m_pSwitch;
#endif //SWITCH

	DPoint2 m_p;	// utm coordinates of center
	int m_iRoads;	// number of roads meeting here
	int m_id;		// only used for reading from DLG/RMF

	// angle of each road, not initialized till SortRoadsByAngle is called
	float *m_fRoadAngle;

	Node *m_pNext;
protected:
	IntersectionType *m_IntersectTypes;	//intersection types of the roads at this node.
	LightStatus *m_Lights;  //lights of the roads at this node.
	class Road **m_r;  //array of roads that intersect this node.
};


//
// a series of points, connecting one node to another
//
class Road : public DLine2
{
public:
	Road();
	Road(Road &ref);
	~Road();

	// comparison
	bool operator==(Road &ref);

	void SetNode(int n, Node *pNode) { m_pNode[n] = pNode; }
	Node *GetNode(int n) { return m_pNode[n]; }

	// closest distance from target to the road
	double DistanceToPoint(DPoint2 target);

	// is the road a loop?
	bool IsLoop() { return (m_pNode[0] == m_pNode[1]); }

	// accessors for flag properties
	void SetFlag(int flag, bool value);
	bool GetFlag(int flag);

	// Height (unused; these methods may be redesigned or removed)
	float GetHeightAt(int i);		// height at node (0 or 1).
	float GetHeightAt(Node *node);	// height at node
	void SetHeightAt(int i, float height);	//set the height at a node (0 or 1)
	void SetHeightAt(Node *node, float height);	//set the height at a node

	// Return length of road centerline.
	float Length();
	float EstimateWidth(bool bIncludeSidewalk = true);

	float	m_fWidth;		// road width in meters
	int		m_iLanes;		// number of lanes
	SurfaceType m_Surface;
	int		m_iHwy;			// highway number: -1 for normal roads
	int		m_id;			// only used for reading from DLG
	Road	*m_pNext;		// the next Road, if roads are maintained in link list form
	int		m_iFlags;		// a flag to be used to holding any addition info.

protected:
	Node	*m_pNode[2];	// "from" and "to" nodes
	float	m_fHeight[2];
};

typedef Road *RoadPtr;
typedef Node *NodePtr;

#define intSize 4
#define floatSize 4
#define doubleSize 8

class vtRoadMap
{
public:
	vtRoadMap();
	~vtRoadMap();

	Node *FindNodeByID(int id);
	void DeleteElements();
	DRECT GetMapExtent();		// get the geographical extent of the road map
	void ComputeExtents();

	int		NumRoads() const;	// returns number of roads in the road map
	int		NumNodes() const;	// returns number of nodes in the road map

	Road	*GetFirstRoad() { return m_pFirstRoad; }
	Node	*GetFirstNode() { return m_pFirstNode; }

	virtual Node *NewNode() { return new Node; }
	virtual Road *NewRoad() { return new Road; }

	void	AddNode(Node *pNode)
	{
		pNode->m_pNext = m_pFirstNode;
		m_pFirstNode = pNode;
	}
	void	AddRoad(Road *pRoad)
	{
		pRoad->m_pNext = m_pFirstRoad;
		m_pFirstRoad = pRoad;
	}

	// cleaning function: remove unused nodes, return the number removed
	int RemoveUnusedNodes();

	bool ReadRMF(const char *filename, bool bHwy, bool bPaved, bool bDirt);
	bool WriteRMF(const char *filename);

	vtProjection &GetProjection() { return m_proj; }

protected:
	DRECT	m_extents;			// the extent of the roads in the RoadMap
	bool	m_bValidExtents;	// true when extents are computed

	Road	*m_pFirstRoad;
	Node	*m_pFirstNode;

	vtProjection	m_proj;
};

#endif
