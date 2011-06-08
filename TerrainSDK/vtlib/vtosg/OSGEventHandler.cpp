//
// OSGEventHandler.cpp
//
// Handle OSG events, turn them into VTP events.
//
// Copyright (c) 2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "OSGEventHandler.h"
#include "vtdata/vtLog.h"

#define GEA		osgGA::GUIEventAdapter				// shorthand
#define GEType	osgGA::GUIEventAdapter::EventType	// shorthand
#define GEMask	osgGA::GUIEventAdapter::ModKeyMask	// shorthand
#define GEKey	osgGA::GUIEventAdapter::KeySymbol	// shorthand

bool vtOSGEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	GEType etype = ea.getEventType();

	//if (etype != osgGA::GUIEventAdapter::FRAME)
	//	VTLOG("getEventType %d button %d\n", etype, ea.getButton());

	switch (etype)
	{
	case GEType::KEYDOWN:
	case GEType::KEYUP:
		handleKeyEvent(ea);
		break;
	case GEType::PUSH:
	case GEType::RELEASE:
	case GEType::DOUBLECLICK:
	case GEType::DRAG:
	case GEType::MOVE:
		handleMouseEvent(ea);
		break;
	case GEType::FRAME:
		break;
	case GEType::RESIZE:
		VTLOG("RESIZE %d %d\n", ea.getWindowWidth(), ea.getWindowHeight());
		vtGetScene()->SetWindowSize(ea.getWindowWidth(), ea.getWindowHeight());
		break;
	case GEType::SCROLL:
		VTLOG("SCROLL\n");
		break;
	case GEType::CLOSE_WINDOW:
		VTLOG("CLOSE_WINDOW\n");
		break;
	case GEType::QUIT_APPLICATION:
		VTLOG("QUIT_APPLICATION\n");
		break;
	}
	return false;
}

void vtOSGEventHandler::handleKeyEvent(const osgGA::GUIEventAdapter& ea)
{
	int vtkey = ea.getKey();

	int flags = 0;
	int mkm = ea.getModKeyMask();
	if (mkm & GEMask::MODKEY_SHIFT) flags |= VT_SHIFT;
	if (mkm & GEMask::MODKEY_CTRL) flags |= VT_CONTROL;
	if (mkm & GEMask::MODKEY_ALT) flags |= VT_ALT;

	switch (ea.getKey())
	{
	case GEKey::KEY_Home: vtkey = VTK_HOME; break;
	case GEKey::KEY_Left: vtkey = VTK_LEFT; break;
	case GEKey::KEY_Up: vtkey = VTK_UP; break;
	case GEKey::KEY_Right: vtkey = VTK_RIGHT; break;
	case GEKey::KEY_Down: vtkey = VTK_DOWN; break;

	case GEKey::KEY_Page_Up: vtkey = VTK_PAGEUP; break;
	case GEKey::KEY_Page_Down: vtkey = VTK_PAGEDOWN; break;

	case GEKey::KEY_F1 : vtkey = VTK_F1; break;
	case GEKey::KEY_F2 : vtkey = VTK_F2; break;
	case GEKey::KEY_F3 : vtkey = VTK_F3; break;
	case GEKey::KEY_F4 : vtkey = VTK_F4; break;
	case GEKey::KEY_F5 : vtkey = VTK_F5; break;
	case GEKey::KEY_F6 : vtkey = VTK_F6; break;
	case GEKey::KEY_F7 : vtkey = VTK_F7; break;
	case GEKey::KEY_F8 : vtkey = VTK_F8; break;
	case GEKey::KEY_F9 : vtkey = VTK_F9; break;
	case GEKey::KEY_F10: vtkey = VTK_F10; break;
	case GEKey::KEY_F11: vtkey = VTK_F11; break;
	case GEKey::KEY_F12: vtkey = VTK_F12; break;

	case GEKey::KEY_Shift_L: vtkey = VTK_SHIFT; break;
	case GEKey::KEY_Shift_R: vtkey = VTK_SHIFT; break;
	case GEKey::KEY_Control_L: vtkey = VTK_CONTROL; break;
	case GEKey::KEY_Control_R: vtkey = VTK_CONTROL; break;
	case GEKey::KEY_Alt_L: vtkey = VTK_ALT; break;
	case GEKey::KEY_Alt_R: vtkey = VTK_ALT; break;
	}
	vtGetScene()->OnKey(vtkey, flags);
}

void vtOSGEventHandler::handleMouseEvent(const osgGA::GUIEventAdapter& ea)
{
	// Turn OSG mouse event into a VT mouse event
	vtMouseEvent event;
	event.flags = 0;
	event.pos.Set(ea.getX(), ea.getWindowHeight()-1-ea.getY());		// Flip Y from OSG to everyone else

	int mkm = ea.getModKeyMask();
	if (mkm & GEMask::MODKEY_SHIFT) event.flags |= VT_SHIFT;
	if (mkm & GEMask::MODKEY_CTRL) event.flags |= VT_CONTROL;
	if (mkm & GEMask::MODKEY_ALT) event.flags |= VT_ALT;

	GEType etype = ea.getEventType();
	switch (etype)
	{
	case GEType::PUSH:
		event.type = VT_DOWN;
		if (ea.getButton() == GEA::LEFT_MOUSE_BUTTON)
			event.button = VT_LEFT;
		else if (ea.getButton() == GEA::MIDDLE_MOUSE_BUTTON)
			event.button = VT_MIDDLE;
		else if (ea.getButton() == GEA::RIGHT_MOUSE_BUTTON)
			event.button = VT_RIGHT;
		vtGetScene()->OnMouse(event);
		break;
	case GEType::RELEASE:
		event.type = VT_UP;
		if (ea.getButton() == GEA::LEFT_MOUSE_BUTTON)
			event.button = VT_LEFT;
		else if (ea.getButton() == GEA::MIDDLE_MOUSE_BUTTON)
			event.button = VT_MIDDLE;
		else if (ea.getButton() == GEA::RIGHT_MOUSE_BUTTON)
			event.button = VT_RIGHT;
		vtGetScene()->OnMouse(event);
		break;
	case GEType::DOUBLECLICK:
		break;
	case GEType::DRAG:
	case GEType::MOVE:
		event.type = VT_MOVE;
		if (ea.getButton() == 0)
			event.button = VT_LEFT;
		else if (ea.getButton() == 1)
			event.button = VT_MIDDLE;
		else if (ea.getButton() == 2)
			event.button = VT_RIGHT;
		vtGetScene()->OnMouse(event);
		break;
	}
}