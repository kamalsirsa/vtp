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

class TimeTarget : public vtTarget
{
public:
	virtual void SetTime(time_t time) {}
};

///////////////////////////////////////////////////

class TimeEngine : public vtEngine
{
public:
	TimeEngine(int start_hour = -1);

	void Eval();

	void GetCurrentTime();
	void SetSpeed(float factor);
	void SetTime(int hr, int min, int sec);
//	void SetTime(time_t time);
	void GetTime(int &hr, int &min, int &sec);
	void Increment(int secs);

protected:
	void _UpdateTM();
	void _InformTarget();

	tm m_tm;
	time_t m_time;
	float m_fSeconds;
	float m_fSpeed;
	float m_last_time;
};

#endif	// TIMEENGINESH

