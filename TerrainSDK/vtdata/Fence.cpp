//
// Fence.cpp
//
// Implemented vtFence;
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Fence.h"

#define FENCE_SPACING		(2.5f * m_fFenceScale)	// 2.5 meters

#define LONGEST_FENCE 2000   // in meters

///////////////////

vtFence::vtFence()
{
	m_FenceType = FT_WIRE;
	m_fHeight = FENCE_DEFAULT_HEIGHT;
	m_fSpacing = FENCE_DEFAULT_SPACING;
}

vtFence::vtFence(FenceType type, float fHeight, float fSpacing)
{
	//if (m_FenceType != FT_WIRE || m_FenceType != FT_CHAINLINK)
	m_FenceType = type;
	m_fHeight = fHeight;
	m_fSpacing = fSpacing;

	if (m_FenceType == FT_WIRE)
		m_PostSize.Set(0.13f, m_fHeight, 0.13f);
	else if (m_FenceType == FT_CHAINLINK)
		m_PostSize.Set(0.05f, m_fHeight, 0.05f);
}


void vtFence::AddPoint(const DPoint2 &epos)
{
	int numfencepts = m_pFencePts.GetSize();

	// check distance
	if (numfencepts > 0)
	{
		DPoint2 LastPt = m_pFencePts.GetAt(numfencepts - 1);

		double distance = (LastPt - epos).Length();

		if (distance <= LONGEST_FENCE)
			m_pFencePts.Append(epos);
	}
	else
		m_pFencePts.Append(epos);
}

bool vtFence::GetExtents(DRECT &rect)
{
	int size = m_pFencePts.GetSize();

	if (size < 1)
		return false;

	rect.SetRect(1E9, -1E9, -1E9, 1E9);
	for (int i = 0; i < size; i++)
		rect.GrowToContainPoint(m_pFencePts.GetAt(i));
	return true;
}

/**
 * Given a 2d point, return the fence point closest to it.
 */
void vtFence::GetClosestPoint(const DPoint2 &point, DPoint2 &closest_point)
{
	DPoint2 pos;
	double dist, closest = 1E8;

	int size = m_pFencePts.GetSize();
	for (int i = 0; i < size; i++)
	{
		pos = m_pFencePts.GetAt(i);
		dist = (pos - point).Length();
		if (dist < closest)
		{
			closest = dist;
			closest_point = pos;
		}
	}
}

void vtFence::WriteXML(FILE *fp, bool bDegrees)
{
	int i;
	const char *coord_format;
	if (bDegrees)
		coord_format = "%lg";
	else
		coord_format = "%.2lf";

	// Write the XML to describe this fence to a built-structure XML file.
	fprintf(fp, "\t<structure type=\"linear\">\n");

	int points = m_pFencePts.GetSize();
	fprintf(fp, "\t\t<points num=\"%d\" coords=\"", points);
	for (i = 0; i < points; i++)
	{
		DPoint2 p = m_pFencePts.GetAt(i);
		fprintf(fp, coord_format, p.x);
		fprintf(fp, " ");
		fprintf(fp, coord_format, p.y);
		if (i != points-1)
			fprintf(fp, " ");
	}
	fprintf(fp, "\" />\n");

	// This must be expanded when we support more than 2 kinds of fence!
	const char *post_type = (m_FenceType == FT_WIRE) ? "wood" : "steel";
	const char *conn_type = (m_FenceType == FT_WIRE) ? "wire" : "chain-link";

	fprintf(fp, "\t\t<height abs=\"%.2f\" />\n", m_fHeight);
	fprintf(fp, "\t\t<posts type=\"%s\" size=\"%.2f, %.2f\" spacing=\"%.2f\" />\n",
		post_type, m_PostSize.x, m_PostSize.z, m_fSpacing);
	fprintf(fp, "\t\t<connect type=\"%s\" />\n", conn_type);

	fprintf(fp, "\t</structure>\n");
}


//////////////////////////////////////////////////////////////////

