//
// Engine.cpp
//
// Copyright (c) 2001-2006 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Engine.h"
#include "Event.h"

vtEngine::vtEngine() : vtEnabledBase()
{
	m_pWindow = NULL;
}

osg::Referenced *vtEngine::GetTarget(unsigned int which)
{
	if (which < NumTargets())
		return m_Targets.GetAt(which);
	else
		return NULL;
}

void vtEngine::RemoveTarget(osg::Referenced *ptr)
{
	int index = m_Targets.Find(ptr);
	if (index != -1)
		m_Targets.RemoveAt(index);
}

void vtEngine::Eval()
{
}

void vtEngine::RemoveChild(vtEngine *pEngine)
{
	for (unsigned int i = 0; i < NumChildren(); i++)
	{
		if (m_Children[i] == pEngine)
			m_Children.erase(m_Children.begin()+i);
	}
}

void vtEngine::AddChildrenToList(vtArray<vtEngine*> &list, bool bEnabledOnly)
{
	if (bEnabledOnly && !GetEnabled())
		return;
	list.Append(this);
	for (unsigned int i = 0; i < NumChildren(); i++)
		GetChild(i)->AddChildrenToList(list, bEnabledOnly);
}

void vtEngine::OnMouse(vtMouseEvent &event)
{
}

void vtEngine::OnKey(int key, int flags)
{
}

void vtEngine::OnWindowSize(int width, int height)
{
}

//
// vtLastMouse
//
vtLastMouse::vtLastMouse() : vtEngine()
{
	m_buttons = VT_NONE;
	m_pos.Set(0, 0);
}

void vtLastMouse::OnMouse(vtMouseEvent &event)
{
	switch (event.type)
	{
	case VT_UP:
		m_buttons &= (~(int)event.button);
		break;
	case VT_DOWN:
		m_buttons |= (int)event.button;
		break;
	}
	m_pos = event.pos;
	m_flags = event.flags;
}

// Helper for GetNormalizedMouseCoords
void apply_dead_zone(float amount, float &val)
{
	if (val > 0) {
		val -= amount;
		if (val < 0)
			val = 0;
	}
	else {
		val += amount;
		if (val > 0) val = 0;
	}
	val *= (1.0f / (1.0f - amount));
}

#define DEAD_ZONE_SIZE 0.07f

/**
 * Returns the mouse coordinates in the window, normalized such that X and Y
 * range from -1 to 1 (left to right, top to bottom across the window.
 */
void vtLastMouse::GetNormalizedMouseCoords(float &mx, float &my)
{
	vtWindow *window = m_pWindow;
	if (!window) window = vtGetScene()->GetWindow(0);

	IPoint2	WinSize = window->GetSize();
	mx = (((float) m_pos.x / WinSize.x) - 0.5f) * 2;
	my = (((float) m_pos.y / WinSize.y) - 0.5f) * 2;
	apply_dead_zone(DEAD_ZONE_SIZE, mx);
	apply_dead_zone(DEAD_ZONE_SIZE, my);
}

//////////////////////////////////////////////////////////////////////

vtSimpleBillboardEngine::vtSimpleBillboardEngine(float fAngleOffset)
{
	m_fAngleOffset = fAngleOffset;
	m_bPitch = true;
}

void vtSimpleBillboardEngine::Eval()
{
	// Potential optimization: store last camera pos and only update on change

	//get the target and it's location
	for (unsigned int i = 0; i < NumTargets(); i++)
	{
		vtCamera *cam = vtGetScene()->GetCamera();
		vtTransform* pTarget = (vtTransform*) GetTarget(i);
		if (!pTarget)
			return;

		FPoint3 pos = pTarget->GetTrans();
		if (cam->IsOrtho())
		{
			// rotate them up to face an orthographic top-down camera
			pTarget->Identity();
			pTarget->SetTrans(pos);
			pTarget->RotateLocal(FPoint3(1,0,0), -PID2f);
		}
		else
		{
			FPoint3 camera_loc = cam->GetTrans();
			FPoint3 target_loc = pTarget->GetTrans();
			FPoint3 diff = camera_loc - target_loc;

			float angle = atan2f(-diff.z, diff.x);

			// TODO: make this more efficient!
			pTarget->Identity();
			pTarget->SetTrans(pos);
			pTarget->RotateLocal(FPoint3(0,1,0), m_fAngleOffset + angle);
		}
#if 0
		if (m_bPitch)
		{
			// also attempt to pitch (rotate around local X) toward the camera
			float diff_xz = sqrt(diff.x*diff.x+diff.z*diff.z);
			float pitch = atan2f(diff.y, diff_xz);
			pTarget->RotateLocal(FPoint3(1,0,0), pitch);
		}
#endif
	}
}

