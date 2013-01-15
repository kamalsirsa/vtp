//
// UtilityMap.cpp
//
// Copyright (c) 2001-2013 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdlib.h>
#include "UtilityMap.h"
#include "vtString.h"
#include "vtLog.h"
#include "Version.h"
#include "shapelib/shapefil.h"
#include "xmlhelper/easyxml.hpp"


void vtLine::MakePolyline(DLine2 &polyline)
{
	const uint num = m_poles.size();
	polyline.SetSize(num);
	for (uint i = 0; i < num; i++)
		polyline[i] = m_poles[i]->m_p;
}

vtUtilityMap::vtUtilityMap()
{
	m_iNextAvailableID = 0;
}

vtUtilityMap::~vtUtilityMap()
{
	for (uint i = 0; i < m_Poles.size(); i++)
		delete m_Poles[i];
	m_Poles.clear();

	for (uint i = 0; i < m_Lines.size(); i++)
		delete m_Lines[i];
	m_Lines.clear();
}

vtPole *vtUtilityMap::ClosestPole(const DPoint2 &p)
{
	uint npoles = m_Poles.size();
	if (npoles == 0)
		return NULL;

	uint k, ret=0;
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

void vtUtilityMap::GetPoleExtents(DRECT &rect)
{
	if (m_Poles.empty())
		return;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);

	const int size = m_Poles.size();
	for (int i = 0; i < size; i++)
	{
		vtPole *pole = m_Poles[i];
		rect.GrowToContainPoint(pole->m_p);
	}
}

bool vtUtilityMap::WriteOSM(const char *pathname)
{
	FILE *fp = fopen(pathname, "wb");
	if (!fp)
		return false;

	// OSM only understands Geographic WGS84, so convert to that.
	vtProjection wgs84_geo;
	wgs84_geo.SetGeogCSFromDatum(EPSG_DATUM_WGS84);
	OCT *transform = CreateCoordTransform(&m_proj, &wgs84_geo);
	if (!transform)
	{
		VTLOG1(" Couldn't transform coordinates\n");
		return false;
	}

	fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fp, "<osm version=\"0.6\" generator=\"VTP %s\">\n", VTP_VERSION);

	for (uint i = 0; i < m_Poles.size(); i++)
	{
		const vtPole *pole = m_Poles[i];
		DPoint2 p = pole->m_p;
		transform->Transform(1, &p.x, &p.y);

		fprintf(fp, " <node id=\"%d\" lat=\"%.8lf\" lon=\"%.8lf\" version=\"1\">\n",
			pole->m_id, p.y, p.x);

		fprintf(fp, "  <tag k=\"power\" v=\"tower\"/>\n");

		const int num_tags = pole->NumTags();
		for (uint j = 0; j < pole->NumTags(); j++)
		{
			fprintf(fp, "  tag k=\"%s\" v=\"%s\"/>\n",
				pole->GetTag(j)->name, pole->GetTag(j)->value);
		}
		fprintf(fp, " </node>\n");
	}
	for (uint i = 0; i < m_Lines.size(); i++)
	{
		fprintf(fp, " <way id=\"%d\" version=\"1\">\n");

		const vtLine *line = m_Lines[i];
		const uint num_poles = line->m_poles.size();
		const uint num_tags = line->NumTags();

		for (uint j = 0; j < num_poles; j++)
			fprintf(fp, "  <nd ref=\"%d\"/>\n", line->m_poles[j]->m_id);

		fprintf(fp, "  <tag k=\"power\" v=\"line\"/>\n");

		for (uint j = 0; j < line->NumTags(); j++)
		{
			fprintf(fp, "  tag k=\"%s\" v=\"%s\"/>\n",
				line->GetTag(j)->name, line->GetTag(j)->value);
		}
		fprintf(fp, " </way>\n");
	}
	fprintf(fp, "</osm>\n");
	fclose(fp);

	delete transform;
	return true;
}

