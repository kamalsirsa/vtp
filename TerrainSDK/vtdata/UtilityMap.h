//
// UtilityMap.h
//
// High-level representation of a utility network, currently supporting
// power networks (poles and lines).
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef UTILITYH
#define UTILITYH

#include "MathTypes.h"
#include "Projections.h"
#include "Content.h"

/**
 * vtPole represent any kind of node in a power network, such as a
 * simple utility pole, a transmission tower, or simply a connection
 * point at a switch or transformer.
 */
struct vtPole : public vtTagArray
{
	int m_id;		// More efficient than storing id in the tagarray.
	DPoint2 m_p;
};

typedef std::vector<vtPole*> vtPoleArray;

/**
 * vtLine represent any connection between two or more poles.  It does not
 * explicitly specify the numer or location of each conductor, merely
 * the high-level topology of the power network.
 */
struct vtLine : public vtTagArray
{
	void MakePolyline(DLine2 &polyline);

	int m_id;		// More efficient than storing id in the tagarray.
	vtPoleArray m_poles;
};

/**
 * vtUtilityMap represents a full high-level network of poles and lines.
 */
class vtUtilityMap
{
public:
	vtUtilityMap();
	~vtUtilityMap();

	void AddPole(vtPole *pole) { m_Poles.push_back(pole); }
	void AddLine(vtLine *line) { m_Lines.push_back(line); }

	void GetPoleExtents(DRECT &rect);
	bool WriteOSM(const char *pathname);

protected:
	vtPole *ClosestPole(const DPoint2 &p);

	std::vector<vtPole *> m_Poles;
	std::vector<vtLine *> m_Lines;

	vtProjection m_proj;

	int	m_iNextAvailableID;
};

#endif // UTILITYH
