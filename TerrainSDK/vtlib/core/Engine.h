//
// Engine.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTENGINEH
#define VTENGINEH

class vtWindow;

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
	virtual ~vtEngine() {}

	/**
	 * Get a target from the engine.  Most engines will have only a single
	 * target.
	 * \param which The number of the target (default 0).
	 */
	vtTarget *GetTarget(unsigned int which = 0)
	{
		if (which < NumTargets())
			return m_Targets.GetAt(which);
		else
			return NULL;
	}
	/**
	 * Adds a target to the engine.
	 */
	void AddTarget(vtTarget *ptr) { m_Targets.Append(ptr); }
	/**
	 * Sets a single target for this engine (for backward compatibility.)
	 */
	void SetTarget(vtTarget *ptr) { m_Targets.SetAt(0, ptr); }

	/// Return the number of targets for this engine.
	unsigned int NumTargets() { return m_Targets.GetSize(); }

	void SetName2(const char *str) { m_strName = str; }
	const char *GetName2() { return m_strName; }

	/// Virtual handler, to catch mouse events, can be overridden by your engine class.
	virtual void OnMouse(vtMouseEvent &event);

	/// Virtual handler, to catch keyboard events, can be overridden by your engine class.
	virtual void OnKey(int key, int flags);

	/// Virtual handler, to catch resize events, can be overridden by your engine class.
	virtual void OnWindowSize(int width, int height);

	/** Virtual handler, will be called every frame to do the work of the engine.  You
	 * must override this if you want your engine to be useful.
	 */
	virtual void Eval();

	// an engine may be associate with a window
	void SetWindow(vtWindow *pWin) { m_pWindow = pWin; }
	vtWindow *GetWindow() { return m_pWindow; }

	// Engine tree methods
	void AddChild(vtEngine *pEngine) { m_Children.Append(pEngine); }
	void RemoveChild(vtEngine *pEngine);
	vtEngine *GetChild(unsigned int i) { return m_Children[i]; }
	unsigned int NumChildren() { return m_Children.GetSize(); }

	void AddChildrenToList(Array<vtEngine*> &list);

protected:
	Array<vtTarget*> m_Targets;
	Array<vtEngine*> m_Children;
	vtString		 m_strName;
	vtWindow		*m_pWindow;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS

class vtEngineArray : public Array<vtEngine*>
{
public:
	vtEngineArray(vtEngine *pTop)
	{
		if (pTop)
			pTop->AddChildrenToList(*this);
	}
};

#endif	// DOXYGEN_SHOULD_SKIP_THIS

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
	virtual ~vtLastMouse() {}

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
	virtual ~SimpleBillboardEngine() {}

	void SetPitch(bool bFlag) { m_bPitch = bFlag; }

	void Eval();

	bool m_bPitch;
	float m_fAngleOffset;
};

#endif	// VTENGINEH

