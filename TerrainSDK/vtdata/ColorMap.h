//
// ColorMap.h
//
// Copyright (c) 2002-2012 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef VTDATA_COLORMAP_H
#define VTDATA_COLORMAP_H

#include "MathTypes.h"

/**
 * This small class describes how to map elevation (as from a heightfield)
 * onto a set of colors.
 */
class ColorMap
{
public:
	ColorMap();
	bool Save(const char *fname) const;
	bool Load(const char *fname);
	void Add(float elev, const RGBi &color);
	void RemoveAt(int num);
	void Clear();
	int Num() const;
	void GenerateColors(std::vector<RGBi> &table, int iTableSize, float fMin, float fMax) const;

	bool m_bBlend;
	bool m_bRelative;
	std::vector<float> m_elev;
	std::vector<RGBi> m_color;
};

#endif  // VTDATA_COLORMAP_H
