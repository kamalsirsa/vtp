//
// Structure.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include "Structure.h"
#include "Building.h"
#include "Fence.h"

vtStructInstance::vtStructInstance()
{
	m_p.Set(0, 0);
	m_fRotation = 0.0f;
}

void vtStructInstance::WriteXML(FILE *fp, bool bDegrees)
{
	int i;

	const char *coord_format;
	if (bDegrees)
		coord_format = "%lg";
	else
		coord_format = "%.2lf";

	fprintf(fp, "\t<structure type=\"instance\">\n");

	// first write the placement
	fprintf(fp, "\t\t<placement location=\"");
	fprintf(fp, coord_format, m_p.x);
	fprintf(fp, " ");
	fprintf(fp, coord_format, m_p.y);
	fprintf(fp, "\"");
	if (m_fRotation != 0.0f)
	{
		fprintf(fp, " rotation=\"%f\"", m_fRotation);
	}
	fprintf(fp, " />\n");

	// now write any and all tags
	for (i = 0; i < NumTags(); i++)
	{
		vtTag *tag = GetTag(i);
		fprintf(fp, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
			(const char *)tag->value, (const char *)tag->name);
	}

	fprintf(fp, "\t</structure>\n");
}

bool vtStructInstance::GetExtents(DRECT &rect)
{
	// we have no way (yet) of knowing the extents of an external
	// reference, so just give a placeholder of a single point.
	rect.SetRect(m_p.x, m_p.y, m_p.x, m_p.y);
	return true;
}

void vtStructInstance::Offset(const DPoint2 &delta)
{
	m_p += delta;
}

///////////////////////////////////////////////////////////////////////

vtStructure::vtStructure()
{
	m_type = ST_NONE;
	m_pBuilding = NULL;
}

vtStructure::~vtStructure()
{
	switch (m_type)
	{
	case ST_BUILDING:
		delete m_pBuilding;
		break;

	case ST_FENCE:
		delete m_pFence;
		break;

	case ST_INSTANCE:
		delete m_pInstance;
		break;
	}
	m_type = ST_NONE;
	m_pBuilding = NULL;
}

bool vtStructure::GetExtents(DRECT &rect)
{
	switch (m_type)
	{
	case ST_BUILDING:
		return m_pBuilding->GetExtents(rect);

	case ST_FENCE:
		return m_pFence->GetExtents(rect);

	case ST_INSTANCE:
		return m_pInstance->GetExtents(rect);
	}
	return false;
}

bool vtStructure::IsContainedBy(const DRECT &rect)
{
	switch (m_type)
	{
	case ST_BUILDING:
		return rect.ContainsPoint(m_pBuilding->GetLocation());

	case ST_FENCE:
	{
		const DLine2 &pts = m_pFence->GetFencePoints();
		return rect.ContainsLine(pts);
	}

	case ST_INSTANCE:
		return rect.ContainsPoint(m_pInstance->m_p);
	}
	return false;
}

