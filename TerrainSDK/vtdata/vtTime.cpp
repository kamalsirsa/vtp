//
// vtTime.cpp
//
// Copyright (c) 2004 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "vtTime.h"

///////////////////////////////////////////////////////

time_t vtTime::s_DifferenceFromGMT = (time_t) -1;

vtTime::vtTime()
{
	if (s_DifferenceFromGMT == (time_t) -1)
	{
		// Determine the offset between local (meaning the timezome of the
		//  computer which is running this software) and Greenwich Mean time,
		//  and use it to compensate for the fact mktime always tries to
		//  factor in the "local time zone".
		time_t dummy = 20000;
		struct tm tm_gm, tm_local;
		tm_gm = *gmtime(&dummy);
		tm_local = *localtime(&dummy);
		s_DifferenceFromGMT = mktime(&tm_local) - mktime(&tm_gm);
	}

	// default to "current" time
	GetSystemTime();
}

void vtTime::_UpdateTM()
{
	struct tm *t = gmtime(&m_time);
	m_tm = *t;
}

void vtTime::Increment(int secs)
{
	m_time += secs;
	_UpdateTM();
}

bool vtTime::SetFromString(const vtString &str)
{
	const char *sz = str;
	int num = sscanf(sz, "%d %d %d %d %d %d", &m_tm.tm_year, &m_tm.tm_mon,
		&m_tm.tm_mday, &m_tm.tm_hour, &m_tm.tm_min, &m_tm.tm_sec);
	if (num != 6)
		return false;

	m_time = mktime(&m_tm);
	m_time += s_DifferenceFromGMT;
	return true;
}

vtString vtTime::GetAsString()
{
	vtString str;
	str.Format("%d %d %d %d %d %d", m_tm.tm_year, m_tm.tm_mon,
		m_tm.tm_mday, m_tm.tm_hour, m_tm.tm_min, m_tm.tm_sec);
	return str;
}

void vtTime::GetSystemTime()
{
	time(&m_time);
	_UpdateTM();
}

void vtTime::SetDate(int year, int month, int day)
{
	m_tm.tm_year = year - 1900;

	// note that the tm structure has a 0-based month
	m_tm.tm_mon = month-1;

	m_tm.tm_mday = day;
	m_time = mktime(&m_tm);
	m_time += s_DifferenceFromGMT;
	_UpdateTM();
}

void vtTime::GetDate(int &year, int &month, int &day) const
{
	year = 1900 + m_tm.tm_year;

	// note that the tm structure has a 0-based month
	month = m_tm.tm_mon + 1;

	day = m_tm.tm_mday;
}

void vtTime::SetTimeOfDay(int hr, int min, int sec)
{
	m_tm.tm_hour = hr;
	m_tm.tm_min = min;
	m_tm.tm_sec = sec;
	m_time = mktime(&m_tm);

	m_time += s_DifferenceFromGMT;
	_UpdateTM();
}

void vtTime::GetTimeOfDay(int &hr, int &min, int &sec) const
{
	hr = m_tm.tm_hour;
	min = m_tm.tm_min;
	sec = m_tm.tm_sec;
}

int vtTime::GetSecondOfDay() const
{
	return (m_tm.tm_hour * 60 + m_tm.tm_min) * 60 + m_tm.tm_sec;
}

time_t vtTime::GetTime() const
{
	return m_time;
}

