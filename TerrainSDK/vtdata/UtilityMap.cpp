//
// UtilityMap.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include "UtilityMap.h"
#include "shapelib/shapefil.h"

vtUtilityMap::vtUtilityMap()
{
}

vtUtilityMap::~vtUtilityMap()
{
}


bool vtUtilityMap::ImportPolesFromSHP(const char *fname)
{
	SHPHandle hSHP;
	int		nEntities, nShapeType;
	DPoint2 point;

	hSHP = SHPOpen(fname, "rb");
	if (!hSHP)
		return false;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, NULL, NULL);
	if (nShapeType != SHPT_POINT)
		return false;

	for (int i = 0; i < nEntities; i++)
	{
		SHPObject *psShape = SHPReadObject(hSHP, i);
		point.x = psShape->padfX[0];
		point.y = psShape->padfY[0];
		vtPole *pole = new vtPole;
		pole->m_p = point;
		m_Poles.Append(pole);
		SHPDestroyObject(psShape);
	}
	SHPClose(hSHP);
	return true;
}

vtPole *vtUtilityMap::ClosestPole(const DPoint2 &p)
{
	unsigned int npoles = m_Poles.GetSize();
	if (npoles == 0)
		return NULL;

	unsigned int k, ret=0;
	double	dist, closest = 1E9;

	for (k = 0; k < npoles; k++)
	{
		dist = (p - m_Poles[k]->m_p).Length();
		if (dist < closest)
		{
			closest = dist;
			ret = k;
		}
	}
	return m_Poles[ret];
}

bool vtUtilityMap::ImportLinesFromSHP(const char *fname)
{
	SHPHandle hSHP;
	int		nEntities, nShapeType;
	int		i, j, verts;

	hSHP = SHPOpen(fname, "rb");
	if (!hSHP)
		return false;

	SHPGetInfo(hSHP, &nEntities, &nShapeType, NULL, NULL);
	if (nShapeType != SHPT_ARC)
		return false;

	for (i = 0; i < nEntities; i++)
	{
		SHPObject *psShape = SHPReadObject(hSHP, i);

		verts = psShape->nVertices;
		vtLine *line = new vtLine;
		line->SetSize(verts);

		// Store each SHP Poly Coord in Line
		for (j = 0; j < verts; j++)
		{
			line->GetAt(j).x = psShape->padfX[j];
			line->GetAt(j).y = psShape->padfY[j];
		}
		SHPDestroyObject(psShape);

		// Guess source and destination poles by using location
		line->m_src = ClosestPole(line->GetAt(0));
		line->m_dst = ClosestPole(line->GetAt(verts-1));

		// avoid degenerate lines
		if (line->m_src == line->m_dst)
		{
			delete line;
			continue;
		}

		// tweak start and end of line to match poles
		line->SetAt(0, line->m_src->m_p);
		line->SetAt(verts-1, line->m_dst->m_p);

		m_Lines.Append(line);
	}
	SHPClose(hSHP);
	return true;
}


bool vtUtilityMap::ImportFromSHP(const char *dirname, const vtProjection &proj)
{
	char	fname[256];

	m_proj = proj;

	// 1. Pole Positions
	//
	strcpy(fname, dirname);
	strcat(fname, "/poles.shp");
	if (!ImportPolesFromSHP(fname))
		return false;

	// 2. Fuses
	//
	strcpy(fname, dirname);
	strcat(fname, "/fuses.shp");
	if (!ImportPolesFromSHP(fname))
		return false;

	// 3. Transformers
	//
	strcpy(fname, dirname);
	strcat(fname, "/transformers.shp");
	if (!ImportPolesFromSHP(fname))
		return false;

	// 4. Servpnts
	//
	strcpy(fname, dirname);
	strcat(fname, "/servpnts.shp");
	if (!ImportPolesFromSHP(fname))
		return false;

	// Now, lines
	//
	// 1. Primaries
	//
	strcpy(fname, dirname);
	strcat(fname, "/primaries.shp");
	if (!ImportLinesFromSHP(fname))
		return false;

	// 2. Secondaries
	//
	strcpy(fname, dirname);
	strcat(fname, "/secondaries.shp");
	if (!ImportLinesFromSHP(fname))
		return false;
	return true;
}

void vtUtilityMap::GetPoleExtents(DRECT &rect)
{
	if (m_Poles.IsEmpty())
		return;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);

	int i, size = m_Poles.GetSize();
	for (i = 0; i < size; i++)
	{
		vtPole *pole = m_Poles.GetAt(i);
		rect.GrowToContainPoint(pole->m_p);
	}
}

