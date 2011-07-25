//
// Engines.cpp
//
// Engines used by Enviro
//
// Copyright (c) 2001-2011 Virtual Terrain Project
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


//////////////////////////////////////////////////////////////////////

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

	for (unsigned int i = 0; i < NumTargets(); i++)
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
	m_fTargetScale = 0.01f;
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
			pTarget->Identity();
			pTarget->SetTrans(m_GroundPoint);
			pTarget->PointTowards(m_GroundPoint * 2.0f);
			pTarget->Scale(m_fTargetScale);
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
		float dx = -(m_pos.x - m_startpos.x) * 0.0015f;
		float dy = (m_pos.y - m_startpos.y) * 0.0015f;

		pos.x += (dx * (pos.z-0.8f));
		pos.y += (dy * (pos.z-0.8f));
		pTarget->SetTrans(pos);
	}
	if (m_bZoom)
	{
		float dz = -(m_pos.y - m_startpos.y) * 0.02f;
		pos.z += dz;

		pTarget->SetTrans(pos);
	}
}

//////////////////////////////////////////////////////////////////////


GrabFlyer::GrabFlyer(float fSpeed) : vtTerrainFlyer(fSpeed)
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
		float dx = -(m_pos.x - m_startpos.x) * 0.02f * m_fHeight;
		float dz = (m_pos.y - m_startpos.y) * 0.02f * m_fHeight;

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

		float dz = -(m_pos.y - m_startpos.y) * 0.02f;
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


