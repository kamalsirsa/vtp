//
// AttribMap.h
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#ifndef ATTRIBMAPH
#define ATTRIBMAPH

#include "vtdata/vtDIB.h"

struct AttribTableEntry {
	unsigned int rgb;
	int attrib;
};

class AttribMap : public vtDIB
{
public:
	AttribMap();
	~AttribMap();

	bool Load(const char *fname_att, const char *fname_bmp);
	int GetAttribute(float utm_x, float utm_y);

	int m_iNumAttribs;
	int m_xmin, m_xmax, m_ymin, m_ymax;
	AttribTableEntry *m_AttribTable;
};

#endif

