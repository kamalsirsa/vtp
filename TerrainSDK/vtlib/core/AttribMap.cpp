//
// AttribMap.cpp
//
// This class gives you the ability to load in a colored bitmap representating
// a geographical coverage, and query it for attribute given a world coordinate.
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

// To construct, you must provide both the coverage bitmap and
// the filename of a file which describes the coverage, in the format
// of the following example:
//
/*
west: 176764
east: 310954
south: 2091656
north: 2244376
types: 3
type 1 ff00ff <optional description, ignored>
... for each type
*/

#include <stdio.h>
#include "AttribMap.h"


AttribMap::AttribMap(const char *fname_att, const char *fname_bmp) : vtDIB(fname_bmp)
{
	m_AttribTable = NULL;

	// also read attribute description file
	FILE *fp = fopen(fname_att, "r");
	if (!fp) return;

	fscanf(fp, "west: %d\n", &m_xmin);
	fscanf(fp, "east: %d\n", &m_xmax);
	fscanf(fp, "south: %d\n", &m_ymin);
	fscanf(fp, "north: %d\n", &m_ymax);

	char buf[80];
	int att;
	unsigned int rgb;

	fscanf(fp, "types: %d\n", &m_iNumAttribs);
	m_AttribTable = new AttribTableEntry[m_iNumAttribs];
	for (int i = 0; i < m_iNumAttribs; i++)
	{
		fgets(buf, 80, fp);
		sscanf(buf, "type %d %x ", &att, &rgb);
		m_AttribTable[i].attrib = att-1;
		m_AttribTable[i].rgb = rgb;
	}
	fclose(fp);
}

AttribMap::~AttribMap()
{
	if (m_AttribTable) delete m_AttribTable;
}

int AttribMap::GetAttribute(float utm_x, float utm_y)
{
	if (!m_AttribTable) return -1;

	int x_offset = (int) (((float)(utm_x - m_xmin) / (m_xmax - m_xmin)) * GetWidth());
	int y_offset = (int) (((float)(m_ymax - utm_y) / (m_ymax - m_ymin)) * GetHeight());

	unsigned int rgb = GetPixel24(x_offset, y_offset);
	for (int i = 0; i < m_iNumAttribs; i++)
	{
		if (rgb == m_AttribTable[i].rgb)
			return m_AttribTable[i].attrib;
	}
	return -1;
}


