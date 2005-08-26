//
// Location classes
//
// Copyright (c) 2002-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef LOCATION_H
#define LOCATION_H

#include "vtdata/vtString.h"
#include "vtdata/Projections.h"
#include "vtdata/LocalConversion.h"

#if SUPPORT_WSTRING
	#define LocNameString wstring2
#else
	#define LocNameString std::string
#endif

/** \addtogroup nav */
/*@{*/

/**
 * A location is represented as a earth-coordinate point, an elevation,
 * and an orientation.  The point is always in Geographic coordinates
 * (Latitude/Longitude, WGS84).  It should be converted to and from
 * the actual, desired CS as appropriate.
 */
class vtLocation
{
public:
	vtLocation() {}
	vtLocation(const vtLocation &v) { *this = v; }
	vtLocation &operator=(const vtLocation &v)
	{
		m_strName = v.m_strName;
		m_pos1 = v.m_pos1;
		m_fElevation1 = v.m_fElevation1;
		m_pos2 = v.m_pos2;
		m_fElevation2 = v.m_fElevation2;
		return *this;
	}

	LocNameString m_strName;

	// Location and view direction are encoded as two 3D points.  The first
	//  is the location, the second is the "Look At" point.  Heading and
	//  pitch are implicit in the second point.
	//
	// The distance between the two points is arbitrary, but it should be
	//  neither very small nor very large, to avoid numerical problems.
	//
	DPoint2	m_pos1;
	float	m_fElevation1;
	DPoint2 m_pos2;
	float	m_fElevation2;
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
	friend class LocationVisitor;
public:
	vtLocationSaver();
	vtLocationSaver(const vtString &fname);
	~vtLocationSaver();

	bool Read(const vtString &fname);
	bool Write(const vtString &fname = "");
	void Remove(int num);
	int GetNumLocations() { return m_loc.GetSize(); }
	vtLocation *GetLocation(int num) const { return m_loc[num]; }
	int FindLocation(const vtString &locname);
	void Empty();

	// you must call these 3 methods before this class is useful
	void SetTransform(vtTransform *trans) { m_pTransform = trans; }
	vtTransform *GetTransform() { return m_pTransform; }

	void SetConversion(const vtLocalConversion &conv) { m_conv = conv; }

	void SetProjection(const vtProjection &proj);
	const vtProjection &GetAtProjection() const { return m_proj; }

	bool StoreTo(unsigned int num, const LocNameString &name = "");
	bool RecallFrom(int num);

protected:
	// Implementation
	// Store information necessary to convert from global earth CS
	// to the local CS
	vtLocalConversion	m_conv;
	vtProjection		m_proj;
	vtTransform			*m_pTransform;

	vtString	m_strFilename;
	OCT			*m_pConvertToWGS;
	OCT			*m_pConvertFromWGS;

	Array<vtLocation*> m_loc;
};

/*@}*/  // nav

#endif	// LOCATION_H
