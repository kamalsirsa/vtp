//
// Engine.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTENGINEH
#define VTENGINEH

#include "vtdata/vtString.h"

/**
 * The vtEngine class represents an Engine, which is a convenient way to
 * encapsulate an application behavior which occurs every frame.  An Engine
 * typically has a target, which is another object which it affects.  The
 * Engine's method Eval() is called each frame, at which point it can do
 * whatever simulation or computation it desires, and then update its target.
 * \par
 * For example, you may have a node in your scene graph which represents a fish.
 * You could then create a Engine class (e.g. FishEngine, subclass of vtEngine)
 * which simulates the behavior of a fish.  For each fish you create, you would
 * also create a FishEngine and set the Engine's target to be the fish.
 */
class vtEngine : public vtEnabledBase
{
public:
	vtEngine();

	vtTarget *GetTarget(int which = 0)
	{
		if (which < NumTargets())
			return m_pTargets.GetAt(which);
		else
			return NULL;
	}
	void SetTarget(vtTarget *ptr) { m_pTargets.SetAt(0, ptr); }
	void AddTarget(vtTarget *ptr) { m_pTargets.Append(ptr); }
	int NumTargets() { return m_pTargets.GetSize(); }

	void SetName2(const char *str) { m_strName = str; }
	const char *GetName2() { return m_strName; }

	virtual void OnMouse(vtMouseEvent &event);
	virtual void OnKey(int key, int flags);
	virtual void OnWindowSize(int width, int height);
	virtual void Eval();

protected:
	Array<vtTarget*> m_pTargets;
	vtString		 m_strName;
};

/**
 * This simple engine extends the base class vtEngine with the ability to
 * remember the last state of the mouse.  This is useful for engines which
 * simply need to know mouse state (cursor position and button state).  They
 * can simply subclass from vtLastMouse and use the inherited fields rather
 * than having to catch mouse events.
 */
class vtLastMouse : public vtEngine
{
public:
	vtLastMouse();

	void OnMouse(vtMouseEvent &event);

	void GetNormalizedMouseCoords(float &mx, float &my);

	/** The state of the mouse buttons */
	int m_buttons;

	/** The location of the mouse cursor */
	IPoint2 m_pos;

	/** Last known state of the modifier flags */
	int m_flags;
};

/**
 * A simple "Billboard" engine which turns its target to face the
 * camera each frame.
 */
class SimpleBillboardEngine : public vtEngine
{
public:
	SimpleBillboardEngine(float fAngleOffset = 0.0f);
	void Eval();

	float m_fAngleOffset;
};

#endif	// VTENGINEH

