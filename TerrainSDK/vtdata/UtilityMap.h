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
 * vtLine represent any connection between two poles.  It does not
 * explicitly specify the numer or location of each conductor, merely
 * the high-level topology of the power network.
 */
struct vtLine
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

	void GetPoleExtents(DRECT &rect);
//	bool ImportFromSHP(const char *filename, const vtProjection &proj);
//	bool ReadXML(const char *pathname, bool progress_callback(int));

protected:
//	bool ImportPolesFromSHP(const char *fname);
//	bool ImportLinesFromSHP(const char *fname);
	vtPole *ClosestPole(const DPoint2 &p);

	std::vector<vtPole *> m_Poles;
	std::vector<vtLine *> m_Lines;

	vtProjection m_proj;
};

#endif // UTILITYH
