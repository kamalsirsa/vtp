//
// Engines.cpp
//
// Engines used by Enviro
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/Route.h"
#include "vtlib/core/Roads.h"

#include "Engines.h"
#include "Hawaii.h"
#include "Globe.h"
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

PlaneEngine::PlaneEngine(float fSpeedExag, AirportCodes code) : vtEngine()
{
	m_fSpeedExag = fSpeedExag;

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
	x = 10000.0f * WORLD_SCALE;
	y = 5000.0f * WORLD_SCALE;
	z = -130000.0f * WORLD_SCALE;

	m_hoop_pos[0].Set(x, y, z);
	m_hoop_speed[0] = 200.0f * WORLD_SCALE;
	// done setting initial positions and speed

	// begin approach
	utm_points[3].z = 800.0f;
	g_Conv.ConvertFromEarth(utm_points[3], m_hoop_pos[1]);
	m_hoop_speed[1] = 150.0f * WORLD_SCALE;

	// touchdown
	// center of plane is 15m above ground + 1.5m airport above ground + .5m of runway thickness
	utm_points[0].z = 17.0f;
	g_Conv.ConvertFromEarth(utm_points[0], m_hoop_pos[2]);
	m_hoop_speed[2] = 25.0f * WORLD_SCALE;

	// speeding up to takeoff point
	g_Conv.ConvertFromEarth(utm_points[1], m_hoop_pos[3]);
	m_hoop_speed[3] = 5.0f * WORLD_SCALE;

	// takeoff to this point
	g_Conv.ConvertFromEarth(utm_points[2], m_hoop_pos[4]);
	m_hoop_speed[4] = 35.0f * WORLD_SCALE;

	// point to loop to
	utm_points[4].z = 800.0f;
	g_Conv.ConvertFromEarth(utm_points[4], m_hoop_pos[5]);
	m_hoop_speed[5] = 150.0f * WORLD_SCALE;

	// saving last hoop info
	x = 2000.0f * WORLD_SCALE;
	y = 5000.0f * WORLD_SCALE;
	z = -40000.0f * WORLD_SCALE;
	m_hoop_pos[6].Set(x, y, z);
	m_hoop_speed[6] = 200.0f * WORLD_SCALE;

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

void RoadFollowEngine::PickRoad(RoadGeom *last_road)
{
	int i = 0;
	NodeGeom *node = m_pLastNode;

	if (node->m_iRoads != 1 && last_road != NULL)
	{
		i = node->m_iRoads;

		// pick next available road
		if (last_road->m_iHwy > 0)
		{
			// special logic: follow the highway
			for (i = 0; i < node->m_iRoads; i++)
			{
				if (node->GetRoad(i) != last_road && node->GetRoad(i)->m_iHwy > 0)
					break;
			}
			// if no highway, do normal logic
		}
		if (i == node->m_iRoads)
		{
			for (i = 0; i < node->m_iRoads; i++)
			{
				if (node->GetRoad(i) == last_road)
				{
					i++;
					i %= node->m_iRoads;
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

	float fInc = 25.0f * WORLD_SCALE;
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

BenchEngine::BenchEngine(const char *loc_fname)
{
	m_idone = 0;
	m_iviewpoint = 1;
	m_iframesElapsed = 0;
	m_fresults = 0;

#if 0
	// try to read the location file
	if (m_LSaver.ReadStoredLocations(loc_fname))
	{
		// success
		m_LSaver.SetCamera(pScene->GetCamera());
		m_LSaver.JumpToSavedLocation(m_iviewpoint);
		m_fp = fopen("output.txt", "w");
		m_fstartTime = vtGetTime();
//		m_FCounter.SetName2("Benchmark Framecounter");
	}
	else
		m_fp = NULL;
#endif
}

void BenchEngine::Eval()
{
	if (!m_fp)
		return;

#if 0
	if (m_idone == 0)
	{
	  m_iframesElapsed++;
	  float total_time = vtGetTime();
	  total_time -= m_fstartTime;
	  if ((m_iframesElapsed >= 50) && (total_time >= 3)) //enough time has gone by at this viewpoint
	  {
//		float result = m_FCounter.GetFrameRate();
		float result = m_statlog->GetAverage(STAT_FrameRate);

		m_fresults += result;
		Convert(result, m_fp, 0);

		m_iframesElapsed = 0; //reset frame counter
		m_iviewpoint++; //move to the next viewpoint

		if (m_iviewpoint == 7) //we're done collecting, now average and output result
		{
			m_idone = 1;
			m_fresults = m_fresults/6;
			Convert(m_fresults, m_fp, 1);
			fclose(m_fp);
		}
		else //not done, move camera to next viewpoint
		{
			m_LSaver.JumpToSavedLocation(m_iviewpoint);
			m_fstartTime = vtGetTime();
		}
	  }
	}
#endif
}

void BenchEngine::Convert(float result, FILE* fp, bool last)
{
	if (result > 10)
	{
		result = ((int)(result*10+0.5))/10.0f;
		fprintf(fp, "%2.1f", result);
	}
	else if (result > 1)
	{
		result = ((int)(result*100+0.5))/100.0f;
		fprintf(fp, "%1.2f", result);
	}
	else 
	{
		result = ((int)(result*1000+0.5))/1000.0f;
		fprintf(fp, "%0.3f", result);
	}
	if (!last)
		fprintf(fp, ", ");
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
	target->TranslateLocal(FPoint3(0.0f, params.m_iMinHeight*WORLD_SCALE*3, 0.0f));

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


/*FollowerEngine::FollowerEngine(vtTransform* model, vtCamera* camera)
{
	m_model = model;
	SetTarget(camera);
	m_pCamera = camera;
}

void FollowerEngine::Eval()
{
	//get the target and it's location
	vtCamera* target = (vtCamera*) GetTarget();
	if (!target)
		return;

	FMatrix4 model_loc;
	model_loc.Copy(m_model->GetTransform());
	model_loc.Translate(FPoint3(0.0f, 5.0f * WORLD_SCALE, 0.0f * WORLD_SCALE));

	//make a matrix and move it so it will be behind and above the target afterwards
//	FMatrix4* matrix = new FMatrix4f();
//	matrix->TranslationMatrix(FPoint3(0.0f, 5.0f, -10.0f));
//	matrix->PreMul(*model_loc);

	//update the camera's location
	target->SetTransform2(&model_loc);
	target->RotateLocal(FPoint3(0.0f, 1.0f, 0.0f), PIf);

//	delete matrix;
}*/

ProximityEngine::ProximityEngine(vtTransform* far_model, vtTransform* near_model, vtCamera* camera,
								 float distance)
{
	m_FarModel = far_model;
	m_NearModel = near_model;
	SetTarget(camera);
	m_pCamera = camera;
	m_bFarOn = true;
	m_distance = distance;
}

void ProximityEngine::Eval()
{
	//get the target and it's location
	vtCamera* pTarget = (vtCamera*) GetTarget();
	if (!pTarget)
		return;

	FPoint3 current_model_loc;
	if (m_bFarOn)
		current_model_loc = m_FarModel->GetTrans();
	else
		current_model_loc = m_NearModel->GetTrans();

	FPoint3 target_loc = m_pCamera->GetTrans();

	FPoint3 result = current_model_loc - target_loc;
	float length = result.Length();

	if ( (length < m_distance) && (m_bFarOn) ) //if we're within 5 km
	{
		m_bFarOn = false;
		m_NearModel->SetEnabled(true);
		m_FarModel->SetEnabled(false);
	}
	else if ( (length > m_distance) && (!m_bFarOn) )
	{
		m_bFarOn = true;
		m_NearModel->SetEnabled(false);
		m_FarModel->SetEnabled(true);
	}
}

SimpleBBEngine::SimpleBBEngine(vtTransform* model, vtCamera* camera)
{
	m_model = model;
	SetTarget(camera);
	m_pCamera = camera;
	m_fAngle = 0;
}

void SimpleBBEngine::Eval()
{
	//get the target and it's location
	vtCamera* pTarget = (vtCamera*) GetTarget();
	if (!pTarget)
		return;
	FPoint3 target_loc = m_pCamera->GetTrans();
	FPoint3 model_loc = m_model->GetTrans();

	float angle = atan2f(-(target_loc.z - model_loc.z), target_loc.x - model_loc.x);

	m_model->RotateLocal(FPoint3(0.0f, 1.0f, 0.0f), angle - m_fAngle);
	m_fAngle = angle;
}


//
// Terrain picking ability
//
TerrainPicker::TerrainPicker() : vtLastMouse()
{
	m_pHeightField = NULL;
	m_bOnTerrain = false;
}

void TerrainPicker::Eval()
{
	if (!m_pHeightField) return;

	FPoint3 pos, dir, result;

	vtGetScene()->CameraRay(m_pos, pos, dir);

	// test whether we hit the heightfield
	m_bOnTerrain = m_pHeightField->CastRayToSurface(pos, dir, result);
	if (m_bOnTerrain)
	{
		vtTransform *pTarget = (vtTransform *) GetTarget();
		if (pTarget)
			pTarget->SetTrans(result);

		// save result
		m_GroundPoint = result;

		// Find corresponding UTM coordinates
		g_Conv.ConvertToEarth(m_GroundPoint, m_EarthPos); 
	}
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
	m_pGlobeMGeom = NULL;
}

void GlobePicker::Eval()
{
	FPoint3 pos, dir;

	vtGetScene()->CameraRay(m_pos, pos, dir);

	// test whether we hit the globe
	FSphere sphere(FPoint3(0.0f, 0.0f, 0.0f), (float)m_fRadius);
	FPoint3 akPoint[2];
	int riQuantity;

	m_bOnTerrain = FindIntersection(pos, dir, sphere, riQuantity, akPoint);
	if (m_bOnTerrain)
	{
		// save result
		m_GroundPoint = akPoint[0];

		if (m_pGlobeMGeom)
		{
			// rotate to find position relative to globe's rotation
			FMatrix4 rot;
			m_pGlobeMGeom->GetTransform1(rot);
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

		// apply global position to target (which is not a child of the globe)
		vtTransform *pTarget = (vtTransform *) GetTarget();
		if (pTarget)
		{
			pTarget->SetTrans(m_GroundPoint);
			pTarget->PointTowards(m_GroundPoint * 2);
		}

		// Find corresponding UTM coordinates
		xyz_to_geo(m_fRadius, m_GroundPoint, m_EarthPos);

		int foo = 1;
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

