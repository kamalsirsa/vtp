//
// Structure.cpp
//
// Copyright (c) 2001 Virtual Terrain Project
// Free for all uses, see license.txt for details.
//

#include <stdio.h>
#include "Structure.h"
#include "StructArray.h"
#include "Building.h"
#include "Fence.h"
#include "xmlhelper/easyxml.hpp"
#include "vtLog.h"
#include "FilePath.h"

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
		coord_format = "%.9lg";
	else
		coord_format = "%.2lg";

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
		coord_format = "%.9lg";
	else
		coord_format = "%.2lg";

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
		fprintf(fp, "\t\t<Rotation>%g</Rotation>\n", m_fRotation);
	}
	if (m_fScale != 1.0f)
	{
		fprintf(fp, "\t\t<Scale>%g</Scale>\n", m_fScale);
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
	m_fElevationOffset = 0.0f;
	m_fOriginalElevation = -1E9;
#ifdef VIAVTDATA
	m_bIsVIAContributor = false;
	m_bIsVIATarget = false;
#endif
}

vtStructure::~vtStructure()
{
	m_type = ST_NONE;
}

void vtStructure::WriteTags(FILE *fp)
{
	// now write all extra tags (attributes) for this structure
	for (unsigned int i = 0; i < NumTags(); i++)
	{
		vtTag *tag = GetTag(i);
		fprintf(fp, "\t\t<%s>%s</%s>\n", (const char *)tag->name,
			(const char *)tag->value, (const char *)tag->name);
	}
}

////////////////////////////////////////////////////////////////////////


// XML parser for MaterialDescriptorArray

class MaterialDescriptorArrayVisitor : public XMLVisitor
{
public:
	MaterialDescriptorArrayVisitor(vtMaterialDescriptorArray *MDA) : m_state(0), m_pMDA(MDA) {}
	void startXML() { m_state = 0; }
	void endXML() { m_state = 0; }
	void startElement(const char *name, const XMLAttributes &atts);

private:
	int m_state;

	vtMaterialDescriptorArray *m_pMDA;
};

void MaterialDescriptorArrayVisitor::startElement(const char *name, const XMLAttributes &atts)
{
	const char *attval;

	if (m_state == 0 && !strcmp(name, "MaterialDescriptorArray"))
		m_state = 1;
	else if (m_state == 1)
	{
		if (!strcmp(name, "MaterialDescriptor"))
		{
			vtMaterialDescriptor *pDescriptor = new vtMaterialDescriptor;
			attval = atts.getValue("Name");
			if (attval)
				pDescriptor->SetName(*(new vtString(attval)));

			pDescriptor->SetColorable(VT_MATERIAL_SELFCOLOURED_TEXTURE);
			attval = atts.getValue("Colorable");
			if (attval && !strcmp(attval, "true"))
				pDescriptor->SetColorable(VT_MATERIAL_COLOURABLE_TEXTURE);

			attval = atts.getValue("Type");
			if (attval)
				pDescriptor->SetType(atoi(attval));
			attval = atts.getValue("Source");
			if (attval)
				pDescriptor->SetSourceName(attval);
			attval = atts.getValue("Scale");
			if (attval)
				pDescriptor->SetUVScale((float)atof(attval));
			attval = atts.getValue("RGB");
			if (attval)
			{
				short r, g, b;
				sscanf(attval, "%2hx %2hx %2hx", &r, &g, &b);
				pDescriptor->SetRGB(RGBi(r, g, b));
			}
			m_pMDA->Append(pDescriptor);
		}
	}
}


///////////////////////////////////////////////////////////////////////
// Methods for vtMaterialDescriptor
//
vtMaterialDescriptor::vtMaterialDescriptor()
{
	m_pName = NULL;
	m_Type = 0;
	m_Colorable = VT_MATERIAL_SELFCOLOURED_TEXTURE;
	m_fUVScale = 0.0;
	m_RGB.Set(0,0,0);
	m_iMaterialIndex = -1;
}

vtMaterialDescriptor::vtMaterialDescriptor(const char *name,
	const vtString &SourceName, const vtMaterialColorEnum Colorable,
	const float UVScale, RGBi Color)
{
	m_pName = new vtString(name);	//GetGlobalMaterials()->FindName(name);
	m_Type = 0;
	m_SourceName = SourceName;
	m_Colorable = Colorable;
	m_fUVScale = UVScale;
	m_RGB = Color;
	m_iMaterialIndex = -1;
}

vtMaterialDescriptor::~vtMaterialDescriptor()
{
	if (m_pName)
		delete m_pName;
}


///////////////////////////////////////////////////////////////////////
// Methods for vtMaterialDescriptorArray
//

void vtMaterialDescriptorArray::CreatePlain()
{
	// First provide plain material, which does need to be serialized
	Append(new vtMaterialDescriptor(BMAT_NAME_PLAIN, "", VT_MATERIAL_COLOURED, 1.0f));
}

bool vtMaterialDescriptorArray::Load(const char *szFileName)
{
	VTLOG("Loading materials from %s\n", szFileName);

	MaterialDescriptorArrayVisitor Visitor(this);
	try
	{
		readXML(szFileName, Visitor);
	}
	catch (xh_exception &e)
	{
		// TODO: would be good to pass back the error message.
		VTLOG("vtMaterialDescriptorArray::Load xml error %s\n", e.getMessage().c_str());
		return false;
	}
	return true;
}

const vtString *vtMaterialDescriptorArray::FindName(const char *name)
{
	int iIndex;
	int iSize = GetSize();

	for (iIndex = 0; iIndex < iSize; iIndex++)
	{
		const vtString *pFoundName = &GetAt(iIndex)->GetName();
		if (pFoundName && *pFoundName == name)
			return pFoundName;
	}
	return NULL;
}

bool vtMaterialDescriptorArray::LoadExternalMaterials(const vtStringArray &paths)
{
	// we always need at least 1 internal material
	CreatePlain();

	const char *fname = "Culture/materials.xml";
	vtString matfile = FindFileOnPaths(paths, fname);
	if (matfile == "")
	{
		VTLOG("Couldn't find '%s' on Data paths, building materials will "\
			"not be available.\n", fname);
		return false;
	}
	return Load(matfile);
}

//
// Global material descriptors
//
static vtMaterialDescriptorArray *g_pMaterials = NULL;

bool LoadGlobalMaterials(const vtStringArray &paths)
{
	GetGlobalMaterials();
	return g_pMaterials->LoadExternalMaterials(paths);
}

void SetGlobalMaterials(vtMaterialDescriptorArray *mats)
{
	g_pMaterials = mats;
}

vtMaterialDescriptorArray *GetGlobalMaterials()
{
	if (!g_pMaterials)
		g_pMaterials = new vtMaterialDescriptorArray;
	return g_pMaterials;
}

void FreeGlobalMaterials()
{
	delete g_pMaterials;
	g_pMaterials = NULL;
}

