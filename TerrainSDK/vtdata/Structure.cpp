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

vtStructInstance::vtStructInstance() : vtStructure()
{
	SetType(ST_INSTANCE);

	m_p.Set(0, 0);
	m_fRotation = 0.0f;
	m_fScale = 1.0f;
}

void vtStructInstance::WriteXML_Old(FILE *fp, bool bDegrees)
{
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
/*
	// now write any and all tags
	int i;
	for (i = 0; i < NumTags(); i++)
	{
		vtTag *tag = GetTag(i);
		fprintf(fp, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
			(const char *)tag->value, (const char *)tag->name);
	}
*/
	fprintf(fp, "\t</structure>\n");
}

void vtStructInstance::WriteXML(FILE *fp, bool bDegrees)
{
	const char *coord_format;
	if (bDegrees)
		coord_format = "%lg";
	else
		coord_format = "%.2lf";

	fprintf(fp, "\t<Imported>\n");

	// first write the placement
	fprintf(fp, "\t\t<Location>\n");
	fprintf(fp, "\t\t\t<gml:coordinates>");
	fprintf(fp, coord_format, m_p.x);
	fprintf(fp, ",");
	fprintf(fp, coord_format, m_p.y);
	fprintf(fp, "</gml:coordinates>\n");
	fprintf(fp, "\t\t</Location>\n");

	if (m_fRotation != 0.0f)
	{
		fprintf(fp, "\t\t<Rotation Radians=\"%f\">\n", m_fRotation);
	}
	WriteTags(fp);
	fprintf(fp, "\t</Imported>\n");
}

bool vtStructInstance::GetExtents(DRECT &rect) const
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


bool vtStructInstance::IsContainedBy(const DRECT &rect) const
{
	return rect.ContainsPoint(m_p);
}


///////////////////////////////////////////////////////////////////////

vtStructure::vtStructure()
{
	m_type = ST_NONE;
}

vtStructure::~vtStructure()
{
	m_type = ST_NONE;
}

void vtStructure::WriteTags(FILE *fp)
{
	// now write all extra tags (attributes) for this structure
	int i;
	for (i = 0; i < NumTags(); i++)
	{
		vtTag *tag = GetTag(i);
		fprintf(fp, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
			(const char *)tag->value, (const char *)tag->name);
	}
}

