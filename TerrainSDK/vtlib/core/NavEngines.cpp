//
// NavEngines.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/vtTin3d.h"
#include "NavEngines.h"

#define GRAVITY_CONSTANT 9.81	// g = 9.81 meters/sec^2

//
// vtFlyer: basic class, moves target based on mouse position
//
vtFlyer::vtFlyer(float fSpeed, bool bPreventRoll) : vtLastMouse()
{
	m_fSpeed = fSpeed;
	m_bPreventRoll = bPreventRoll;
	m_bAlwaysMove = false;
}

void vtFlyer::SetAlwaysMove(bool bMove)
{
	m_bAlwaysMove = bMove;
}

void vtFlyer::Eval()
{
	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	vtScene* scene = vtGetScene();
	IPoint2	WinSize = scene->GetWindowSize();
	float	mx = (float) m_pos.x / WinSize.x;
	float	my = (float) m_pos.y / WinSize.y;

	//	Left button: forward-backward, yaw
	if (m_bAlwaysMove ||
		((m_buttons & VT_LEFT) && !(m_buttons & VT_RIGHT)))
	{
		float trans = (my - 0.5f) * m_fSpeed;
		float rotate = -(mx - 0.5f) / 15.0f;

		pTarget->TranslateLocal(FPoint3(0.0f, 0.0f, trans));
		if (m_bPreventRoll)
			pTarget->RotateParent(FPoint3(0.0f, 1.0f, 0.0f), rotate);
		else
			pTarget->RotateLocal(FPoint3(0.0f, 1.0f, 0.0f), rotate);
	}

	//  Right button: up-down, left-right
	if ((m_buttons & VT_RIGHT) && !(m_buttons & VT_LEFT))
	{
		FPoint3 pos = pTarget->GetTrans();

		float updown = -(my - 0.5f) * m_fSpeed;
		float leftright = (mx - 0.5f) * m_fSpeed;

		pTarget->TranslateLocal(FPoint3(leftright, updown, 0.0f));
	}

	//  Both buttons: pitch, roll
	if ((m_buttons & VT_LEFT) && (m_buttons & VT_RIGHT))
	{
		float updown = -(my - 0.5f) / 20.0f;
		float leftright = (mx - 0.5f) / 20.0f;
		if (fabs(updown) > fabs(leftright))
			pTarget->RotateLocal(FPoint3(1.0f, 0.0f, 0.0f), updown);
		else if (!m_bPreventRoll)
			pTarget->RotateLocal(FPoint3(0.0f, 0.0f, 1.0f), -leftright);
	}
}


//
// Fly engine specifically for following terrain
//
vtTerrainFlyer::vtTerrainFlyer(float fSpeed, float fHeightAboveTerrain, bool bMin)
 : vtFlyer(fSpeed, true)
{
	m_fHeightAboveTerrain = fHeightAboveTerrain;
	m_bMin = bMin;
	m_bFollow = true;
	m_pHeightField = NULL;
	m_bMaintain = false;
	m_fMaintainHeight = 0;
}

void vtTerrainFlyer::FollowTerrain(bool bFollow)
{
	m_bFollow = bFollow;
}

void vtTerrainFlyer::Eval()
{
	vtFlyer::Eval();
	KeepAboveGround();
}

//
// Keep the target above the the terrain surface.
//
void vtTerrainFlyer::KeepAboveGround()
{
	if (!m_pHeightField)
		return;

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	FPoint3 pos = pTarget->GetTrans();

	float fAltitude;
	bool bOverTerrain = m_pHeightField->FindAltitudeAtPoint(pos, fAltitude);

	if (bOverTerrain)
	{
		// set y value based on location
		if (m_bMaintain)
		{
			if (m_fMaintainHeight == 0)
				m_fMaintainHeight = pos.y - fAltitude;
			pos.y = fAltitude + m_fMaintainHeight;
		}
		else if (!m_bMin)
			pos.y = fAltitude + m_fHeightAboveTerrain;
		else
		{
			if (pos.y <= fAltitude + m_fHeightAboveTerrain)
				pos.y = fAltitude + m_fHeightAboveTerrain;
		}
		pTarget->SetTrans(pos);
	}
}


//////////////////////
//
// vtTinFlyer
//

vtTinFlyer::vtTinFlyer(float fSpeed) : vtLastMouse()
{
	m_pTin = NULL;
	m_fSpeed = fSpeed;
	m_fHeightAboveTerrain = 1.0f;
	m_fPitch = 0.0f;
}

void vtTinFlyer::SetTin(vtTin3d *pTin)
{
	m_pTin = pTin;
}

void vtTinFlyer::Eval()
{
	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	vtScene* scene = vtGetScene();
	IPoint2	WinSize = scene->GetWindowSize();
	float	mx = (float) m_pos.x / WinSize.x;
	float	my = (float) m_pos.y / WinSize.y;

	FPoint3 pos = pTarget->GetTrans();
	float maintain_y = pos.y;

	//	Left button: forward-backward, parent yaw
	if ((m_buttons & VT_LEFT) && !(m_buttons & VT_RIGHT))
	{
		float trans = (my - 0.5f) * m_fSpeed;
		float rotate = -(mx - 0.5f) / 15.0f;

		pTarget->TranslateLocal(FPoint3(0.0f, 0.0f, trans));
		pTarget->RotateParent(FPoint3(0.0f, 1.0f, 0.0f), rotate);
	}

	//  Right button: up-down, left-right
	if ((m_buttons & VT_RIGHT) && !(m_buttons & VT_LEFT))
	{
		FPoint3 pos = pTarget->GetTrans();

		float updown = -(my - 0.5f) * m_fSpeed;
		float leftright = (mx - 0.5f) * m_fSpeed;

		maintain_y += updown;
		pTarget->TranslateLocal(FPoint3(leftright, 0.0f, 0.0f));
	}

	//  Both buttons: pitch (constrained to a range)
	if ((m_buttons & VT_LEFT) && (m_buttons & VT_RIGHT))
	{
		float updown = -(my - 0.5f) / 20.0f;

		float fPrevious = m_fPitch;
		m_fPitch += updown;
		if (m_fPitch < -PID2f)
			m_fPitch = -PID2f;
		if (m_fPitch > 0.0f)
			m_fPitch = 0.0f;
		float diff = m_fPitch - fPrevious;
		pTarget->RotateLocal(FPoint3(1.0f, 0.0f, 0.0f), diff);
	}

	pos = pTarget->GetTrans();

	if (m_pTin)
	{
		float mini = (m_pTin->m_fMinHeight - 4.5);
		float maxi = (m_pTin->m_fMaxHeight + 100);
		if (maintain_y < mini) maintain_y = mini;
		if (maintain_y > maxi) maintain_y = maxi;

		FPoint3 vec = m_pTin->FindVectorToClosestVertex(pos);
		float horiz_dist = sqrt(vec.x*vec.x + vec.z*vec.z);
		if (horiz_dist > 100.0f)
		{
			vec *= 0.05f;
			pos += vec;
		}
	}

	pos.y = maintain_y;
	pTarget->SetTrans(pos);
}


//////////////////////
//
// VFlyer
//

VFlyer::VFlyer(float scale, float fHeightAboveTerrain, bool bMin)
 : vtTerrainFlyer(0.4f, fHeightAboveTerrain, bMin)	// hardcode scale override
{
	m_Velocity.Set(0, 0, 0);
	m_bGravity = false;
}

void VFlyer::Eval()
{
	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	bool bUpDown = false;
	vtScene* scene = vtGetScene();
	IPoint2	WinSize = scene->GetWindowSize();
	float	mx = (float) m_pos.x / WinSize.x;
	float	my = (float) m_pos.y / WinSize.y;

	// Left button: forward-backward, yaw
	if ((m_buttons & VT_LEFT) && !(m_buttons & VT_RIGHT))
	{
		float trans = (my - 0.5f) * m_fSpeed;
		float rotate = -(mx - 0.5f) / 15.0f;

		m_Velocity.z += trans;
		if (m_bPreventRoll)
			pTarget->RotateParent(FPoint3(0.0f, 1.0f, 0.0f), rotate);
		else
			pTarget->RotateLocal(FPoint3(0.0f, 1.0f, 0.0f), rotate);
	}

	// Right button: up-down, left-right
	if ((m_buttons & VT_RIGHT) && !(m_buttons & VT_LEFT))
	{
		FPoint3 pos = pTarget->GetTrans();

		float updown = -(my - 0.5f) * m_fSpeed;
		float leftright = (mx - 0.5f) * m_fSpeed;
		if (updown != 0.0f) bUpDown = true;

		m_Velocity.x += leftright;
		m_Velocity.y += updown;
	}

	// Both buttons: pitch, roll
	if ((m_buttons & VT_LEFT) && (m_buttons & VT_RIGHT))
	{
		float updown = -(my - 0.5f) / 20.0f;
		float leftright = (mx - 0.5f) / 20.0f;
		if (fabs(updown) > fabs(leftright))
			pTarget->RotateLocal(FPoint3(1.0f, 0.0f, 0.0f), updown);
		else if (!m_bPreventRoll)
			pTarget->RotateLocal(FPoint3(0.0f, 0.0f, 1.0f), -leftright);
	}

	m_Velocity *= 0.9f;
	pTarget->TranslateLocal(m_Velocity);

	// allow the user to move up-down even in maintain-height mode
	bool bPreserveMaintain;
	if (bUpDown)
	{
		bPreserveMaintain = m_bMaintain;
		m_bMaintain = false;
	}
	KeepAboveGround();
	if (bUpDown)
		m_bMaintain = bPreserveMaintain;
}

void VFlyer::SetUpwardVelocity(float velocity)
{
	m_Velocity.y = velocity;
}


///////////////////////////////////////////////
// Quake-style navigation
//
QuakeFlyer::QuakeFlyer(float scale, float fHeightAboveTerrain, bool bMin)
 : vtTerrainFlyer(0.4f, fHeightAboveTerrain, bMin)	// hardcode scale override
{
	m_sWrap = 0;
	m_bNavEnable = true;
}

void QuakeFlyer::Eval()
{
	if (!m_bNavEnable)
		return;

	if (!m_bFollow)
	{
		vtFlyer::Eval();
		return;
	}

	if (!m_pHeightField)
		return;

	// Find the camera (or other target) that we are going to move
	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget) return;

	// Find the window we are navigating in
	vtScene* scene = vtGetScene();
	IPoint2	WinSize = scene->GetWindowSize();

	// save mousepoint
	if (m_LastMousePt.x == 0 && m_LastMousePt.y == 0)
		m_LastMousePt = m_pos;

	// mouse movement
	float leftright = -( (float) m_pos.x - m_LastMousePt.x) / WinSize.x;
	float updown = -( (float) m_pos.y - m_LastMousePt.y) / WinSize.y;

	// stop moving if mouse stopped
	if (m_flastmovex == leftright && m_flastmovey == updown)
		return;

	m_flastmovex = leftright;
	m_flastmovey = updown;

	// save mouse points
	m_LastMousePt = m_pos;

	// Minor hack... delay waits 8 eval calls so frame doesn't jump
	if (m_sWrap > 0)
	{
		m_sWrap++;
		if (m_sWrap >= 8)
			m_sWrap = 0;
		return;
	}

	pTarget->Rotate2(FPoint3(0,1,0), leftright);
	pTarget->Rotate2(FPoint3(1,0,0), updown);

	//  Left button down
	if ((m_buttons & VT_LEFT) && !(m_buttons & VT_RIGHT))
	{
		pTarget->Translate1(FPoint3(0.0f, -0.1f, 0.0f));
	}

	//  Right button down
	if ((m_buttons & VT_RIGHT) && !(m_buttons & VT_LEFT))
	{
		pTarget->Translate1(FPoint3(0.0f, 0.1f, 0.0f));
	}

	// conform to terrain
	KeepAboveGround();
}

//
// Move the viewpoint with the keyboard, like Quake does
// (untested)
//
void QuakeFlyer::OnKey(int key, int flags)
{
	char da_key = (char) key;

	if (!m_pHeightField)
		return;

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	switch (da_key)
	{
		case 't': // toggle
			m_bNavEnable = !m_bNavEnable;
			break;

		// flyer
		case 'w':  // forward
			pTarget->TranslateLocal(FPoint3(0, 0, -1 * m_fSpeed));
			break;
		case 's':  // back
			pTarget->TranslateLocal(FPoint3(0, 0, m_fSpeed));
			break;
		case 'a': // left
			pTarget->TranslateLocal(FPoint3(-1 * m_fSpeed, 0, 0));
			break;
		case 'd': // right
			pTarget->TranslateLocal(FPoint3(m_fSpeed, 0, 0));
			break;

		// shift held down
		case 'W':  // forward
			pTarget->TranslateLocal(FPoint3(0, 0, -3 * m_fSpeed));
			break;
		case 'S':  // back
			pTarget->TranslateLocal(FPoint3(0, 0, 3 * m_fSpeed));
			break;
		case 'A': // left
			pTarget->TranslateLocal(FPoint3(-3 * m_fSpeed, 0, 0));
			break;
		case 'D': // right
			pTarget->TranslateLocal(FPoint3(3 * m_fSpeed, 0, 0));
			break;
	}

	switch (key)
	{
		case 294:  // forward
			pTarget->TranslateLocal(FPoint3(0, 0, -1 * m_fSpeed));
			break;
		case 296:  // back
			pTarget->TranslateLocal(FPoint3(0, 0, m_fSpeed));
			break;
		case 293: // left
			pTarget->TranslateLocal(FPoint3(-1 * m_fSpeed, 0, 0));
			break;
		case 295: // right
			pTarget->TranslateLocal(FPoint3(m_fSpeed, 0, 0));
			break;
	}

	// conform to terrain
	KeepAboveGround();
}


//////////////////////////////////////////////////////////////////////////
// vtTrackball class implementation
//

vtTrackball::vtTrackball(float fRadius)
{
	m_Pos.Set(0.0f, 0.0f, fRadius);
	m_bRotate = false;
	m_bZoom = false;
	m_fZoomScale = 1.0f;

	// Default button assignments: can be changed
	m_rotate_button = VT_MIDDLE;
	m_rotate_modifier = 0;
	m_zoom_button = VT_MIDDLE;
	m_zoom_modifier = VT_SHIFT;
}

void vtTrackball::SetRadius(float fRadius)
{
	m_Pos.z = fRadius;
}

float vtTrackball::GetRadius()
{
	return m_Pos.z;
}

void vtTrackball::SetZoomScale(float s)
{
	m_fZoomScale = s;
}

void vtTrackball::SetRotateButton(int button, int modifier)
{
	m_rotate_button = button;
	m_rotate_modifier = modifier;
}

void vtTrackball::SetZoomButton(int button, int modifier)
{
	m_zoom_button = button;
	m_zoom_modifier = modifier;
}

bool vtTrackball::_IsRotate(vtMouseEvent &event)
{
	if (event.button != m_rotate_button) return false;
	if (m_rotate_modifier != 0 &&
		!(event.flags & m_rotate_modifier)) return false;
	if (m_rotate_modifier == 0 && event.flags != 0) return false;
	return true;
}

bool vtTrackball::_IsZoom(vtMouseEvent &event)
{
	if (event.button != m_zoom_button) return false;
	if (m_zoom_modifier != 0 &&
		!(event.flags & m_zoom_modifier)) return false;
	if (m_zoom_modifier == 0 && event.flags != 0) return false;
	return true;
}

void vtTrackball::OnMouse(vtMouseEvent &event)
{
	if (event.type == VT_DOWN)
	{
		if (!m_bRotate && _IsRotate(event))
		{
			m_Start = m_Pos;
			m_MouseStart = event.pos;
			m_bRotate = true;
		}
		if (!m_bZoom && _IsZoom(event))
		{
			m_Start = m_Pos;
			m_MouseStart = event.pos;
			m_bZoom = true;
		}
	}
	if (event.type == VT_MOVE)
	{
		if (m_bZoom)
			m_Pos.z = m_Start.z - (event.pos.y - m_MouseStart.y) / 200.0f * m_fZoomScale;
		if (m_bRotate)
		{
			m_Pos.x = m_Start.x + (event.pos.x - m_MouseStart.x) / 200.0f;
			m_Pos.y = m_Start.y + (event.pos.y - m_MouseStart.y) / 200.0f;
		}
	}
	if (event.type == VT_UP)
		m_bRotate = m_bZoom = false;
}

void vtTrackball::Eval()
{
	vtTransform *pTarget = (vtTransform *) GetTarget();
	if (!pTarget)
		return;

	pTarget->Identity();
	pTarget->Translate1(FPoint3(0.0, 0.0, m_Pos.z));
	pTarget->Rotate2(FPoint3(1.0f, 0.0f, 0.0f), -m_Pos.y);
	pTarget->Rotate2(FPoint3(0.0f, 1.0f, 0.0f), -m_Pos.x);
}


