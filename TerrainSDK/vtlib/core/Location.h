//
// Location classes
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LOCATION_H
#define LOCATION_H

/**
 * Currently, a location is represented as a world-coordinate
 * (not earth-coordinate) view matrix.  This should really be
 * improved to use separate:
 *	1. geographic location, potentially including datum etc.
 *  2. altitude
 *  3. orientation
 */
class vtLocation
{
public:
	vtLocation() {}
	vtLocation(const vtLocation &v) { *this = v; }
	vtLocation &operator=(const vtLocation &v)
	{
		m_name = v.m_name;
		m_mat = v.m_mat;
		return *this;
	}

	vtString m_name;
	FMatrix4 m_mat;
};

/**
 * This class manages a set of named locations (position and orientation).
 * It can save and load the locations to a file, and jump back to
 * any stored location.
 *
 * \sa vtLocation
 */
class vtLocationSaver
{
public:
	vtLocationSaver();
	vtLocationSaver(const char* fname);
	~vtLocationSaver();

	void SetTarget(vtTransform* target) { m_pTarget = target; }
	bool Read(const char *fname);
	bool Write(const char *fname = NULL);
	void StoreTo(int num, const char *name = NULL);
	void RecallFrom(int num);
	void Remove(int num);
	int GetNumLocations() { return m_loc.GetSize(); }
	vtLocation *GetLocation(int num) { return m_loc[num]; }

// Implementation
protected:
	vtTransform *m_pTarget;
	vtString m_strFilename;

	Array<vtLocation*> m_loc;
};

#endif	// LOCATION_H
