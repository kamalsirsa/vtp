//
// TimeEngines.cpp
//
// Copyright (c) 2001-2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TimeEngines.h"

TimeEngine::TimeEngine(int start_hour) : vtEngine()
{
	m_last_time = -1.0f;
	m_fSpeed = 0.0f;
	m_fSeconds = 0.0f;

	if (start_hour != -1)
		m_time.SetTimeOfDay(start_hour, 0, 0);
}

void TimeEngine::Eval()
{
	float time = vtGetTime();

	if (m_last_time == -1.0f)
		m_last_time = time;

	if (m_fSpeed != 0.0f)
	{
		float elapsed = (time - m_last_time);
		m_fSeconds += (elapsed * m_fSpeed);
		if (m_fSeconds > 1.0f)
		{
			int full = (int) m_fSeconds;
			m_time.Increment(full);
			_InformTarget();
			m_fSeconds -= full;
		}
	}
	m_last_time = time;
}

void TimeEngine::SetSpeed(float factor)
{
	m_fSpeed = factor;
}

void TimeEngine::SetTime(vtTime &time)
{
	m_time = time;
}

void TimeEngine::GetTime(vtTime &time) const
{
	time = m_time;
}

vtTime TimeEngine::GetTime() const
{
	return m_time;
}

void TimeEngine::_InformTarget()
{
	for (int i = 0; i < NumTargets(); i++)
	{
		TimeTarget* pTarget = (TimeTarget *)GetTarget(i);
		pTarget->SetTime(m_time);
	}
}

