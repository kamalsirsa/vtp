//
// Fence.cpp
//
// Implemented vtFence;
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Fence.h"

#define LONGEST_FENCE 2000   // in meters

///////////////////

vtFence::vtFence() : vtStructure()
{
	SetType(ST_LINEAR);

	m_FenceType = FT_WIRE;
	m_fHeight = FENCE_DEFAULT_HEIGHT;
	m_fSpacing = FENCE_DEFAULT_SPACING;
}

vtFence::vtFence(FenceType type, float fHeight, float fSpacing)
{
	SetType(ST_LINEAR);

	SetFenceType(type);
	m_fHeight = fHeight;
	m_fSpacing = fSpacing;
	m_PostSize.y = m_fHeight;
}

vtFence &vtFence::operator=(const vtFence &v)
{
	SetFenceType(v.m_FenceType);
	m_fHeight = v.m_fHeight;
	m_fSpacing = v.m_fSpacing;
	m_pFencePts = v.m_pFencePts;
	SetElevationOffset(v.GetElevationOffset());
	SetOriginalElevation(v.GetOriginalElevation());
	return *this;
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

void vtFence::SetOptions(const LinStructOptions &opt)
{
	SetFenceType(opt.eType);
	m_fHeight = opt.fHeight;
	m_fSpacing = opt.fSpacing;
	m_PostSize.y = opt.fHeight;
}

void vtFence::SetFenceType(const FenceType type)
{
	m_FenceType = type;

	switch (m_FenceType)
	{
	case FT_WIRE:
		m_PostSize.Set(0.13f, m_fHeight, 0.13f);
		break;

	case FT_CHAINLINK:
		m_PostSize.Set(0.05f, m_fHeight, 0.05f);
		break;

	case FT_HEDGEROW:
		m_PostSize.Set(0.05f, m_fHeight, 0.05f);
		break;

	case FT_DRYSTONE:
		m_PostSize.Set(0.05f, m_fHeight, 0.05f);
		break;

	case FT_PRIVET:
		m_PostSize.Set(0.05f, m_fHeight, 0.05f);
		break;

	case FT_STONE:
		m_PostSize.Set(0.05f, m_fHeight, 0.05f);
		break;

	case FT_BEECH:
		m_PostSize.Set(0.05f, m_fHeight, 0.05f);

	default:
		m_PostSize.Set(0.13f, m_fHeight, 0.13f);
		break;
	}
}

bool vtFence::GetExtents(DRECT &rect) const
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

/**
 * Given a 2d point, return the distance to the closest point on the centerline.
 */
double vtFence::GetDistanceToLine(const DPoint2 &point)
{
	int i, size = m_pFencePts.GetSize();
	double dist, closest = 1E8;
	for (i = 0; i < size-1; i++)
	{
		DPoint2 p0 = m_pFencePts[i];
		DPoint2 p1 = m_pFencePts[i+1];
		dist = DistancePointToLine(p0, p1, point);
		if (dist < closest)
			closest = dist;
	}
	return closest;
}

void vtFence::WriteXML_Old(FILE *fp, bool bDegrees)
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
	const char *post_type;
	const char *conn_type;
	switch (m_FenceType)
	{
	case FT_WIRE:
		post_type = "wood";
		conn_type = "wire";
		break;

	case FT_CHAINLINK:
		post_type = "steel";
		conn_type = "chain-link";
		break;

	case FT_HEDGEROW:
		post_type = "hedgerow";
		conn_type = "hedgerow";
		break;

	case FT_DRYSTONE:
		post_type = "drystone";
		conn_type = "drystone";
		break;

	case FT_PRIVET:
		post_type = "privet";
		conn_type = "privet";
		break;

	case FT_STONE:
		post_type = "stone";
		conn_type = "stone";
		break;

	case FT_BEECH:
		post_type = "beech";
		conn_type = "beech";
		break;

	default:
		post_type = "wood";
		conn_type = "wire";
		break;
	}

	fprintf(fp, "\t\t<height abs=\"%.2f\" />\n", m_fHeight);
	fprintf(fp, "\t\t<posts type=\"%s\" size=\"%.2f, %.2f\" spacing=\"%.2f\" />\n",
		post_type, m_PostSize.x, m_PostSize.z, m_fSpacing);
	fprintf(fp, "\t\t<connect type=\"%s\" />\n", conn_type);

	fprintf(fp, "\t</structure>\n");
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
	fprintf(fp, "\t<Linear Height=\"%.2f\">\n", m_fHeight);

	fprintf(fp, "\t\t<Path>\n");
	fprintf(fp, "\t\t\t<gml:coordinates>");
	int points = m_pFencePts.GetSize();
	for (i = 0; i < points; i++)
	{
		DPoint2 p = m_pFencePts.GetAt(i);
		fprintf(fp, coord_format, p.x);
		fprintf(fp, ",");
		fprintf(fp, coord_format, p.y);
		if (i != points-1)
			fprintf(fp, " ");
	}
	fprintf(fp, "</gml:coordinates>\n");
	fprintf(fp, "\t\t</Path>\n");

	// This must be expanded when we support more than 2 kinds of fence!
	const char *post_type = (m_FenceType == FT_WIRE) ? "wood" : "steel";
	const char *conn_type = (m_FenceType == FT_WIRE) ? "wire" : "chain-link";

	fprintf(fp, "\t\t<Posts Type=\"%s\" Size=\"%.2f,%.2f\" Spacing=\"%.2f\" />\n",
		post_type, m_PostSize.x, m_PostSize.z, m_fSpacing);
	fprintf(fp, "\t\t<Connect Type=\"%s\" />\n", conn_type);

	WriteTags(fp);
	fprintf(fp, "\t</Linear>\n");
}


//////////////////////////////////////////////////////////////////

bool vtFence::IsContainedBy(const DRECT &rect) const
{
	return rect.ContainsLine(m_pFencePts);
}

