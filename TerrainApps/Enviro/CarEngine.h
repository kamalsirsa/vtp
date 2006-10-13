//
// CarEngine.h
// header file for CarEngine.cpp
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef CARENGINEH
#define CARENGINEH

#include "vtdata/HeightField.h"
#include "vtlib/core/Roads.h"
#include "vtlib/core/IntersectionEngine.h"

enum CarEngineMode { NONE, JUST_DRIVE, FOLLOW_ROAD, FOLLOW_PATH };

class CarEngine : public vtEngine
{
public:
	// pos is initial position
	// grid is the elevation grid
	// target_speed is the max speed car will drive at
	// wRadius is the wheel radius
	// node is which node the vehicle should start driving from
	// lane is which lane to drive on.
	// road height is how high road is off the ground.
	// target_speed in km per hour.
	CarEngine(vtHeightField3d *grid, float target_speed, float wRadius, const FPoint3 &pos);
	//drives from the given node.  takes position from given node.
	CarEngine(vtHeightField3d *grid, float target_speed, float wRadius, TNode *n, int lane=1);

	/*	finds and sets the tire variables in the model.  assumes that the tires are under a group name ending
		in "tires" and the 4 tires are the children of the group.  the 4 tire names should end with their
		location names: "front left" "front right" "rear left" "rear right"
	*/
	bool FindWheelTransforms();

	void Eval();
	void IgnoreElapsedTime();

	FPoint3 GetCurPos() { return m_vCurPos; }
	DPoint2 GetEarthPos();
	void SetEarthPos(const DPoint2 &pos);
	void SetTargetSpeed(float fMeterPerSec);
	float GetRotation() { return m_fCurRotation; }
	void SetRotation(float fRot);
	void ApplyCurrentLocation(bool bAlignOnGround);

private:
	//called to constructor for basic setup
	void Init(vtHeightField3d *grid, float target_speed, float wRadius);

	//undo orientation of the car (make it level to horizontal plane.)
	void UndoOrientation();
	float SetPitch();  //returns new height for car.
	//change rotation about Y axis based on current and next position.
	//height is average of tire points
	float DetermineYawPitchAndHeight(const FPoint3 &next_pos);

	//pick the first road to follow
	void PickFirstRoad();
	//pick the next road to follow.
	void PickRoad();
	//common code between PickFirstRoad and PickRoad()
	void PickNextRoad();
	//picks the next lane.
	int PickLane();

	//move car to the given vector
	void MoveCarTo(const FPoint3 &pos);
	//spin all wheels for distance traveled.
	void SpinWheels(float dist);

	//get next point to drive toward
	FPoint3 GetNextTarget(float fCurTime);
	//turn toward coordinates, but there will be a limit on how much of a turn.
	void TurnToward(const FPoint3 &target, float time);
	//adjusts the speed based traffic rules, etc.
	void AdjustSpeed(float fDeltaTime);

	vtHeightField3d *m_pHeightField;

	//what mode the engine is operating at.
	CarEngineMode m_eMode;

	FPoint3 m_vCurPos;		// current position
	float m_fCurRotation;	// rotation around Y axis(Yaw)
	float m_fCurPitch;		// rotation around orientation axis (Pitch)
	FPoint3 m_vAxis;		// orientation axis, determined by where wheels are w/respect to ground

	float m_fSpeed;			// speed in meters per second
	float m_fTargetSpeed;	// target speed
	float m_fPrevTime;		// last time that eval was run on.

	//tires
	vtTransform *m_pFrontLeft;
	vtTransform *m_pFrontRight;
	vtTransform *m_pRearLeft;
	vtTransform *m_pRearRight;
	float m_fWheelRadius;
	float m_fWheelSteerRotation;

	//circle test
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
	float m_fAngle;		//angle of the next road

	//path following stuff.
	int m_iStartNode;	//the start node
	int m_iEndNode;		//the end node of path
	int *m_iRoads;		//array of road IDs to follow
	int m_iNumRoads;	//number of roads to follow
	int m_iNextRoad;	//index to next road to follow
	bool m_bPathReverse;//when we reach end of path, want to reverse traverse it.
};

#endif // CARENGINEH
