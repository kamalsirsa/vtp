//
// TimeEngines.h
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TIMEENGINESH
#define TIMEENGINESH

#include "Engine.h"
#include <time.h>

/**
 * A TimeTarget is a kind of vtTarget which expects to be told what time
 * it is.
 */
class TimeTarget : public vtTarget
{
public:
	virtual void SetTime(time_t time) {}
};

///////////////////////////////////////////////////

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

	void GetCurrentTime();
	void SetSpeed(float factor);
	float GetSpeed() { return m_fSpeed; }
	void SetDate(int year, int month, int day);
	void SetLocalTime(int hr, int min, int sec);
	void SetGMT(int hr, int min, int sec);
//	void SetTime(time_t time);
	void GetTime(int &hr, int &min, int &sec);
	time_t GetTime();
	void Increment(int secs);

protected:
	void _UpdateTM();
	void _InformTarget();

	tm m_tm;			// always stores GMT
	time_t m_time;		// always stores GMT
	float m_fSeconds;
	float m_fSpeed;
	float m_last_time;
};

#endif	// TIMEENGINESH

