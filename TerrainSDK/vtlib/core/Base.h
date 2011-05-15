//
// Base.h
//
// Copyright (c) 2001-2011 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTLIB_BASEH
#define VTLIB_BASEH

class vtCamera;
class vtGroup;
class vtEngine;
struct vtMouseEvent;

/** \defgroup sg Scene Graph
 * These classes are used for scene graph functionality: geometry, cameras,
 * rendering, LOD, and simulation.
 */
/*@{*/

/**
 * This class simply provides the ability to store whether an object is
 * "enabled" or not.  This is generally useful, such as for Nodes or Engines
 * which can be turned on and off.
 */
class vtEnabledBase
{
public:
	vtEnabledBase() { m_bEnabled = true; }

	virtual void SetEnabled(bool bOn) { m_bEnabled = bOn; }
	bool GetEnabled() { return m_bEnabled; }

protected:
	bool m_bEnabled;
};

/** This class is a placeholder parent class for all objects which can
 * be the target of an Engine (vtEngine).  Given a vtTarget point, you
 * can use dynamic_cast<> to downcast safely to any specific child class.
 */
class vtTarget
{
public:
	// need at least one method to make this class polymorphic
	virtual void PlaceHolder() {}
};

// Visibility return codes
#define	VT_Visible	1
#define	VT_AllVisible	2

/*@}*/	// Group sg

#endif	// VTLIB_BASEH

