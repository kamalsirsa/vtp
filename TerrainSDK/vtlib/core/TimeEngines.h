//
// TimeEngines.h
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TIMEENGINESH
#define TIMEENGINESH

#include "Engine.h"
#include "vtdata/vtTime.h"

/**
 * A TimeTarget is a kind of vtTarget which expects to be told what time
 * it is.
 */
class TimeTarget : public vtTarget
{
public:
	virtual void SetTime(const vtTime &time) {}
};

/**
 * The TimeEngine class keeps track of time of day (hours, minutes, seconds
 * since midnight) and when evaluated, informs each of its target of the time.
 *
 * Time can advance at either real time (1 second simulated = 1 real second),
 * or at any faster or slower rate.
 */
class TimeEngine : public vtEngine
{
public:
	TimeEngine(int start_hour = -1);

	void Eval();

	void SetTime(vtTime &time);
	void GetTime(vtTime &time) const;
	vtTime GetTime() const;

	void SetSpeed(float factor);
	float GetSpeed() { return m_fSpeed; }

protected:
	void _InformTarget();

	float m_fSeconds;
	float m_fSpeed;
	float m_last_time;

	vtTime m_time;
};

#endif	// TIMEENGINESH

