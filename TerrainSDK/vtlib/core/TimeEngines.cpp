//
// TimeEngines.cpp
//
// Copyright (c) 2001-2003 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TimeEngines.h"

///////////////////////////////////////////////////////

TimeEngine::TimeEngine(int start_hour) : vtEngine()
{
	// default to current time
	GetCurrentTime();

	m_last_time = -1.0f;
	m_fSpeed = 0.0f;
	m_fSeconds = 0.0f;

	if (start_hour != -1)
		SetTime(start_hour, 0, 0);
}

void TimeEngine::_UpdateTM()
{
	struct tm *t = gmtime(&m_time);
	m_tm = *t;
}

void TimeEngine::GetCurrentTime()
{
	time(&m_time);
	_UpdateTM();
}

/*void TimeEngine::SetTime(time_t time)
{
	m_time = time;
	_UpdateTM();
}*/

void TimeEngine::SetTime(int hr, int min, int sec)
{
	m_tm.tm_hour = hr;
	m_tm.tm_min = min;
	m_tm.tm_sec = sec;
	m_time = mktime(&m_tm);
}

void TimeEngine::GetTime(int &hr, int &min, int &sec)
{
	hr = m_tm.tm_hour;
	min = m_tm.tm_min;
	sec = m_tm.tm_sec;
}

time_t TimeEngine::GetTime()
{
	return m_time;
}

void TimeEngine::SetSpeed(float factor)
{
	m_fSpeed = factor;
}

void TimeEngine::Increment(int secs)
{
	m_time += secs;
	_UpdateTM();
}

void TimeEngine::Eval()
{
	// increment
	if (m_fSpeed == 0.0f)
		return;

	float time = vtGetTime();
	if (m_last_time == -1.0f)
		m_last_time = time;
	float elapsed = (time - m_last_time);
	m_fSeconds += (elapsed * m_fSpeed);
	if (m_fSeconds > 1.0f)
	{
		int full = (int) m_fSeconds;
		Increment(full);
		_InformTarget();
		m_fSeconds -= full;
	}
	m_last_time = time;
}

void TimeEngine::_InformTarget()
{
	for (int i = 0; i < NumTargets(); i++)
	{
		TimeTarget* pTarget = (TimeTarget *)GetTarget(i);
		pTarget->SetTime(m_time);
	}
}

