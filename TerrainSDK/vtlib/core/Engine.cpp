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
	m_pTarget = NULL;
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
// vtLastMouse is a simple engine which remembers the state of
// the mouse buttons.
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
