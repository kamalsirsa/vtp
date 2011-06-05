//
// CarEngine.cpp
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "CarEngine.h"
#include "vtdata/vtLog.h"

using namespace std;

// acceleration in meters per second^2 (for the car.)
#define ACCELERATION 4.0f
#define ACCEL (ACCELERATION*1000/60/60)

static FPoint3 XAXIS = FPoint3(1, 0, 0);
static FPoint3 YAXIS = FPoint3(0, 1, 0);


///////////////////////////////////////////////////////////////////////
// Helpers

// Difference of 2 angles in radians.  value will be between PI and -PI.
float angleDifference(float a, float b)
{
	float val = a - b;
	while (val > PIf)
		val -= PI2f;
	while (val < -PIf)
		val += PI2f;
	return val;
}

// Adjust angle (in radians) so that return value will be between PI and -PI.
float angleNormal(float val)
{
	while (val > PIf)
		val -= PI2f;
	while (val < -PIf)
		val += PI2f;
	return val;
}

// Simple 2D world distance (XZ plane)
float Distance2D(const FPoint3 &p, const FPoint3 &q)
{
	return sqrt((q.z - p.z)*(q.z - p.z) + (q.x - p.x)*(q.x - p.x));
}

// Calculate angle between two 3D vectors, in XZ plane.
float Angle(const FPoint3 &center, const FPoint3 &p1, const FPoint3 &p2)
{
	FPoint3 v1 = p1-center, v2 = p2-center;
	if (v1 == v2)
		return 0.0f;

	//dot product:  a.b = |a||b|cos(THETA)
	return acosf(v1.Dot(v2)/(v1.Length()*v2.Length()));
}


///////////////////////////////////////////////////////////////////////
// CarEngine class

//
// Setup engine to drive freely.
// Target speed in kilometers per hour
//
CarEngine::CarEngine(vtHeightField3d *grid, float target_speed,
					 float wRadius, const FPoint3 &pos)
{
	Init(grid, target_speed, wRadius);
	m_pCurNode = NULL;
	m_vCurPos = pos;
	m_eMode = JUST_DRIVE;
	m_fSpeed = m_fTargetSpeed;
}

//
// Setup engine to drive on roads, starting from node n.
// Takes position from given node.
//
CarEngine::CarEngine(vtHeightField3d *grid, float target_speed,
					 float wRadius, TNode *n, int lane)
{
	Init(grid, target_speed, wRadius);
	m_pCurNode = n;
	m_vCurPos = ((NodeGeom*)n)->m_p3;

	m_iLane = lane-1;
	PickFirstRoad();
	m_eMode = FOLLOW_ROAD;
}

// Shared constructor
void CarEngine::Init(vtHeightField3d *grid, float target_speed, float wRadius)
{
	m_pHeightField = grid;

	//convert speed to meters/second (from km/h)
	m_fTargetSpeed = target_speed * 1000/60/60;
	m_fSpeed = 0;

	m_fWheelSteerRotation = 0;
	m_fCurRotation = PID2f;
	m_fCurPitch = 0;
	m_vAxis = XAXIS;
	m_fWheelRadius = wRadius;

	m_bFirstTime = true;

	m_fPrevTime = 0.0f;		// we will use engine time (t) from now on

	m_fCircleRadius = 0;
	m_eMode = NONE;

	m_bForwards = true;

	m_iLane = 0;
	m_fStopTime = 0.0f;
	m_bStopped = false;

	m_iRCoord = 0;
	m_pCurNode = NULL;
	m_pNextNode = NULL;
	m_pCurRoad = NULL;
	m_pNextRoad = NULL;
}

// Evaluate the car engine.
void CarEngine::Eval()
{
	float t = vtGetTime();
	float fDeltaTime = t - m_fPrevTime;

	// Don't get too jumpy on low framerate, such as when the program is paused
	if (fDeltaTime > 1.0f)
		fDeltaTime = 1.0f;

	vtTransform *pTarget = dynamic_cast<vtTransform*> (GetTarget());
	if (!pTarget)
		return;

	//find Next location
	FPoint3 vNext;
	switch (m_eMode)
	{
	case NONE:	//go nowhere.
		vNext = m_vCurPos;
		MoveCarTo(vNext);
		break;
	case JUST_DRIVE:
		//go straight.  try to match speed.
		vNext.x = m_vCurPos.x + fDeltaTime*m_fSpeed*cosf(m_fCurRotation);
		vNext.z = m_vCurPos.z - fDeltaTime*m_fSpeed*sinf(m_fCurRotation);
		MoveCarTo(vNext);
		break;
	case FOLLOW_PATH:
	case FOLLOW_ROAD:
		if (!m_pCurRoad)
			return;  //safety check

		if (m_bFirstTime)
		{
			vNext = ((LinkGeom*)m_pCurRoad)->m_centerline[m_iRCoord];
			pTarget->SetTrans(vNext);
			m_vCurPos = vNext;
			m_bFirstTime = false;
			break;
		}
		//find where the road is
		FPoint3 target = GetNextTarget(t);

		//if we are not stopped, then move
		if (!m_bStopped)
		{
			//adjust vehicle speed.
			AdjustSpeed(fDeltaTime);

			//turn appropriately - but there is a limit on how much the car can turn.
			TurnToward(target, fDeltaTime);

			// slow down on curves
			m_fSpeed *= (1 - fabs(m_fWheelSteerRotation)/2);

			//move forward based on speed.
			FPoint3 delta;
			delta.x = fDeltaTime*m_fSpeed * cosf (m_fCurRotation);
			delta.y = 0.0f;
			delta.z = fDeltaTime*m_fSpeed * -sinf (m_fCurRotation);

			vNext = m_vCurPos + delta;

			//VTLOG("Rotation: %.3f\tDelta: %.3f %.3f %.3f\n", m_fCurRotation,
			//	delta.x,delta.y,delta.z);
			//VTLOG("curpos: %.1f, %.1f, %.1f\tnext: %.1f, %.1f, %.1f\n",
			//	m_vCurPos.x,m_vCurPos.y,m_vCurPos.z, vNext.x,vNext.y,vNext.z);
			//VTLOG("target: %.1f, %.1f, %.1f\tdist: %.2f\n",
			//	target.x, target.y, target.z, Distance2D(m_vCurPos,target));

			MoveCarTo(vNext);
		}
		break;
	}
	// spin the wheels, adjusted for speed.
	if (!m_bStopped)
		SpinWheels(fDeltaTime*m_fSpeed/m_fWheelRadius);

	m_fPrevTime = t;
}

void CarEngine::IgnoreElapsedTime()
{
	m_fPrevTime = vtGetTime();
}

void CarEngine::SetTargetSpeed(float fMetersPerSec)
{
	m_fTargetSpeed = fMetersPerSec;
	m_fSpeed = fMetersPerSec;
}

DPoint2 CarEngine::GetEarthPos()
{
	// convert terrain to earth coords
	DPoint3 d3;
	g_Conv.ConvertToEarth(m_vCurPos, d3);
	return DPoint2(d3.x, d3.y);
}

void CarEngine::SetEarthPos(const DPoint2 &pos)
{
	// convert earth to terrain coords
	DPoint3 d3(pos.x, pos.y, 0);
	g_Conv.ConvertFromEarth(d3, m_vCurPos);

	ApplyCurrentLocation(true);
}

void CarEngine::SetRotation(float fRot)
{
	m_fCurRotation = fRot;
	ApplyCurrentLocation(true);
}

void CarEngine::ApplyCurrentLocation(bool bAlignOnGround)
{
	vtTransform *car = dynamic_cast<vtTransform*> (GetTarget());
	if (!car)
		return;

	// re-settle vehicle on terrain surface
	car->Identity();

	car->SetTrans(m_vCurPos);

	// Angle is measure from +X, but our car's "forward" is -Z.  That's a difference
	//  in angle of PI/2 between them.
	car->RotateLocal(FPoint3(0,1,0), m_fCurRotation-PID2f);

	if (bAlignOnGround)
	{
		m_vCurPos.y = SetPitch();
		car->SetTrans(m_vCurPos);
	}
}

//
// Determines the pitch of the car, based on tire positions.
// Returns height of the center of the car.
//
float CarEngine::SetPitch()
{
	vtTransform *car = dynamic_cast<vtTransform*> (GetTarget());
	if (!car)
		return 1.0f;

	FSphere fL, fR, rL, rR;

	// find tire locations in current orientation
	m_pFrontLeft->GetBoundSphere(fL, true);
	m_pFrontRight->GetBoundSphere(fR, true);
	m_pRearLeft->GetBoundSphere(rL, true);
	m_pRearRight->GetBoundSphere(rR, true);

	m_pHeightField->FindAltitudeAtPoint(fL.center, fL.center.y, false, CE_ROADS);
	m_pHeightField->FindAltitudeAtPoint(fR.center, fR.center.y, false, CE_ROADS);
	m_pHeightField->FindAltitudeAtPoint(rL.center, rL.center.y, false, CE_ROADS);
	m_pHeightField->FindAltitudeAtPoint(rR.center, rR.center.y, false, CE_ROADS);

	// find midpoints between the tires.
	FPoint3 rM, fM;
	rM = (rL.center + rR.center)/2;
	fM = (fL.center + fR.center)/2;

	FPoint3 back_side = rR.center - rL.center;
	FPoint3 left_side = fL.center - rL.center;
	back_side.Normalize();
	left_side.Normalize();

	// vNormal the upwards vector
	FPoint3 vNormal = back_side.Cross(left_side);
	vNormal.Normalize();

	// new pitch
	FPoint3 horiz = left_side;
	horiz.y = 0;
	float xz = horiz.Length();

	// tan(pitch) = y / xz, so pitch = atan(y/xz);
	m_fCurPitch = atanf(left_side.y / xz);

	car->RotateLocal(FPoint3(1,0,0), m_fCurPitch);

	// new roll
	horiz = back_side;
	horiz.y = 0;
	xz = horiz.Length();

	// tan(roll) = y / xz, so roll = atan(y/xz);
	float fRoll = atanf(back_side.y / xz);

	car->RotateLocal(FPoint3(0,0,1), fRoll);

	// return height of midpoint of all wheels.
	return (fM.y+rM.y)/2;
}

//sets orientation of car.  next_pos is modified to be new location.
float CarEngine::DetermineYawPitchAndHeight(const FPoint3 &next_pos)
{
	vtTransform *car = dynamic_cast<vtTransform*> (GetTarget());
	if (!car)
		return 0.0f;

	FPoint3 tempVec;
	float deltax,deltaz;

	deltax = next_pos.x - m_vCurPos.x;
	deltaz = next_pos.z - m_vCurPos.z;
	if (deltax != 0.0f || deltaz != 0.0f)
	{
		// Turn car based on how the steering wheel is turned
		m_fCurRotation += m_fWheelSteerRotation;
		m_fCurRotation = angleNormal(m_fCurRotation);

		//VTLOG("Wheel: %-f\t m_CurRot: %f \n", m_fWheelSteerRotation, m_fCurRotation);
#if 0
		// Yaw the wheel objects to show wheel angle?  Don't bother for now,
		//  it's too subtle an effect.
		FPoint3 trans;
		trans = m_pFrontLeft->GetTrans();
		m_pFrontLeft->Identity();
		m_pFrontLeft->Rotate2(YAXIS, m_fWheelSteerRotation);
		m_pFrontLeft->Translate1(trans);

		trans = m_pFrontRight->GetTrans();
		m_pFrontRight->Identity();
		m_pFrontRight->Rotate2(YAXIS, m_fWheelSteerRotation);
		m_pFrontRight->Translate1(trans);
#endif
	}
	// Angle is measure from +X, but our car's "forward" is -Z.  That's a difference
	//  in angle of PI/2 between them.
	car->RotateLocal(FPoint3(0,1,0), m_fCurRotation-PID2f);

	// Modify the orientation of the car to match the terrain
	//  points of the tires
	float elev;
	if (m_bFirstTime)
	{
		m_pHeightField->FindAltitudeAtPoint(m_vCurPos, elev, false, CE_ROADS);
		m_bFirstTime = false;
	}
	else
		elev = SetPitch();

	return elev;
}


/*
 Finds and sets the tire variables in the model.  assumes that the tires are
 under a group name ending in "tires" and the 4 tires are the children of the
 group.  the 4 tire names should end with their location names: "front left"
 "front right" "rear left" "rear right"
*/
bool CarEngine::FindWheelTransforms()
{

	vtTransform *car = dynamic_cast<vtTransform*> (GetTarget());
	if (!car)
		return false;
#if 0
	osg::Node *tModel;
	tModel = FindDescendent(car, "front_left_xform");
	if (tModel)
	{
		m_pFrontLeft = new vtTransform;
		m_pFrontLeft->addChild(tModel);
	}

	tModel = FindDescendent(car, "front_right_xform");
	if (tModel)
	{
		m_pFrontRight = new vtTransform;
		m_pFrontRight->addChild(tModel);
	}

	tModel = FindDescendent(car, "rear_left_xform");
	if (tModel)
	{
		m_pRearLeft = new vtTransform;
		m_pRearLeft->addChild(tModel);
	}

	tModel = FindDescendent(car, "rear_right_xform");
	if (tModel)
	{
		m_pRearRight = new vtTransform;
		m_pRearRight->addChild(tModel);
	}
#endif
	return true;
}

//pick the first road to drive on.
void CarEngine::PickFirstRoad()
{
	int i = 0;

	// safety check
	if (!m_pCurNode)
	{
		m_pCurRoad = NULL;
		m_pNextNode = NULL;
		m_pNextRoad = NULL;
		return;
	}
	if (m_eMode == FOLLOW_PATH)
	{
		//pick road based on path.
		int roadID = m_iRoads[m_iNextRoad];
		for (i = 0; i < m_pCurNode->m_iLinks; i++)
		{
			TLink *r = m_pCurNode->GetLink(i);
			if (r->m_id == roadID)
			{
				//found road.
				//determine next road to follow.
				if (!m_bPathReverse)
				{
					if (m_iNextRoad < m_iNumRoads - 1)
						m_iNextRoad++;
					else
						m_bPathReverse = true;
				}
				else
				{
					if (m_iNextRoad > 1)
						m_iNextRoad--;
					else
						m_bPathReverse = false;
				}
				break;
			}
		}
	}
	else
	{
		//road following.
		//make sure car can go in the direction of the road.
		for (i = 0; i < m_pCurNode->m_iLinks; i++)
		{
			TLink *r = m_pCurNode->GetLink(i);
			if ((r->m_iFlags & RF_FORWARD && r->GetNode(0) == m_pCurNode) ||
				(r->m_iFlags & RF_REVERSE && r->GetNode(1) == m_pCurNode))
			{
				break;
			}
		}
	}

	//the first road to follow.
	m_pCurRoad = m_pCurNode->GetLink(i);
	m_bForwards = (m_pCurRoad->GetNode(0) == m_pCurNode);

	//determine what's the next intersect type (so we know whether to slow down or not.)
	if (m_bForwards)
	{
		m_pNextNode = m_pCurRoad->GetNode(1);
		int iLinkNum = m_pNextNode->GetLinkNum(m_pCurRoad, false);	// ends there
		m_iNextIntersect = m_pNextNode->GetIntersectType(iLinkNum);
		//reset coord index
		m_iRCoord = 0;
		//m_iLane = 0;
	}
	else
	{
		m_pNextNode = m_pCurRoad->GetNode(0);
		int iLinkNum = m_pNextNode->GetLinkNum(m_pCurRoad, true);	// begins there
		m_iNextIntersect = m_pNextNode->GetIntersectType(iLinkNum);
		//reset coord index
		m_iRCoord = m_pCurRoad->GetSize()-1;
		//m_iLane = m_pCurRoad->m_iLanes - 1;
	}

	//what's the next road?
	PickNextRoad();
}

//gets the next road (which is actually already determined.)
//we have to look one road ahead so that we know if we are going to make a turn later.
void CarEngine::PickRoad()
{
	// safety check
	if (!m_pCurNode)
	{
		m_pCurRoad = NULL;
		m_pNextNode = NULL;
		m_pNextRoad = NULL;
		return;
	}

	//VTLOG("New Road: %i", m_pNextRoad->m_id);
	//figure out which lane to follow.
	/*
	m_iLane = m_pNextNode->m_pSwitch->LaneChoice(m_pCurRoad, m_iLane, m_pNextRoad);
	if (m_iLane < 0) {
		m_iLane = 0;
	}
	*/
	m_pCurRoad = m_pNextRoad;
	m_pCurNode = m_pNextNode;

	//the first road to follow.
	m_bForwards = (m_pCurRoad->GetNode(0) == m_pCurNode);

	//determine what's the next intersect type (so we know whether to slow down or not.)
	if (m_bForwards)
	{
		m_pNextNode = m_pCurRoad->GetNode(1);
		int iLinkNum = m_pNextNode->GetLinkNum(m_pCurRoad, false);	// ends there
		m_iNextIntersect = m_pNextNode->GetIntersectType(iLinkNum);
		//reset coord index
		m_iRCoord = 0;
	}
	else
	{
		m_pNextNode = m_pCurRoad->GetNode(0);
		int iLinkNum = m_pNextNode->GetLinkNum(m_pCurRoad, true);	// begins there
		m_iNextIntersect = m_pNextNode->GetIntersectType(iLinkNum);
		//reset coord index
		m_iRCoord = m_pCurRoad->GetSize()-1;
	}

	//get the next road to follow.
	PickNextRoad();
}

//picks the next road.
void CarEngine::PickNextRoad()
{
	int i=0;
	if (m_eMode == FOLLOW_PATH)
	{
		//pick road based on path.
		int roadID = m_iRoads[m_iNextRoad];
		for (i = 0; i < m_pNextNode->m_iLinks; i++)
		{
			TLink *r = m_pNextNode->GetLink(i);
			if (r->m_id == roadID)
			{
				//found road.
				//determine next road to follow.
				if (!m_bPathReverse)
				{
					if (m_iNextRoad < m_iNumRoads - 1)
						m_iNextRoad++;
					else
						m_bPathReverse = true;
				}
				else
				{
					if (m_iNextRoad > 1)
						m_iNextRoad--;
					else
						m_bPathReverse = false;
				}
				break;
			}
		}
	}
	else
	{
		//select the next road to follow.
		//find index for current road
		int r_index= m_pNextNode->GetLinkNum(m_pCurRoad, !m_bForwards);
		//wrap around
		i = (r_index+1) % m_pNextNode->m_iLinks;

		int temp_i = -1;
		//find next road available with traffic going out from the node.
		for (i = 1; i < m_pNextNode->m_iLinks; i++)
		{
			//wrap around
			temp_i = (r_index + i)% m_pNextNode->m_iLinks;
			TLink *r = m_pNextNode->GetLink(temp_i);
			if ((r->m_iFlags & RF_FORWARD && r->GetNode(0) == m_pNextNode)
				||
				(r->m_iFlags & RF_REVERSE && r->GetNode(1) == m_pNextNode))
			{
				i = temp_i;
				break;
			}
		}
	}
	if (i < m_pNextNode->m_iLinks)
		m_pNextRoad = m_pNextNode->GetLink(i);
	else
		m_pNextRoad = m_pCurRoad;

	/*
	now that we know the next road, how sharp of an angle do we have to turn to
	transition from the current road to the next road?
	we only look at the 2D case.
	*/
	FPoint3 center(((NodeGeom*)m_pNextNode)->m_p3.x,
					0,
					((NodeGeom*)m_pNextNode)->m_p3.z);
	int index;
	if (m_bForwards)
		index = m_pCurRoad->GetSize()-2;
	else
		index = 1;

	FPoint3 curVec(((LinkGeom*)m_pCurRoad)->m_centerline[index].x,
					0,
					((LinkGeom*)m_pCurRoad)->m_centerline[index].z);

	if (m_pNextRoad->GetNode(0) == m_pNextNode)
		//we're going forward
		index = 1;
	else
		//going from node 1 to 0
		index = m_pNextRoad->GetSize()-2;

	FPoint3 nextVec(((LinkGeom*)m_pNextRoad)->m_centerline[index].x,
					0,
					((LinkGeom*)m_pNextRoad)->m_centerline[index].z);

	//find the angle between curVec and nextVec
	m_fAngle = fabsf(Angle(center, curVec, nextVec));
	//VTLOG("angle:%f", m_fAngle);
}

//a simple lane picking algorithm
int CarEngine::PickLane()
{
	if (m_bForwards)
		return m_iLane;
	else
		return m_pCurRoad->m_iLanes - m_iLane -1;
}

void CarEngine::MoveCarTo(const FPoint3 &pos)
{
	vtTransform *pTarget = dynamic_cast<vtTransform*> (GetTarget());
	if (!pTarget)
		return;

	// Place the car in the XZ plane
	pTarget->Identity();
	pTarget->SetTrans(pos);

	// Rotate (yaw) body to head toward vNext location, pitch it based on tire
	//  locations, and determine overall height of vehicle.
	// Steering of wheels is done in here too.
	float elev = DetermineYawPitchAndHeight(pos);

	// New position is now current
	m_vCurPos = pos;
	m_vCurPos.y = elev;
	pTarget->SetTrans(m_vCurPos);
}

//spin the wheels base on how much we've driven
void CarEngine::SpinWheels(float dist)
{
	m_pFrontLeft->RotateLocal(XAXIS, -dist);
	m_pFrontRight->RotateLocal(XAXIS, -dist);
	m_pRearLeft->RotateLocal(XAXIS, -dist);
	m_pRearRight->RotateLocal(XAXIS, -dist);
}

//get next point to drive toward
FPoint3 CarEngine::GetNextTarget(float fCurTime)
{
	int lane = PickLane();

	//VTLOG("GetNextTarget: %i of %i\n", m_iRCoord, m_pCurRoad->GetSize());

	const FLine3 &lanepoints = ((LinkGeom*)m_pCurRoad)->m_Lanes[lane];
	FPoint3 nextPoint = lanepoints.GetAt(m_iRCoord);

	//if we're stopped, figure out if we can move!
	if (m_bStopped)
	{
		if (m_iNextIntersect == IT_STOPSIGN)
		{
			//have we waited long enough?
			//VTLOG(" stopped at %f.  it's now %f\n", m_fStopTime, fCurTime);
			if (fCurTime - m_fStopTime > 3.0f) //stop for three seconds
			{
				m_bStopped = false;
				PickRoad();
				nextPoint = ((LinkGeom*)m_pCurRoad)->m_Lanes[lane].GetAt(m_iRCoord);
			}
			else
				return nextPoint;
		}
		if (m_iNextIntersect == IT_LIGHT)
		{
			//go only if green
			// if we are moving 'forward', then our current link ends at the next node
			// if we are moving 'backward', then our current link begins at the next node
			bool bStart = !m_bForwards;
			int iLinkNum = m_pNextNode->GetLinkNum(m_pCurRoad, bStart);
			if (m_pNextNode->GetLightStatus(iLinkNum) == LT_GREEN)
			{
				m_bStopped = false;
				PickRoad();
				nextPoint = ((LinkGeom*)m_pCurRoad)->m_Lanes[lane].GetAt(m_iRCoord);
			}
			else
				return nextPoint;
		}
	}

	int endIndex;
	if (m_bForwards)
		endIndex = m_pCurRoad->GetSize() -1;
	else
		endIndex = 0;

	//if we're getting close, look at the next point to reach.
	float dist = Distance2D(nextPoint, m_vCurPos);
	float threshold = 2.0f;
	if (dist < threshold)
	{
		VTLOG("Reached target, dist=%f, threshold %f\n", dist, threshold);
		//assume that we've past target, get next target.
		bool newroad = false;
		if (m_bForwards)
		{
			m_iRCoord++;
			if (m_iRCoord == (int) m_pCurRoad->GetSize())
				newroad = true;
		}
		else
		{
			m_iRCoord--;
			if (m_iRCoord < 0)
				newroad = true;
		}
		if (newroad)
		{
			bool bStart = !m_bForwards;
			int iLinkNum = m_pNextNode->GetLinkNum(m_pCurRoad, bStart);

			//out of coords, need to look at next road.
			if (m_iNextIntersect == IT_STOPSIGN ||
				(m_iNextIntersect == IT_LIGHT &&
				m_pNextNode->GetLightStatus(iLinkNum) == LT_RED))
			{
				m_bStopped = true;
				m_fStopTime = fCurTime;
				VTLOG1("Stopped!\n");
				//do NOT pick a new road just yet?
				return FPoint3(0,0,0);
			}
			else
				PickRoad();
		}
		lane = PickLane();
		nextPoint = ((LinkGeom*)m_pCurRoad)->m_Lanes[lane].GetAt(m_iRCoord);
		VTLOG(" nextPoint %.1f %.1f %.1f\n", nextPoint.x, nextPoint.y, nextPoint.z);
	}

	return nextPoint;
}

//turn toward coordinates, but there will be a limit on how much of a turn.
void CarEngine::TurnToward(const FPoint3 &target, float time)
{
	float deltax = target.x - m_vCurPos.x;
	float deltaz = target.z - m_vCurPos.z;
	float desired_angle = atan2f(-deltaz,deltax);

	// Restrict how much the car will turn per sec.
	float diff = angleDifference(desired_angle, m_fCurRotation);
	m_fWheelSteerRotation = diff/20.0f;
}

#define ANGLETOLERANCE PI2f/3

void CarEngine::AdjustSpeed(float fDeltaTime)
{
	if (m_bStopped)
	{
		m_fSpeed = 0;
		return;
	}

	bool bStart = !m_bForwards;
	int iLinkNum = m_pNextNode->GetLinkNum(m_pCurRoad, bStart);

	//calculate distance to go.
	FPoint3 endOfRoad = ((NodeGeom*)m_pNextNode)->m_p3;

	float distToGo = Distance2D(endOfRoad, m_vCurPos);
	switch (m_iNextIntersect)
	{
	case IT_NONE:
		//if the turn up ahead is too steep, slow down
		if (m_fAngle < ANGLETOLERANCE && distToGo < (m_fSpeed * m_fSpeed) / (ACCEL))
		{
			//getting close.  slow down.
			m_fSpeed -= (m_fSpeed*m_fSpeed)/distToGo*fDeltaTime;
			//VTLOG("dead end");
		}
		else if (m_fSpeed < m_fTargetSpeed)
		{
			// plenty of room left, so speed up
			m_fSpeed += ACCEL*fDeltaTime;
			//VTLOG("vroom!");
		}
		break;
	case IT_LIGHT:
		//adjust speed based on light condition.
		switch (m_pNextNode->GetLightStatus(iLinkNum))
		{
		case LT_RED:
			//stop if we're getting close
			//VTLOG("RED");
			if (distToGo < (m_fSpeed * m_fSpeed) / (ACCEL))
			{
				//getting to close.  slow down.  (stop ASAP!)
				m_fSpeed -= (m_fSpeed*m_fSpeed)/distToGo*fDeltaTime;
				//VTLOG("Red: slowing down");
			}
			else if (m_fSpeed < m_fTargetSpeed)
				m_fSpeed += ACCEL*fDeltaTime;
			break;
		case LT_YELLOW:
			//VTLOG("YELLOW");
			//can we stop in time?  if not, plow on through
			if (distToGo > m_fSpeed * (LT_SWITCH_TIME2) &&
				distToGo < (.5f *m_fSpeed * m_fSpeed) / (ACCEL))
			{
				//slow down.
				//VTLOG("Yellow: slowing down");
				m_fSpeed -= ACCEL*fDeltaTime;
			}
			else if (m_fSpeed < m_fTargetSpeed)
				m_fSpeed += ACCEL*fDeltaTime;
			break;
		case LT_GREEN:
			//VTLOG("GREEN");
			//if the turn up ahead is too much, slow down
			if (m_fAngle < ANGLETOLERANCE && distToGo < (m_fSpeed * m_fSpeed) / (ACCEL))
			{
				//getting to close.  slow down.
				m_fSpeed -= (m_fSpeed*m_fSpeed)/distToGo*fDeltaTime;
			}
			else if (m_fSpeed < m_fTargetSpeed)
			{
				//speed up if necessary
				m_fSpeed += ACCEL*fDeltaTime;
				//VTLOG("Green: speeding up");
			}
			break;
		}
	case IT_STOPSIGN:
		//VTLOG("Stop Sign");
		if (distToGo < (.5 * m_fSpeed * m_fSpeed) / (ACCEL))
		{
			//getting too close.  slow down.
			//VTLOG("Stop: slowing down");
			m_fSpeed -= ACCEL*fDeltaTime;
		} else if (m_fSpeed < m_fTargetSpeed)
		{
			//VTLOG("Stop: speeding up");
			m_fSpeed += ACCEL*fDeltaTime;
		}
		break;
	default:
		//uh oh!
		break;
	}
	//bounds on speed.
	if (m_fSpeed > m_fTargetSpeed)
		m_fSpeed = m_fTargetSpeed;

	//don't actually stop completely.
	if (m_fSpeed < 0)
		m_fSpeed = .1f*1000/60/60;
}

