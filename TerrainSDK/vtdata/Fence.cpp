//
// Fence.cpp
//
// Implemented vtFence;
//
// Copyright (c) 2001-2005 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include "Fence.h"

#define LONGEST_FENCE 2000   // in meters

///////////////////

void vtLinearParams::Defaults()
{
	ApplyStyle(FS_WOOD_POSTS_WIRE);
}

void vtLinearParams::Blank()
{
	m_PostType = "none";
	m_fPostSpacing = 1.0f;
	m_fPostHeight = 1.0f;
	m_fPostWidth = 0.0f;
	m_fPostDepth = 0.0f;
	//
	m_ConnectType = "none";
	m_fConnectTop = 1.0f;
	m_fConnectBottom = 0.0f;
	m_fConnectWidth = 0.0f;
}

void vtLinearParams::ApplyStyle(vtLinearStyle style)
{
	switch (style)
	{
	case FS_WOOD_POSTS_WIRE:
		//
		m_PostType = "wood";
		m_fPostSpacing = 2.5f;
		m_fPostHeight = 1.2f;
		m_fPostWidth = 0.13f;
		m_fPostDepth = 0.13f;
		//
		m_ConnectType = "wire";
		m_fConnectTop = 1.1f;
		m_fConnectBottom = 0.5f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_METAL_POSTS_WIRE:
		//
		m_PostType = "steel";
		m_fPostSpacing = 2.5f;
		m_fPostHeight = 1.2f;
		m_fPostWidth = 0.05f;
		m_fPostDepth = 0.05f;
		//
		m_ConnectType = "wire";
		m_fConnectTop = 1.1f;
		m_fConnectBottom = 0.5f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_CHAINLINK:
		//
		m_PostType = "steel";
		m_fPostSpacing = 3.0f;
		m_fPostHeight = 2.0f;
		m_fPostWidth = 0.05f;
		m_fPostDepth = 0.05f;
		//
		m_ConnectType = "chain-link";
		m_fConnectTop = m_fPostHeight;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_DRYSTONE:
		//
		m_PostType = "none";
		//
		m_ConnectType = "drystone";
		m_fConnectTop = 1.5f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.3f;
		break;
	case FS_STONE:
		//
		m_PostType = "none";
		//
		m_ConnectType = "stone";
		m_fConnectTop = 1.5f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.3f;
		break;
	case FS_PRIVET:
		//
		m_PostType = "none";
		//
		m_ConnectType = "privet";
		m_fConnectTop = 1.5f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.5f;
		break;
	case FS_RAILING_ROW:
		//
		m_PostType = "none";
		//
		m_ConnectType = "railing_pipe";
		m_fConnectTop = 0.70f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_RAILING_CABLE:
		//
		m_PostType = "none";
		//
		m_ConnectType = "railing_wire";
		m_fConnectTop = 0.70f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	case FS_RAILING_EU:
		//
		m_PostType = "none";
		//
		m_ConnectType = "railing_eu";
		m_fConnectTop = 0.70f;
		m_fConnectBottom = 0.0f;
		m_fConnectWidth = 0.0f;
		break;
	}
}

void vtLinearParams::WriteXML(GZOutput &out) const
{
	if (m_PostType != "none")
	{
		gfprintf(out, "\t\t<Posts Type=\"%s\" Spacing=\"%.2f\" Height=\"%.2f\" Size=\"%.2f,%.2f\"",
			(const char *)m_PostType, m_fPostSpacing, m_fPostHeight, m_fPostWidth, m_fPostDepth);
		gfprintf(out, " />\n");
	}
	if (m_ConnectType != "none")
	{
		gfprintf(out, "\t\t<Connect Type=\"%s\"", (const char *)m_ConnectType);
		gfprintf(out, " Top=\"%.2f\"", m_fConnectTop);
		if (m_fConnectBottom != 0.0f)
			gfprintf(out, " Bottom=\"%.2f\"", m_fConnectBottom);
		if (m_fConnectWidth != 0.0f)
			gfprintf(out, " Width=\"%.2f\"", m_fConnectWidth);
		gfprintf(out, " />\n");
	}
}


///////////////////

vtFence::vtFence() : vtStructure()
{
	SetType(ST_LINEAR);	// structure type
	m_Params.Defaults();
}

vtFence &vtFence::operator=(const vtFence &v)
{
	// copy parent data
	vtStructure::CopyFrom(v);

	// copy class data
	m_Params = v.m_Params;
	m_pFencePts = v.m_pFencePts;
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

void vtFence::ApplyStyle(vtLinearStyle style)
{
	m_Params.ApplyStyle(style);
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

void vtFence::WriteXML(GZOutput &out, bool bDegrees) const
{
	int i;
	const char *coord_format = "%.9lg";	// up to 9 significant digits

	// Write the XML to describe this fence to a built-structure XML file.
	gfprintf(out, "\t<Linear>\n");

	gfprintf(out, "\t\t<Path>\n");
	gfprintf(out, "\t\t\t<gml:coordinates>");
	int points = m_pFencePts.GetSize();
	for (i = 0; i < points; i++)
	{
		DPoint2 p = m_pFencePts.GetAt(i);
		gfprintf(out, coord_format, p.x);
		gfprintf(out, ",");
		gfprintf(out, coord_format, p.y);
		if (i != points-1)
			gfprintf(out, " ");
	}
	gfprintf(out, "</gml:coordinates>\n");
	gfprintf(out, "\t\t</Path>\n");

	m_Params.WriteXML(out);

	WriteTags(out);
	gfprintf(out, "\t</Linear>\n");
}


//////////////////////////////////////////////////////////////////

bool vtFence::IsContainedBy(const DRECT &rect) const
{
	return rect.ContainsLine(m_pFencePts);
}

