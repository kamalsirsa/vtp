//
// NavEngines.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "vtlib/core/vtTin3d.h"
#include "vtdata/vtLog.h"
#include "NavEngines.h"
#include "Event.h"

#define GRAVITY_CONSTANT 9.81	// g = 9.81 meters/sec^2

//
// vtFlyer: basic class, moves target based on mouse position
//
vtFlyer::vtFlyer(float fSpeed, bool bPreventRoll) : vtLastMouse()
{
	m_fSpeed = fSpeed;
	m_bAlwaysMove = false;

	// by default, all degrees of freedom are allowed
	for (int i = 0; i < 6; i++)
		m_bDOF[i] = true;

	m_bDOF[DOF_ROLL] = !bPreventRoll;
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

	float elapsed = vtGetFrameTime();

	float mx, my;
	GetNormalizedMouseCoords(mx, my);

	//	Left button: forward-backward, yaw
	if (m_bAlwaysMove ||
		((m_buttons & VT_LEFT) && !(m_buttons & VT_RIGHT)))
	{
		float trans = my * m_fSpeed * elapsed;
		float rotate = -mx * elapsed;

		if (m_bDOF[DOF_Z])
			pTarget->TranslateLocal(FPoint3(0.0f, 0.0f, trans));

		if (m_bDOF[DOF_YAW])
		{
			if (m_bDOF[DOF_ROLL])
				pTarget->RotateLocal(FPoint3(0.0f, 1.0f, 0.0f), rotate);
			else
				pTarget->RotateParent(FPoint3(0.0f, 1.0f, 0.0f), rotate);
		}

/*		static float temp = 0;
		temp += elapsed;
		if (temp > 1)
		{
			VTLOG("m_fSpeed %f, trans w/o elapsed %.1f\n", m_fSpeed, my * m_fSpeed);
			temp = 0;
		}*/
	}

	//  Right button: up-down, left-right
	if ((m_buttons & VT_RIGHT) && !(m_buttons & VT_LEFT))
	{
		float updown = -my * m_fSpeed * elapsed;
		float leftright = mx * m_fSpeed * elapsed;

		if (m_bDOF[DOF_X])
			pTarget->TranslateLocal(FPoint3(leftright, 0, 0.0f));

		if (m_bDOF[DOF_Y])
			pTarget->TranslateLocal(FPoint3(0, updown, 0.0f));
	}

	//  Both buttons: pitch, roll
	if ((m_buttons & VT_LEFT) && (m_buttons & VT_RIGHT))
	{
		float updown = -my * elapsed;
		float leftright = mx * elapsed;

		if (fabs(updown) > fabs(leftright) && m_bDOF[DOF_PITCH])
			pTarget->RotateLocal(FPoint3(1.0f, 0.0f, 0.0f), updown);
		else if (m_bDOF[DOF_ROLL])
			pTarget->RotateLocal(FPoint3(0.0f, 0.0f, 1.0f), -leftright);
	}
	DoKeyNavigation();
}

void vtFlyer::DoKeyNavigation()
{
	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	float elapsed = vtGetFrameTime();

	vtScene *sc = vtGetScene();

	// shift: move faster
	float fMult = 1;
	if (sc->GetKeyState(VTK_SHIFT))
		fMult = 5;

	bool up = sc->GetKeyState(VTK_UP);
	bool down = sc->GetKeyState(VTK_DOWN);
	bool right = sc->GetKeyState(VTK_RIGHT);
	bool left = sc->GetKeyState(VTK_LEFT);

	if (sc->GetKeyState(VTK_CONTROL))
	{
		//  With control key: translate up-down, left-right
		if ((up || down) && m_bDOF[DOF_Y])
		{
			float updown = 0.2 * m_fSpeed * fMult * elapsed;
			if (down)
				updown = -updown;
			pTarget->TranslateLocal(FPoint3(0, updown, 0.0f));
		}
		if ((right || left) && m_bDOF[DOF_X])
		{
			float leftright = 0.2 * m_fSpeed * fMult * elapsed;
			if (left)
				leftright = -leftright;
			pTarget->TranslateLocal(FPoint3(leftright, 0, 0.0f));
		}
	}
	else
	{
		if ((up || down) && m_bDOF[DOF_Z])
		{
			float trans = 0.2 * m_fSpeed * fMult * elapsed;
			if (up)
				trans = -trans;

			pTarget->TranslateLocal(FPoint3(0.0f, 0.0f, trans));
		}

		if ((right || left) && m_bDOF[DOF_YAW])
		{
			float rotate = 0.4 * fMult * elapsed;
			if (right)
				rotate = -rotate;

			if (m_bDOF[DOF_ROLL])
				pTarget->RotateLocal(FPoint3(0.0f, 1.0f, 0.0f), rotate);
			else
				pTarget->RotateParent(FPoint3(0.0f, 1.0f, 0.0f), rotate);
		}
	}

	// pitch
	bool pup = sc->GetKeyState(VTK_PAGEUP);
	bool pdown = sc->GetKeyState(VTK_PAGEDOWN);
	if ((pup || pdown) && m_bDOF[DOF_PITCH])
	{
		float updown = 0.2 * fMult * elapsed;
		if (pdown)
			updown = -updown;
		pTarget->RotateLocal(FPoint3(1.0f, 0.0f, 0.0f), updown);
	}

}

//
// Fly engine specifically for an orthographic camera (e.g. top-down view)
//
vtOrthoFlyer::vtOrthoFlyer(float fSpeed) : vtFlyer(fSpeed, true)
{
}

void vtOrthoFlyer::Eval()
{
	float elapsed = vtGetFrameTime();

	vtCamera *pCamera = (vtCamera*) GetTarget();
	if (!pCamera)
		return;

	float mx, my;
	GetNormalizedMouseCoords(mx, my);

	//	Left button: forward-backward (zoom), yaw
	if ((m_buttons & VT_LEFT) && !(m_buttons & VT_RIGHT))
	{
		float trans = my * elapsed;
		float rotate = -mx * elapsed;

		pCamera->SetWidth(pCamera->GetWidth() * (1.0 + trans));
		pCamera->RotateLocal(FPoint3(0.0f, 0.0f, 1.0f), rotate);
	}

	//  Right button: up-down, left-right
	if ((m_buttons & VT_RIGHT) && !(m_buttons & VT_LEFT))
	{
		float updown = -my * m_fSpeed * elapsed;
		float leftright = mx * m_fSpeed * elapsed;

		pCamera->TranslateLocal(FPoint3(leftright, updown, 0.0f));
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
	m_fCurrentSpeed = 0;
}

void vtTerrainFlyer::FollowTerrain(bool bFollow)
{
	m_bFollow = bFollow;
}

void vtTerrainFlyer::Eval()
{
	float elapsed = vtGetFrameTime();
	vtTransform *pTarget = (vtTransform*) GetTarget();

	if (elapsed == 0 || !pTarget) // safety check
		return;

	FPoint3 previous_pos = pTarget->GetTrans();

	vtFlyer::Eval();
	KeepAboveGround();

	FPoint3 current_pos = pTarget->GetTrans();
	m_fCurrentSpeed = (current_pos - previous_pos).Length() / elapsed;
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


//
// vtPanoFlyer: moves target based on mouse position, like a QTVR or other panorama viewer
//
vtPanoFlyer::vtPanoFlyer(float fSpeed, float fHeightAboveTerrain, bool bMin)
 : vtTerrainFlyer(fSpeed, fHeightAboveTerrain, bMin)
{
	m_Velocity = 0.0f;
}

void vtPanoFlyer::Eval()
{
	float elapsed = vtGetFrameTime();

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	FPoint3 previous_pos = pTarget->GetTrans();

	float mx, my;
	GetNormalizedMouseCoords(mx, my);

	//	No button: pitch, yaw
	if (!(m_buttons & VT_RIGHT))
	{
		float rotate = -mx * elapsed;
		float updown = -my * elapsed;

		pTarget->RotateLocal(FPoint3(1.0f, 0.0f, 0.0f), updown);
		pTarget->RotateParent(FPoint3(0.0f, 1.0f, 0.0f), rotate);
	}

	//  left button: Accelerate forward
	if ((m_buttons & VT_LEFT) && !(m_buttons & VT_RIGHT))
	{
		// speed up
		m_Velocity += (m_fSpeed * elapsed * .25);

		if (m_Velocity > m_fSpeed) m_Velocity = m_fSpeed; // clamp at m_fSpeed

		float trans = -m_Velocity * elapsed;

		pTarget->TranslateLocal(FPoint3(0.0f, 0.0f, trans));
	}
	else
	{
		// slow down
		if (m_Velocity > 0.0)
		{
			m_Velocity -= (m_fSpeed * elapsed * .05);
			if (m_Velocity < 0) m_Velocity = 0.0f; // clamp at 0
		}
	}

	//  Right button: up-down, left-right
	if (!(m_buttons & VT_LEFT) && (m_buttons & VT_RIGHT))
	{
		float updown = -my * m_fSpeed * elapsed;
		float leftright = mx * m_fSpeed * elapsed;

		pTarget->TranslateLocal(FPoint3(leftright, updown, 0.0f));
	}

	//  Both buttons: nothing
	if ((m_buttons & VT_LEFT) && (m_buttons & VT_RIGHT))
	{
	}

	KeepAboveGround();

	FPoint3 current_pos = pTarget->GetTrans();
	m_fCurrentSpeed = (current_pos - previous_pos).Length() / elapsed;
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
		float mini, maxi;
		m_pTin->GetHeightExtents(mini, maxi);
		mini -= 4.5;
		maxi += 100;
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
	m_last_time = -1.0f;
}

void VFlyer::Eval()
{
	float elapsed = vtGetFrameTime();

	vtTransform *pTarget = (vtTransform*) GetTarget();
	if (!pTarget)
		return;

	float mx, my;
	GetNormalizedMouseCoords(mx, my);

	// Left button: forward-backward, yaw
	if ((m_buttons & VT_LEFT) && !(m_buttons & VT_RIGHT))
	{
		float trans = my * m_fSpeed * elapsed * 8;
		float rotate = -mx * elapsed;

		m_Velocity.z += trans;
		if (m_bDOF[DOF_ROLL])
			pTarget->RotateLocal(FPoint3(0.0f, 1.0f, 0.0f), rotate);
		else
			pTarget->RotateParent(FPoint3(0.0f, 1.0f, 0.0f), rotate);
	}

	// Right button: up-down, left-right
	bool bUpDown = false;
	if ((m_buttons & VT_RIGHT) && !(m_buttons & VT_LEFT))
	{
		FPoint3 pos = pTarget->GetTrans();

		float updown = -my * m_fSpeed * elapsed * 8;
		float leftright = mx * m_fSpeed * elapsed * 8;
		if (updown != 0.0f) bUpDown = true;

		m_Velocity.x += leftright;
		m_Velocity.y += updown;
	}

	// Both buttons: pitch, roll
	if ((m_buttons & VT_LEFT) && (m_buttons & VT_RIGHT))
	{
		float updown = -my * elapsed;
		float leftright = mx * elapsed;
		if (fabs(updown) > fabs(leftright))
			pTarget->RotateLocal(FPoint3(1.0f, 0.0f, 0.0f), updown);
		else if (m_bDOF[DOF_ROLL])
			pTarget->RotateLocal(FPoint3(0.0f, 0.0f, 1.0f), -leftright);
	}

	// dampen velocity based on elapsed time
	if (elapsed > 0)
	{
		float damp = powf(0.5f, elapsed * 10);
		m_Velocity *= damp;
	}
	pTarget->TranslateLocal(m_Velocity * elapsed);

/*	static float temp = 0;
	temp += elapsed;
	if (temp > 1)
	{
		VTLOG("m_fSpeed %f, m_Velocity %.1f %.1f, %.1f\n", m_fSpeed, m_Velocity.x, m_Velocity.y, m_Velocity.z);
		temp = 0;
	}*/

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

void VFlyer::SetVerticalVelocity(float velocity)
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
	m_Pos.Set(0, 0, fRadius);
	m_Trans.Set(0, 0, 0);
	m_bRotate = false;
	m_bZoom = false;
	m_fZoomScale = 1.0f;

	// Default button assignments: can be changed
	m_rotate_button = VT_MIDDLE;
	m_rotate_modifier = 0;
	m_zoom_button = VT_MIDDLE;
	m_zoom_modifier = VT_SHIFT;
	// default to no translation
	m_trans_button = -1;
	m_zoom_modifier = -1;
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

void vtTrackball::SetRotateButton(int button, int modifier, bool bExact)
{
	m_rotate_button = button;
	m_rotate_modifier = modifier;
	m_rotate_exact = bExact;
}

void vtTrackball::SetZoomButton(int button, int modifier, bool bExact)
{
	m_zoom_button = button;
	m_zoom_modifier = modifier;
	m_zoom_exact = bExact;
}

void vtTrackball::SetTranslateButton(int button, int modifier, bool bExact)
{
	m_trans_button = button;
	m_trans_modifier = modifier;
	m_trans_exact = bExact;
}

bool vtTrackball::_IsRotate()
{
	if (m_rotate_exact)
	{
		if (m_buttons != m_rotate_button) return false;
	}
	else
	{
		if ((m_buttons & m_rotate_button) == 0) return false;
	}
	if (m_rotate_modifier != 0 &&
		!(m_flags & m_rotate_modifier)) return false;
	if (m_rotate_modifier == 0 && m_flags != 0) return false;
	return true;
}

bool vtTrackball::_IsZoom()
{
	if (m_zoom_exact)
	{
		if (m_buttons != m_zoom_button) return false;
	}
	else
	{
		if ((m_buttons & m_zoom_button) == 0) return false;
	}
	if (m_zoom_button != 0 && (m_buttons & m_zoom_button) != m_zoom_button) return false;
	if (m_zoom_modifier != 0 &&
		!(m_flags & m_zoom_modifier)) return false;
	if (m_zoom_modifier == 0 && m_flags != 0) return false;
	return true;
}

bool vtTrackball::_IsTranslate()
{
	if (m_trans_exact)
	{
		if (m_buttons != m_trans_button) return false;
	}
	else
	{
		if ((m_buttons & m_trans_button) == 0) return false;
	}
	if (m_trans_button != 0 && (m_buttons & m_trans_button) != m_trans_button) return false;
	if (m_trans_modifier != 0 &&
		!(m_flags & m_trans_modifier)) return false;
	if (m_trans_modifier == 0 && m_flags != 0) return false;
	return true;
}

void vtTrackball::OnMouse(vtMouseEvent &event)
{
	vtLastMouse::OnMouse(event);

	if (!m_bRotate && _IsRotate())
	{
		m_Start = m_Pos;
		m_MouseStart = event.pos;
		m_bRotate = true;
		m_bZoom = false;
		m_bTrans = false;
	}
	else if (!m_bZoom && _IsZoom())
	{
		m_Start = m_Pos;
		m_MouseStart = event.pos;
		m_bRotate = false;
		m_bZoom = true;
		m_bTrans = false;
	}
	else if (!m_bTrans && _IsTranslate())
	{
		m_Start = m_Trans;
		m_MouseStart = event.pos;
		m_bRotate = false;
		m_bZoom = false;
		m_bTrans = true;
	}
	if (m_bRotate && !_IsRotate())
		m_bRotate = false;
	if (m_bZoom && !_IsZoom())
		m_bZoom = false;
	if (m_bTrans && !_IsTranslate())
		m_bTrans = false;

	if (event.type == VT_MOVE)
	{
		if (m_bZoom)
			m_Pos.z = m_Start.z - (event.pos.y - m_MouseStart.y) / 200.0f * m_fZoomScale;
		if (m_bRotate)
		{
			m_Pos.x = m_Start.x + (event.pos.x - m_MouseStart.x) / 200.0f;
			m_Pos.y = m_Start.y + (event.pos.y - m_MouseStart.y) / 200.0f;
		}
		if (m_bTrans)
		{
			m_Trans.x = m_Start.x - (event.pos.x - m_MouseStart.x) / 100.0f;
			m_Trans.y = m_Start.y + (event.pos.y - m_MouseStart.y) / 100.0f;
		}
	}
}

void vtTrackball::SetDirection(float fTheta, float fPhi)
{
	m_Pos.x = fTheta;
	m_Pos.y = fPhi;
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

	pTarget->TranslateLocal(m_Trans);
}

