//
// Engine.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "Engine.h"

vtEngine::vtEngine() : vtEnabledBase()
{
}

void vtEngine::Eval()
{
}

void vtEngine::OnMouse(vtMouseEvent &event)
{
}

void vtEngine::OnKey(int key, int flags)
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
	const char *name = GetName2();

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
	val *= (1.0 / (1.0 - amount));
}

#define DEAD_ZONE_SIZE 0.07f

/** 
 * Returns the mouse coordinates in the window, normalized such that X and Y
 * range from -1 to 1 (left to right, top to bottom across the window.
 */
void vtLastMouse::GetNormalizedMouseCoords(float &mx, float &my)
{
	IPoint2	WinSize = vtGetScene()->GetWindowSize();
	mx = (((float) m_pos.x / WinSize.x) - 0.5f) * 2;
	my = (((float) m_pos.y / WinSize.y) - 0.5f) * 2;
	apply_dead_zone(DEAD_ZONE_SIZE, mx);
	apply_dead_zone(DEAD_ZONE_SIZE, my);
}

