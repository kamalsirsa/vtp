//
// CarEngine.h
// header file for CarEngine.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ISLAND_CARENGINESH
#define ISLAND_CARENGINESH

#include "vtdata/HeightField.h"
#include "vtlib/core/Roads.h"
#include "vtlib/core/IntersectionEngine.h"

static FPoint3 XAXIS = FPoint3(1, 0, 0);
static FPoint3 YAXIS = FPoint3(0, 1, 0);
static FPoint3 ZAXIS = FPoint3(0, 0, 1);

enum CarEngineMode { NONE, STRAIGHT, CIRCLE, ROAD, PATH };

class CarEngine : public vtEngine
{
public:
	// pos is initial position
	// grid is the elevation grid
	// target_speed is the max speed car will drive at
	// wRadius is the wheel radius
	// center is center of a circle the vehicle may drive in
	// node is which node the vehicle should start driving from
	// lane is which lane to drive on.
	// road height is how high road is off the ground.
	//take target_speed in km per hour.  drives in a straight line
	CarEngine(const FPoint3 &pos, vtHeightField3d *grid, float target_speed, float wRadius);
	//drives in a circle.
	CarEngine(const FPoint3 &pos, vtHeightField3d *grid, float target_speed, float wRadius, FPoint3 center);
	//drives from the given node.  ignores pos value.  takes position from given node.
	CarEngine(const FPoint3 &pos, vtHeightField3d *grid, float target_speed, float wRadius, TNode *n, int lane=1, float roadheight=0);

	/*	finds and sets the tire variables in the model.  assumes that the tires are under a group name ending
		in "tires" and the 4 tires are the children of the group.  the 4 tire names should end with their
		location names: "front left" "front right" "rear left" "rear right"
	*/
	bool SetTires();

	void Eval();

private:
	//called to constructor for basic setup
	void SharedConstructor(const FPoint3 &pos, vtHeightField3d *grid, float target_speed, float wRadius);

	//find the next destination point when driving in a circle (simple test case.)
	void Circle(FPoint3 &next_pos, float t);

	//undo orientation of the car (make it level to horizontal plane.)
	void UndoOrientation();
	float SetOrientation();  //returns new height for car.
	//change rotation about Y axis based on current and next position.  height is average of tire points
	void SetOrientationAndHeight(FPoint3 &next_pos);

	//find the tire models of the car - to be called be SetTires.
	vtGroup* FindTires(vtGroup *model);

	//pick the first road to follow
	void PickFirstRoad();
	//pick the next road to follow.
	void PickRoad();
	//common code between PickFirstRoad and PickRoad()
	void PickNextRoad();
	//picks the next lane.
	int PickLane();

	//move car to the given vector
	void MoveCar(FPoint3 vNext);
	//spin all wheels for distance traveled.
	void SpinWheels(float dist);

	//get next point to drive toward
	FPoint3 GetNextTarget(float fCurTime);
	//turn toward coordinates, but there will be a limit on how much of a turn.
	void TurnToward(FPoint3 target, float time);
	//adjusts the speed based traffic rules, etc.
	void AdjustSpeed(float fDeltaTime);

	//calculates angle in x-z plane.
	float Angle(FPoint3 center, FPoint3 curVec, FPoint3 nextVec);

	vtHeightField3d *m_pHeightField;

	//what mode the engine is operating at.
	CarEngineMode m_eMode;

	FPoint3 m_vCurPos;  //current position
	float m_fCurRotation; //current rotation
	float m_fCurPitch;	//rotation about orientation axis
	FPoint3 m_vAxis; //orientation axis, determined by where wheels are w/ respect to ground.

	float m_fSpeed;	//maintained in meters per second (multiplied by world coordinates)
	float m_fTargetSpeed;  //target speed
	float m_fPrevTime;	//last time that eval was run on.

	//tires
	vtTransform *m_pFrontLeft;
	vtTransform *m_pFrontRight;
	vtTransform *m_pRearLeft;
	vtTransform *m_pRearRight;
	float m_fWheelRadius;
	float m_fWheelSteerRotation;

	//circle test
	FPoint3 m_vCenterPos;
	float m_fCircleRadius;

	//a setup flag - first eval doesn't run right...
	bool m_bFirstTime;

	//road following
	TNode* m_pCurNode;	//the last node the we visited.
	TNode* m_pNextNode;	//the next node we're going to visit
	TLink* m_pCurRoad;	//current road we're driving on (from m_pCurNode to m_pNextNode)
	TLink* m_pNextRoad;	//the next road to drive on (after NextNode)
	bool m_bForwards;	//true if we are driving  from node 0-1 on current road
	int m_iLane;  //lane to drive on
	IntersectionType m_iNextIntersect;	//intersection type of next node.
	float m_fStopTime;	//when we stopped
	float m_bStopped;	//how long we stopped for
	int m_iRCoord;		//current road coordinate that we are driving to
	float m_fRDistance;	//distance to next road point to follow.
	float m_fAngle;		//angle of the next road
	float m_fRoadHeight;	//height of road (offset for placement.)

	//path following stuff.
	int m_iStartNode;	//the start node
	int m_iEndNode;		//the end node of path
	int *m_iRoads;		//array of road IDs to follow
	int m_iNumRoads;	//number of roads to follow
	int m_iNextRoad;	//index to next road to follow
	bool m_bPathReverse;//when we reach end of path, want to reverse traverse it.
};

#endif
