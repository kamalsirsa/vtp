//
// Engines.cpp
//
// Engines used by Enviro
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Route.h"
#include "vtlib/core/Roads.h"
#include "vtlib/core/Globe.h"
#include "vtdata/vtLog.h"

#include "Engines.h"
#include "Hawaii.h"
#include "Enviro.h"	// for GetCurrentTerrain

//////////////////////////////////////////////////////////////

float utm_points_ito[5][2] = {
	{ 287050, 2182130}, 	// begin approach
	{ 286820, 2182140}, 	// touchdown point
	{ 286760, 2182194}, 	// begin takeoff point
	{ 287450, 2182128},		// come back
	{ 286400, 2182200}  	// end takeoff point
};

float utm_points_koa[5][2] = {
	{ 180665, 2188287 },	// A 0
	{ 180507, 2186715 },	// B 1
	{ 180326, 2184920 },	// C 2
	{ 181022, 2193263 },	// D 3
	{ 179963, 2179740 }		// E 4
};

PlaneEngine::PlaneEngine(float fSpeedExag, float fScale, AirportCodes code) : vtEngine()
{
	m_fSpeedExag = fSpeedExag;
	m_fScale = fScale;

	// set up some initial points
	float x, y, z;
	DPoint3 utm_points[5];

	for (int i = 0; i < 5; i++)
	{
		if (code == KOA)
		{
			utm_points[i].x = utm_points_koa[i][0];
			utm_points[i].y = utm_points_koa[i][1];
		}
		else
		if (code == ITO)
		{
			utm_points[i].x = utm_points_ito[i][0];
			utm_points[i].y = utm_points_ito[i][1];
		}
		utm_points[i].z = 0.0;
	}

	// 1 mile = 1.60934 km
	// a typical flight speed, 450 mph = 725 kmph
	// 1 kmph = 16.66 meters per minute = .2777777 meters per second
	// 725 kmph = 200 meter per second, slowing to 25 m/s at touchdown
	x = 10000.0f;
	y = 5000.0f;
	z = -130000.0f;

	m_hoop_pos[0].Set(x, y, z);
	m_hoop_speed[0] = 200.0f;
	// done setting initial positions and speed

	// begin approach
	utm_points[3].z = 800.0f;
	g_Conv.ConvertFromEarth(utm_points[3], m_hoop_pos[1]);
	m_hoop_speed[1] = 150.0f;

	// touchdown
	// center of plane is 15m above ground + 2m airport above ground
	// plus distance from center of plane to base of landing gear
	double ground_offset = 17.0f + (m_fScale * 13.5f);

	utm_points[0].z = ground_offset;
	g_Conv.ConvertFromEarth(utm_points[0], m_hoop_pos[2]);
	m_hoop_speed[2] = 25.0f;

	// speeding up to takeoff point
	utm_points[1].z = ground_offset;
	g_Conv.ConvertFromEarth(utm_points[1], m_hoop_pos[3]);
	m_hoop_speed[3] = 5.0f;

	// takeoff to this point
	utm_points[2].z = ground_offset;
	g_Conv.ConvertFromEarth(utm_points[2], m_hoop_pos[4]);
	m_hoop_speed[4] = 35.0f;

	// point to loop to
	utm_points[4].z = 800.0f;
	g_Conv.ConvertFromEarth(utm_points[4], m_hoop_pos[5]);
	m_hoop_speed[5] = 150.0f;

	// saving last hoop info
	x = 2000.0f;
	y = 5000.0f;
	z = -40000.0f;
	m_hoop_pos[6].Set(x, y, z);
	m_hoop_speed[6] = 200.0f;

	m_hoops = 7;
	m_hoop = 0;

	m_pos = m_hoop_pos[0];

	m_fLastTime = vtGetTime();
}


void PlaneEngine::Eval()
{
	// determine vectors between last hoop, current position, and next hoop
	FPoint3 pos_next = m_hoop_pos[m_hoop+1];
	FPoint3 diff1 = m_pos - m_hoop_pos[m_hoop];
	FPoint3 diff2 = pos_next - m_pos;
	FPoint3 diff3 = pos_next - m_hoop_pos[m_hoop];

	// and their magnitudes
	float mag1 = diff1.Length();
	float mag2 = diff2.Length();
	float mag3 = diff3.Length();

	// simple linear interpolation of speed
	float speed_diff = (m_hoop_speed[m_hoop+1] - m_hoop_speed[m_hoop]);
	float speed = m_hoop_speed[m_hoop] + ((1.0f - (mag2/mag3)) * speed_diff);
	float factor = (speed / mag2);

	// moderate to real time
	float time = vtGetTime();
	float elapsed = time - m_fLastTime;		// time per frame
	m_fLastTime = time;
	factor *= elapsed;

	// potentially faster than real life
	factor *= m_fSpeedExag;

	// scale difference vector by speed to produce direction vector
	diff2 *= factor;

	// have we reached/passed the next hoop?
	if (mag1/mag3 > 1.0f)
	{
		m_hoop++;
		if (m_hoop == m_hoops-1)
			m_hoop = 0;
		m_pos = m_hoop_pos[m_hoop];
		return;
	}

	// determine position next frame
	m_pos += diff2;

	// turn plan to point toward next frame's position
	// Yaw the object to face the point indicated
	Vehicle *pTarget = (Vehicle *) GetTarget();
	if (!pTarget) return;

	pTarget->Identity();
	float angle = atan2f(-diff2.z, diff2.x) - PID2f;
	pTarget->RotateLocal(FPoint3(0,1,0), angle);

	// restore scaling information lost in setting rotation
	float scale = pTarget->m_fSize;
	pTarget->Scale3(scale, scale, scale);

	// set the plane to next frame's position
	pTarget->SetTrans(m_pos);
}

void PlaneEngine::SetHoop(int i)
{
	m_hoop = i;
	m_pos = m_hoop_pos[i];
}


//////////////////////////////////////////////////////////////

RoadFollowEngine::RoadFollowEngine(NodeGeom *pStartNode)
{
	m_pLastNode = pStartNode;

	PickRoad(NULL);
}

void RoadFollowEngine::PickRoad(LinkGeom *last_link)
{
	int i = 0;
	NodeGeom *node = m_pLastNode;

	if (node->m_iLinks != 1 && last_link != NULL)
	{
		i = node->m_iLinks;

		// pick next available road
		if (last_link->m_iHwy > 0)
		{
			// special logic: follow the highway
			for (i = 0; i < node->m_iLinks; i++)
			{
				if (node->GetRoad(i) != last_link && node->GetRoad(i)->m_iHwy > 0)
					break;
			}
			// if no highway, do normal logic
		}
		if (i == node->m_iLinks)
		{
			for (i = 0; i < node->m_iLinks; i++)
			{
				if (node->GetRoad(i) == last_link)
				{
					i++;
					i %= node->m_iLinks;
					break;
				}
			}
		}
	}
	m_pCurrentRoad = node->GetRoad(i);
	if (m_pCurrentRoad)
	{
		m_pCurrentRoadLength = m_pCurrentRoad->Length();
		forwards = (m_pCurrentRoad->GetNode(0) == m_pLastNode);
	}
	fAmount = 0.0f;
}

void RoadFollowEngine::Eval()
{
	vtTransform *pTarget = (vtTransform *) GetTarget();
	if (!pTarget) return;

	if (!m_pCurrentRoad) return;

	float fInc = 25.0f;
	fAmount += fInc;
	float fRightAmount = forwards ? fAmount : (m_pCurrentRoadLength - fAmount);

	FPoint3 pos1 = m_pCurrentRoad->FindPointAlongRoad(fRightAmount);
	FPoint3 pos(pos1.x, pos1.y, pos1.z);
	pTarget->SetTrans(pos);

	if (fAmount + fInc >= m_pCurrentRoadLength)
	{
		if (forwards)
			m_pLastNode = m_pCurrentRoad->GetNode(1);
		else
			m_pLastNode = m_pCurrentRoad->GetNode(0);
		PickRoad(m_pCurrentRoad);
	}
}


//////////////////////////////////////////////////////////////////////

//
// RouteFollowerEngine
//
RouteFollowerEngine::RouteFollowerEngine(vtRoute* route)
{
	m_pHeightField = GetCurrentTerrain()->GetHeightField();
	m_pRoute = route;
	m_cur = 0;
	m_next = 1;
	m_inc = 0.0f;
}

void RouteFollowerEngine::Eval()
{
	// Position the View to the beginning of the route
	//	and follow the route with the camera.
	vtCamera *target = (vtCamera*) GetTarget();
	if (!target)
		return;
	if (!m_pRoute)
		return;

	vtUtilNode *st0 = m_pRoute->GetAt(m_cur);
	vtUtilNode *st1 = m_pRoute->GetAt(m_next);

	DPoint2 diff = st1->m_Point - st0->m_Point;
	diff *= m_inc;
	DPoint2 ep = st0->m_Point + diff;

	FPoint3 fp, fp2;
	m_pHeightField->ConvertEarthToSurfacePoint(ep, fp);
	m_pHeightField->ConvertEarthToSurfacePoint(st1->m_Point, fp2);
	FPoint3 dir = fp2 - fp;

	double angle = atan2(-dir.z, dir.x) - PID2d;

	target->Identity();
	target->SetTrans(fp);
	target->RotateParent(FPoint3(0.0f, 1.0f, 0.0f), angle);

	TParams &params = GetCurrentTerrain()->GetParams();
	target->TranslateLocal(FPoint3(0.0f, params.GetValueInt(STR_MINHEIGHT) * 3, 0.0f));

	m_inc += 0.03f;

	if (m_inc >= 1.0f)
	{
		m_cur++;
		if (m_cur > m_pRoute->GetSize() - 2)
			m_cur = 0;
		m_next = m_cur+1;
		m_inc = 0.0f;
	}
}


/////////////////////////////////////////////////////////////////////////////

//
// Terrain picking ability
//
TerrainPicker::TerrainPicker() : vtLastMouse()
{
	m_pHeightField = NULL;
	m_bOnTerrain = false;
}

void TerrainPicker::OnMouse(vtMouseEvent &event)
{
	vtLastMouse::OnMouse(event);

	FindGroundPoint();
}

void TerrainPicker::FindGroundPoint()
{
	if (!m_pHeightField) return;

	FPoint3 pos, dir, result;

	vtGetScene()->CameraRay(m_pos, pos, dir);

	// test whether we hit the heightfield
	m_bOnTerrain = m_pHeightField->CastRayToSurface(pos, dir, result);
	if (!m_bOnTerrain)
		return;

	for (int i = 0; i < NumTargets(); i++)
	{
		vtTransform *pTarget = (vtTransform *) GetTarget(i);
		pTarget->SetTrans(result);
	}

	// save result
	m_GroundPoint = result;

	// Find corresponding earth coordinates
	g_Conv.ConvertToEarth(m_GroundPoint, m_EarthPos); 
}

void TerrainPicker::Eval()
{
	// Don't calculate here, since we can get OnMouse events out of synch
	//  with the Paint events that trigger the scene update and engine Eval().
}

bool TerrainPicker::GetCurrentPoint(FPoint3 &p)
{
	if (m_bOnTerrain)
		p = m_GroundPoint;
	return m_bOnTerrain;
}

bool TerrainPicker::GetCurrentEarthPos(DPoint3 &p)
{
	if (m_bOnTerrain)
		p = m_EarthPos;
	return m_bOnTerrain;
}


//
// Globe picking ability
//
GlobePicker::GlobePicker() : vtLastMouse()
{
	m_fRadius = 1.0;
	m_bOnTerrain = false;
	m_pGlobe = NULL;
}

void GlobePicker::Eval()
{
	FPoint3 pos, dir;

	vtGetScene()->CameraRay(m_pos, pos, dir);

	// test whether we hit the globe
	FSphere sphere(FPoint3(0.0f, 0.0f, 0.0f), (float)m_fRadius);
	FPoint3 akPoint[2];
	int riQuantity;

	m_bOnTerrain = RaySphereIntersection(pos, dir, sphere, riQuantity, akPoint);
	if (m_bOnTerrain)
	{
		// save result
		m_GroundPoint = akPoint[0];

		// apply global position to target (which is not a child of the globe)
		vtTransform *pTarget = (vtTransform *) GetTarget();
		if (pTarget)
		{
			float sc = 0.05f;
			pTarget->Identity();
			pTarget->SetTrans(m_GroundPoint);
			pTarget->PointTowards(m_GroundPoint * 2);
			pTarget->Scale3(sc, sc, sc);
		}

		if (m_pGlobe)
		{
			// rotate to find position relative to globe's rotation
			vtTransform *xform = m_pGlobe->GetTop();
			FMatrix4 rot;
			xform->GetTransform1(rot);
			FMatrix4 inverse;
			inverse.Invert(rot);
			FPoint3 newpoint;

			// work around SML bug: matrices flagged as identity but
			// will still transform by their components
			if (! inverse.IsIdentity())
			{
				inverse.Transform(m_GroundPoint, newpoint);
				m_GroundPoint = newpoint;
			}
		}

		// Find corresponding geographic coordinates
		xyz_to_geo(m_fRadius, m_GroundPoint, m_EarthPos);
	}
}

bool GlobePicker::GetCurrentPoint(FPoint3 &p)
{
	if (m_bOnTerrain)
		p = m_GroundPoint;
	return m_bOnTerrain;
}

bool GlobePicker::GetCurrentEarthPos(DPoint3 &p)
{
	if (m_bOnTerrain)
		p = m_EarthPos;
	return m_bOnTerrain;
}

//////////////////////////////////////////////////////////////////////

FlatFlyer::FlatFlyer()
{
	m_bDrag = false;
	m_bZoom = false;
}

void FlatFlyer::OnMouse(vtMouseEvent &event)
{
	int previous = m_buttons;

	vtLastMouse::OnMouse(event);

	//  Left button down
	bool bLeft = (m_buttons & VT_LEFT) != 0;
	bool bRight = (m_buttons & VT_RIGHT) != 0;

	if (!bLeft && !bRight)
	{
		m_bDrag = false;
		m_bZoom = false;
	}

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	if (bLeft || bRight)
	{
		if (m_buttons != previous)
		{
			// Previously a different mouse button was down, so capture
			//  the starting state.
			m_start_wp = pTarget->GetTrans();
			m_startpos = m_pos;
		}
		if (!bLeft && bRight)
		{
			m_bDrag = true;
			m_bZoom = false;
		}
		if (bLeft && bRight)
		{
			m_bDrag = false;
			m_bZoom = true;
		}
	}
}

void FlatFlyer::Eval()
{
	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	FPoint3 pos = m_start_wp;
	if (m_bDrag)
	{
		// Consider XZ plane of ray from initial eye to ground
		float dx = -(m_pos.x - m_startpos.x) * 0.0015;
		float dy = (m_pos.y - m_startpos.y) * 0.0015;

		pos.x += (dx * (pos.z-0.8));
		pos.y += (dy * (pos.z-0.8));
		pTarget->SetTrans(pos);
	}
	if (m_bZoom)
	{
		float dz = -(m_pos.y - m_startpos.y) * 0.02;
		pos.z += dz;

		pTarget->SetTrans(pos);
	}
}

//////////////////////////////////////////////////////////////////////


GrabFlyer::GrabFlyer(float fSpeed, float fHeightAboveTerrain, bool bMin) :
	vtTerrainFlyer(fSpeed, fHeightAboveTerrain, bMin)
{
	m_bDrag = false;
	m_bPivot = false;
	m_bZoom = false;
	m_pTP = NULL;
}

void GrabFlyer::OnMouse(vtMouseEvent &event)
{
	if (!m_pTP)
		return;

	int previous = m_buttons;

	vtLastMouse::OnMouse(event);

	//  Left button down
	bool bLeft = (m_buttons & VT_LEFT) != 0;
	bool bRight = (m_buttons & VT_RIGHT) != 0;

	if (!bLeft && !bRight)
	{
		m_bDrag = false;
		m_bPivot = false;
		m_bZoom = false;
	}

	FPoint3 wp;
	bool bOnGround = m_pTP->GetCurrentPoint(wp);
	if (!bOnGround)
		return;

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	if (bLeft || bRight)
	{
		if (m_buttons != previous)
		{
			// Previously a different mouse button was down, so capture
			//  the starting state.
			m_start_wp = wp;
			m_startpos = m_pos;
			pTarget->GetTransform1(m_start_eye);
		}
		if (bLeft && !bRight)
		{
			m_bPivot = true;
		}
		if (!bLeft && bRight)
		{
			m_bDrag = true;
			// store initial elevation
			FPoint3 pos = pTarget->GetTrans();
			float fAltitude;
			m_pHeightField->FindAltitudeAtPoint(pos, fAltitude);
			m_fHeight = pos.y - fAltitude;
		}
		if (bLeft && bRight)
		{
			m_bZoom = true;
		}
	}
}

void GrabFlyer::Eval()
{
	if (!m_pTP)
		return;

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	FMatrix4 rot;
	rot.Identity();

	if (m_bPivot)
	{
		pTarget->SetTransform1(m_start_eye);
		FPoint3 pos = pTarget->GetTrans();
		FPoint3 ray = pos - m_start_wp;
		FPoint3 ray2;

		double radians1 = (m_pos.x - m_startpos.x) * 0.006;
		double radians2 = -(m_pos.y - m_startpos.y) * 0.006;

		FPoint3 up(0, 1, 0);
		FPoint3 side = ray.Cross(up);
		side.Normalize();

		rot.AxisAngle(side, radians2);
		rot.Transform(ray, ray2);

		ray = ray2;

		rot.AxisAngle(up, radians1);
		rot.Transform(ray, ray2);

		FPoint3 out;
		out = m_start_wp + ray2;

		float fAltitude;
		bool bOverTerrain = m_pHeightField->FindAltitudeAtPoint(out, fAltitude);
		if (bOverTerrain)
		{
			if (out.y < fAltitude + 10)
				out.y = fAltitude + 10;
		}

		pTarget->SetTrans(out);

		pTarget->RotateParent(side, radians2);
		pTarget->RotateParent(up, radians1);
	}
	if (m_bDrag)
	{
		pTarget->SetTransform1(m_start_eye);
		FPoint3 pos = pTarget->GetTrans();
		FPoint3 ray = m_start_wp - pos;

		ray.Normalize();
		FPoint3 ray1(ray.x, 0.0f, ray.z);
		FPoint3 ray2(-ray.z, 0.0f, ray.x);

		// Consider XZ plane of ray from initial eye to ground
		float dx = -(m_pos.x - m_startpos.x) * 0.02 * m_fHeight;
		float dz = (m_pos.y - m_startpos.y) * 0.02 * m_fHeight;

		pos += (ray1 * dz);
		pos += (ray2 * dx);

		float fAltitude;
		bool bOverTerrain = m_pHeightField->FindAltitudeAtPoint(pos, fAltitude);
		if (bOverTerrain)
			pos.y = fAltitude + m_fHeight;

		pTarget->SetTrans(pos);
	}
	if (m_bZoom)
	{
		pTarget->SetTransform1(m_start_eye);
		FPoint3 pos = pTarget->GetTrans();
		FPoint3 ray = m_start_wp - pos;

		float dz = -(m_pos.y - m_startpos.y) * 0.02;
		pos += (ray * dz);

		float fAltitude;
		bool bOverTerrain = m_pHeightField->FindAltitudeAtPoint(pos, fAltitude);
		if (bOverTerrain)
		{
			if (pos.y < fAltitude + 10)
				pos.y = fAltitude + 10;
		}

		pTarget->SetTrans(pos);
	}
}

