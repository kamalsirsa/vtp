//
// Engine.h
//
// Copyright (c) 2001 Virtual Terrain Project
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

	void *GetTarget() { return m_pTarget; }
	void SetTarget(void *ptr) { m_pTarget = ptr; }

	void SetName2(const char *str) { m_strName = str; }
	const char *GetName2() { return m_strName; }

	virtual void OnMouse(vtMouseEvent &event);
	virtual void OnKey(int key, int flags);

	virtual void Eval();

protected:
	void	 *m_pTarget;
	vtString m_strName;
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

	/** The state of the mouse buttons */
	int m_buttons;

	/** The location of the mouse cursor */
	IPoint2 m_pos;
};

#endif
