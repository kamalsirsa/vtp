//
// IntersectionEngine.h
//
// Copyright (c) 2001-2009 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef INTERSECTIONENGINE
#define INTERSECTIONENGINE

#include "Roads.h"
#include "Engine.h"

/** \addtogroup transp */
/*@{*/

//traffic light times
#define LT_SWITCH_TIME 10
#define LT_SWITCH_TIME2 1  //for yellow lights.

/**
 * Operates signal lights.  changes the lights at a node.
 * based on a state machine.
 * engine creates states and simply cycles through them.
 *
 * Assumes that the initial color of lights at a road determines the relationship of lights
 * at that intersection.  for example, in a normal intersection, the non-perpendicular road
 * segments should have the same color - one pair green, the other red.
 */
class vtIntersectionEngine : public vtEngine
{
public:
	//builds the intersection engine.  links it to the node.
	vtIntersectionEngine(NodeGeom *node);
	~vtIntersectionEngine();
	void Eval();

protected:
	NodeGeom *m_pNode;
	LightStatus *m_Lights;	//an array of light states

	int m_iCurState;
	int m_iTotalStates;

	float m_fNextChange;
	bool m_bFirstTime;
};

/*@}*/  // transp

#endif	// INTERSECTIONENGINE

