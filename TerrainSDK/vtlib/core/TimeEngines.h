//
// TimeEngines.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef TIMEENGINESH
#define TIMEENGINESH

#include "Engine.h"

class vtTerrainScene;

///////////////////////////////////////////////////

class TimeEngine : public vtEngine
{
public:
	TimeEngine(vtTerrainScene* pTerrainScene, int start_hour = 0);

	void Eval();
	void SetFrameIncrement(int min, int sec);
	void SetRealIncrement(float factor);
	void SetTime(int hr, int min, float sec = 0.0f) { hours = hr; minutes = min; seconds = sec; }
	void SetTime(unsigned int time);
	void GetTime(int &hr, int &min, int &sec) { hr = hours; min = minutes; sec = (int) seconds; }

protected:
	vtTerrainScene* m_pTerrainScene;
	int hours, minutes;
	float seconds;

	bool m_bReal;
	int min_per_frame, sec_per_frame;
	float m_fIncFactor;
};

#endif
