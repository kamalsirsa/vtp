//
// TimeEngines.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtlib/vtlib.h"
#include "TimeEngines.h"
#include "TerrainScene.h"
#include "SkyDome.h"

///////////////////////////////////////////////////////

TimeEngine::TimeEngine(vtTerrainScene* pTerrainScene, int start_hour) : vtEngine()
{
	m_pTerrainScene = pTerrainScene;
	hours = start_hour;
	minutes = 0;
	seconds = 0.0f;
	SetFrameIncrement(0, 0);
}

void TimeEngine::SetTime(unsigned int time)
{
	hours = time/3600;
	time -= (hours * 3600);
	minutes = time/60;
	time -= (minutes * 60);
	seconds = time;
}

void TimeEngine::SetFrameIncrement(int min, int sec)
{
	min_per_frame = min;
	sec_per_frame = sec;
	m_bReal = false;
}

void TimeEngine::SetRealIncrement(float factor)
{
	m_fIncFactor = factor;
	m_bReal = true;
}

void TimeEngine::Eval()
{
	static float last_time = -1.0f;

	// increment
	if (m_bReal)
	{
		float time = vtGetTime();
		if (last_time == -1.0f)
			last_time = time;
		float elapsed = (time - last_time);
		seconds += (elapsed * m_fIncFactor);
		last_time = time;
	}
	else
	{
		seconds += sec_per_frame;
		minutes += min_per_frame;
	}

	// rollover
	if (seconds >= 60)
	{
		int min_inc = (int)(seconds / 60);
		minutes += min_inc;
		seconds -= (min_inc * 60);
	}
	if (minutes >= 60)
	{
		hours += (minutes / 60);
		minutes = (minutes % 60);
	}
	if (hours >= 24) hours -= 24;

	m_pTerrainScene->SetTimeOfDay(TIME_TO_INT(hours, minutes, seconds));
}

