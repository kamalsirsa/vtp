//
// vtTime.h
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTTIMEH
#define VTTIMEH

#include <time.h>
#include "vtString.h"

///////////////////////////////////////////////////

/**
 * This class encapsulates time (in increments of whole seconds.)
 * It always stores UTC (GMT).  If you need "local time" for some specific
 *  time zone, you will need to call other methods to compute that.
 */
class vtTime
{
public:
	vtTime();

	void SetDate(int year, int month, int day);
	void GetDate(int &year, int &month, int &day) const;

	void SetTimeOfDay(int hr, int min, int sec);

	void GetTimeOfDay(int &hr, int &min, int &sec) const;
	int GetSecondOfDay() const;

	time_t GetTime() const;
	const tm &GetTM() const { return m_tm; }
	void Increment(int secs);

	bool SetFromString(const vtString &str);
	vtString GetAsString();

	void GetSystemTime();

protected:
	void _UpdateTM();

	tm m_tm;			// always stores GMT
	time_t m_time;		// always stores GMT

	static time_t s_DifferenceFromGMT;
};

#endif // VTTIMEH

